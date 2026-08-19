from kivy.clock import Clock
from kivymd.app import MDApp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel
from kivymd.uix.button import MDRaisedButton
from kivy.uix.image import Image

from services.bluetooth import BluetoothService

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

        self.padding = [30, 5, 70, 90]
        self.spacing = 15

        self.bluetooth_service = BluetoothService()

        # ==========================================
        # ESTADO DEL BASTÓN
        # ==========================================

        self.estado_baston = MDLabel(
            text="BASTÓN DESCONECTADO",
            halign="center",
            font_style="H5",
            bold=True,
            theme_text_color="Custom",
            text_color=ROJO_EMERGENCIA,
            size_hint_y=None,
            height=45,
        )

        self.add_widget(self.estado_baston)

        # ==========================================
        # IMAGEN DEL BASTÓN
        # ==========================================

        self.ilustracion = Image(
            source="assets/images/baston_centinela.png",
            size_hint_y=None,
            height=250,
            allow_stretch=True,
            keep_ratio=True,
        )

        self.add_widget(self.ilustracion)

        # ==========================================
        # BATERÍA
        # ==========================================

        self.bateria_label = MDLabel(
            text="Batería: -- %",
            halign="center",
            font_style="H5",
            bold=True,
            theme_text_color="Custom",
            text_color=VERDE_ESTADO,
            size_hint_y=None,
            height=50,
        )

        self.add_widget(self.bateria_label)

        # ==========================================
        # BOTÓN NAVEGACIÓN
        # ==========================================

        self.boton_navegar = MDRaisedButton(
            text="NAVEGACIÓN",
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=55,
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )

        self.boton_navegar.bind(
            on_release=self.ir_a_gps
        )

        self.add_widget(self.boton_navegar)

        # ==========================================
        # BOTÓN CONECTAR
        # ==========================================

        self.boton_conectar = MDRaisedButton(
            text="CONECTAR BASTÓN",
            pos_hint={"center_x": 0.5},
            size_hint=(0.70, None),
            height=50,
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )

        self.boton_conectar.bind(
            on_release=self.alternar_conexion
        )

        self.add_widget(self.boton_conectar)

        # ==========================================
        # ACTUALIZACIÓN
        # ==========================================

        Clock.schedule_interval(
            self.actualizar_estado,
            0.5
        )

    def actualizar_estado(self, dt):

        self.bluetooth_service.update(dt)

        if self.bluetooth_service.connected:

            self.estado_baston.text = (
                "BASTÓN CONECTADO"
            )

            self.estado_baston.text_color = (
                VERDE_ESTADO
            )

            self.boton_conectar.text = (
                "DESCONECTAR BASTÓN"
            )

        else:

            self.estado_baston.text = (
                "BASTÓN DESCONECTADO"
            )

            self.estado_baston.text_color = (
                ROJO_EMERGENCIA
            )

            self.boton_conectar.text = (
                "CONECTAR BASTÓN"
            )

        bateria = self.bluetooth_service.state.get(
            "BATERIA",
            "--"
        )

        self.bateria_label.text = (
            f"Batería: {bateria} %"
        )

    def alternar_conexion(self, instance):

        if self.bluetooth_service.connected:

            self.bluetooth_service.disconnect()

        else:

            self.bluetooth_service.connect()

        self.actualizar_estado(0)

    def ir_a_gps(self, instance):

        app = MDApp.get_running_app()

        if hasattr(app, "switch_screen"):

            app.switch_screen(
                "navegacion"
            )