from kivy.clock import Clock
from kivy.utils import platform
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.label import MDLabel
from kivymd.uix.button import MDRaisedButton

from services.gps import UbicacionService


class NavegacionScreen(MDBoxLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = 20
        self.spacing = 16

        # =========================
        # SERVICIO GPS
        # =========================
        self.gps = UbicacionService()

        # =========================
        # TÍTULO
        # =========================
        self.titulo = MDLabel(
            text="Navegación",
            halign="center",
            font_style="H4",
            bold=True,
            size_hint_y=None,
            height=60
        )
        self.add_widget(self.titulo)

        # =========================
        # ESTADO GPS
        # =========================
        self.estado_gps = MDLabel(
            text="GPS: Sin iniciar",
            halign="center",
            bold=True
        )
        self.add_widget(self.estado_gps)

        # =========================
        # LATITUD
        # =========================
        self.latitud = MDLabel(
            text="Latitud: --",
            halign="center",
            bold=True
        )
        self.add_widget(self.latitud)

        # =========================
        # LONGITUD
        # =========================
        self.longitud = MDLabel(
            text="Longitud: --",
            halign="center",
            bold=True
        )
        self.add_widget(self.longitud)

        # =========================
        # BOTÓN INICIAR
        # =========================
        self.boton_iniciar = MDRaisedButton(
            text="Iniciar ubicación",
            pos_hint={"center_x": 0.5},
            md_bg_color=(0.0, 0.6, 0.2, 1)
        )

        self.boton_iniciar.bind(
            on_release=self.iniciar_ubicacion
        )

        self.add_widget(self.boton_iniciar)

        # =========================
        # BOTÓN DETENER
        # =========================
        self.boton_detener = MDRaisedButton(
            text="Detener ubicación",
            pos_hint={"center_x": 0.5}
        )

        self.boton_detener.bind(
            on_release=self.detener_ubicacion
        )

        self.add_widget(self.boton_detener)

        # Actualización
        Clock.schedule_interval(
            self.actualizar_gps,
            1
        )

    # ==================================================
    # INICIAR GPS
    # ==================================================

    def iniciar_ubicacion(self, instance):

        try:
            self.gps.iniciar()

            self.estado_gps.text = "GPS: Buscando ubicación..."

        except Exception as e:

            self.estado_gps.text = (
                f"GPS: Error - {e}"
            )

    # ==================================================
    # DETENER GPS
    # ==================================================

    def detener_ubicacion(self, instance):

        try:
            self.gps.detener()

            self.estado_gps.text = (
                "GPS: Detenido"
            )

        except Exception:

            self.estado_gps.text = (
                "GPS: Detenido"
            )

    # ==================================================
    # ACTUALIZAR DATOS
    # ==================================================

    def actualizar_gps(self, dt):

        try:

            datos = self.gps.obtener_ubicacion()

            if not datos:
                return

            lat = datos.get("latitud")
            lon = datos.get("longitud")

            if lat is not None:
                self.latitud.text = (
                    f"Latitud: {lat:.6f}"
                )

            if lon is not None:
                self.longitud.text = (
                    f"Longitud: {lon:.6f}"
                )

            if lat is not None and lon is not None:

                self.estado_gps.text = (
                    "GPS: Ubicación disponible"
                )

        except Exception:
            pass