from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel
from kivymd.uix.button import MDRaisedButton

from components.estilos_centinela import (
    CREMA_CLARO,
    BORDO,
    BORDO_OSCURO,
    DORADO_APAGADO,
)


class RutasScreen(MDBoxLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = [30, 5, 70, 90]
        self.spacing = 15

        # ==========================================
        # TÍTULO
        # ==========================================

        self.titulo = MDLabel(
            text="RUTAS",
            halign="center",
            font_style="H3",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=70,
        )

        self.add_widget(self.titulo)

        # ==========================================
        # ESTADO
        # ==========================================

        self.estado = MDLabel(
            text="Rutas guardadas: 0",
            halign="center",
            font_style="H5",
            bold=True,
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=45,
        )

        self.add_widget(self.estado)

        # ==========================================
        # GUARDAR RUTA
        # ==========================================

        self.boton_guardar = MDRaisedButton(
            text="GUARDAR RUTA",
            size_hint=(0.85, None),
            height=55,
            pos_hint={"center_x": 0.5},
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )

        self.boton_guardar.bind(
            on_release=self.guardar_ruta
        )

        self.add_widget(self.boton_guardar)

        # ==========================================
        # MIS RUTAS
        # ==========================================

        self.boton_mis_rutas = MDRaisedButton(
            text="MIS RUTAS",
            size_hint=(0.85, None),
            height=55,
            pos_hint={"center_x": 0.5},
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )

        self.boton_mis_rutas.bind(
            on_release=self.mostrar_rutas
        )

        self.add_widget(self.boton_mis_rutas)

        # ==========================================
        # INICIAR RUTA
        # ==========================================

        self.boton_iniciar = MDRaisedButton(
            text="INICIAR RUTA",
            size_hint=(0.85, None),
            height=55,
            pos_hint={"center_x": 0.5},
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )

        self.boton_iniciar.bind(
            on_release=self.iniciar_ruta
        )

        self.add_widget(self.boton_iniciar)

        # ==========================================
        # ELIMINAR RUTA
        # ==========================================

        self.boton_eliminar = MDRaisedButton(
            text="ELIMINAR RUTA",
            size_hint=(0.85, None),
            height=55,
            pos_hint={"center_x": 0.5},
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )

        self.boton_eliminar.bind(
            on_release=self.eliminar_ruta
        )

        self.add_widget(self.boton_eliminar)

    # ==========================================
    # FUNCIONES
    # ==========================================

    def guardar_ruta(self, instance):
        self.estado.text = "Guardar ruta"

    def mostrar_rutas(self, instance):
        self.estado.text = "No hay rutas guardadas"

    def iniciar_ruta(self, instance):
        self.estado.text = "Seleccione una ruta"

    def eliminar_ruta(self, instance):
        self.estado.text = "No hay rutas para eliminar"