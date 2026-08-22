import json
import math
import ssl
import threading
import urllib.parse
import urllib.request
from datetime import datetime
from pathlib import Path

from kivymd.app import MDApp


class RutasService:
    """Rutas GPS guardadas en el almacenamiento privado del teléfono."""

    ARCHIVO = "rutas_centinela.json"
    DISTANCIA_MINIMA_METROS = 8
    # Se llega a la esquina dentro de 8 m para no anticipar el giro y cortar
    # diagonalmente una cuadra.
    DISTANCIA_LLEGADA_METROS = 8
    DISTANCIA_ACTUALIZACION_METROS = 5
    ANGULO_GIRO_ESQUINA = 55
    _grabando = False
    _puntos_actuales = []
    _ruta_en_seguimiento = None
    _indice_siguiente = 0
    _ultima_ubicacion_seguimiento = None

    @classmethod
    def interpolar_puntos_cada_8m(cls, puntos_base):
        """Interpola puntos GPS para que haya un punto cada ~8 metros a lo largo de la ruta."""
        if not puntos_base:
            return []
        puntos_densos = [puntos_base[0]]
        for i in range(len(puntos_base) - 1):
            p1 = puntos_base[i]
            p2 = puntos_base[i + 1]
            dist = cls.distancia_metros(p1, p2)
            if dist > cls.DISTANCIA_MINIMA_METROS:
                cant_segmentos = int(dist // cls.DISTANCIA_MINIMA_METROS)
                for seg in range(1, cant_segmentos + 1):
                    frac = seg / (cant_segmentos + 1)
                    lat_int = p1["latitud"] + frac * (p2["latitud"] - p1["latitud"])
                    lon_int = p1["longitud"] + frac * (p2["longitud"] - p1["longitud"])
                    puntos_densos.append({"latitud": lat_int, "longitud": lon_int})
            puntos_densos.append(p2)
        return puntos_densos

    @classmethod
    def _generar_texto_paso(cls, tipo, modificador, calle, dist_m):
        calle_str = f" por {calle}" if calle else ""
        dist_str = f"{dist_m:.0f} m"
        if tipo == "depart":
            return f"Iniciá el recorrido{calle_str} por {dist_str}."
        elif tipo == "arrive":
            return "Llegaste a tu destino."
        elif "right" in modificador:
            return f"Girá a la derecha{calle_str} y avanzá {dist_str}."
        elif "left" in modificador:
            return f"Girá a la izquierda{calle_str} y avanzá {dist_str}."
        elif "uturn" in modificador:
            return f"Da la vuelta en U{calle_str} y avanzá {dist_str}."
        else:
            return f"Avanzá {dist_str}{calle_str}."

    @classmethod
    def obtener_ruta_online(cls, destino_texto, ubicacion_actual, callback):
        """
        Consulta en segundo plano el recorrido y pasos a pie usando geocodificación
        y OSRM para devolver las indicaciones y puntos GPS cada 8 m directamente dentro de la app.
        """
        def _tarea():
            try:
                lat1 = None
                lon1 = None
                if ubicacion_actual and "latitud" in ubicacion_actual and "longitud" in ubicacion_actual:
                    lat1 = ubicacion_actual["latitud"]
                    lon1 = ubicacion_actual["longitud"]

                if lat1 is None or lon1 is None:
                    # Intento de lectura puntual
                    from services.gps import UbicacionService
                    puntual = UbicacionService.obtener_ubicacion_actual()
                    if puntual:
                        lat1, lon1 = puntual

                if lat1 is None or lon1 is None:
                    callback(None, "Se requiere que el GPS obtenga una coordenada para calcular la ruta.")
                    return

                ctx = ssl._create_unverified_context()
                headers = {"User-Agent": "CentinelaApp/1.0 (centinela.app@gmail.com)"}
                query_encoded = urllib.parse.quote(destino_texto.strip())
                geo_url = f"https://nominatim.openstreetmap.org/search?q={query_encoded}&format=json&limit=1"
                req = urllib.request.Request(geo_url, headers=headers)
                with urllib.request.urlopen(req, context=ctx, timeout=10) as resp:
                    geo_data = json.loads(resp.read().decode("utf-8"))

                if not geo_data:
                    callback(None, f"No se encontró la ubicación de «{destino_texto}».")
                    return

                lat2 = float(geo_data[0]["lat"])
                lon2 = float(geo_data[0]["lon"])
                nombre_encontrado = geo_data[0].get("display_name", destino_texto).split(",")[0]

                # Consultar ruta a pie OSRM
                osrm_url = f"https://router.project-osrm.org/route/v1/foot/{lon1},{lat1};{lon2},{lat2}?overview=full&geometries=geojson&steps=true"
                req_osrm = urllib.request.Request(osrm_url, headers=headers)
                with urllib.request.urlopen(req_osrm, context=ctx, timeout=10) as resp_osrm:
                    route_data = json.loads(resp_osrm.read().decode("utf-8"))

                if "routes" not in route_data or not route_data["routes"]:
                    callback(None, "No se pudo calcular el recorrido a pie.")
                    return

                primera_ruta = route_data["routes"][0]
                coords = primera_ruta["geometry"]["coordinates"]
                puntos_crudos = [{"latitud": float(c[1]), "longitud": float(c[0])} for c in coords]
                # Interpolar puntos para que queden guardados cada 8 metros
                puntos = cls.interpolar_puntos_cada_8m(puntos_crudos)
                distancia_total = primera_ruta.get("distance", 0)

                pasos = []
                for leg in primera_ruta.get("legs", []):
                    for step in leg.get("steps", []):
                        maneuver = step.get("maneuver", {})
                        tipo = maneuver.get("type", "continue")
                        modificador = maneuver.get("modifier", "")
                        nombre_calle = step.get("name", "")
                        dist_m = step.get("distance", 0)
                        pasos.append({
                            "tipo": tipo,
                            "modificador": modificador,
                            "calle": nombre_calle,
                            "distancia": dist_m,
                            "instruccion": cls._generar_texto_paso(tipo, modificador, nombre_calle, dist_m),
                        })

                ruta = {
                    "nombre": f"Ruta a {destino_texto}",
                    "destino": destino_texto,
                    "destino_completo": nombre_encontrado,
                    "creada": datetime.now().isoformat(timespec="seconds"),
                    "puntos": puntos,
                    "pasos": pasos,
                    "distancia_metros": distancia_total,
                }
                callback(ruta, None)
            except Exception as e:
                callback(None, f"Error en cálculo de ruta: {e}")

        threading.Thread(target=_tarea, daemon=True).start()

    @classmethod
    def iniciar_grabacion(cls, nombre):
        cls._grabando = True
        cls._puntos_actuales = []
        cls._nombre_actual = nombre.strip()

    @classmethod
    def esta_grabando(cls):
        return cls._grabando

    @classmethod
    def registrar_punto(cls, latitud, longitud):
        if not cls._grabando:
            return False
        punto = {"latitud": float(latitud), "longitud": float(longitud)}
        if cls._puntos_actuales:
            anterior = cls._puntos_actuales[-1]
            if cls.distancia_metros(anterior, punto) < cls.DISTANCIA_MINIMA_METROS:
                return False
        cls._puntos_actuales.append(punto)
        return True

    @classmethod
    def finalizar_grabacion(cls):
        cls._grabando = False
        if len(cls._puntos_actuales) < 2:
            cls._puntos_actuales = []
            return None
        rutas = cls.cargar_rutas()
        ruta = {
            "nombre": cls._nombre_actual or datetime.now().strftime("Ruta %d-%m-%Y %H:%M"),
            "creada": datetime.now().isoformat(timespec="seconds"),
            "puntos": cls._puntos_actuales,
        }
        rutas.append(ruta)
        cls._guardar_rutas(rutas)
        cls._puntos_actuales = []
        cls._nombre_actual = ""
        return ruta

    @classmethod
    def cargar_rutas(cls):
        try:
            with cls._ruta_archivo().open("r", encoding="utf-8") as archivo:
                rutas = json.load(archivo)
            return rutas if isinstance(rutas, list) else []
        except (OSError, json.JSONDecodeError):
            return []

    @classmethod
    def ultima_ruta(cls):
        rutas = cls.cargar_rutas()
        return rutas[-1] if rutas else None

    @classmethod
    def eliminar_ultima(cls):
        rutas = cls.cargar_rutas()
        if not rutas:
            return False
        rutas.pop()
        cls._guardar_rutas(rutas)
        return True

    @classmethod
    def eliminar_ruta_por_indice(cls, indice):
        rutas = cls.cargar_rutas()
        if 0 <= indice < len(rutas):
            rutas.pop(indice)
            cls._guardar_rutas(rutas)
            if cls._ruta_en_seguimiento:
                cls.detener_seguimiento()
            return True
        return False

    @classmethod
    def renombrar_ruta(cls, indice, nuevo_nombre):
        rutas = cls.cargar_rutas()
        if 0 <= indice < len(rutas) and nuevo_nombre.strip():
            rutas[indice]["nombre"] = nuevo_nombre.strip()
            cls._guardar_rutas(rutas)
            return True
        return False

    @classmethod
    def guardar_ruta_destino(cls, nombre, destino, punto_actual=None):
        rutas = cls.cargar_rutas()
        puntos = []
        if punto_actual and "latitud" in punto_actual and "longitud" in punto_actual:
            puntos.append(punto_actual)
        ruta = {
            "nombre": nombre.strip() or f"Ruta a {destino}",
            "destino": destino.strip(),
            "creada": datetime.now().isoformat(timespec="seconds"),
            "puntos": puntos,
        }
        rutas.append(ruta)
        cls._guardar_rutas(rutas)
        return ruta

    @classmethod
    def esta_siguiendo(cls):
        return cls._ruta_en_seguimiento is not None

    @classmethod
    def buscar_ruta(cls, nombre):
        """Busca una ruta por nombre; si está vacío devuelve la última."""
        rutas = cls.cargar_rutas()
        if not rutas:
            return None
        consulta = (nombre or "").strip().lower()
        if not consulta:
            return rutas[-1]
        for ruta in reversed(rutas):
            nombre_ruta = ruta.get("nombre", "").lower()
            if consulta == nombre_ruta or consulta in nombre_ruta:
                return ruta
        return None

    @classmethod
    def iniciar_seguimiento(cls, ruta, ubicacion):
        puntos = ruta.get("puntos", []) if ruta else []
        if not puntos or ubicacion is None:
            return None
        indice, _punto = cls.punto_mas_cercano(ruta, ubicacion)
        cls._ruta_en_seguimiento = ruta
        cls._indice_siguiente = min(indice or 0, len(puntos) - 1)
        cls._ultima_ubicacion_seguimiento = None
        return cls.indicacion_actual(ubicacion, forzar=True)

    @classmethod
    def detener_seguimiento(cls):
        cls._ruta_en_seguimiento = None
        cls._indice_siguiente = 0
        cls._ultima_ubicacion_seguimiento = None

    @classmethod
    def indicacion_actual(cls, ubicacion, forzar=False):
        """Guía por tramos rectos y anuncia cada esquina, no cada punto GPS."""
        ruta = cls._ruta_en_seguimiento
        if ruta is None or ubicacion is None:
            return None
        puntos = ruta.get("puntos", [])
        if not puntos:
            cls.detener_seguimiento()
            return None

        rumbo_avance = None
        if cls._ultima_ubicacion_seguimiento is not None:
            avance = cls.distancia_metros(cls._ultima_ubicacion_seguimiento, ubicacion)
            if avance < cls.DISTANCIA_ACTUALIZACION_METROS and not forzar:
                return None
            rumbo_avance = cls.rumbo_grados(cls._ultima_ubicacion_seguimiento, ubicacion)
        cls._ultima_ubicacion_seguimiento = dict(ubicacion)

        indice_esquina, rumbo_despues = cls._proxima_esquina(puntos, cls._indice_siguiente)
        destino = puntos[indice_esquina]
        distancia = cls.distancia_metros(ubicacion, destino)
        while distancia <= cls.DISTANCIA_LLEGADA_METROS and indice_esquina < len(puntos) - 1:
            cls._indice_siguiente = indice_esquina
            indice_esquina, rumbo_despues = cls._proxima_esquina(puntos, cls._indice_siguiente)
            destino = puntos[indice_esquina]
            distancia = cls.distancia_metros(ubicacion, destino)

        if indice_esquina == len(puntos) - 1 and distancia <= cls.DISTANCIA_LLEGADA_METROS:
            nombre = ruta.get("nombre", "Ruta")
            cls.detener_seguimiento()
            return f"Indicaciones: llegaste al final de «{nombre}»."

        rumbo_objetivo_grados = cls._rumbo_del_tramo(puntos, cls._indice_siguiente)
        rumbo = cls.rumbo_cuadra_desde_grados(rumbo_objetivo_grados)
        if rumbo_avance is not None:
            diferencia = abs((rumbo_avance - rumbo_objetivo_grados + 180) % 360 - 180)
            if diferencia > 67.5:
                rumbo_real = cls.rumbo_cardinal_desde_grados(rumbo_avance)
                return (
                    f"Indicaciones: vas hacia el {rumbo_real}; corregí hacia el {rumbo} "
                    f"por {distancia:.0f} m."
                )
        giro = f" Luego girá hacia el {rumbo_despues}." if rumbo_despues else ""
        return (
            f"Indicaciones: avanzá {distancia:.0f} m hacia el {rumbo}.{giro}"
        )

    @classmethod
    def _proxima_esquina(cls, puntos, inicio):
        """Agrupa puntos del mismo tramo y devuelve su esquina final."""
        ultimo = len(puntos) - 1
        if inicio >= ultimo:
            return ultimo, None
        rumbo_base = cls._rumbo_del_tramo(puntos, inicio)
        for indice in range(inicio + 1, ultimo):
            rumbo_siguiente = cls._rumbo_del_tramo(puntos, indice)
            diferencia = abs((rumbo_siguiente - rumbo_base + 180) % 360 - 180)
            if diferencia >= cls.ANGULO_GIRO_ESQUINA:
                return indice, cls.rumbo_cuadra_desde_grados(rumbo_siguiente)
        return ultimo, None

    @classmethod
    def _rumbo_del_tramo(cls, puntos, indice):
        if indice >= len(puntos) - 1:
            return cls.rumbo_grados(puntos[-2], puntos[-1])
        return cls.rumbo_grados(puntos[indice], puntos[indice + 1])

    @staticmethod
    def distancia_metros(origen, destino):
        radio = 6_371_000
        lat1, lon1 = math.radians(origen["latitud"]), math.radians(origen["longitud"])
        lat2, lon2 = math.radians(destino["latitud"]), math.radians(destino["longitud"])
        a = math.sin((lat2 - lat1) / 2) ** 2 + math.cos(lat1) * math.cos(lat2) * math.sin((lon2 - lon1) / 2) ** 2
        return 2 * radio * math.asin(math.sqrt(a))

    @staticmethod
    def rumbo_cardinal(origen, destino):
        return RutasService.rumbo_cardinal_desde_grados(RutasService.rumbo_grados(origen, destino))

    @staticmethod
    def rumbo_grados(origen, destino):
        lat1, lon1 = math.radians(origen["latitud"]), math.radians(origen["longitud"])
        lat2, lon2 = math.radians(destino["latitud"]), math.radians(destino["longitud"])
        return math.degrees(math.atan2(
            math.sin(lon2 - lon1) * math.cos(lat2),
            math.cos(lat1) * math.sin(lat2) - math.sin(lat1) * math.cos(lat2) * math.cos(lon2 - lon1),
        )) % 360

    @staticmethod
    def rumbo_cardinal_desde_grados(angulo):
        rumbos = ("norte", "noreste", "este", "sudeste", "sur", "sudoeste", "oeste", "noroeste")
        return rumbos[int((angulo + 22.5) // 45) % len(rumbos)]

    @staticmethod
    def rumbo_cuadra_desde_grados(angulo):
        rumbos = ("norte", "este", "sur", "oeste")
        return rumbos[int((angulo + 45) // 90) % len(rumbos)]

    @classmethod
    def punto_mas_cercano(cls, ruta, ubicacion):
        puntos = ruta.get("puntos", [])
        if not puntos:
            return None, None
        indice, punto = min(enumerate(puntos), key=lambda item: cls.distancia_metros(ubicacion, item[1]))
        return indice, punto

    @classmethod
    def _guardar_rutas(cls, rutas):
        try:
            ruta = cls._ruta_archivo()
            ruta.parent.mkdir(parents=True, exist_ok=True)
            with ruta.open("w", encoding="utf-8") as archivo:
                json.dump(rutas, archivo, ensure_ascii=False)
        except Exception:
            pass

    @classmethod
    def _ruta_archivo(cls):
        try:
            app = MDApp.get_running_app()
            if app is not None and getattr(app, "user_data_dir", None):
                base = Path(app.user_data_dir)
            else:
                base = Path.home() / ".centinela"
        except Exception:
            base = Path.cwd()
        try:
            base.mkdir(parents=True, exist_ok=True)
        except Exception:
            pass
        return base / cls.ARCHIVO
