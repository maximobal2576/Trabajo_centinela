# -*- coding: utf-8 -*-
"""
widgets/tarjeta_metrica.py - Fila o tarjeta de métrica de telemetría para Centinela.

Muestra de forma clara y accesible (WCAG AAA) un indicador del bastón:
- Etiqueta del parámetro (Conexión, Batería, Sistema, Evento, Dirección).
- Valor actual formateado con color semántico según su estado:
  - Verde: óptimo / conectado.
  - Dorado: normal / informativo.
  - Rojo: desconectado / alerta crítica.
"""

from kivy.metrics import dp, sp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.card import MDCard
from kivymd.uix.label import MDLabel

from components.estilos_centinela import (
    BORDO_OSCURO,
    CREMA_CLARO,
    DORADO_APAGADO,
    OSCURO,
    VERDE_ESTADO,
    ROJO_EMERGENCIA,
)


class TarjetaMetrica(MDCard):
    """Tarjeta horizontal compacta para telemetría de sensores del bastón."""

    def __init__(self, titulo: str, valor_inicial: str = "---", color_valor=CREMA_CLARO, **kwargs):
        super().__init__(**kwargs)
        self.orientation = "horizontal"
        self.size_hint = (1, None)
        self.height = dp(46)
        self.padding = [dp(14), dp(4), dp(14), dp(4)]
        self.spacing = dp(8)
        self.md_bg_color = OSCURO
        self.line_color = BORDO_OSCURO
        self.line_width = dp(1)
        self.elevation = 2

        # Etiqueta / Nombre de la métrica
        self.lbl_titulo = MDLabel(
            text=titulo,
            bold=True,
            font_style="Subtitle2",
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint=(0.42, 1),
            valign="middle",
        )
        self.add_widget(self.lbl_titulo)

        # Valor medido en tiempo real
        self.lbl_valor = MDLabel(
            text=valor_inicial,
            bold=True,
            font_style="Subtitle1",
            theme_text_color="Custom",
            text_color=color_valor,
            size_hint=(0.58, 1),
            halign="right",
            valign="middle",
        )
        self.add_widget(self.lbl_valor)

    def actualizar(self, texto_valor: str, color=None):
        """Actualiza el texto y opcionalmente el color del indicador."""
        self.lbl_valor.text = texto_valor
        if color is not None:
            self.lbl_valor.text_color = color