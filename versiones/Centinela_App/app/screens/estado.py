from kivy.clock import Clock
from kivy.metrics import dp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel

from kivy.uix.widget import Widget

from services.bluetooth import get_bluetooth_service

from components.estilos_centinela import (
    CREMA_CLARO,
    BORDO,
    DORADO_APAGADO,
    VERDE_ESTADO,
    ROJO_EMERGENCIA,
)


class EstadoScreen(MDBoxLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = [dp(20), dp(100), dp(20), dp(24)]
        self.spacing = dp(10)
        # =========================
        # SERVICIO BLUETOOTH
        # =========================

        self.bluetooth_service = get_bluetooth_service()

        # =========================
        # TÍTULO
        # =========================

        self.titulo = MDLabel(
            text="ESTADO DEL BASTÓN",
            halign="center",
            font_style="H4",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(56),
        )

        self.add_widget(self.titulo)

        # Espaciador para separar el título y empujar los datos abajo
        self.add_widget(Widget(size_hint_y=1))

        # =========================
        # CONEXIÓN
        # =========================

        self.conexion = MDLabel(
            text="Conexión: Desconectado",
            halign="center",
            font_style="H5",
            bold=True,
            theme_text_color="Custom",
            text_color=ROJO_EMERGENCIA,
            size_hint_y=None,
            height=dp(44),
        )

        self.add_widget(self.conexion)

        # =========================
        # BATERÍA
        # =========================

        self.bateria = MDLabel(
            text="Batería: --%",
            halign="center",
            font_style="H5",
            bold=True,
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=dp(44),
        )

        self.add_widget(self.bateria)

        # =========================
        # SISTEMA
        # =========================

        self.sistema = MDLabel(
            text="Sistema: Sin datos",
            halign="center",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(44),
        )

        self.add_widget(self.sistema)

        # =========================
        # EVENTO
        # =========================

        self.evento = MDLabel(
            text="Evento: Sin datos",
            halign="center",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(44),
        )

        self.add_widget(self.evento)

        # =========================
        # DIRECCIÓN
        # =========================

        self.direccion = MDLabel(
            text="Dirección: Sin datos",
            halign="center",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(44),
        )

        self.add_widget(self.direccion)

        # =========================
        # ACTUALIZACIÓN
        # =========================

        Clock.schedule_interval(
            self.actualizar_estado,
            0.5
        )

    # ==================================================
    # ACTUALIZAR ESTADO
    # ==================================================

    def actualizar_estado(self, dt):

        # Actualizamos Bluetooth
        self.bluetooth_service.update(dt)

        # =========================
        # CONEXIÓN
        # =========================

        if self.bluetooth_service.connected:

            self.conexion.text = (
                "Conexión: Conectado"
            )

            self.conexion.text_color = (
                VERDE_ESTADO
            )

        else:

            self.conexion.text = (
                "Conexión: Desconectado"
            )

            self.conexion.text_color = (
                ROJO_EMERGENCIA
            )

        # =========================
        # DATOS DEL BASTÓN
        # =========================

        estado = self.bluetooth_service.state.get(
            "ESTADO"
        )

        evento = self.bluetooth_service.state.get(
            "EVENTO"
        )

        direccion = self.bluetooth_service.state.get(
            "DIRECCION"
        )

        bateria = self.bluetooth_service.state.get(
            "BATERIA",
            "--"
        )

        # =========================
        # MOSTRAR DATOS
        # =========================

        self.bateria.text = (
            f"Batería: {bateria}%"
        )

        self.sistema.text = (
            f"Sistema: {self._nombre_estado(estado)}"
        )

        self.evento.text = (
            f"Evento: {self._nombre_evento(evento)}"
        )

        self.direccion.text = (
            f"Dirección: {self._nombre_direccion(direccion)}"
        )

    # ==================================================
    # TRADUCCIÓN DE ESTADOS
    # ==================================================

    @staticmethod
    def _nombre_estado(valor):

        estados = {
            "0": "Inicio",
            "1": "Normal",
            "2": "Alerta",
            "3": "Pozo",
            "4": "Caída",
            "5": "Diagnóstico",
            "6": "Error"
        }

        return estados.get(
            str(valor),
            "Sin datos"
        )

    # ==================================================
    # TRADUCCIÓN DE EVENTOS
    # ==================================================

    @staticmethod
    def _nombre_evento(valor):

        eventos = {
            "0": "Normal",
            "1": "Obstáculo frontal",
            "2": "Obstáculo de radar",
            "3": "Pozo",
            "4": "Caída",
            "5": "Batería baja",
            "6": "Error"
        }

        return eventos.get(
            str(valor),
            "Sin datos"
        )

    # ==================================================
    # TRADUCCIÓN DE DIRECCIONES
    # ==================================================

    @staticmethod
    def _nombre_direccion(valor):

        direcciones = {
            "0": "Ninguna",
            "1": "Izquierda",
            "2": "Diagonal izquierda",
            "3": "Centro",
            "4": "Diagonal derecha",
            "5": "Derecha",
            "6": "Frontal",
            "7": "Suelo"
        }

        return direcciones.get(
            str(valor),
            "Sin datos"
        )
