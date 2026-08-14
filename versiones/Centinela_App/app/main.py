from kivymd.app import MDApp
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.screenmanager import ScreenManager, Screen

from screens.inicio import InicioScreen
from screens.estado import EstadoScreen
from screens.navegacion import NavegacionScreen
from screens.emergencia import EmergenciaScreen
from screens.configuracion import ConfiguracionScreen
from screens.rutas import RutasScreen

from widgets.menu_inferior import MenuInferior
from components.fondo_centinela import FondoCentinela


class CentinelaApp(MDApp):

    def build(self):

        self.theme_cls.primary_palette = "Green"
        self.theme_cls.theme_style = "Dark"
        self.title = "Centinela"

        # ============================================================
        # CONTENEDOR PRINCIPAL
        # ============================================================

        root = FloatLayout()

        # ============================================================
        # FONDO
        # ============================================================

        fondo = FondoCentinela(
            size_hint=(1, 1),
            pos_hint={"x": 0, "y": 0}
        )

        root.add_widget(fondo)

        # ============================================================
        # PANTALLAS
        # ============================================================

        self.sm = ScreenManager(
            size_hint=(1, 1),
            pos_hint={"x": 0, "y": 0}
        )

        self._add_screen("inicio", InicioScreen())
        self._add_screen("estado", EstadoScreen())
        self._add_screen("navegacion", NavegacionScreen())
        self._add_screen("emergencia", EmergenciaScreen())
        self._add_screen("configuracion", ConfiguracionScreen())
        self._add_screen("rutas", RutasScreen())

        root.add_widget(self.sm)

        # ============================================================
        # MENÚ INFERIOR
        # ============================================================

        menu = MenuInferior()

        # IMPORTANTE:
        # No usamos size_hint_x.
        menu.size_hint_x = None
        menu.size_hint_y = None

        menu.height = 70
        menu.width = root.width * 0.96

        menu.y = 0
        menu.x = (root.width - menu.width) / 2

        root.add_widget(menu)

        # ============================================================
        # MANTENER MENÚ CENTRADO AL CAMBIAR TAMAÑO
        # ============================================================

        def actualizar_menu(*args):

            menu.width = root.width * 0.96
            menu.x = (root.width - menu.width) / 2
            menu.y = 0

        root.bind(
            size=actualizar_menu
        )

        actualizar_menu()

        return root

    # ================================================================
    # AGREGAR PANTALLA
    # ================================================================

    def _add_screen(self, name, widget):

        screen = Screen(name=name)
        screen.add_widget(widget)

        self.sm.add_widget(screen)

    # ================================================================
    # CAMBIAR PANTALLA
    # ================================================================

    def switch_screen(self, name):

        if name in self.sm.screen_names:
            self.sm.current = name


# ====================================================================
# EJECUTAR
# ====================================================================

if __name__ == "__main__":
    CentinelaApp().run()