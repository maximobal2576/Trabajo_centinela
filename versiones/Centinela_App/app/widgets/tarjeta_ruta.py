# -*- coding: utf-8 -*-
"""
widgets/tarjeta_ruta.py - Componente visual accesible para mostrar una ruta en el historial.

Representa cada fila de la lista de rutas guardadas:
- Nombre principal de la ruta en texto claro de alto contraste.
- Subtitulo con detalles (destino, cantidad de puntos GPS o fecha).
- Respuesta tactil y cambio visual al ser seleccionada.
"""

from kivy.metrics import dp
from kivymd.uix.card import MDCard
from kivymd.uix.label import MDLabel

from components.estilos_centinela import (
    BORDO,
    BORDO_OSCURO,
    CREMA_CLARO,
    DORADO_APAGADO,
    OSCURO,
)


class TarjetaRutaItem(MDCard):
    """Tarjeta interactiva que representa una ruta individual del historial."""

    def __init__(self, nombre, detalle, indice, on_select_cb=None, **kwargs):
        super().__init__(**kwargs)
        self.indice = indice
        self.on_select_cb = on_select_cb

        self.orientation = "vertical"
        self.size_hint = (1, None)
        self.height = dp(64)
        self.padding = [dp(12), dp(8), dp(12), dp(8)]
        self.spacing = dp(2)
        self.md_bg_color = OSCURO
        self.line_color = BORDO_OSCURO
        self.line_width = dp(1)
        self.elevation = 2
        self.ripple_behavior = True

        # Titulo / Nombre de la ruta
        self.lbl_nombre = MDLabel(
            text=nombre,
            bold=True,
            font_style="Subtitle1",
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(26),
        )
        self.add_widget(self.lbl_nombre)

        # Detalle secundario
        self.lbl_detalle = MDLabel(
            text=detalle,
            font_style="Caption",
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=dp(20),
        )
        self.add_widget(self.lbl_detalle)

        self.bind(on_release=self._on_click)

    def set_seleccionada(self, seleccionada):
        """Alterna el aspecto visual segun si esta seleccionada o inactiva."""
        if seleccionada:
            self.md_bg_color = BORDO
            self.line_color = DORADO_APAGADO
        else:
            self.md_bg_color = OSCURO
            self.line_color = BORDO_OSCURO

    def _on_click(self, _instance):
        if self.on_select_cb:
            self.on_select_cb(self.indice)