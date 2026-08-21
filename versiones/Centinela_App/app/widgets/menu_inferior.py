from kivy.metrics import dp, sp
from kivy.uix.scrollview import ScrollView
from kivymd.app import MDApp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDRaisedButton
from kivy.uix.widget import Widget

from components.estilos_centinela import CREMA_CLARO, BORDO, BORDO_OSCURO


class MenuInferior(Widget):
    """Barra inferior deslizable horizontalmente con accesos a todas las pantallas."""

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.do_scroll_x = False
        self.do_scroll_y = True
        self.bar_color = [0, 0, 0, 0]
        self.bar_inactive_color = [0, 0, 0, 0]

        contenedor = MDBoxLayout(
            orientation="vertical",
            size_hint=(None, 1),
            padding=[dp(8), dp(8), dp(8), dp(8)],
            spacing=dp(8),
        )
        contenedor.bind(minimum_width=contenedor.setter("width"))

        for texto, color, pantalla in (
            ("Inicio", BORDO, "inicio"),
            ("Estado", BORDO_OSCURO, "estado"),
            ("Navegación", BORDO, "navegacion"),
            ("Emergencia", BORDO_OSCURO, "emergencia"),
            ("Configuración", BORDO, "configuracion"),
            ("Rutas", BORDO_OSCURO, "rutas"),
        ):
            boton = MDRaisedButton(
                text=texto,
                size_hint=(None, 1),
                width=dp(110),
                font_size=sp(12),
                md_bg_color=color,
                theme_text_color="Custom",
                text_color=CREMA_CLARO,
                on_release=lambda _button, destino=pantalla: self._switch(destino),
            )
            contenedor.add_widget(boton)

        self.add_widget(contenedor)

    @staticmethod
    def _switch(screen_name):
        app = MDApp.get_running_app()
        if app and hasattr(app, "switch_screen"):
            app.switch_screen(screen_name)
