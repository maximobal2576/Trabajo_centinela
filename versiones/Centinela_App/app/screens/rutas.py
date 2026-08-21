from kivy.clock import Clock
from kivy.metrics import dp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDRaisedButton
from kivymd.uix.label import MDLabel
from kivymd.uix.textfield import MDTextField

from components.estilos_centinela import CREMA_CLARO, BORDO, BORDO_OSCURO, DORADO_APAGADO, VERDE_ESTADO
from services.gps import get_ubicacion_service
from services.rutas import RutasService


class RutasScreen(MDBoxLayout):
    """Graba y retoma recorridos usando exclusivamente almacenamiento local."""

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.orientation = "vertical"
        self.padding = [dp(20), dp(5), dp(20), dp(88)]
        self.spacing = dp(12)
        self.gps = get_ubicacion_service()

        self.titulo = MDLabel(text="RUTAS SIN CONEXIÓN", halign="center", font_style="H4", bold=True,valign="top",
                              theme_text_color="Custom", text_color=CREMA_CLARO, size_hint_y=None, height=dp(56))
        self.estado = MDLabel(text="", halign="center", theme_text_color="Custom", text_color=DORADO_APAGADO,valign="top",
                              size_hint_y=None, height=dp(64))
        self.add_widget(self.titulo)
        self.add_widget(self.estado)

        self.nombre_input = MDTextField(
            hint_text="Nombre del recorrido (ej.: Casa a farmacia)",
            mode="rectangle", size_hint_y=None, height=dp(52),
        )
        self.add_widget(self.nombre_input)

        self.boton_guardar = self._boton("INICIAR GUARDADO", BORDO, self.guardar_ruta)
        self.add_widget(self.boton_guardar)
        self.add_widget(self._boton("VER RECORRIDOS GUARDADOS", BORDO_OSCURO, self.mostrar_rutas))
        self.add_widget(self._boton("RETOMAR ÚLTIMA RUTA", BORDO, self.iniciar_ruta))
        self.add_widget(self._boton("ELIMINAR ÚLTIMA RUTA", BORDO_OSCURO, self.eliminar_ruta))
        Clock.schedule_interval(self._actualizar_grabacion, 1)
        self.mostrar_rutas(None)

    def _boton(self, texto, color, accion):
        return MDRaisedButton(text=texto, size_hint=(0.85, None), height=dp(52),
                              pos_hint={"center_x": 0.5}, md_bg_color=color,
                              theme_text_color="Custom", text_color=CREMA_CLARO, on_release=accion)

    def guardar_ruta(self, _instance):
        if not RutasService.esta_grabando():
            if not self.gps.activo:
                self.estado.text = "Primero iniciá Ubicación en Navegación."
                return
            nombre = self.nombre_input.text.strip()
            if not nombre:
                self.estado.text = "Escribí un nombre para el recorrido."
                return
            RutasService.iniciar_grabacion(nombre)
            ubicacion = self.gps.obtener_ubicacion()
            if ubicacion:
                RutasService.registrar_punto(**ubicacion)
            self.boton_guardar.text = "FINALIZAR Y GUARDAR"
            self.estado.text = "Grabando recorrido. Podés navegar sin Internet."
            self.estado.text_color = VERDE_ESTADO
            return

        ruta = RutasService.finalizar_grabacion()
        self.boton_guardar.text = "INICIAR GUARDADO"
        if ruta is None:
            self.estado.text = "Recorré al menos unos metros antes de guardar."
        else:
            self.estado.text = f"Guardada: {ruta['nombre']} ({len(ruta['puntos'])} puntos)."
            self.estado.text_color = VERDE_ESTADO
            self.nombre_input.text = ""

    def _actualizar_grabacion(self, _dt):
        if not RutasService.esta_grabando():
            return
        ubicacion = self.gps.obtener_ubicacion()
        if ubicacion:
            RutasService.registrar_punto(**ubicacion)

    def mostrar_rutas(self, _instance):
        rutas = RutasService.cargar_rutas()
        if not rutas:
            self.estado.text = "No hay rutas guardadas en este teléfono."
            return
        resumen = [f"{ruta['nombre']} ({len(ruta['puntos'])} puntos)" for ruta in rutas[-4:][::-1]]
        self.estado.text = f"{len(rutas)} recorrido(s) guardado(s):\n" + "\n".join(resumen)

    def iniciar_ruta(self, _instance):
        ruta = RutasService.ultima_ruta()
        ubicacion = self.gps.obtener_ubicacion()
        if ruta is None:
            self.estado.text = "No hay rutas para retomar."
            return
        if ubicacion is None:
            self.estado.text = "Iniciá Ubicación en Navegación para retomar la ruta."
            return
        indicacion = RutasService.iniciar_seguimiento(ruta, ubicacion)
        self.estado.text = indicacion or "No se pudo iniciar el seguimiento de la ruta."
        self.estado.text_color = VERDE_ESTADO

    def eliminar_ruta(self, _instance):
        if RutasService.eliminar_ultima():
            self.estado.text = "Última ruta eliminada."
        else:
            self.estado.text = "No hay rutas para eliminar."
