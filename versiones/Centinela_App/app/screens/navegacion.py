from kivy.clock import Clock
from kivy.metrics import dp
from datetime import datetime
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel
from kivymd.uix.button import MDRaisedButton

from services.gps import get_ubicacion_service
from services.rutas import RutasService
from services.voz import VozService
from services.bluetooth import get_bluetooth_service

from components.estilos_centinela import (
    CREMA_CLARO,
    BORDO,
    BORDO_OSCURO,
    VERDE_ESTADO,
)


class NavegacionScreen(MDBoxLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = [dp(20), dp(5), dp(20), dp(88)]
        self.spacing = dp(12)

        # =========================
        # SERVICIOS
        # =========================
        self.gps = get_ubicacion_service()
        self.voz = VozService()
        self.bluetooth = get_bluetooth_service()
        self.ultima_indicacion = ""

        # Registrar callback de ubicación para actualización inmediata en UI
        self.gps.on_location = self._on_nueva_ubicacion

        # =========================
        # TÍTULO
        # =========================
        self.titulo = MDLabel(
            text="NAVEGACIÓN",
            halign="center",
            font_style="H4",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(56)
        )
        self.add_widget(self.titulo)

        self.indicaciones = MDLabel(
            text="Indicaciones: elegí o retomá una ruta guardada.",
            halign="center",
            shorten=True,
            shorten_from="right",
            theme_text_color="Custom",
            text_color=VERDE_ESTADO,
            size_hint_y=None,
            height=dp(32),
        )
        self.add_widget(self.indicaciones)

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
            height=dp(44),
        )
        self.add_widget(self.estado_gps)

        # =========================
        # LATITUD
        # =========================
        self.latitud = MDLabel(
            text="Latitud: --",
            halign="center",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(40),
        )
        self.add_widget(self.latitud)

        # =========================
        # LONGITUD
        # =========================
        self.longitud = MDLabel(
            text="Longitud: --",
            halign="center",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(40),
        )
        self.add_widget(self.longitud)

        # =========================
        # BOTÓN INICIAR
        # =========================
        self.boton_iniciar = MDRaisedButton(
            text="INICIAR UBICACIÓN",
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=dp(52),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )
        self.boton_iniciar.bind(on_release=self.iniciar_ubicacion)
        self.add_widget(self.boton_iniciar)

        # =========================
        # BOTÓN DETENER
        # =========================
        self.boton_detener = MDRaisedButton(
            text="DETENER UBICACIÓN",
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=dp(52),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )
        self.boton_detener.bind(on_release=self.detener_ubicacion)
        self.add_widget(self.boton_detener)

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
        self.latitud.text = f"Latitud: {lat:.6f}"
        self.longitud.text = f"Longitud: {lon:.6f}"
        self.estado_gps.text = "GPS: UBICACIÓN DISPONIBLE"
        self.estado_gps.text_color = VERDE_ESTADO

        ubicacion = {"latitud": lat, "longitud": lon}
        if RutasService.esta_grabando():
            RutasService.registrar_punto(lat, lon)

        indicacion = RutasService.indicacion_actual(ubicacion)
        if indicacion:
            self._mostrar_indicacion(indicacion)

    # ==================================================
    # INICIAR GPS
    # ==================================================

    def iniciar_ubicacion(self, instance):
        try:
            if self.gps.iniciar():
                self.estado_gps.text = "GPS: Buscando ubicación…"
                self.estado_gps.text_color = VERDE_ESTADO
            else:
                self.estado_gps.text = f"GPS: {self.gps.ultimo_error}"
                self.estado_gps.text_color = CREMA_CLARO
        except Exception as e:
            self.estado_gps.text = f"GPS: Error - {e}"

    # ==================================================
    # DETENER GPS
    # ==================================================

    def detener_ubicacion(self, instance):
        try:
            self.gps.detener()
            self.estado_gps.text = "GPS: Detenido"
            self.estado_gps.text_color = CREMA_CLARO
        except Exception:
            self.estado_gps.text = "GPS: Detenido"
            self.estado_gps.text_color = CREMA_CLARO

    # ==================================================
    # COMANDOS DE VOZ
    # ==================================================

    def escuchar_comando(self, _instance):
        self.estado_gps.text = "GPS: Escuchando comando…"
        self.voz.escuchar(self._procesar_comando)

    def _procesar_comando(self, texto, error):
        if error:
            self.estado_gps.text = f"GPS: {error}"
            return
        comando = texto.lower().strip()
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
            self.estado_gps.text = f"GPS: Entendí «{texto}». Decí iniciar, guardar o retomar ruta."
            self.estado_gps.text_color = CREMA_CLARO

    def _iniciar_ruta_por_voz(self, texto):
        """Inicia la misma grabación que usa la pantalla Rutas."""
        if not self.gps.activo:
            self.estado_gps.text = "GPS: Primero decí iniciar ubicación."
            return
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
            self.estado_gps.text = "GPS: Primero decí iniciar ubicación."
            return
        nombre = texto.lower()
        for frase in ("retomar ruta", "seguir ruta", "volver por"):
            if frase in nombre:
                nombre = nombre.split(frase, 1)[1].strip()
                break
        ruta = RutasService.buscar_ruta(nombre)
        if ruta is None:
            self.estado_gps.text = "GPS: No encontré esa ruta guardada."
            return
        indicacion = RutasService.iniciar_seguimiento(ruta, self.gps.obtener_ubicacion())
        if indicacion is None:
            self.estado_gps.text = "GPS: Esperando una ubicación para retomar la ruta."
            return
        self.estado_gps.text = f"GPS: Retomando «{ruta['nombre']}»."
        self._mostrar_indicacion(indicacion)

    def _mostrar_indicacion(self, texto):
        self.indicaciones.text = texto
        if texto != self.ultima_indicacion:
            self.ultima_indicacion = texto
            # Protocolo de texto para el Arduino: I:<indicación> y salto de línea.
            self.bluetooth.send_command(f"I:{texto}\n")

    # ==================================================
    # ACTUALIZACIÓN DE UI (Clock fallback — 1 seg)
    # ==================================================

    def actualizar_gps(self, dt):
        """Respaldo de UI: actualiza etiquetas si el callback directo no llegó."""
        try:
            datos = self.gps.obtener_ubicacion()
            if not datos:
                if self.gps.activo and self.gps.ultimo_error:
                    self.estado_gps.text = f"GPS: {self.gps.ultimo_error}"
                return

            lat = datos.get("latitud")
            lon = datos.get("longitud")

            if lat is not None:
                self.latitud.text = f"Latitud: {lat:.6f}"
                if lon is not None:
                    RutasService.registrar_punto(lat, lon)

            if lon is not None:
                self.longitud.text = f"Longitud: {lon:.6f}"

            if lat is not None and lon is not None:
                indicacion = RutasService.indicacion_actual(datos)
                if indicacion:
                    self._mostrar_indicacion(indicacion)
                self.estado_gps.text = "GPS: UBICACIÓN DISPONIBLE"
                self.estado_gps.text_color = VERDE_ESTADO

        except Exception:
            pass
