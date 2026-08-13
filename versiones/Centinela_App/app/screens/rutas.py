from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel
from kivymd.uix.button import MDRaisedButton


class RutasScreen(MDBoxLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = [30, 20, 30, 90]
        self.spacing = 15

        self.titulo = MDLabel(
            text="RUTAS",
            halign="center",
            font_style="H3",
            bold=True,
            theme_text_color="Custom",
            text_color=(0.95, 0.89, 0.75, 1),
            size_hint_y=None,
            height=70,
        )

        self.add_widget(self.titulo)

        self.estado = MDLabel(
            text="Rutas guardadas: 0",
            halign="center",
            bold=True,
            theme_text_color="Custom",
            text_color=(0.95, 0.89, 0.75, 1),
            size_hint_y=None,
            height=40,
        )

        self.add_widget(self.estado)

        self.boton_guardar = MDRaisedButton(
            text="GUARDAR RUTA",
            size_hint=(0.85, None),
            height=55,
            pos_hint={"center_x": 0.5},
            md_bg_color=(0.42, 0.12, 0.18, 1),
        )

        self.boton_guardar.bind(
            on_release=self.guardar_ruta
        )

        self.add_widget(self.boton_guardar)

        self.boton_mis_rutas = MDRaisedButton(
            text="MIS RUTAS",
            size_hint=(0.85, None),
            height=55,
            pos_hint={"center_x": 0.5},
            md_bg_color=(0.09, 0.09, 0.09, 1),
        )

        self.boton_mis_rutas.bind(
            on_release=self.mostrar_rutas
        )

        self.add_widget(self.boton_mis_rutas)

        self.boton_iniciar = MDRaisedButton(
            text="INICIAR RUTA",
            size_hint=(0.85, None),
            height=55,
            pos_hint={"center_x": 0.5},
            md_bg_color=(0.42, 0.12, 0.18, 1),
        )

        self.boton_iniciar.bind(
            on_release=self.iniciar_ruta
        )

        self.add_widget(self.boton_iniciar)

        self.boton_eliminar = MDRaisedButton(
            text="ELIMINAR RUTA",
            size_hint=(0.85, None),
            height=55,
            pos_hint={"center_x": 0.5},
            md_bg_color=(0.09, 0.09, 0.09, 1),
        )

        self.boton_eliminar.bind(
            on_release=self.eliminar_ruta
        )

        self.add_widget(self.boton_eliminar)

    def guardar_ruta(self, instance):
        self.estado.text = "Guardar ruta"

    def mostrar_rutas(self, instance):
        self.estado.text = "No hay rutas guardadas"

    def iniciar_ruta(self, instance):
        self.estado.text = "Seleccione una ruta"

    def eliminar_ruta(self, instance):
        self.estado.text = "No hay rutas para eliminar"