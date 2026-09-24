# -*- coding: utf-8 -*-
"""
widgets/panel_sos.py - Panel de botones y cuenta regresiva de emergencia para Centinela.

Diseñado con accesibilidad visual de alto contraste y botones de gran tamaño (WCAG AAA):
- Botón 911 en Rojo Emergencia (#A6292E).
- Botón Contacto en Bordó Institucional (#6E1F2B).
- Botón Ubicación en Bordó Oscuro (#45121B).
- Botón Cancelar visible únicamente durante la cuenta regresiva.
"""

from kivy.metrics import dp, sp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDRaisedButton
from kivymd.uix.label import MDLabel

from components.estilos_centinela import (
    BORDO,
    BORDO_OSCURO,
    CREMA_CLARO,
    DORADO_APAGADO,
    OSCURO,
    ROJO_EMERGENCIA,
)


class PanelBotonesSOS(MDBoxLayout):
    """Contenedor de acciones rápidas de emergencia y botón de cancelación."""

    def __init__(self, on_call_911=None, on_call_contacto=None, on_send_gps=None, on_cancelar=None, **kwargs):
        super().__init__(**kwargs)
        self.orientation = "vertical"
        self.spacing = dp(12)
        self.size_hint_y = None
        self.bind(minimum_height=self.setter("height"))

        # Botón Llamar al 911 (Prioridad máxima)
        self.btn_911 = MDRaisedButton(
            text="LLAMAR 911",
            md_bg_color=ROJO_EMERGENCIA,
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=dp(52),
            on_release=on_call_911,
        )
        self.add_widget(self.btn_911)

        # Botón Contacto Personal de Emergencia
        self.btn_contacto = MDRaisedButton(
            text="LLAMAR CONTACTO DE EMERGENCIA",
            md_bg_color=BORDO,
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=dp(52),
            on_release=on_call_contacto,
        )
        self.add_widget(self.btn_contacto)

        # Botón Enviar SMS con Coordenadas GPS
        self.btn_ubicacion = MDRaisedButton(
            text="ENVIAR UBICACIÓN AL CONTACTO",
            md_bg_color=BORDO_OSCURO,
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=dp(52),
            on_release=on_send_gps,
        )
        self.add_widget(self.btn_ubicacion)

        # Botón Cancelar (oculto por defecto hasta que se activa la cuenta regresiva)
        self.btn_cancelar = MDRaisedButton(
            text="CANCELAR",
            md_bg_color=OSCURO,
            pos_hint={"center_x": 0.5},
            size_hint=(0.70, None),
            height=dp(52),
            disabled=True,
            opacity=0,
            on_release=on_cancelar,
        )
        self.add_widget(self.btn_cancelar)

    def activar_modo_cuenta_regresiva(self):
        """Bloquea los botones de llamada y muestra el botón de cancelación."""
        self.btn_cancelar.disabled = False
        self.btn_cancelar.opacity = 1
        self.btn_911.disabled = True
        self.btn_contacto.disabled = True
        self.btn_ubicacion.disabled = True

    def restablecer_botones(self):
        """Reactiva los botones primarios y oculta la opción de cancelar."""
        self.btn_cancelar.disabled = True
        self.btn_cancelar.opacity = 0
        self.btn_911.disabled = False
        self.btn_contacto.disabled = False
        self.btn_ubicacion.disabled = False