from kivy.metrics import dp, sp
from kivy.uix.floatlayout import FloatLayout
from kivymd.uix.button import MDIconButton
from kivymd.uix.card import MDCard
from kivymd.uix.label import MDLabel

from components.estilos_centinela import (
    BORDO,
    OSCURO,
    CREMA_CLARO,
    DORADO_APAGADO,
    VERDE_ESTADO,
    ROJO_EMERGENCIA,
)


class TopBar(FloatLayout):
    """Componente flotante superior para control por voz e indicador de bateria."""

    def __init__(self, on_mic_press=None, **kwargs):
        super().__init__(**kwargs)
        self.size_hint = (1, None)
        self.height = dp(60)
        self.pos_hint = {"top": 1, "x": 0}

        # Boton de Microfono Global
        self.btn_mic = MDIconButton(
            icon="microphone",
            md_bg_color=BORDO,
            theme_icon_color="Custom",
            icon_color=CREMA_CLARO,
            pos_hint={"right": 0.97, "top": 0.98},
            size_hint=(None, None),
            size=(dp(48), dp(48)),
            icon_size=sp(26),
            on_release=on_mic_press,
        )
        self.add_widget(self.btn_mic)

        # Badge de Bateria Global
        self.badge_bateria = MDCard(
            orientation="horizontal",
            size_hint=(None, None),
            size=(dp(84), dp(34)),
            pos_hint={"center_x": 0.5, "top": 0.975},
            padding=[dp(6), dp(2), dp(8), dp(2)],
            spacing=dp(4),
            md_bg_color=(*OSCURO[:3], 0.88),
            line_color=DORADO_APAGADO,
            line_width=dp(1.2),
            radius=[dp(17)],
            elevation=3,
        )

        self.icono_bateria = MDIconButton(
            icon="battery-bluetooth",
            theme_icon_color="Custom",
            icon_color=VERDE_ESTADO,
            icon_size=sp(16),
            size_hint=(None, 1),
            width=dp(22),
            ripple_scale=0,
        )

        self.lbl_bateria = MDLabel(
            text="--%",
            halign="center",
            font_style="Caption",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint=(1, 1),
            font_size=sp(11),
        )

        self.badge_bateria.add_widget(self.icono_bateria)
        self.badge_bateria.add_widget(self.lbl_bateria)
        self.add_widget(self.badge_bateria)

    def actualizar_bateria(self, conectado, bateria_valor):
        """Actualiza el aspecto del indicador de bateria segun la telemetria del baston."""
        if not conectado:
            self.lbl_bateria.text = "OFF"
            self.lbl_bateria.text_color = ROJO_EMERGENCIA
            self.icono_bateria.icon = "bluetooth-off"
            self.icono_bateria.icon_color = ROJO_EMERGENCIA
            return

        bateria_str = str(bateria_valor).strip()
        if bateria_str.isdigit():
            nivel = int(bateria_str)
            self.lbl_bateria.text = f"{nivel}%"
            if nivel > 60:
                self.icono_bateria.icon = "battery-high"
                self.icono_bateria.icon_color = VERDE_ESTADO
                self.lbl_bateria.text_color = CREMA_CLARO
            elif nivel > 20:
                self.icono_bateria.icon = "battery-medium"
                self.icono_bateria.icon_color = DORADO_APAGADO
                self.lbl_bateria.text_color = CREMA_CLARO
            else:
                self.icono_bateria.icon = "battery-alert"
                self.icono_bateria.icon_color = ROJO_EMERGENCIA
                self.lbl_bateria.text_color = ROJO_EMERGENCIA
        else:
            self.lbl_bateria.text = "--%"
            self.icono_bateria.icon = "battery-bluetooth"
            self.icono_bateria.icon_color = DORADO_APAGADO
            self.lbl_bateria.text_color = CREMA_CLARO