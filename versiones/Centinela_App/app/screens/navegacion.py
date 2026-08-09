from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel


class NavegacionScreen(MDBoxLayout):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = 20
        self.spacing = 16

        self.add_widget(MDLabel(
            text="Pantalla GPS",
            halign="center",
            font_style="H4",
        ))
        self.add_widget(MDLabel(
            text="Aquí va la interfaz de navegación.",
            halign="center",
        ))
