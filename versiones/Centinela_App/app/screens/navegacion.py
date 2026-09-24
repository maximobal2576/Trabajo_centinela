import urllib.parse
import webbrowser
from datetime import datetime

from kivy.clock import Clock
from kivy.metrics import dp, sp
from kivy.utils import platform
from kivy.uix.widget import Widget
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDIconButton, MDRaisedButton
from kivymd.uix.card import MDCard
from kivymd.uix.label import MDLabel
from kivymd.uix.textfield import MDTextField

from components.estilos_centinela import (
    BORDO,
    BORDO_OSCURO,
    CREMA_CLARO,
    DORADO_APAGADO,
    OSCURO,
    VERDE_ESTADO,
)
from services.bluetooth import get_bluetooth_service
from services.gps import get_ubicacion_service
from services.notificaciones import NotificacionService
from services.rutas import RutasService
from services.voz import VozService
from widgets.dialogo_ruta import DialogoGuardarRuta


class NavegacionScreen(MDBoxLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = [dp(20), dp(100), dp(20), dp(24)]
        self.spacing = dp(10)

        # =========================
        # SERVICIOS
        # =========================
        self.gps = get_ubicacion_service()
        self.voz = VozService()
        self.bluetooth = get_bluetooth_service()
        self.ultima_indicacion = ""
        self.destino_pendiente = ""

        # Registrar callback de ubicación para actualización inmediata en UI
        self.gps.on_location = self._on_nueva_ubicacion
        self._recalculando_desvio = False
        self._buzzer_esquina_activo = False
        self._calculo_pendiente_destino = None
        self._calculo_pendiente_guardar = None
        self._bloqueo_tiempo_llegada = False

        # =========================
        # TÍTULO E INDICACIONES (SUPERIOR)
        # =========================
        self.titulo = MDLabel(
            text="NAVEGACIÓN",
            halign="center",
            font_style="H4",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(52),
        )
        self.add_widget(self.titulo)

        self.indicaciones = MDLabel(
            text="Indicaciones: elegí o dictá un destino con el micrófono superior.",
            halign="center",
            font_style="Subtitle1",
            bold=True,
            theme_text_color="Custom",
            text_color=VERDE_ESTADO,
            size_hint_y=None,
            height=dp(68),
        )
        self.add_widget(self.indicaciones)

        # Espaciador para separar el título y empujar los controles abajo
        self.add_widget(Widget(size_hint_y=1))

        # =========================
        # ESTADO GPS
        # =========================
        self.estado_gps = MDLabel(
            text="GPS: Iniciando…",
            halign="center",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(38),
        )
        self.add_widget(self.estado_gps)

        # =========================
        # COORDENADAS
        # =========================
        fila_coords = MDBoxLayout(
            orientation="horizontal",
            size_hint_y=None,
            height=dp(36),
            spacing=dp(8),
        )
        self.latitud = MDLabel(
            text="Lat: --",
            halign="center",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )
        self.longitud = MDLabel(
            text="Lon: --",
            halign="center",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )
        fila_coords.add_widget(self.latitud)
        fila_coords.add_widget(self.longitud)
        self.add_widget(fila_coords)

        # =========================
        # BUSCAR DESTINO
        # =========================
        self.input_destino = MDTextField(
            hint_text="Destino (ej. Farmacia Central, Plaza...)",
            mode="rectangle",
            size_hint=(1, None),
            height=dp(52),
        )
        self.add_widget(self.input_destino)

        # =========================
        # BOTÓN IR
        # =========================
        self.boton_ir = MDRaisedButton(
            text="BUSCAR Y NAVEGAR",
            pos_hint={"center_x": 0.5},
            size_hint=(0.95, None),
            height=dp(48),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self._iniciar_navegacion_destino,
        )
        self.add_widget(self.boton_ir)

        # =========================
        # BOTÓN DETENER GPS (único — las indicaciones se emiten automáticamente)
        # =========================
        self.boton_detener = MDRaisedButton(
            text="DETENER GPS",
            pos_hint={"center_x": 0.5},
            size_hint=(0.95, None),
            height=dp(46),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self.detener_ubicacion,
        )
        self.add_widget(self.boton_detener)

        # =========================
        # =========================
        # PANEL DIÁLOGO ACCESIBLE (Guardar Ruta)
        # =========================
        self.tarjeta_dialogo = DialogoGuardarRuta(
            al_confirmar_guardar=self._on_confirmar_guardar_dialogo,
            al_confirmar_solo_navegar=self._on_confirmar_solo_navegar_dialogo,
        )
        self.add_widget(self.tarjeta_dialogo)

        # Clock de respaldo: refresca la UI si el callback no llegó aún
        Clock.schedule_interval(self.actualizar_gps, 1)

    # ==================================================
    # CICLO DE VIDA — AUTO-INICIO GPS
    # ==================================================

    def on_enter(self, *args):
        """Inicia el GPS automáticamente al entrar a la pantalla."""
        if not self.gps.activo:
            self.iniciar_ubicacion(None)

    # ==================================================
    # CALLBACK DIRECTO DESDE EL LISTENER JAVA
    # ==================================================

    def _on_nueva_ubicacion(self, lat, lon):
        """Recibe coordenadas directamente desde onLocationChanged (sin polling)."""
        self.latitud.text = f"Lat: {lat:.5f}"
        self.longitud.text = f"Lon: {lon:.5f}"
        self.estado_gps.text = "GPS: UBICACIÓN DISPONIBLE"
        self.estado_gps.text_color = VERDE_ESTADO

        ubicacion = {"latitud": lat, "longitud": lon}
        if RutasService.esta_grabando():
            RutasService.registrar_punto(lat, lon)

        # Si había un cálculo de ruta esperando señal de GPS, lanzarlo inmediatamente
        if self._calculo_pendiente_destino:
            dest = self._calculo_pendiente_destino
            guardar = self._calculo_pendiente_guardar
            self._calculo_pendiente_destino = None
            self._calculo_pendiente_guardar = None
            self._iniciar_recorrido_interno(dest, guardar_nombre=guardar)

        if RutasService.esta_siguiendo():
            ruta_act = RutasService._ruta_en_seguimiento
            if ruta_act and ruta_act.get("puntos"):
                puntos = ruta_act["puntos"]
                # 1. Comprobar baliza buzzer a 3 metros de la esquina
                idx = getattr(RutasService, "_indice_siguiente", 0)
                idx_esquina, _rumbo = RutasService._proxima_esquina(puntos, idx)
                if idx_esquina < len(puntos):
                    esquina = puntos[idx_esquina]
                    dist_esquina = RutasService.distancia_metros(ubicacion, esquina)
                    if dist_esquina <= 3.5:
                        if not self._buzzer_esquina_activo:
                            self._buzzer_esquina_activo = True
                            self.bluetooth.send_command("B:1\n")
                    else:
                        if self._buzzer_esquina_activo:
                            self._buzzer_esquina_activo = False
                            self.bluetooth.send_command("B:0\n")

                # 2. Comprobar si hay desvío mayor a 100m
                _idx, punto_cercano = RutasService.punto_mas_cercano(ruta_act, ubicacion)
                if punto_cercano:
                    dist_desvio = RutasService.distancia_metros(ubicacion, punto_cercano)
                    if dist_desvio > 100 and not self._recalculando_desvio:
                        self._recalcular_por_desvio(ubicacion, dist_desvio)
                        return

        if not self._bloqueo_tiempo_llegada:
            indicacion = RutasService.indicacion_actual(ubicacion)
            if indicacion:
                self._mostrar_indicacion(indicacion)

    # ==================================================
    # INICIAR Y DETENER GPS
    # ==================================================

    def iniciar_ubicacion(self, instance=None):
        """Solo se llama automáticamente en on_enter. No registra listeners duplicados."""
        try:
            if not self.gps.activo:
                if self.gps.iniciar():
                    self.estado_gps.text = "GPS: Buscando ubicación…"
                    self.estado_gps.text_color = VERDE_ESTADO
                    NotificacionService.activar_modo_segundo_plano()
                else:
                    self.estado_gps.text = f"GPS: {self.gps.ultimo_error}"
                    self.estado_gps.text_color = CREMA_CLARO
        except Exception as e:
            self.estado_gps.text = f"GPS: Error - {e}"

    def _forzar_indicacion_actual(self, instance=None):
        """Repite la indicación actual en pantalla y por Bluetooth, sin reiniciar el GPS."""
        ubicacion = self.gps.obtener_ubicacion()
        if not ubicacion:
            self.estado_gps.text = "GPS: Sin señal. Activá la ubicación en ajustes."
            return
        if RutasService.esta_siguiendo():
            indicacion = RutasService.indicacion_actual(ubicacion, forzar=True)
            if indicacion:
                self._mostrar_indicacion(indicacion)
        else:
            self.estado_gps.text = "GPS: No hay ruta activa."

    def detener_ubicacion(self, instance=None):
        """Detiene el GPS, el seguimiento activo y limpia las notificaciones."""
        try:
            RutasService.detener_seguimiento()
            self._bloqueo_tiempo_llegada = False
            self.bluetooth.send_command("B:0\n")
        except Exception:
            pass
        try:
            self.gps.detener()
        except Exception:
            pass
        self.estado_gps.text = "GPS: Detenido"
        self.estado_gps.text_color = CREMA_CLARO
        self.indicaciones.text = "Indicaciones: —"
        NotificacionService.desactivar_modo_segundo_plano()
        NotificacionService.cancelar()

    # ==================================================
    # FLUJO DE NAVEGACIÓN Y GUARDADO DENTRO DE LA APP
    # ==================================================

    def _dictar_destino(self, _instance=None):
        self.estado_gps.text = "GPS: Decí a dónde querés ir…"
        self.voz.escuchar(self._al_dictar_destino)

    def _al_dictar_destino(self, texto, error):
        if error or not texto:
            self.estado_gps.text = f"GPS: {error or 'No se entendió el destino.'}"
            return
        destino = texto.strip()
        for prefijo in ("ir a", "ir al", "ir a la", "navegar a", "navegar al", "cómo llegar a", "llegar a"):
            if destino.lower().startswith(prefijo):
                destino = destino[len(prefijo):].strip()
                break
        self.input_destino.text = destino
        self._iniciar_navegacion_destino(destino=destino)

    def _iniciar_navegacion_destino(self, _instance=None, destino=None):
        destino = destino or self.input_destino.text.strip()
        if not destino:
            self.estado_gps.text = "Escribí o dictá un destino para navegar."
            return

        self.destino_pendiente = destino
        self.tarjeta_dialogo.mostrar(destino, nombre_sugerido=f"Ruta a {destino}")
        self.estado_gps.text = f"¿Desea guardar la ruta a «{destino}»? Decí Sí o No."

    def _escuchar_respuesta_dialogo(self, _instance=None):
        self.estado_gps.text = "Escuchando: decí «Sí» para guardar o «No» para solo navegar…"
        self.voz.escuchar(self._procesar_respuesta_dialogo_voz)

    def _procesar_respuesta_dialogo_voz(self, texto, error):
        if error or not texto:
            self.estado_gps.text = f"GPS: {error or 'No se entendió la respuesta.'}"
            return
        respuesta = texto.lower().strip()
        if any(w in respuesta for w in ("sí", "si", "guardar", "grabar", "correcto")):
            self._confirmar_guardar_ruta()
        elif any(w in respuesta for w in ("no", "solo", "navegar", "abrir")):
            self._confirmar_solo_navegar()
        else:
            self.estado_gps.text = f"Entendí «{texto}». Decí «Sí» para guardar o «No» para solo navegar."

    def _on_confirmar_guardar_dialogo(self, nombre):
        nombre_final = nombre or f"Ruta a {self.destino_pendiente}"
        self._iniciar_recorrido_interno(self.destino_pendiente, guardar_nombre=nombre_final)

    def _on_confirmar_solo_navegar_dialogo(self):
        self._iniciar_recorrido_interno(self.destino_pendiente, guardar_nombre=None)

    def _confirmar_guardar_ruta(self, _instance=None):
        nombre = self.tarjeta_dialogo.input_nombre.text.strip() or f"Ruta a {self.destino_pendiente}"
        self.tarjeta_dialogo.ocultar()
        self._iniciar_recorrido_interno(self.destino_pendiente, guardar_nombre=nombre)

    def _confirmar_solo_navegar(self, _instance=None):
        self.tarjeta_dialogo.ocultar()
        self._iniciar_recorrido_interno(self.destino_pendiente, guardar_nombre=None)

    def _ocultar_dialogo(self):
        self.tarjeta_dialogo.ocultar()

    def _iniciar_recorrido_interno(self, destino, guardar_nombre=None):
        """Calcula el recorrido en segundo plano y muestra indicaciones dentro de la app."""
        if not self.gps.activo:
            self.iniciar_ubicacion()

        ubicacion = self.gps.obtener_ubicacion()
        if not ubicacion:
            self._calculo_pendiente_destino = destino
            self._calculo_pendiente_guardar = guardar_nombre
            self.estado_gps.text = f"GPS: Esperando señal satelital para guiar hacia «{destino}»…"
            self.estado_gps.text_color = DORADO_APAGADO
            self.indicaciones.text = f"Indicaciones: calculando ruta a «{destino}» en cuanto haya GPS..."
            return

        self.estado_gps.text = f"Calculando indicaciones hacia «{destino}»…"
        self.estado_gps.text_color = VERDE_ESTADO

        def _al_recibir_ruta(ruta, error):
            Clock.schedule_once(lambda dt: self._procesar_ruta_recibida(ruta, error, destino, guardar_nombre), 0)

        RutasService.obtener_ruta_online(destino, ubicacion, _al_recibir_ruta)

    def _procesar_ruta_recibida(self, ruta, error, destino, guardar_nombre):
        ubicacion = self.gps.obtener_ubicacion()
        if error or not ruta:
            # Fallback si no hay conexión a internet
            self.estado_gps.text = f"GPS: Sin conexión. Navegación directa hacia «{destino}»."
            indicacion = f"Indicaciones: avanzá hacia «{destino}». Sin mapa online disponible."
            self._mostrar_indicacion(indicacion)
            return

        # Guardar la ruta OSRM completa al JSON inmediatamente si el usuario dijo Sí
        if guardar_nombre:
            ruta["nombre"] = guardar_nombre
            rutas = RutasService.cargar_rutas()
            # Evitar duplicados del mismo nombre
            rutas = [r for r in rutas if r.get("nombre") != guardar_nombre]
            rutas.append(ruta)
            RutasService._guardar_rutas(rutas)
            puntos_cant = len(ruta.get("puntos", []))
            self.estado_gps.text = f"Ruta «{guardar_nombre}» guardada ({puntos_cant} puntos)."
        else:
            self.estado_gps.text = "GPS: Ruta iniciada (sin guardar)."

        self.estado_gps.text_color = VERDE_ESTADO

        # ── Nombre real del destino encontrado (siempre se muestra, sea exacto o similar) ──
        # destino_completo viene del geocoder: puede ser "Farmacia del Sol, Belgrano 450 (a 600 m)"
        # Si no existe ese campo, usar el texto que buscó el usuario
        nombre_encontrado = ruta.get("destino_completo") or ruta.get("destino") or destino

        # Tiempo estimado de caminata
        tiempo_texto = ruta.get("tiempo_texto", "")
        dist_texto = ruta.get("distancia_texto", "")
        info_tiempo = f"Llegarás en {tiempo_texto} ({dist_texto})" if tiempo_texto else ""

        # ── Anuncio inicial: destino real + tiempo estimado ──
        # Siempre se anuncia aunque el nombre sea idéntico al buscado,
        # para que el ciego confirme que la app entendió bien.
        anuncio_destino = f"Yendo a: {nombre_encontrado}."
        if info_tiempo:
            anuncio_inicial = f"Indicaciones: {anuncio_destino} {info_tiempo}. Iniciando..."
        else:
            anuncio_inicial = f"Indicaciones: {anuncio_destino} Iniciando..."

        # Iniciar seguimiento paso a paso dentro de la app
        indicacion = RutasService.iniciar_seguimiento(ruta, ubicacion)
        primer_paso = ""
        if ruta.get("pasos") and len(ruta["pasos"]) > 0:
            primer_paso = ruta["pasos"][0].get("instruccion", "")

        # Mostrar anuncio inicial (destino + tiempo) durante 6 s, luego el primer paso
        self._bloqueo_tiempo_llegada = True
        self._mostrar_indicacion(anuncio_inicial)

        def _desbloquear_y_mostrar_primer_paso(dt):
            self._bloqueo_tiempo_llegada = False
            guia = primer_paso or indicacion or "Avanzá según las indicaciones."
            self._mostrar_indicacion(f"Indicaciones: {guia}")

        Clock.schedule_once(_desbloquear_y_mostrar_primer_paso, 6.0)

    def _recalcular_por_desvio(self, ubicacion, dist_desvio):
        """Si el usuario se desvía más de 100 m, recalcula la ruta automáticamente."""
        self._recalculando_desvio = True
        ruta_act = RutasService._ruta_en_seguimiento
        destino_nombre = ruta_act.get("destino", "") or ruta_act.get("nombre", "")
        self.estado_gps.text = f"GPS: Desvío de {dist_desvio:.0f} m. Recalculando ruta…"
        self.estado_gps.text_color = DORADO_APAGADO

        def _al_recalcular(nueva_ruta, error):
            self._recalculando_desvio = False
            if nueva_ruta and not error:
                RutasService.iniciar_seguimiento(nueva_ruta, ubicacion)
                primer_paso = nueva_ruta["pasos"][0].get("instruccion", "") if nueva_ruta.get("pasos") else ""
                msg = f"Ruta actualizada por desvío de {dist_desvio:.0f} m. {primer_paso or 'Seguí las nuevas indicaciones.'}"
                self._mostrar_indicacion(msg)
                self.estado_gps.text = "GPS: Ruta actualizada."
                self.estado_gps.text_color = VERDE_ESTADO
            else:
                # Si no hay internet, indicar rumbo hacia el punto más cercano de la ruta guardada
                _idx, punto_cercano = RutasService.punto_mas_cercano(ruta_act, ubicacion)
                rumbo = RutasService.rumbo_cardinal(ubicacion, punto_cercano)
                msg = f"Ruta actualizada: te desviaste {dist_desvio:.0f} m. Acercate hacia el {rumbo} para retomar."
                self._mostrar_indicacion(msg)

        RutasService.obtener_ruta_online(destino_nombre, ubicacion, _al_recalcular)

    def explorar_entorno(self, _instance=None):
        """Modo Exploración: anuncia calle y altura o lugar actual para personas ciegas."""
        if not self.gps.activo:
            self.iniciar_ubicacion()
        ubicacion = self.gps.obtener_ubicacion()
        self.estado_gps.text = "GPS: Identificando calle y entorno actual…"
        self.estado_gps.text_color = VERDE_ESTADO

        def _al_explorar(resultado, error):
            def _ui(dt):
                if resultado:
                    self._mostrar_indicacion(resultado)
                    self.estado_gps.text = "GPS: Exploración completada."
                else:
                    msg = error or "No se pudo obtener la dirección actual."
                    self._mostrar_indicacion(f"Indicaciones: {msg}")
                    self.estado_gps.text = f"GPS: {msg}"
            Clock.schedule_once(_ui, 0)

        RutasService.explorar_ubicacion_actual(ubicacion, _al_explorar)

    # ==================================================
    # COMANDOS DE VOZ GENERALES EN NAVEGACIÓN
    # ==================================================

    def escuchar_comando(self, _instance=None):
        self.estado_gps.text = "GPS: Escuchando comando o destino…"
        self.voz.escuchar(self._procesar_comando)

    def _procesar_comando(self, texto, error):
        if error or not texto:
            self.estado_gps.text = f"GPS: {error or 'No se reconoció comando.'}"
            return
        comando = texto.lower().strip()

        # Modo Exploración
        if any(frase in comando for frase in ("dónde estoy", "donde estoy", "explorar", "mi ubicación", "ubicación actual", "mi posicion")):
            self.explorar_entorno()
            return

        # Si el comando es un destino hacia donde ir
        if any(comando.startswith(p) for p in ("ir a", "ir al", "ir a la", "navegar a", "navegar al", "cómo llegar a", "llegar a")):
            destino = comando
            for p in ("ir a la", "ir al", "ir a", "navegar al", "navegar a", "cómo llegar a", "llegar a"):
                if destino.startswith(p):
                    destino = destino[len(p):].strip()
                    break
            self.input_destino.text = destino
            self._iniciar_navegacion_destino(destino=destino)
            return

        if any(frase in comando for frase in ("retomar ruta", "seguir ruta", "volver por")):
            self._retomar_ruta_por_voz(texto)
        elif any(frase in comando for frase in ("finalizar ruta", "terminar ruta", "detener guardado")):
            self._finalizar_ruta_por_voz()
        elif any(frase in comando for frase in ("guardar ruta", "grabar ruta", "iniciar ruta")):
            self._iniciar_ruta_por_voz(texto)
        elif any(palabra in comando for palabra in ("iniciar", "activar", "prender")):
            self.iniciar_ubicacion(None)
        elif any(palabra in comando for palabra in ("detener", "parar", "apagar")):
            self.detener_ubicacion(None)
        else:
            # Interpretar directamente como destino
            self.input_destino.text = texto.strip()
            self._iniciar_navegacion_destino(destino=texto.strip())

    def _iniciar_ruta_por_voz(self, texto):
        """Inicia la misma grabación que usa la pantalla Rutas."""
        if not self.gps.activo:
            self.iniciar_ubicacion()
        if RutasService.esta_grabando():
            self.estado_gps.text = "GPS: Ya se está grabando una ruta. Decí finalizar ruta."
            return
        nombre = ""
        for frase in ("guardar ruta", "grabar ruta", "iniciar ruta"):
            if frase in texto.lower():
                nombre = texto.lower().split(frase, 1)[1].strip()
                break
        nombre = nombre or datetime.now().strftime("Ruta por voz %d-%m %H:%M")
        RutasService.iniciar_grabacion(nombre)
        ubicacion = self.gps.obtener_ubicacion()
        if ubicacion:
            RutasService.registrar_punto(**ubicacion)
        self.estado_gps.text = f"GPS: Grabando «{nombre}». Decí finalizar ruta al terminar."
        self.estado_gps.text_color = VERDE_ESTADO

    def _finalizar_ruta_por_voz(self):
        if not RutasService.esta_grabando():
            self.estado_gps.text = "GPS: No hay una ruta en grabación."
            return
        ruta = RutasService.finalizar_grabacion()
        if ruta is None:
            self.estado_gps.text = "GPS: No hubo suficientes puntos para guardar la ruta."
        else:
            self.estado_gps.text = f"GPS: Ruta «{ruta['nombre']}» guardada."
            self.estado_gps.text_color = VERDE_ESTADO

    def _retomar_ruta_por_voz(self, texto):
        if not self.gps.activo:
            self.iniciar_ubicacion()
        nombre = texto.lower()
        for frase in ("retomar ruta", "seguir ruta", "volver por"):
            if frase in nombre:
                nombre = nombre.split(frase, 1)[1].strip()
                break
        ruta = RutasService.buscar_ruta(nombre)
        if ruta is None:
            self.estado_gps.text = "GPS: No encontré esa ruta guardada."
            return

        # Si la ruta guardada tiene destino de Maps, abrirlo
        if ruta.get("destino"):
            self._abrir_en_google_maps(ruta["destino"])
            self.estado_gps.text = f"Abriendo Maps para «{ruta['nombre']}»."
            return

        indicacion = RutasService.iniciar_seguimiento(ruta, self.gps.obtener_ubicacion())
        if indicacion is None:
            self.estado_gps.text = "GPS: Esperando una ubicación para retomar la ruta."
            return
        self.estado_gps.text = f"GPS: Retomando «{ruta['nombre']}»."
        self._mostrar_indicacion(indicacion)

    def _mostrar_indicacion(self, texto):
        self.indicaciones.text = texto
        texto_limpio = texto.replace("Indicaciones: ", "").strip()

        tiempo_est = None
        ubicacion = self.gps.obtener_ubicacion()
        if RutasService.esta_siguiendo() and ubicacion:
            tiempo_est, _dist = RutasService.tiempo_restante_texto(ubicacion)

        if tiempo_est:
            titulo = f"Centinela • Llegada en {tiempo_est}"
            cuerpo = f"{texto_limpio} (Tiempo estimado: {tiempo_est})"
            NotificacionService.mostrar_indicacion(titulo, cuerpo, tiempo_estimado=tiempo_est)
        else:
            NotificacionService.mostrar_indicacion("Centinela Navegación", texto_limpio)

        if texto != self.ultima_indicacion:
            self.ultima_indicacion = texto
            self.bluetooth.send_command(f"I:{texto}\n")

    # ==================================================
    # ACTUALIZACIÓN DE UI (Clock fallback — 1 seg)
    # ==================================================

    def actualizar_gps(self, dt):
        """Reloj de 1 segundo: actualiza coords y emite indicación continua durante navegación activa."""
        try:
            datos = self.gps.obtener_ubicacion()
            if not datos:
                if self.gps.activo and self.gps.ultimo_error:
                    self.estado_gps.text = f"GPS: {self.gps.ultimo_error}"
                return

            lat = datos.get("latitud")
            lon = datos.get("longitud")

            if lat is not None:
                self.latitud.text = f"Lat: {lat:.5f}"
                if lon is not None and RutasService.esta_grabando():
                    RutasService.registrar_punto(lat, lon)

            if lon is not None:
                self.longitud.text = f"Lon: {lon:.5f}"

            if lat is not None and lon is not None and not self._bloqueo_tiempo_llegada:
                if RutasService.esta_siguiendo():
                    # Emitir siempre durante navegación (forzado cada ciclo de 1 seg)
                    indicacion = RutasService.indicacion_actual(datos, forzar=True)
                    if indicacion:
                        self._mostrar_indicacion(indicacion)
                else:
                    self.estado_gps.text = "GPS: UBICACIÓN DISPONIBLE"
                    self.estado_gps.text_color = VERDE_ESTADO

        except Exception:
            pass
