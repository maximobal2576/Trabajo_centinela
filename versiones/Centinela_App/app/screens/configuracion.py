from kivymd.uix.boxlayout import MDBoxLayout
from kivy.metrics import dp
from kivymd.uix.label import MDLabel
from kivymd.uix.button import MDRaisedButton
from kivymd.uix.textfield import MDTextField

from services.almacenamiento import AlmacenamientoService

from components.estilos_centinela import (
    CREMA_CLARO,
    BORDO,
    DORADO_APAGADO,
)


class ConfiguracionScreen(MDBoxLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = [dp(20), dp(5), dp(20), dp(88)]
        self.spacing = dp(12)

        # ==========================================
        # TÍTULO
        # ==========================================

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

        # ==========================================
        # CONTACTO
        # ==========================================

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

        # ==========================================
        # CONFIGURACIÓN
        # ==========================================

        configuracion = (
            AlmacenamientoService.cargar_configuracion()
        )

        self.contacto_input = MDTextField(
            text=configuracion[
                "contacto_emergencia"
            ],
            hint_text="Número con código de país",
            input_filter="int",
            mode="rectangle",
            size_hint_y=None,
            height=dp(52),
        )

        self.add_widget(self.contacto_input)

        # ==========================================
        # GUARDAR
        # ==========================================

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

        # ==========================================
        # MENSAJE
        # ==========================================

        self.mensaje = MDLabel(
            text="",
            halign="center",
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=dp(48),
        )

        self.add_widget(self.mensaje)

    # ==========================================
    # GUARDAR CONTACTO
    # ==========================================

    def guardar_contacto(self, instance):

        numero = self.contacto_input.text.strip()

        if not numero:

            self.mensaje.text = (
                "Ingresá un número de contacto."
            )

            return

        AlmacenamientoService.guardar_contacto_emergencia(
            numero
        )

        self.mensaje.text = (
            "Contacto de emergencia guardado."
        )
