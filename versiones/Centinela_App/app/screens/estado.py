from kivy.clock import Clock
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel

from services.bluetooth import BluetoothService

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
        self.padding = [30, 5, 70, 90]
        self.spacing = 14

        # =========================
        # SERVICIO BLUETOOTH
        # =========================

        self.bluetooth_service = BluetoothService()

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
            height=60,
        )

        self.add_widget(self.titulo)

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
            height=45,
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
            height=45,
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
            height=45,
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
            height=45,
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
            height=45,
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