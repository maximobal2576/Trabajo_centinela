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
_ANCHO_PANEL = dp(220)
# Velocidad de la animación en segundos
_DURACION_ANIM = 0.22


class _PanelLateral(FloatLayout):
    """Panel deslizable que contiene los ítems de navegación."""

    DESTINOS = [
        ("home",          "Inicio",        "inicio"),
        ("signal",        "Estado",        "estado"),
        ("compass",       "Navegación",    "navegacion"),
        ("alert-octagon", "Emergencia",    "emergencia"),
        ("cog",           "Configuración", "configuracion"),
        ("map-marker-path","Rutas",        "rutas"),
    ]

    def __init__(self, cerrar_cb, **kwargs):
        super().__init__(**kwargs)
        self._cerrar_cb = cerrar_cb
        self.size_hint = (None, 1)
        self.width = _ANCHO_PANEL

        # --- Fondo del panel ---
        with self.canvas.before:
            Color(*OSCURO[:3], 0.97)
            self._rect = RoundedRectangle(
                pos=self.pos, size=self.size,
                radius=[0, dp(16), dp(16), 0]
            )
        self.bind(pos=self._actualizar_rect, size=self._actualizar_rect)

        contenedor = MDBoxLayout(
            orientation="vertical",
            padding=[dp(12), dp(48), dp(12), dp(16)],
            spacing=dp(8),
            size_hint=(1, 1),
        )

        # Título del menú (solo visual y accesible)
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
        contenedor.add_widget(Widget(size_hint_y=None, height=dp(8)))

        # Ítems de navegación
        for _icono, texto, pantalla in self.DESTINOS:
            fila = MDBoxLayout(
                orientation="horizontal",
                size_hint=(1, None),
                height=dp(52),
                spacing=dp(10),
                padding=[dp(8), 0, dp(8), 0],
            )

            # Etiqueta del ítem — TalkBack la lee al navegar por accesibilidad
            etiqueta = MDRaisedButton(
                text=texto,
                size_hint=(1, None),
                height=dp(48),
                font_size=sp(14),
                md_bg_color=BORDO if self.DESTINOS.index((_icono, texto, pantalla)) % 2 == 0 else BORDO_OSCURO,
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
        self._rect.pos = self.pos
        self._rect.size = self.size

    def _navegar(self, pantalla):
        self._cerrar_cb()
        app = MDApp.get_running_app()
        if app and hasattr(app, "switch_screen"):
            app.switch_screen(pantalla)


class MenuLateral(FloatLayout):
    """
    Contenedor principal del menú lateral.

    Agrega al root:
      · Un overlay oscuro semitransparente (se muestra al abrir el panel).
      · El panel deslizable con los ítems.
      · El botón hamburguesa flotante (sin texto, accesible via hint_text).
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._abierto = False

        # --- Overlay semitransparente ---
        self._overlay = Widget(size_hint=(1, 1), opacity=0)
        with self._overlay.canvas.before:
            Color(0, 0, 0, 0.45)
            self._overlay_rect = RoundedRectangle(
                pos=self._overlay.pos, size=self._overlay.size
            )
        self._overlay.bind(
            pos=self._upd_overlay,
            size=self._upd_overlay,
            on_touch_down=self._overlay_tocado,
        )
        self._overlay.opacity = 0
        self.add_widget(self._overlay)

        # --- Panel lateral ---
        self._panel = _PanelLateral(cerrar_cb=self.cerrar)
        # Empieza fuera de pantalla (a la izquierda)
        self._panel.x = -_ANCHO_PANEL
        self.add_widget(self._panel)

        # --- Botón hamburguesa ---
        # hint_text es leído por TalkBack como nombre accesible del botón.
        # No tiene texto visible — solo el ícono "menu".
        self._boton = MDIconButton(
            icon="menu",
            md_bg_color=BORDO,
            theme_icon_color="Custom",
            icon_color=CREMA_CLARO,
            pos_hint={"x": 0.02, "top": 0.98},
            size_hint=(None, None),
            size=(dp(52), dp(52)),
            # hint_text es el nombre accesible que TalkBack anuncia
            user_font_size=sp(26),
        )
        # Workaround: MDIconButton no expone hint_text directo en todas las
        # versiones de KivyMD; lo asignamos manualmente para accesibilidad.
        try:
            self._boton.hint_text = "Menú"
        except AttributeError:
            pass

        self._boton.bind(on_release=self.alternar)
        self.add_widget(self._boton)

    # ------------------------------------------------------------------
    # OVERLAY
    # ------------------------------------------------------------------

    def _upd_overlay(self, widget, *_):
        self._overlay_rect.pos = widget.pos
        self._overlay_rect.size = widget.size

    def _overlay_tocado(self, widget, touch):
        if self._abierto and widget.collide_point(*touch.pos):
            self.cerrar()
            return True
        return False

    # ------------------------------------------------------------------
    # ABRIR / CERRAR
    # ------------------------------------------------------------------

    def alternar(self, *_):
        if self._abierto:
            self.cerrar()
        else:
            self.abrir()

    def abrir(self):
        if self._abierto:
            return
        self._abierto = True
        Animation(x=0, duration=_DURACION_ANIM, t="out_quad").start(self._panel)
        Animation(opacity=1, duration=_DURACION_ANIM).start(self._overlay)

    def cerrar(self):
        if not self._abierto:
            return
        self._abierto = False
        Animation(x=-_ANCHO_PANEL, duration=_DURACION_ANIM, t="in_quad").start(
            self._panel
        )
        Animation(opacity=0, duration=_DURACION_ANIM).start(self._overlay)
