"""
MenuLateral — Cajón de navegación lateral para Centinela App.

Reemplaza al antiguo MenuInferior. Se compone de:
  - Un botón hamburguesa flotante (sin texto visible) en la esquina superior
    izquierda. TalkBack lo anuncia como "Menú, botón" gracias al hint_text.
  - Un MDNavigationDrawer que se desliza desde la izquierda con los ítems
    de navegación. El texto de cada ítem SÍ es visible para que TalkBack
    pueda leerlo al navegar por la pantalla.

Uso desde main.py:
    from widgets.menu_lateral import MenuLateral
    menu = MenuLateral(screen_manager=self.sm)
    root.add_widget(menu)
"""

from kivy.metrics import dp, sp
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.relativelayout import RelativeLayout
from kivy.animation import Animation
from kivy.graphics import Color, RoundedRectangle
from kivymd.app import MDApp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDIconButton, MDRaisedButton
from kivymd.uix.label import MDLabel
from kivy.uix.widget import Widget

from components.estilos_centinela import (
    BORDO,
    BORDO_OSCURO,
    OSCURO,
    CREMA_CLARO,
    DORADO_APAGADO,
)

# Ancho del panel lateral cuando está abierto
_ANCHO_PANEL = dp(240)
# Velocidad de la animación en segundos
_DURACION_ANIM = 0.22


class _PanelLateral(RelativeLayout):
    """Panel deslizable que contiene los ítems de navegación."""

    DESTINOS = [
        ("home", "Inicio", "inicio"),
        ("signal", "Estado", "estado"),
        ("compass", "Navegación", "navegacion"),
        ("alert-octagon", "Emergencia", "emergencia"),
        ("cog", "Configuración", "configuracion"),
        ("map-marker-path", "Rutas", "rutas"),
    ]

    def __init__(self, cerrar_cb, **kwargs):
        super().__init__(**kwargs)
        self._cerrar_cb = cerrar_cb
        self.size_hint = (None, 1)
        self.width = _ANCHO_PANEL

        # Fondo del panel
        with self.canvas.before:
            Color(*OSCURO[:3], 0.98)
            self._rect = RoundedRectangle(
                pos=(0, 0), size=self.size,
                radius=[0, dp(18), dp(18), 0]
            )
        self.bind(size=self._actualizar_rect)

        contenedor = MDBoxLayout(
            orientation="vertical",
            padding=[dp(16), dp(56), dp(16), dp(20)],
            spacing=dp(10),
            size_hint=(1, 1),
            pos_hint={"x": 0, "y": 0},
        )

        # Título del menú
        titulo = MDLabel(
            text="CENTINELA",
            halign="center",
            bold=True,
            font_style="H6",
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=dp(44),
        )
        contenedor.add_widget(titulo)
        contenedor.add_widget(Widget(size_hint_y=None, height=dp(6)))

        # Ítems de navegación
        for i, (_icono, texto, pantalla) in enumerate(self.DESTINOS):
            fila = MDBoxLayout(
                orientation="horizontal",
                size_hint=(1, None),
                height=dp(50),
                spacing=dp(8),
            )

            etiqueta = MDRaisedButton(
                text=texto,
                size_hint=(1, 1),
                font_size=sp(14),
                md_bg_color=BORDO if i % 2 == 0 else BORDO_OSCURO,
                theme_text_color="Custom",
                text_color=CREMA_CLARO,
                elevation=0,
            )
            etiqueta.bind(
                on_release=lambda _btn, dest=pantalla: self._navegar(dest)
            )
            fila.add_widget(etiqueta)
            contenedor.add_widget(fila)

        self.add_widget(contenedor)

    def _actualizar_rect(self, *_):
        self._rect.pos = (0, 0)
        self._rect.size = self.size

    def _navegar(self, pantalla):
        self._cerrar_cb()
        app = MDApp.get_running_app()
        if app and hasattr(app, "switch_screen"):
            app.switch_screen(pantalla)


class MenuLateral(FloatLayout):
    """
    Contenedor principal del menú lateral deslizable.
    Mantiene el panel oculto a la izquierda y lo muestra al presionar el botón hamburguesa.
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._abierto = False

        # --- Overlay semitransparente ---
        self._overlay = Widget(size_hint=(1, 1), opacity=0)
        with self._overlay.canvas.before:
            Color(0, 0, 0, 0.5)
            self._overlay_rect = RoundedRectangle(
                pos=self.pos, size=self.size
            )
        self._overlay.bind(
            pos=self._upd_overlay,
            size=self._upd_overlay,
        )
        self.add_widget(self._overlay)

        # --- Panel lateral ---
        self._panel = _PanelLateral(cerrar_cb=self.cerrar)
        self._panel.x = -_ANCHO_PANEL
        self.add_widget(self._panel)

        # --- Botón hamburguesa superior izquierdo ---
        self._boton = MDIconButton(
            icon="menu",
            md_bg_color=BORDO,
            theme_icon_color="Custom",
            icon_color=CREMA_CLARO,
            pos_hint={"x": 0.03, "top": 0.98},
            size_hint=(None, None),
            size=(dp(48), dp(48)),
            icon_size=sp(26),
        )
        self._boton.bind(on_release=self.alternar)
        self.add_widget(self._boton)

    def on_touch_down(self, touch):
        # 1. Si toca el botón de hamburguesa, procesarlo
        if self._boton.collide_point(*touch.pos):
            return self._boton.on_touch_down(touch)

        # 2. Si el menú está abierto:
        if self._abierto:
            # Si toca dentro del panel, pasar el toque al panel
            if self._panel.collide_point(*touch.pos):
                return self._panel.on_touch_down(touch)
            # Si toca afuera del panel (en el overlay), cerrar el menú y consumir el toque
            self.cerrar()
            return True

        # 3. Si el menú está cerrado, NO interceptar ningún toque: pasa directo a las pantallas
        return False

    def on_touch_move(self, touch):
        if self._abierto and self._panel.collide_point(*touch.pos):
            return self._panel.on_touch_move(touch)
        return False

    def on_touch_up(self, touch):
        if self._boton.collide_point(*touch.pos):
            return self._boton.on_touch_up(touch)
        if self._abierto and self._panel.collide_point(*touch.pos):
            return self._panel.on_touch_up(touch)
        return False

    def on_size(self, *_):
        """Al cambiar el tamaño de pantalla (ej. teclado), reposicionar el panel
        sin interrumpir animaciones activas."""
        if not self._abierto:
            self._panel.x = -_ANCHO_PANEL

    def _upd_overlay(self, widget, *_):
        self._overlay_rect.pos = widget.pos
        self._overlay_rect.size = widget.size

    def alternar(self, *_):
        if self._abierto:
            self.cerrar()
        else:
            self.abrir()

    def abrir(self):
        if self._abierto:
            return
        self._abierto = True
        Animation.stop_all(self._panel)
        Animation.stop_all(self._overlay)
        Animation(x=0, duration=_DURACION_ANIM, t="out_quad").start(self._panel)
        Animation(opacity=1, duration=_DURACION_ANIM).start(self._overlay)

    def cerrar(self):
        if not self._abierto:
            return
        self._abierto = False
        Animation.stop_all(self._panel)
        Animation.stop_all(self._overlay)
        anim_panel = Animation(x=-_ANCHO_PANEL, duration=_DURACION_ANIM, t="in_quad")
        anim_overlay = Animation(opacity=0, duration=_DURACION_ANIM)
        anim_panel.start(self._panel)
        anim_overlay.start(self._overlay)
