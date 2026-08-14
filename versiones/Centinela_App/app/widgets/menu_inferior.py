from kivymd.app import MDApp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDRaisedButton

from components.estilos_centinela import (
    CREMA_CLARO,
    BORDO,
    BORDO_OSCURO,
)


class MenuInferior(MDBoxLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "horizontal"

        self.size_hint_y = None
        self.height = 70

        self.padding = [5, 8, 5, 8]
        self.spacing = 5

        # ==========================================
        # BOTÓN INICIO
        # ==========================================

        self.boton_inicio = MDRaisedButton(
            text="Inicio",
            size_hint=(1, 1),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=lambda *args: self._switch("inicio")
        )
        self.add_widget(self.boton_inicio)

        # ==========================================
        # BOTÓN ESTADO
        # ==========================================

        self.boton_estado = MDRaisedButton(
            text="Estado",
            size_hint=(1, 1),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=lambda *args: self._switch("estado")
        )
        self.add_widget(self.boton_estado)

        # ==========================================
        # BOTÓN NAVEGACIÓN
        # ==========================================

        self.boton_navegacion = MDRaisedButton(
            text="Navegación",
            size_hint=(1, 1),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=lambda *args: self._switch("navegacion")
        )
        self.add_widget(self.boton_navegacion)

        # ==========================================
        # BOTÓN EMERGENCIA
        # ==========================================

        self.boton_emergencia = MDRaisedButton(
            text="Emergencia",
            size_hint=(1, 1),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=lambda *args: self._switch("emergencia")
        )
        self.add_widget(self.boton_emergencia)

        # ==========================================
        # BOTÓN CONFIGURACIÓN
        # ==========================================

        self.boton_configuracion = MDRaisedButton(
            text="Configuración",
            size_hint=(1, 1),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=lambda *args: self._switch("configuracion")
        )
        self.add_widget(self.boton_configuracion)

        # ==========================================
        # BOTÓN RUTAS
        # ==========================================

        self.boton_rutas = MDRaisedButton(
            text="Rutas",
            size_hint=(1, 1),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=lambda *args: self._switch("rutas")
        )
        self.add_widget(self.boton_rutas)

    def _switch(self, screen_name):

        app = MDApp.get_running_app()

        if hasattr(app, "switch_screen"):
            app.switch_screen(screen_name)