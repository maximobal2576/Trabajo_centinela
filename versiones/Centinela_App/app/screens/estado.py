# -*- coding: utf-8 -*-
"""
screens/estado.py - Pantalla de telemetría y diagnóstico en tiempo real del bastón.

Muestra los datos enviados por el ESP32 a través del protocolo Bluetooth SPP:
- Estado de conexión con el bastón (Conectado / Desconectado).
- Porcentaje de batería del bastón inteligente.
- Estado del sistema (Inicio, Normal, Alerta, Pozo, Caída, Error).
- Evento sensorial inmediato (Obstáculo frontal, Obstáculo radar, Pozo, etc.).
- Orientación de la brújula y dirección del bastón.
"""

from kivy.clock import Clock
from kivy.metrics import dp
from kivy.uix.widget import Widget
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel

from services.bluetooth import get_bluetooth_service
from widgets.tarjeta_metrica import TarjetaMetrica

from components.estilos_centinela import (
    CREMA_CLARO,
    DORADO_APAGADO,
    VERDE_ESTADO,
    ROJO_EMERGENCIA,
)


class EstadoScreen(MDBoxLayout):
    """Muestra la telemetría en tiempo real recibida desde el bastón inteligente."""

    # Tablas de decodificación del firmware del ESP32
    ESTADOS = {
        "0": "Inicio",
        "1": "Normal",
        "2": "Alerta",
        "3": "Pozo",
        "4": "Caída",
        "5": "Diagnóstico",
        "6": "Error",
    }

    EVENTOS = {
        "0": "Normal",
        "1": "Obstáculo frontal",
        "2": "Obstáculo de radar",
        "3": "Pozo",
        "4": "Caída",
        "5": "Batería baja",
        "6": "Error",
    }

    DIRECCIONES = {
        "0": "Ninguna",
        "1": "Izquierda",
        "2": "Diagonal izquierda",
        "3": "Centro",
        "4": "Diagonal derecha",
        "5": "Derecha",
        "6": "Frontal",
        "7": "Suelo",
    }

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.orientation = "vertical"
        self.padding = [dp(20), dp(100), dp(20), dp(24)]
        self.spacing = dp(10)

        # Servicio Bluetooth compartido
        self.bluetooth_service = get_bluetooth_service()

        # =========================
        # TÍTULO DE PANTALLA
        # =========================
        self.titulo = MDLabel(
            text="ESTADO DEL BASTÓN",
            halign="center",
            font_style="H4",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(56),
        )
        self.add_widget(self.titulo)

        # Espaciador vertical para distribuir el contenido
        self.add_widget(Widget(size_hint_y=1))

        # =========================
        # MÉTRICAS Y TELEMETRÍA (TARJETAS MODULARES)
        # =========================
        self.card_conexion = TarjetaMetrica(
            titulo="Conexión",
            valor_inicial="Desconectado",
            color_valor=ROJO_EMERGENCIA,
        )
        self.add_widget(self.card_conexion)

        self.card_bateria = TarjetaMetrica(
            titulo="Batería",
            valor_inicial="--%",
            color_valor=DORADO_APAGADO,
        )
        self.add_widget(self.card_bateria)

        self.card_sistema = TarjetaMetrica(
            titulo="Sistema",
            valor_inicial="Sin datos",
            color_valor=CREMA_CLARO,
        )
        self.add_widget(self.card_sistema)

        self.card_evento = TarjetaMetrica(
            titulo="Evento",
            valor_inicial="Sin datos",
            color_valor=CREMA_CLARO,
        )
        self.add_widget(self.card_evento)

        self.card_direccion = TarjetaMetrica(
            titulo="Dirección",
            valor_inicial="Sin datos",
            color_valor=CREMA_CLARO,
        )
        self.add_widget(self.card_direccion)

        # Alias retrocompatibles de propiedades por si main.py u otra pantalla los lee
        self.conexion = self.card_conexion.lbl_valor
        self.bateria = self.card_bateria.lbl_valor
        self.sistema = self.card_sistema.lbl_valor
        self.evento = self.card_evento.lbl_valor
        self.direccion = self.card_direccion.lbl_valor

        # Reloj de refresco continuo de telemetría a 2 Hz (0.5 s)
        Clock.schedule_interval(self.actualizar_estado, 0.5)

    def actualizar_estado(self, dt):
        """Lee el estado del servicio Bluetooth y refresca los indicadores visuales."""
        self.bluetooth_service.update(dt)

        # 1. Estado de Conexión
        if self.bluetooth_service.connected:
            self.card_conexion.actualizar("Conectado", color=VERDE_ESTADO)
        else:
            self.card_conexion.actualizar("Desconectado", color=ROJO_EMERGENCIA)

        # 2. Lectura de variables del firmware
        estado_raw = self.bluetooth_service.state.get("ESTADO")
        evento_raw = self.bluetooth_service.state.get("EVENTO")
        direccion_raw = self.bluetooth_service.state.get("DIRECCION")
        bateria_raw = self.bluetooth_service.state.get("BATERIA", "--")

        # 3. Formateo y asignación de telemetría
        bateria_str = str(bateria_raw).strip()
        if bateria_str.isdigit():
            nivel = int(bateria_str)
            color_bat = VERDE_ESTADO if nivel > 60 else (DORADO_APAGADO if nivel > 20 else ROJO_EMERGENCIA)
            self.card_bateria.actualizar(f"{nivel}%", color=color_bat)
        else:
            self.card_bateria.actualizar(f"{bateria_raw}%", color=DORADO_APAGADO)

        # 4. Decodificación de estados del sistema y eventos
        nom_estado = self.ESTADOS.get(str(estado_raw), "Sin datos")
        color_est = ROJO_EMERGENCIA if nom_estado in ("Alerta", "Caída", "Pozo", "Error") else CREMA_CLARO
        self.card_sistema.actualizar(nom_estado, color=color_est)

        nom_evento = self.ESTADOS.get(str(evento_raw), self.EVENTOS.get(str(evento_raw), str(evento_raw or "Sin datos")))
        self.card_evento.actualizar(nom_evento, color=CREMA_CLARO)

        nom_direccion = self.DIRECCIONES.get(str(direccion_raw), str(direccion_raw or "Sin datos"))
        self.card_direccion.actualizar(nom_direccion, color=CREMA_CLARO)