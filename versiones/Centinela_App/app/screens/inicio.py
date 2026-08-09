from kivy.clock import Clock
from kivymd.app import MDApp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel
from kivymd.uix.button import MDRaisedButton

from services.bluetooth import BluetoothService

class InicioScreen(MDBoxLayout):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = 20
        self.spacing = 16

        self.bluetooth_service = BluetoothService()

        self.titulo = MDLabel(
            text="Centinela",
            halign="center",
            font_style="H4",
        )
        self.add_widget(self.titulo)

        self.estado_baston = MDLabel(
            text="Bastón: Desconectado",
            halign="center",
        )
        self.add_widget(self.estado_baston)

        self.bateria_label = MDLabel(
            text="Batería: --%",
            halign="center",
            theme_text_color="Custom",
            text_color=(0.1, 0.7, 0.2, 1)
        )
        self.add_widget(self.bateria_label)

        self.estado_sistema = MDLabel(
            text="Sistema: Sin datos",
            halign="center",
        )
        self.add_widget(self.estado_sistema)

        self.evento_label = MDLabel(
            text="Evento: Sin datos",
            halign="center",
        )
        self.add_widget(self.evento_label)

        self.direccion_label = MDLabel(
            text="Dirección: Sin datos",
            halign="center",
        )
        self.add_widget(self.direccion_label)

        self.boton_conectar = MDRaisedButton(
            text="Conectar bastón",
            md_bg_color=(0.1, 0.5, 0.9, 1),
            pos_hint={"center_x": 0.5},
        )
        self.boton_conectar.bind(on_release=self.alternar_conexion)
        self.add_widget(self.boton_conectar)

        self.conexion_label = MDLabel(
            text="Estado de conexión: Desconectado",
            halign="center",
        )
        self.add_widget(self.conexion_label)

        self.boton_navegar = MDRaisedButton(
            text="Ir a navegación \n 🗺️",
            md_bg_color=(0.0, 0.6, 0.2, 1),
            pos_hint={"center_x": 0.5},
        )
        self.boton_navegar.bind(on_release=self.ir_a_gps)
        self.add_widget(self.boton_navegar)

        Clock.schedule_interval(self.actualizar_estado, 0.5)

    def actualizar_estado(self, dt):
        self.bluetooth_service.update(dt)
        self.estado_baston.text = (
            "Bastón: Conectado" if self.bluetooth_service.connected else "Bastón: Desconectado"
        )
        self.bateria_label.text = f"Batería: {self.bluetooth_service.state.get('BATERIA', '--')}%"
        self.estado_sistema.text = (
            f"Sistema: {self._nombre_estado(self.bluetooth_service.state.get('ESTADO'))}"
        )
        self.evento_label.text = (
            f"Evento: {self._nombre_evento(self.bluetooth_service.state.get('EVENTO'))}"
        )
        self.direccion_label.text = (
            f"Dirección: {self._nombre_direccion(self.bluetooth_service.state.get('DIRECCION'))}"
        )

        if self.bluetooth_service.connected:
            self.boton_conectar.text = "Desconectar bastón"
            self.conexion_label.text = "Estado de conexión: Conectado"
        else:
            self.boton_conectar.text = "Conectar bastón"
            self.conexion_label.text = (
                self.bluetooth_service.last_error or "Estado de conexión: Desconectado"
            )

    def alternar_conexion(self, instance):
        if self.bluetooth_service.connected:
            self.bluetooth_service.disconnect()
        else:
            self.bluetooth_service.connect()

        self.actualizar_estado(0)

    @staticmethod
    def _nombre_estado(valor):
        estados = {
            "0": "Inicio",
            "1": "Normal",
            "2": "Alerta",
            "3": "Pozo",
            "4": "Caída",
            "5": "Diagnóstico",
            "6": "Error",
        }
        return estados.get(str(valor), "Sin datos")

    @staticmethod
    def _nombre_evento(valor):
        eventos = {
            "0": "Normal",
            "1": "Obstáculo frontal",
            "2": "Obstáculo de radar",
            "3": "Pozo",
            "4": "Caída",
            "5": "Batería baja",
            "6": "Error",
        }
        return eventos.get(str(valor), "Sin datos")

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
            "7": "Suelo",
        }
        return direcciones.get(str(valor), "Sin datos")

    def ir_a_gps(self, instance):
        app = MDApp.get_running_app()
        if hasattr(app, "switch_screen"):
            app.switch_screen("navegacion")
