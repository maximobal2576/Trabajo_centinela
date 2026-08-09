from kivymd.app import MDApp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDRaisedButton


class MenuInferior(MDBoxLayout):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "horizontal"
        self.padding = 40
        self.spacing = 8
        self.size_hint_y = None
        self.height = 70       
        self.size_hint_x = 0.5
        self.pos_hint = {"center_x": 0.5}
        self.boton_inicio = MDRaisedButton(
            text="Inicio",
            on_release=lambda *args: self._switch("inicio")
        )
        self.add_widget(self.boton_inicio)

        self.boton_estado = MDRaisedButton(
            text="Estado",
            on_release=lambda *args: self._switch("estado")
        )
        self.add_widget(self.boton_estado)

        self.boton_navegacion = MDRaisedButton(
            text="Navegación",
            on_release=lambda *args: self._switch("navegacion")
        )
        self.add_widget(self.boton_navegacion)

        self.boton_emergencia = MDRaisedButton(
            text="Emergencia",
            on_release=lambda *args: self._switch("emergencia")
        )
        self.add_widget(self.boton_emergencia)

        self.boton_configuracion = MDRaisedButton(
            text="Configuración",
            on_release=lambda *args: self._switch("configuracion")
        )
        self.add_widget(self.boton_configuracion)

        self.boton_rutas = MDRaisedButton(
            text="Rutas",
            on_release=lambda *args: self._switch("rutas")
        )
        self.add_widget(self.boton_rutas)

    def _switch(self, screen_name):
        app = MDApp.get_running_app()
        if hasattr(app, "switch_screen"):
            app.switch_screen(screen_name)
