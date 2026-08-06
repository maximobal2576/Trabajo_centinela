# MAIN
from kivymd.app import MDApp
from kivy.uix.screenmanager import ScreenManager, Screen

from screens.inicio import InicioScreen


class CentinelaApp(MDApp):
    def build(self):
        self.title = "Centinela"
        sm = ScreenManager()
        pantalla_inicio = Screen(name="inicio")
        pantalla_inicio.add_widget(InicioScreen())
        sm.add_widget(pantalla_inicio)
        return sm


if __name__ == "__main__":
    CentinelaApp().run()