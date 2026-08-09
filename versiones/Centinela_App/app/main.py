# MAIN
from kivymd.app import MDApp
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.screenmanager import ScreenManager, Screen

from screens.inicio import InicioScreen
from screens.estado import EstadoScreen
from screens.navegacion import NavegacionScreen
from screens.emergencia import EmergenciaScreen
from screens.configuracion import ConfiguracionScreen
from screens.rutas import RutasScreen
from widgets.menu_inferior import MenuInferior


class CentinelaApp(MDApp):
    def build(self):
        self.theme_cls.primary_palette = "Green"
        self.theme_cls.theme_style = "Dark"
        self.title = "Centinela"

        root = BoxLayout(orientation="vertical")
        self.sm = ScreenManager()

        self._add_screen("inicio", InicioScreen())
        self._add_screen("estado", EstadoScreen())
        self._add_screen("navegacion", NavegacionScreen())
        self._add_screen("emergencia", EmergenciaScreen())
        self._add_screen("configuracion", ConfiguracionScreen())
        self._add_screen("rutas", RutasScreen())

        root.add_widget(self.sm)
        root.add_widget(MenuInferior())

        return root

    def _add_screen(self, name, widget):
        screen = Screen(name=name)
        screen.add_widget(widget)
        self.sm.add_widget(screen)

    def switch_screen(self, name):
        if name in self.sm.screen_names:
            self.sm.current = name


if __name__ == "__main__":
    CentinelaApp().run()
