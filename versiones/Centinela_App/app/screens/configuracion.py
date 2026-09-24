# -*- coding: utf-8 -*-
"""
screens/configuracion.py - Pantalla de Ajustes y Contacto de Emergencia.

Objetivo técnico y expositivo:
1. Permite al usuario registrar o actualizar el número telefónico de su contacto de emergencia.
2. Almacena la preferencia de forma persistente en SQLite/JSON mediante `AlmacenamientoService`.
3. Este contacto es el destinatario de:
   - Las llamadas rápidas de la pantalla de emergencia.
   - Las alertas automáticas por SMS con coordenadas GPS de Google Maps ante caídas o pulsador SOS.
"""

from kivy.metrics import dp
from kivy.uix.widget import Widget
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDRaisedButton
from kivymd.uix.label import MDLabel
from kivymd.uix.textfield import MDTextField

from services.almacenamiento import AlmacenamientoService
from components.estilos_centinela import (
    CREMA_CLARO,
    BORDO,
    DORADO_APAGADO,
    VERDE_ESTADO,
    ROJO_EMERGENCIA,
)


class ConfiguracionScreen(MDBoxLayout):
    """Permite configurar y persistir el contacto de auxilio para emergencias."""

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.orientation = "vertical"
        self.padding = [dp(20), dp(100), dp(20), dp(24)]
        self.spacing = dp(12)

        # =====================================================================
        # 1. TÍTULO DE PANTALLA
        # =====================================================================
        self.titulo = MDLabel(
            text="CONFIGURACIÓN",
            halign="center",
            font_style="H4",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(56),
        )
        self.add_widget(self.titulo)

        # Espaciador vertical flexible
        self.add_widget(Widget(size_hint_y=1))

        # =====================================================================
        # 2. SUBTÍTULO INFORMATIVO
        # =====================================================================
        self.subtitulo = MDLabel(
            text="CONTACTO DE EMERGENCIA",
            halign="center",
            font_style="H5",
            bold=True,
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=dp(48),
        )
        self.add_widget(self.subtitulo)

        # =====================================================================
        # 3. CAMPO DE ENTRADA DEL CONTACTO
        # =====================================================================
        configuracion = AlmacenamientoService.cargar_configuracion()
        numero_actual = configuracion.get("contacto_emergencia", "")

        self.contacto_input = MDTextField(
            text=numero_actual,
            hint_text="Número con código de área (ej. 54911...)",
            input_filter="int",
            mode="rectangle",
            size_hint_y=None,
            height=dp(52),
        )
        self.add_widget(self.contacto_input)

        # =====================================================================
        # 4. BOTÓN PARA GUARDAR CONTACTO
        # =====================================================================
        self.boton_guardar = MDRaisedButton(
            text="GUARDAR CONTACTO",
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=dp(52),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self.guardar_contacto,
        )
        self.add_widget(self.boton_guardar)

        # =====================================================================
        # 5. MENSAJE DE CONFIRMACIÓN O ERROR
        # =====================================================================
        self.mensaje = MDLabel(
            text="",
            halign="center",
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=dp(36),
        )
        self.add_widget(self.mensaje)

    def guardar_contacto(self, _instance=None):
        """Valida y guarda el número en el almacenamiento persistente del dispositivo."""
        numero = self.contacto_input.text.strip()
        if not numero:
            self.mensaje.text = "Ingresá un número válido."
            self.mensaje.text_color = ROJO_EMERGENCIA
            return

        AlmacenamientoService.guardar_contacto_emergencia(numero)

        self.mensaje.text = "Contacto guardado correctamente."
        self.mensaje.text_color = VERDE_ESTADO