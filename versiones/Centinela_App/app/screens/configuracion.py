from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel
from kivymd.uix.button import MDRaisedButton
from kivymd.uix.textfield import MDTextField

from services.almacenamiento import AlmacenamientoService


class ConfiguracionScreen(MDBoxLayout):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = 20
        self.spacing = 16

        self.add_widget(MDLabel(
            text="Configuración",
            halign="center",
            font_style="H4",
        ))
        self.add_widget(MDLabel(
            text="Contacto de emergencia",
            halign="center",
        ))

        configuracion = AlmacenamientoService.cargar_configuracion()

        self.contacto_input = MDTextField(
            text=configuracion["contacto_emergencia"],
            hint_text="Número con código de país",
            input_filter="int",
            mode="rectangle",
        )
        self.add_widget(self.contacto_input)

        self.boton_guardar = MDRaisedButton(
            text="Guardar contacto",
            pos_hint={"center_x": 0.5},
            on_release=self.guardar_contacto,
        )
        self.add_widget(self.boton_guardar)

        self.mensaje = MDLabel(
            text="",
            halign="center",
        )
        self.add_widget(self.mensaje)

    def guardar_contacto(self, instance):
        numero = self.contacto_input.text.strip()

        if not numero:
            self.mensaje.text = "Ingresá un número de contacto."
            return

        AlmacenamientoService.guardar_contacto_emergencia(numero)
        self.mensaje.text = "Contacto de emergencia guardado."
