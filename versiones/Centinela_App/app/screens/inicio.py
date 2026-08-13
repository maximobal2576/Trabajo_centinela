from kivy.clock import Clock
from kivymd.app import MDApp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel
from kivymd.uix.button import MDRaisedButton
from kivy.uix.image import Image

from services.bluetooth import BluetoothService

class InicioScreen(MDBoxLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = [30, 10, 30, 100]
        self.spacing = 15

        # ============================================================
        # BLUETOOTH
        # ============================================================

        self.bluetooth_service = BluetoothService()

        # ============================================================
        # ESTADO DEL BASTÓN
        # ============================================================

        self.estado_baston = MDLabel(
            text="BASTÓN DESCONECTADO",
            halign="center",
            theme_text_color="Custom",
            text_color=(1.0, 0.35, 0.35, 1),
            size_hint_y=None,
            height=40,
            bold=True
        )

        self.add_widget(self.estado_baston)

        # ============================================================
        # ILUSTRACIÓN DEL BASTÓN
        # ============================================================

        self.ilustracion = Image(
            source="assets/images/baston_centinela.png",
            size_hint_y=None,
            height=260,
            allow_stretch=True,
            keep_ratio=True,
        )

        self.add_widget(self.ilustracion)

        # ============================================================
        # BATERÍA
        # ============================================================

        self.bateria_label = MDLabel(
            text="Batería: -- %",
            halign="center",
            font_style="H5",
            theme_text_color="Custom",
            text_color=(0.30, 1.0, 0.55, 1),
            size_hint_y=None,
            height=55,
        )

        self.add_widget(self.bateria_label)

        # ============================================================
        # NAVEGACIÓN
        # ============================================================

        self.boton_navegar = MDRaisedButton(
            text="NAVEGACIÓN",
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=60,
            md_bg_color=(0.05, 0.70, 0.30, 1),
        )

        self.boton_navegar.bind(
            on_release=self.ir_a_gps
        )

        self.add_widget(self.boton_navegar)

        # ============================================================
        # CONEXIÓN
        # ============================================================

        self.boton_conectar = MDRaisedButton(
            text="CONECTAR BASTÓN",
            pos_hint={"center_x": 0.5},
            size_hint=(0.70, None),
            height=48,
            md_bg_color=(0.08, 0.25, 0.28, 1),
        )

        self.boton_conectar.bind(
            on_release=self.alternar_conexion
        )

        self.add_widget(self.boton_conectar)

        # ============================================================
        # ACTUALIZACIÓN
        # ============================================================

        Clock.schedule_interval(
            self.actualizar_estado,
            0.5
        )

    # ================================================================
    # ACTUALIZAR DATOS
    # ================================================================

    def actualizar_estado(self, dt):

        self.bluetooth_service.update(dt)

        if self.bluetooth_service.connected:

            self.estado_baston.text = "BASTÓN CONECTADO"

            self.estado_baston.text_color = (
                0.30,
                1.0,
                0.50,
                1
            )

            self.boton_conectar.text = "DESCONECTAR BASTÓN"

        else:

            self.estado_baston.text = "BASTÓN DESCONECTADO"

            self.estado_baston.text_color = (
                1.0,
                0.35,
                0.35,
                1
            )

            self.boton_conectar.text = "CONECTAR BASTÓN"

        bateria = self.bluetooth_service.state.get(
            "BATERIA",
            "--"
        )

        self.bateria_label.text = f"Batería: {bateria} %"

    # ================================================================
    # CONECTAR / DESCONECTAR
    # ================================================================

    def alternar_conexion(self, instance):

        if self.bluetooth_service.connected:

            self.bluetooth_service.disconnect()

        else:

            self.bluetooth_service.connect()

        self.actualizar_estado(0)

    # ================================================================
    # IR A NAVEGACIÓN
    # ================================================================

    def ir_a_gps(self, instance):

        app = MDApp.get_running_app()

        if hasattr(app, "switch_screen"):

            app.switch_screen("navegacion")
