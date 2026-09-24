# =============================================================================
# screens/inicio.py — Centinela v2.0
# =============================================================================
# Cambios:
#   - Al crear la pantalla se lanza conectar_automaticamente() en background.
#   - Mientras busca, muestra "Buscando baston..." en lugar de solo "DESCONECTADO".
#   - El boton "CONECTAR BASTON" sigue funcionando para forzar reconexion manual.
#   - Texto del boton se actualiza segun estado.
# =============================================================================

from kivy.clock import Clock
from kivy.metrics import dp
from kivymd.app import MDApp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel
from kivymd.uix.button import MDRaisedButton
from kivy.uix.image import Image
from kivy.uix.widget import Widget

from services.bluetooth import get_bluetooth_service

from components.estilos_centinela import (
    CREMA_CLARO,
    BORDO,
    BORDO_OSCURO,
    DORADO_APAGADO,
    VERDE_ESTADO,
    ROJO_EMERGENCIA,
)


class InicioScreen(MDBoxLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = [dp(20), dp(100), dp(20), dp(24)]
        self.spacing = dp(12)

        self.bluetooth_service = get_bluetooth_service()

        # ==========================================
        # ESTADO DEL BASTON (TITULO SUPERIOR)
        # ==========================================

        self.estado_baston = MDLabel(
            text="Buscando baston...",
            halign="center",
            font_style="H5",
            bold=True,
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=dp(44),
        )

        self.add_widget(self.estado_baston)

        # Espaciador
        self.add_widget(Widget(size_hint_y=1))

        # ==========================================
        # IMAGEN DEL BASTON
        # ==========================================

        self.ilustracion = Image(
            source="assets/images/baston_centinela.png",
            size_hint_y=None,
            height=dp(210),
            allow_stretch=True,
            keep_ratio=True,
        )

        self.add_widget(self.ilustracion)

        # ==========================================
        # BATERIA
        # ==========================================

        self.bateria_label = MDLabel(
            text="Bateria: -- %",
            halign="center",
            font_style="H5",
            bold=True,
            theme_text_color="Custom",
            text_color=VERDE_ESTADO,
            size_hint_y=None,
            height=dp(48),
        )

        self.add_widget(self.bateria_label)

        # ==========================================
        # BOTON CONECTAR
        # ==========================================

        self.boton_conectar = MDRaisedButton(
            text="CONECTAR BASTON",
            pos_hint={"center_x": 0.5},
            size_hint=(0.70, None),
            height=dp(52),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )

        self.boton_conectar.bind(on_release=self.alternar_conexion)

        self.add_widget(self.boton_conectar)

        # ==========================================
        # ACTUALIZACION PERIODICA
        # ==========================================

        Clock.schedule_interval(self.actualizar_estado, 0.5)

        # ==========================================
        # AUTO-CONEXION AL CREAR LA PANTALLA
        # ==========================================
        # Se lanza en background: no bloquea la UI.
        # Si ya hay un hilo corriendo, conectar_automaticamente() lo ignora.
        Clock.schedule_once(self._iniciar_auto_conexion, 0.5)

    def _iniciar_auto_conexion(self, dt):
        """Arranca la conexion automatica en background al cargar la pantalla."""
        self.bluetooth_service.conectar_automaticamente()

    def actualizar_estado(self, dt):

        self.bluetooth_service.update(dt)

        if self.bluetooth_service.connected:

            self.estado_baston.text = "BASTON CONECTADO"
            self.estado_baston.text_color = VERDE_ESTADO
            self.boton_conectar.text = "DESCONECTAR BASTON"

        else:

            error = self.bluetooth_service.last_error or "BASTON DESCONECTADO"
            self.estado_baston.text = error

            # Color segun estado: buscando = dorado, desconectado = rojo
            if "buscando" in error.lower() or "cerca" in error.lower():
                self.estado_baston.text_color = DORADO_APAGADO
            else:
                self.estado_baston.text_color = ROJO_EMERGENCIA

            self.boton_conectar.text = "CONECTAR BASTON"

        bateria = self.bluetooth_service.state.get("BATERIA", "--")
        self.bateria_label.text = f"Bateria: {bateria} %"

    def alternar_conexion(self, instance):

        if self.bluetooth_service.connected:
            self.bluetooth_service.disconnect()
            self.bluetooth_service.detener_auto()
        else:
            # Forzar intento inmediato y arrancar auto-reconexion
            self.estado_baston.text = "Conectando..."
            self.estado_baston.text_color = DORADO_APAGADO
            self.bluetooth_service.conectar_automaticamente()

        self.actualizar_estado(0)
