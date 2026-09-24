import difflib
import json
import math
import ssl
import threading
import unicodedata
import urllib.parse
import urllib.request
from datetime import datetime
from pathlib import Path

from kivymd.app import MDApp

# Modulos especializados de soporte
from services.calculos_geo import (
    DISTANCIA_MINIMA_METROS,
    ANGULO_GIRO_ESQUINA,
    distancia_metros,
    rumbo_grados,
    rumbo_cardinal,
    rumbo_cardinal_desde_grados,
    rumbo_cuadra_desde_grados,
    interpolar_puntos_cada_8m,
    rumbo_del_tramo,
    proxima_esquina,
    punto_mas_cercano,
)
from services.geocodificador import (
    CABECERAS_HTTP,
    remover_tildes,
    normalizar_consulta_destino,
    calcular_similitud_texto,
    evaluar_mejor_candidato,
    reverse_geocoding_actual,
)


class RutasService:
    """Rutas GPS guardadas en el almacenamiento privado del teléfono."""

    ARCHIVO = "rutas_centinela.json"
    DISTANCIA_MINIMA_METROS = 8
    # Se llega a la esquina dentro de 8 m para no anticipar el giro y cortar
    # diagonalmente una cuadra.
    DISTANCIA_LLEGADA_METROS = 8
    # Distancia mínima de avance para recalcular indicaciones.
    # 1 m es suficiente para no silenciar el GPS urbano.
    DISTANCIA_ACTUALIZACION_METROS = 1
    ANGULO_GIRO_ESQUINA = 55
    _grabando = False
    _puntos_actuales = []
    _ruta_en_seguimiento = None
    _indice_siguiente = 0
    _ultima_ubicacion_seguimiento = None
    _ultimo_timestamp_indicacion = 0  # timestamp de la última indicacion emitida

    @classmethod
    def interpolar_puntos_cada_8m(cls, puntos_base):
        """Interpola puntos GPS para que haya un punto cada ~8 metros a lo largo de la ruta."""
        return interpolar_puntos_cada_8m(puntos_base)

    @classmethod
    def _generar_texto_paso(cls, tipo, modificador, calle, dist_m, rumbo=None):
        calle_str = f" por {calle}" if calle else ""
        rumbo_str = f" hacia el {rumbo}" if rumbo else ""
        dist_str = f"{dist_m:.0f} m"
        if tipo == "depart":
            return f"Iniciá el recorrido{calle_str}{rumbo_str} por {dist_str}."
        elif tipo == "arrive":
            return "Llegaste a tu destino."
        elif "right" in modificador:
            return f"Girá a la derecha{calle_str} y avanzá {dist_str}{rumbo_str}."
        elif "left" in modificador:
            return f"Girá a la izquierda{calle_str} y avanzá {dist_str}{rumbo_str}."
        elif "uturn" in modificador:
            return f"Da la vuelta en U{calle_str} y avanzá {dist_str}{rumbo_str}."
        else:
            return f"Avanzá {dist_str}{calle_str}{rumbo_str}."

    @classmethod
    def explorar_ubicacion_actual(cls, ubicacion, callback):
        """Modo Exploración: obtiene la calle, altura o lugar donde se encuentra el usuario."""
        lat = None
        lon = None
        if ubicacion and "latitud" in ubicacion:
            lat = ubicacion["latitud"]
            lon = ubicacion["longitud"]
        else:
            from services.gps import UbicacionService
            puntual = UbicacionService.obtener_ubicacion_actual()
            if puntual:
                lat, lon = puntual

        if lat is None or lon is None:
            callback(None, "Sin señal GPS para explorar el entorno.")
            return

        reverse_geocoding_actual(lat, lon, callback)

    @staticmethod
    def _remover_tildes(texto):
        return remover_tildes(texto)

    @classmethod
    def _normalizar_consulta_destino(cls, texto):
        return normalizar_consulta_destino(texto)

    @classmethod
    def _calcular_similitud_texto(cls, busqueda, texto_candidato):
        return calcular_similitud_texto(busqueda, texto_candidato)

    @classmethod
    def _evaluar_mejor_candidato(cls, destino_buscado, lista_candidatos, lat_usr, lon_usr):
        return evaluar_mejor_candidato(destino_buscado, lista_candidatos, lat_usr, lon_usr)

    @classmethod
    def obtener_ruta_online(cls, destino_texto, ubicacion_actual, callback):
        """
        Consulta en segundo plano el recorrido y pasos a pie usando geocodificación difusa
        inteligente (Photon + Nominatim) y selección por cercanía + similitud de nombre.
        Si el nombre no es exacto, encuentra el lugar más similar y más cercano al usuario.
        """
        def _tarea():
            try:
                lat1 = None
                lon1 = None
                if ubicacion_actual and "latitud" in ubicacion_actual and "longitud" in ubicacion_actual:
                    lat1 = ubicacion_actual["latitud"]
                    lon1 = ubicacion_actual["longitud"]

                if lat1 is None or lon1 is None:
                    from services.gps import UbicacionService
                    puntual = UbicacionService.obtener_ubicacion_actual()
                    if puntual:
                        lat1, lon1 = puntual

                if lat1 is None or lon1 is None:
                    callback(None, "Esperando coordenadas GPS del teléfono para calcular la ruta.")
                    return

                ctx = ssl._create_unverified_context()
                headers = {"User-Agent": "CentinelaApp/1.0 (centinela.app@gmail.com)"}
                # Primera variante = el texto exacto tal como lo escribió el usuario
                variantes = cls._normalizar_consulta_destino(destino_texto)
                texto_exacto = variantes[0]  # sin modificar

                candidatos = []
                candidatos_exactos = []  # solo los del nombre exacto
                vistos = set()

                def _agregar_candidato(lat_c, lon_c, nom_c, disp_c, tipo_c="", exacto=False):
                    try:
                        lat_f = float(lat_c)
                        lon_f = float(lon_c)
                        key = (round(lat_f, 5), round(lon_f, 5))
                        entrada = {
                            "lat": lat_f,
                            "lon": lon_f,
                            "name": nom_c or "",
                            "display_name": disp_c or nom_c or "",
                            "type": tipo_c or "",
                        }
                        if key not in vistos:
                            vistos.add(key)
                            candidatos.append(entrada)
                        if exacto:
                            candidatos_exactos.append(entrada)
                    except Exception:
                        pass

                # ── PASO 1: búsqueda exacta del nombre tal como lo escribió el usuario ──
                q_enc_ex = urllib.parse.quote(texto_exacto)
                for url_ex in [
                    f"https://photon.komoot.io/api/?q={q_enc_ex}&lat={lat1}&lon={lon1}&limit=10",
                    (f"https://nominatim.openstreetmap.org/search?q={q_enc_ex}&format=json&limit=10"
                     f"&viewbox={lon1-0.6},{lat1+0.6},{lon1+0.6},{lat1-0.6}&bounded=0"),
                ]:
                    try:
                        req_ex = urllib.request.Request(url_ex, headers=headers)
                        with urllib.request.urlopen(req_ex, context=ctx, timeout=5) as resp_ex:
                            data_ex = json.loads(resp_ex.read().decode("utf-8"))
                            # Photon devuelve {"features":[...]}, Nominatim devuelve lista
                            items_ex = data_ex.get("features", data_ex) if isinstance(data_ex, dict) else data_ex
                            for it in items_ex:
                                if isinstance(it, dict) and "geometry" in it:
                                    # formato Photon
                                    coords = it.get("geometry", {}).get("coordinates", [])
                                    if len(coords) >= 2:
                                        props = it.get("properties", {})
                                        nom = props.get("name", "")
                                        calle = props.get("street", "")
                                        ciudad = props.get("city", props.get("county", ""))
                                        disp = f"{nom}, {calle}, {ciudad}".strip(", ")
                                        _agregar_candidato(coords[1], coords[0], nom, disp,
                                                           props.get("osm_value", ""), exacto=True)
                                elif isinstance(it, dict) and "lat" in it:
                                    # formato Nominatim
                                    _agregar_candidato(
                                        it.get("lat"), it.get("lon"),
                                        it.get("name", ""), it.get("display_name", ""),
                                        it.get("type", ""), exacto=True
                                    )
                    except Exception:
                        pass

                # ── PASO 2: Si el nombre exacto tuvo resultados suficientes, usarlos directamente ──
                # Solo buscamos con variantes difusas si el exacto no devolvió nada
                if not candidatos_exactos:
                    # Búsqueda con todas las variantes (incluyendo correcciones ortográficas y sinónimos)
                    for q in variantes[1:]:  # Saltar texto_exacto, ya consultado
                        q_enc = urllib.parse.quote(q)
                        photon_url = f"https://photon.komoot.io/api/?q={q_enc}&lat={lat1}&lon={lon1}&limit=15"
                        try:
                            req_ph = urllib.request.Request(photon_url, headers=headers)
                            with urllib.request.urlopen(req_ph, context=ctx, timeout=5) as resp_ph:
                                ph_data = json.loads(resp_ph.read().decode("utf-8"))
                                for feat in ph_data.get("features", []):
                                    coords = feat.get("geometry", {}).get("coordinates", [])
                                    if len(coords) >= 2:
                                        props = feat.get("properties", {})
                                        nom = props.get("name", "")
                                        calle = props.get("street", "")
                                        ciudad = props.get("city", props.get("county", ""))
                                        disp = f"{nom}, {calle}, {ciudad}".strip(", ")
                                        tipo = props.get("osm_value", props.get("type", ""))
                                        _agregar_candidato(coords[1], coords[0], nom, disp, tipo)
                        except Exception:
                            pass

                    for q in variantes[1:]:
                        q_enc = urllib.parse.quote(q)
                        geo_url = (
                            f"https://nominatim.openstreetmap.org/search?q={q_enc}&format=json&limit=10"
                            f"&viewbox={lon1-0.6},{lat1+0.6},{lon1+0.6},{lat1-0.6}&bounded=0"
                        )
                        try:
                            req_nom = urllib.request.Request(geo_url, headers=headers)
                            with urllib.request.urlopen(req_nom, context=ctx, timeout=5) as resp_nom:
                                res_nom = json.loads(resp_nom.read().decode("utf-8"))
                                for item in res_nom:
                                    _agregar_candidato(
                                        item.get("lat"),
                                        item.get("lon"),
                                        item.get("name", ""),
                                        item.get("display_name", ""),
                                        item.get("type", "")
                                    )
                        except Exception:
                            pass

                # ── PASO 3: Si aún no hay candidatos, probar búsqueda global sin coordenadas ──
                if not candidatos:
                    q_enc = urllib.parse.quote(texto_exacto)
                    for url_gl in [
                        f"https://photon.komoot.io/api/?q={q_enc}&limit=10",
                        f"https://nominatim.openstreetmap.org/search?q={q_enc}&format=json&limit=10",
                    ]:
                        try:
                            req_g = urllib.request.Request(url_gl, headers=headers)
                            with urllib.request.urlopen(req_g, context=ctx, timeout=5) as resp_g:
                                data_g = json.loads(resp_g.read().decode("utf-8"))
                                items_g = data_g.get("features", data_g) if isinstance(data_g, dict) else data_g
                                for it in items_g:
                                    if isinstance(it, dict) and "geometry" in it:
                                        coords = it.get("geometry", {}).get("coordinates", [])
                                        if len(coords) >= 2:
                                            props = it.get("properties", {})
                                            nom = props.get("name", "")
                                            _agregar_candidato(coords[1], coords[0], nom, nom,
                                                               props.get("osm_value", ""))
                                    elif isinstance(it, dict) and "lat" in it:
                                        _agregar_candidato(
                                            it.get("lat"), it.get("lon"),
                                            it.get("name", ""), it.get("display_name", ""),
                                            it.get("type", "")
                                        )
                        except Exception:
                            pass

                # Evaluar y seleccionar el lugar con nombre más similar y más cercano al usuario
                # Preferir candidatos exactos si existen
                pool = candidatos_exactos if candidatos_exactos else candidatos
                mejor = cls._evaluar_mejor_candidato(destino_texto, pool, lat1, lon1)

                if not mejor:
                    callback(None, f"No se encontró ningún lugar cercano similar a «{destino_texto}».")
                    return

                lat2 = mejor["lat"]
                lon2 = mejor["lon"]
                nombre_encontrado = mejor["nombre"]

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
                        bearing_after = maneuver.get("bearing_after")
                        rumbo_paso = cls.rumbo_cuadra_desde_grados(bearing_after) if bearing_after is not None else None
                        pasos.append({
                            "tipo": tipo,
                            "modificador": modificador,
                            "calle": nombre_calle,
                            "distancia": dist_m,
                            "rumbo": rumbo_paso,
                            "instruccion": cls._generar_texto_paso(tipo, modificador, nombre_calle, dist_m, rumbo_paso),
                        })

                duracion_seg = float(primera_ruta.get("duration", 0))
                if duracion_seg <= 0 and distancia_total > 0:
                    # 1 min cada 75 m = 1.25 m/s ≈ promedio real de una persona que camina despacio
                    duracion_seg = distancia_total / 1.25

                tiempo_str = cls.formatear_tiempo_caminata(duracion_seg)
                dist_str = cls.formatear_distancia(distancia_total)

                ruta = {
                    "nombre": f"Ruta a {destino_texto}",
                    "destino": destino_texto,
                    "destino_completo": nombre_encontrado,
                    "creada": datetime.now().isoformat(timespec="seconds"),
                    "puntos": puntos,
                    "pasos": pasos,
                    "distancia_metros": distancia_total,
                    "duracion_segundos": duracion_seg,
                    "tiempo_texto": tiempo_str,
                    "distancia_texto": dist_str,
                }
                callback(ruta, None)
            except Exception as e:
                callback(None, f"Error en cálculo de ruta: {e}")

        threading.Thread(target=_tarea, daemon=True).start()

    @staticmethod
    def formatear_tiempo_caminata(segundos):
        minutos = int(round(segundos / 60))
        if minutos < 1:
            return "menos de 1 minuto"
        elif minutos == 1:
            return "1 minuto a pie"
        elif minutos < 60:
            return f"{minutos} min a pie"
        else:
            horas = minutos // 60
            mins_rest = minutos % 60
            if mins_rest > 0:
                return f"{horas} h {mins_rest} min a pie"
            return f"{horas} h a pie"

    @staticmethod
    def formatear_distancia(metros):
        if metros < 1000:
            return f"{metros:.0f} m"
        return f"{metros/1000:.1f} km"

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
    def invertir_ruta(cls, ruta):
        """Devuelve una copia invertida de la ruta (trayecto de vuelta)."""
        if not ruta or not ruta.get("puntos"):
            return ruta
        puntos_inv = list(reversed(ruta["puntos"]))
        nombre_orig = ruta.get("nombre", "Ruta")

        # Invertir el nombre si tiene separadores ("Casa a Trabajo" -> "Trabajo a Casa")
        nombre_inv = None
        for sep in (" a ", " -> ", " - ", " hacia ", " hasta "):
            if sep in nombre_orig:
                partes = nombre_orig.split(sep, 1)
                nombre_inv = f"{partes[1].strip()}{sep}{partes[0].strip()}"
                break
        if not nombre_inv:
            if nombre_orig.lower().startswith("vuelta de ") or nombre_orig.lower().startswith("vuelta: "):
                nombre_inv = nombre_orig.replace("Vuelta de ", "").replace("Vuelta: ", "")
            else:
                nombre_inv = f"Vuelta: {nombre_orig}"

        return {
            "nombre": nombre_inv,
            "destino": ruta.get("origen", "Inicio de ruta"),
            "origen": ruta.get("destino", "Destino"),
            "creada": datetime.now().isoformat(timespec="seconds"),
            "puntos": puntos_inv,
            "es_invertida": True,
        }

    @classmethod
    def buscar_ruta_bidireccional(cls, consulta, ubicacion_actual=None):
        """
        Busca inteligentemente entre las rutas guardadas al derecho o al revés.
        Si el usuario dice 'Trabajo a Casa' o solo 'Casa' estando cerca del Trabajo,
        invierte automáticamente la ruta 'Casa a Trabajo' para guiarlo de regreso.
        """
        rutas = cls.cargar_rutas()
        if not rutas:
            return None

        texto = (consulta or "").strip().lower()
        for prefijo in ("ir a la", "ir al", "ir a", "volver a la", "volver al", "volver a", "volver por", "cómo llegar a", "llegar a", "navegar a", "retomar ruta"):
            if texto.startswith(prefijo):
                texto = texto[len(prefijo):].strip()
                break

        if not texto:
            return rutas[-1]

        # 1. Búsqueda exacta / parcial en nombres de rutas
        for ruta in reversed(rutas):
            nombre = ruta.get("nombre", "").lower()
            puntos = ruta.get("puntos", [])
            if not puntos:
                continue

            # Detectar partes del nombre (origen y destino)
            partes = []
            for sep in (" a ", " - ", " -> ", " hacia ", " hasta "):
                if sep in nombre:
                    partes = [p.strip() for p in nombre.split(sep, 1)]
                    break

            if partes and len(partes) == 2:
                origen_tag, destino_tag = partes[0], partes[1]
                # Si el usuario pide "origen a destino" (ej. Casa a Trabajo)
                if texto == f"{origen_tag} a {destino_tag}" or texto == f"{origen_tag} - {destino_tag}" or texto == destino_tag:
                    return ruta
                # Si el usuario pide "destino a origen" (ej. Trabajo a Casa o solo Casa)
                elif texto == f"{destino_tag} a {origen_tag}" or texto == f"{destino_tag} - {origen_tag}" or texto == origen_tag:
                    return cls.invertir_ruta(ruta)

            # Si coincide con el nombre directo
            if texto in nombre or nombre in texto:
                # Si tenemos ubicación actual, verificar si está más cerca del final (destino) para invertir
                if ubicacion_actual and len(puntos) >= 2:
                    dist_inicio = cls.distancia_metros(ubicacion_actual, puntos[0])
                    dist_fin = cls.distancia_metros(ubicacion_actual, puntos[-1])
                    if dist_fin < dist_inicio and dist_fin < 150:
                        return cls.invertir_ruta(ruta)
                return ruta

        # 2. Si no hubo coincidencia por nombre, buscar la ruta más cercana a la ubicación actual
        if ubicacion_actual:
            mejor_ruta = None
            menor_dist = float("inf")
            debe_invertir = False
            for ruta in rutas:
                puntos = ruta.get("puntos", [])
                if not puntos:
                    continue
                _idx, punto_cercano = cls.punto_mas_cercano(ruta, ubicacion_actual)
                if punto_cercano:
                    dist = cls.distancia_metros(ubicacion_actual, punto_cercano)
                    if dist < menor_dist:
                        menor_dist = dist
                        mejor_ruta = ruta
                        dist_inicio = cls.distancia_metros(ubicacion_actual, puntos[0])
                        dist_fin = cls.distancia_metros(ubicacion_actual, puntos[-1])
                        debe_invertir = dist_fin < dist_inicio
            if mejor_ruta and menor_dist < 100:
                return cls.invertir_ruta(mejor_ruta) if debe_invertir else mejor_ruta

        return rutas[-1]

    @classmethod
    def esta_siguiendo(cls):
        return cls._ruta_en_seguimiento is not None

    @classmethod
    def tiempo_restante_texto(cls, ubicacion):
        """Calcula el tiempo restante de caminata y distancia hasta el destino final."""
        ruta = cls._ruta_en_seguimiento
        if not ruta or not ubicacion:
            return None, 0
        puntos = ruta.get("puntos", [])
        if not puntos:
            return None, 0
        idx = min(getattr(cls, "_indice_siguiente", 0), len(puntos) - 1)
        # Distancia desde ubicación actual al punto de la esquina/siguiente
        dist_total = cls.distancia_metros(ubicacion, puntos[idx])
        # Sumar los tramos restantes de la ruta
        for i in range(idx, len(puntos) - 1):
            dist_total += cls.distancia_metros(puntos[i], puntos[i + 1])

        # 1 min cada 75 m → 1.5 min por 0.1 km (rango natural 1-2 min/0.1km)
        segundos = dist_total / 1.25
        return cls.formatear_tiempo_caminata(segundos), dist_total

    @classmethod
    def buscar_ruta(cls, nombre):
        """Compatibilidad con búsquedas simples."""
        return cls.buscar_ruta_bidireccional(nombre)

    @classmethod
    def iniciar_seguimiento(cls, ruta, ubicacion, auto_direccion=True):
        """
        Inicia el seguimiento desde cualquier punto del camino más cercano.
        Si el usuario se encuentra alejado del camino, la primera instrucción
        lo guía con rumbo y distancia directamente hacia el punto de entrada.
        """
        puntos = ruta.get("puntos", []) if ruta else []
        if not puntos or ubicacion is None:
            return None

        # Si el usuario está al final del recorrido y no se invirtió, invertir automáticamente
        if auto_direccion and len(puntos) >= 2:
            dist_inicio = cls.distancia_metros(ubicacion, puntos[0])
            dist_fin = cls.distancia_metros(ubicacion, puntos[-1])
            if dist_fin < dist_inicio and dist_fin < 40 and not ruta.get("es_invertida"):
                ruta = cls.invertir_ruta(ruta)
                puntos = ruta["puntos"]

        indice, punto_cercano = cls.punto_mas_cercano(ruta, ubicacion)
        cls._ruta_en_seguimiento = ruta
        cls._indice_siguiente = min(indice or 0, len(puntos) - 1)
        cls._ultima_ubicacion_seguimiento = None

        dist_al_camino = cls.distancia_metros(ubicacion, punto_cercano)
        if dist_al_camino > cls.DISTANCIA_LLEGADA_METROS:
            rumbo_entrada = cls.rumbo_cardinal(ubicacion, punto_cercano)
            nombre_ruta = ruta.get("nombre", "Ruta")
            return f"Indicaciones: acercate {dist_al_camino:.0f} m hacia el {rumbo_entrada} para entrar a «{nombre_ruta}»."

        return cls.indicacion_actual(ubicacion, forzar=True)

    @classmethod
    def detener_seguimiento(cls):
        cls._ruta_en_seguimiento = None
        cls._indice_siguiente = 0
        cls._ultima_ubicacion_seguimiento = None

    @classmethod
    def indicacion_actual(cls, ubicacion, forzar=False):
        """Guía por tramos rectos y anuncia cada esquina con rumbo cardinal y distancia."""
        import time
        ruta = cls._ruta_en_seguimiento
        if ruta is None or ubicacion is None:
            return None
        puntos = ruta.get("puntos", [])
        if not puntos:
            cls.detener_seguimiento()
            return None

        ahora = time.time()
        # Forzar actualización si pasaron más de 3 segundos (GPS quieto)
        por_tiempo = (ahora - cls._ultimo_timestamp_indicacion) >= 3.0

        rumbo_avance = None
        if cls._ultima_ubicacion_seguimiento is not None:
            avance = cls.distancia_metros(cls._ultima_ubicacion_seguimiento, ubicacion)
            if avance < cls.DISTANCIA_ACTUALIZACION_METROS and not forzar and not por_tiempo:
                return None
            if avance >= 0.5:
                rumbo_avance = cls.rumbo_grados(cls._ultima_ubicacion_seguimiento, ubicacion)
        cls._ultima_ubicacion_seguimiento = dict(ubicacion)
        cls._ultimo_timestamp_indicacion = ahora

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
            return f"Indicaciones: ¡Llegaste a tu destino en «{nombre}»!"

        rumbo_objetivo_grados = cls._rumbo_del_tramo(puntos, cls._indice_siguiente)
        rumbo = cls.rumbo_cuadra_desde_grados(rumbo_objetivo_grados)

        # Determinar si hay nombre de calle disponible en los pasos
        calle_actual = ""
        pasos = ruta.get("pasos", [])
        if pasos:
            for p in pasos:
                if p.get("calle"):
                    calle_actual = p["calle"]
                    break
        calle_str = f" por {calle_actual}" if calle_actual else ""

        if rumbo_avance is not None:
            diferencia = abs((rumbo_avance - rumbo_objetivo_grados + 180) % 360 - 180)
            if diferencia > 67.5:
                rumbo_real = cls.rumbo_cardinal_desde_grados(rumbo_avance)
                return (
                    f"Indicaciones: vas hacia el {rumbo_real}; corregí hacia el {rumbo} "
                    f"por {distancia:.0f} m{calle_str}."
                )
        giro = f" Luego girá hacia el {rumbo_despues}." if rumbo_despues else ""
        return (
            f"Indicaciones: avanzá {distancia:.0f} m{calle_str} hacia el {rumbo}.{giro}"
        )

    @classmethod
    def _proxima_esquina(cls, puntos, inicio):
        return proxima_esquina(puntos, inicio)

    @classmethod
    def _rumbo_del_tramo(cls, puntos, indice):
        return rumbo_del_tramo(puntos, indice)

    @staticmethod
    def distancia_metros(origen, destino):
        return distancia_metros(origen, destino)

    @staticmethod
    def rumbo_cardinal(origen, destino):
        return rumbo_cardinal(origen, destino)

    @staticmethod
    def rumbo_grados(origen, destino):
        return rumbo_grados(origen, destino)

    @staticmethod
    def rumbo_cardinal_desde_grados(angulo):
        return rumbo_cardinal_desde_grados(angulo)

    @staticmethod
    def rumbo_cuadra_desde_grados(angulo):
        return rumbo_cuadra_desde_grados(angulo)

    @classmethod
    def punto_mas_cercano(cls, ruta, ubicacion):
        puntos = ruta.get("puntos", []) if isinstance(ruta, dict) else ruta
        return punto_mas_cercano(puntos, ubicacion)

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
