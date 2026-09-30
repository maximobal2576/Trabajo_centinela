# -*- coding: utf-8 -*-
"""
screens/inicio.py - Pantalla Principal y de Bienvenida de Centinela.

Objetivo técnico y expositivo:
1. Brindar al usuario invidente o con baja visión una referencia inmediata del estado del bastón:
   - Conexión activa (Verde) o buscando/desconectado (Dorado/Rojo).
   - Nivel de batería Li-Ion en tiempo real con colores semánticos.
2. Permitir la conexión o reconexión manual mediante un botón de alto contraste.
3. Iniciar automáticamente la búsqueda en segundo plano (daemon thread) sin bloquear la interfaz de usuario.
"""

from kivy.clock import Clock
from kivy.metrics import dp
from kivy.uix.image import Image
from kivy.uix.widget import Widget
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDRaisedButton
from kivymd.uix.label import MDLabel

from services.bluetooth import get_bluetooth_service
from components.estilos_centinela import (
    CREMA_CLARO,
    BORDO,
    BORDO_OSCURO,
    DORADO_APAGADO,
    VERDE_ESTADO,
    ROJO_EMERGENCIA,
)


class InicioScreen(MDBoxLayout):
    """Pantalla inicial: estado general del bastón inteligente y botón de enlace."""

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.orientation = "vertical"
        self.padding = [dp(20), dp(100), dp(20), dp(24)]
        self.spacing = dp(12)

        # Servicio Bluetooth compartido (Singleton)
        self.bluetooth_service = get_bluetooth_service()

        # =====================================================================
        # 1. INDICADOR PRINCIPAL DE ESTADO (Texto superior de alto contraste)
        # =====================================================================
        self.estado_baston = MDLabel(
            text="Buscando bastón...",
            halign="center",
            font_style="H5",
            bold=True,
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=dp(44),
        )
        self.add_widget(self.estado_baston)

        # Espaciador vertical flexible
        self.add_widget(Widget(size_hint_y=1))

        # =====================================================================
        # 2. ILUSTRACIÓN ACCESIBLE DEL BASTÓN CENTINELA
        # =====================================================================
        self.ilustracion = Image(
            source="assets/images/baston_centinela.png",
            size_hint_y=None,
            height=dp(210),
            allow_stretch=True,
            keep_ratio=True,
        )
        self.add_widget(self.ilustracion)

        # =====================================================================
        # 3. INDICADOR DE TELEMETRÍA DE BATERÍA
        # =====================================================================
        self.bateria_label = MDLabel(
            text="Batería: -- %",
            halign="center",
            font_style="H5",
            bold=True,
            theme_text_color="Custom",
            text_color=VERDE_ESTADO,
            size_hint_y=None,
            height=dp(48),
        )
        self.add_widget(self.bateria_label)

        # =====================================================================
        # 4. BOTÓN MANUAL DE CONEXIÓN / DESCONEXIÓN
        # =====================================================================
        self.boton_conectar = MDRaisedButton(
            text="CONECTAR BASTÓN",
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=dp(52),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self.alternar_conexion,
        )
        self.add_widget(self.boton_conectar)

        # =====================================================================
        # 5. BOTÓN PROTAGÓNICO SOS DE EMERGENCIA DIRECTO
        # =====================================================================
        self.boton_sos = MDRaisedButton(
            text="🆘 EMERGENCIA SOS",
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=dp(56),
            md_bg_color=ROJO_EMERGENCIA,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self._activar_emergencia_directa,
        )
        self.add_widget(self.boton_sos)

        # Refresco continuo a 2 Hz (0.5 s)
        Clock.schedule_interval(self.actualizar_estado, 0.5)

        # Lanzar la auto-conexión Bluetooth en background al iniciar la app
        Clock.schedule_once(self._iniciar_auto_conexion, 0.5)

    def _activar_emergencia_directa(self, _instance=None):
        """Permite al usuario ciego disparar auxilio inmediato desde la pantalla principal."""
        from services.voz import VozService
        VozService.hablar("Activando auxilio de emergencia.")
        from kivymd.app import MDApp
        app = MDApp.get_running_app()
        if app and hasattr(app, "switch_screen"):
            app.switch_screen("emergencia")

    def _iniciar_auto_conexion(self, _dt):
        """Lanza el hilo de búsqueda automática del bastón ESP32 en segundo plano."""
        self.bluetooth_service.conectar_automaticamente()

    def actualizar_estado(self, _dt):
        """Actualiza el texto, colores semánticos y porcentaje de batería en pantalla."""
        if self.bluetooth_service.connected:
            self.estado_baston.text = "BASTÓN CONECTADO"
            self.estado_baston.text_color = VERDE_ESTADO
            self.boton_conectar.text = "DESCONECTAR BASTÓN"
            self.boton_conectar.md_bg_color = BORDO_OSCURO

            # Nivel de batería Li-Ion recibido por telemetría
            bateria_val = self.bluetooth_service.state.get("BATERIA", "--")
            bateria_str = str(bateria_val).strip()
            if bateria_str.isdigit():
                nivel = int(bateria_str)
                self.bateria_label.text = f"Batería: {nivel}%"
                if nivel > 60:
                    self.bateria_label.text_color = VERDE_ESTADO
                elif nivel > 20:
                    self.bateria_label.text_color = DORADO_APAGADO
                else:
                    self.bateria_label.text_color = ROJO_EMERGENCIA
            else:
                self.bateria_label.text = "Batería: -- %"
                self.bateria_label.text_color = DORADO_APAGADO
        else:
            # Estado buscando o desconectado
            if self.bluetooth_service.last_error and "Buscando" in self.bluetooth_service.last_error:
                self.estado_baston.text = "Buscando bastón..."
                self.estado_baston.text_color = DORADO_APAGADO
            else:
                self.estado_baston.text = "BASTÓN DESCONECTADO"
                self.estado_baston.text_color = ROJO_EMERGENCIA

            self.bateria_label.text = "Batería: -- %"
            self.bateria_label.text_color = DORADO_APAGADO
            self.boton_conectar.text = "CONECTAR BASTÓN"
            self.boton_conectar.md_bg_color = BORDO

    def alternar_conexion(self, _instance=None):
        """Permite al usuario forzar la conexión o desconexión manual del bastón."""
        from services.voz import VozService
        if self.bluetooth_service.connected:
            VozService.hablar("Desconectando el bastón.")
            self.bluetooth_service.detener_auto()
            self.bluetooth_service.disconnect()
            self.actualizar_estado(None)
        else:
            VozService.hablar("Buscando y conectando al bastón.")
            self.estado_baston.text = "Conectando al bastón..."
            self.estado_baston.text_color = DORADO_APAGADO
            self.bluetooth_service.detener_auto()
            self.bluetooth_service.conectar_automaticamente()