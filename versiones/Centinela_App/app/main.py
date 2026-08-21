from kivymd.app import MDApp
from kivymd.uix.button import MDFloatingActionButton
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.core.window import Window
from kivy.metrics import dp

from screens.inicio import InicioScreen
from screens.estado import EstadoScreen
from screens.navegacion import NavegacionScreen
from screens.emergencia import EmergenciaScreen
from screens.configuracion import ConfiguracionScreen
from screens.rutas import RutasScreen

from widgets.menu_lateral import MenuLateral
from components.fondo_centinela import FondoCentinela
from components.estilos_centinela import BORDO


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
        # PANTALLAS — ocupan el 100 % de la pantalla
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
        # MENÚ LATERAL DESPLEGABLE
        # El MenuLateral se coloca sobre el ScreenManager en el FloatLayout.
        # Contiene el botón hamburguesa y el panel deslizable.
        # ============================================================

        self.menu = MenuLateral(
            size_hint=(1, 1),
            pos_hint={"x": 0, "y": 0},
        )
        root.add_widget(self.menu)

        return root

    def on_start(self):
        """El botón Atrás cierra el menú si está abierto, o vuelve al inicio."""
        Window.bind(on_keyboard=self._on_keyboard)

    def _on_keyboard(self, _window, key, *_args):
        if key == 27:
            # Si el menú está abierto, cerrarlo primero
            if hasattr(self, "menu") and self.menu._abierto:
                self.menu.cerrar()
                return True
            if getattr(self, "sm", None) and self.sm.current != "inicio":
                self.switch_screen("inicio", direction="right")
                return True
        return False

    # ================================================================
    # AGREGAR PANTALLA
    # ================================================================

    def _add_screen(self, name, widget):

        screen = Screen(name=name)
        screen.add_widget(widget)

        # En la pantalla de navegación el FAB es el micrófono.
        # En las demás pantallas es el acceso rápido a Navegación.
        if name == "navegacion":
            screen.add_widget(MDFloatingActionButton(
                icon="microphone",
                md_bg_color=BORDO,
                pos_hint={"right": 0.96, "y": 0.02},
                on_release=widget.escuchar_comando,
            ))
        else:
            screen.add_widget(MDFloatingActionButton(
                icon="navigation-variant",
                md_bg_color=BORDO,
                pos_hint={"right": 0.96, "y": 0.02},
                on_release=lambda _button: self.switch_screen("navegacion"),
            ))

        self.sm.add_widget(screen)

    # ================================================================
    # CAMBIAR PANTALLA
    # ================================================================

    def switch_screen(self, name, direction="left"):

        if name in self.sm.screen_names:
            if self.sm.current != name:
                self.sm.transition.direction = direction
                self.sm.current = name


# ====================================================================
# EJECUTAR
# ====================================================================

if __name__ == "__main__":
    CentinelaApp().run()
