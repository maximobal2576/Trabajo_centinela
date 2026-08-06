from kivy.core.text import LabelBase
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel
from kivymd.uix.button import MDRaisedButton


LabelBase.register(
    name="EmojiFont",
    fn_regular=r"C:\Windows\Fonts\seguiemj.ttf"
)


class InicioScreen(MDBoxLayout):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = 20
        self.spacing = 15

        emoji_font = "C:\\Windows\\Fonts\\seguiemj.ttf"

        self.titulo = MDLabel(
            text="Centinela",
            halign="center",
            font_style="H4",
            font_name="EmojiFont"
        )
        self.add_widget(self.titulo)

        self.estado_baston = MDLabel(
            text="Bastón: Desconectado",
            halign="center",

            font_name="EmojiFont"
        )
        self.add_widget(self.estado_baston)

        self.bateria_label = MDLabel(
            text="Batería: --%",
            halign="center",
            font_name="EmojiFont",
            theme_text_color="Custom",
            text_color=(0.0, 0.7, 0.3, 1) 
        )
        self.add_widget(self.bateria_label)

        self.gps = MDLabel(
            text="📍 GPS: Inactivo",
            halign="center",
            font_name="EmojiFont"
        )
        self.add_widget(self.gps)

        self.boton_navegar = MDRaisedButton(
            text="Iniciar Navegación \n 🗺️",
            pos_hint={"center_x": 0.5},
            font_name="EmojiFont"
        
        )
        self.add_widget(self.boton_navegar)
