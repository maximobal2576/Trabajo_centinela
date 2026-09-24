# -*- coding: utf-8 -*-
"""
widgets/dialogo_ruta.py - Tarjeta de diálogo interactiva y accesible para guardar rutas.

Permite al usuario confirmar si desea guardar en memoria local una ruta recién calculada
(para volver a usarla en el futuro) o únicamente navegarla en el momento sin guardarla.
Diseñado con alto contraste y soporte de lectura TalkBack.
"""

from kivy.metrics import dp, sp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDRaisedButton
from kivymd.uix.card import MDCard
from kivymd.uix.label import MDLabel
from kivymd.uix.textfield import MDTextField

from components.estilos_centinela import (
    BORDO,
    BORDO_OSCURO,
    CREMA_CLARO,
    DORADO_APAGADO,
    OSCURO,
)


class DialogoGuardarRuta(MDCard):
    """Tarjeta flotante accesible que pregunta al usuario si desea guardar la ruta."""

    def __init__(self, al_confirmar_guardar=None, al_confirmar_solo_navegar=None, **kwargs):
        super().__init__(**kwargs)
        self.orientation = "vertical"
        self.padding = dp(14)
        self.spacing = dp(10)
        self.size_hint = (1, None)
        self.height = dp(200)
        self.md_bg_color = OSCURO
        self.line_color = DORADO_APAGADO
        self.line_width = dp(1.2)
        self.elevation = 4

        self.al_confirmar_guardar = al_confirmar_guardar
        self.al_confirmar_solo_navegar = al_confirmar_solo_navegar

        # Titulo accesible
        self.lbl_titulo = MDLabel(
            text="¿Desea guardar esta ruta?",
            halign="center",
            bold=True,
            font_style="Subtitle1",
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=dp(28),
        )
        self.add_widget(self.lbl_titulo)

        # Descripcion del destino
        self.lbl_desc = MDLabel(
            text="Destino: --",
            halign="center",
            font_style="Caption",
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(24),
        )
        self.add_widget(self.lbl_desc)

        # Campo para ingresar o modificar el nombre de la ruta
        self.input_nombre = MDTextField(
            hint_text="Nombre de la ruta",
            mode="rectangle",
            size_hint_y=None,
            height=dp(44),
        )
        self.add_widget(self.input_nombre)

        # Fila de botones de accion
        fila_botones = MDBoxLayout(
            orientation="horizontal",
            size_hint_y=None,
            height=dp(42),
            spacing=dp(8),
        )

        self.btn_si = MDRaisedButton(
            text="SÍ (GUARDAR)",
            size_hint=(0.5, 1),
            font_size=sp(12),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self._on_click_guardar,
        )
        self.btn_no = MDRaisedButton(
            text="NO (SOLO NAVEGAR)",
            size_hint=(0.5, 1),
            font_size=sp(12),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self._on_click_solo_navegar,
        )

        fila_botones.add_widget(self.btn_si)
        fila_botones.add_widget(self.btn_no)
        self.add_widget(fila_botones)

        # Estado inicial oculto
        self.ocultar()

    def mostrar(self, destino_texto, nombre_sugerido=""):
        """Hace visible el cuadro de diálogo y enfoca los datos."""
        self.lbl_desc.text = f"Destino: {destino_texto}"
        self.input_nombre.text = nombre_sugerido or destino_texto
        self.opacity = 1
        self.disabled = False
        self.height = dp(200)

    def ocultar(self):
        """Oculta y desactiva el cuadro de diálogo."""
        self.opacity = 0
        self.disabled = True
        self.height = 0

    def _on_click_guardar(self, instance):
        nombre = self.input_nombre.text.strip()
        self.ocultar()
        if self.al_confirmar_guardar:
            self.al_confirmar_guardar(nombre)

    def _on_click_solo_navegar(self, instance):
        self.ocultar()
        if self.al_confirmar_solo_navegar:
            self.al_confirmar_solo_navegar()