from datetime import datetime
from kivy.metrics import dp, sp
from kivy.uix.scrollview import ScrollView
from kivymd.app import MDApp
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDIconButton, MDRaisedButton
from kivymd.uix.card import MDCard
from kivymd.uix.label import MDLabel
from kivymd.uix.textfield import MDTextField

from components.estilos_centinela import (
    BORDO,
    BORDO_OSCURO,
    CREMA_CLARO,
    DORADO_APAGADO,
    OSCURO,
    VERDE_ESTADO,
)
from services.gps import get_ubicacion_service
from services.rutas import RutasService
from services.voz import VozService


class RutasScreen(MDBoxLayout):
    """Muestra el historial de rutas guardadas localmente y permite gestionarlas."""

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.orientation = "vertical"
        self.padding = [dp(20), dp(100), dp(20), dp(24)]
        self.spacing = dp(10)

        self.gps = get_ubicacion_service()
        self.voz = VozService()
        self.ruta_seleccionada_idx = None
        self.botones_rutas = []

        # =========================
        # TÍTULO
        # =========================
        self.titulo = MDLabel(
            text="HISTORIAL DE RUTAS",
            halign="center",
            font_style="H4",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(52),
        )
        self.add_widget(self.titulo)

        # =========================
        # ESTADO / INFO
        # =========================
        self.estado = MDLabel(
            text="Seleccioná una ruta para reanudar, renombrar o eliminar.",
            halign="center",
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=dp(36),
            font_size=sp(12),
        )
        self.add_widget(self.estado)

        # =========================
        # LISTA DESLIZABLE DE RUTAS
        # =========================
        self.scroll_historial = ScrollView(
            size_hint=(1, 1),
            do_scroll_x=False,
            do_scroll_y=True,
        )
        self.contenedor_lista = MDBoxLayout(
            orientation="vertical",
            size_hint_y=None,
            spacing=dp(8),
            padding=[0, dp(4), 0, dp(4)],
        )
        self.contenedor_lista.bind(minimum_height=self.contenedor_lista.setter("height"))
        self.scroll_historial.add_widget(self.contenedor_lista)
        self.add_widget(self.scroll_historial)

        # =========================
        # PANEL DE RENOMBRAR (OCULTO POR DEFECTO)
        # =========================
        self.panel_renombrar = MDCard(
            orientation="vertical",
            padding=dp(10),
            spacing=dp(8),
            size_hint=(1, None),
            height=0,
            opacity=0,
            disabled=True,
            md_bg_color=OSCURO,
            line_color=DORADO_APAGADO,
            line_width=dp(1),
        )
        self.input_nuevo_nombre = MDTextField(
            hint_text="Nuevo nombre de la ruta",
            mode="rectangle",
            size_hint_y=None,
            height=dp(40),
        )
        fila_renombrar_btn = MDBoxLayout(
            orientation="horizontal",
            size_hint_y=None,
            height=dp(38),
            spacing=dp(8),
        )
        self.btn_confirmar_nombre = MDRaisedButton(
            text="GUARDAR NOMBRE",
            size_hint=(0.42, 1),
            font_size=sp(11),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self._guardar_renombrado,
        )
        self.btn_cancelar_nombre = MDRaisedButton(
            text="CANCELAR",
            size_hint=(0.38, 1),
            font_size=sp(11),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self._cerrar_panel_renombrar,
        )
        self.btn_mic_nombre = MDIconButton(
            icon="microphone",
            md_bg_color=BORDO,
            theme_icon_color="Custom",
            icon_color=CREMA_CLARO,
            icon_size=sp(20),
            size_hint=(None, 1),
            width=dp(38),
            on_release=self._dictar_nuevo_nombre,
        )
        fila_renombrar_btn.add_widget(self.btn_confirmar_nombre)
        fila_renombrar_btn.add_widget(self.btn_cancelar_nombre)
        fila_renombrar_btn.add_widget(self.btn_mic_nombre)

        self.panel_renombrar.add_widget(self.input_nuevo_nombre)
        self.panel_renombrar.add_widget(fila_renombrar_btn)
        self.add_widget(self.panel_renombrar)

        # =========================
        # BOTONES DE ACCIÓN (HABILITADOS AL SELECCIONAR)
        # =========================
        # =========================
        # BOTONES DE ACCIÓN (HABILITADOS AL SELECCIONAR)
        # =========================
        fila_acciones_1 = MDBoxLayout(
            orientation="horizontal",
            size_hint_y=None,
            height=dp(46),
            spacing=dp(8),
        )
        self.btn_reanudar = MDRaisedButton(
            text="REANUDAR (IDA)",
            size_hint=(0.5, 1),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            disabled=True,
            on_release=self._reanudar_ruta_seleccionada,
        )
        self.btn_invertir = MDRaisedButton(
            text="VOLVER (INVERTIR)",
            size_hint=(0.5, 1),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            disabled=True,
            on_release=self._invertir_y_reanudar_seleccionada,
        )
        fila_acciones_1.add_widget(self.btn_reanudar)
        fila_acciones_1.add_widget(self.btn_invertir)
        self.add_widget(fila_acciones_1)

        fila_acciones_2 = MDBoxLayout(
            orientation="horizontal",
            size_hint_y=None,
            height=dp(44),
            spacing=dp(6),
        )
        self.btn_renombrar = MDRaisedButton(
            text="RENOMBRAR",
            size_hint=(0.33, 1),
            font_size=sp(11),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            disabled=True,
            on_release=self._abrir_panel_renombrar,
        )
        self.btn_eliminar = MDRaisedButton(
            text="ELIMINAR",
            size_hint=(0.33, 1),
            font_size=sp(11),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            disabled=True,
            on_release=self._eliminar_ruta_seleccionada,
        )
        self.btn_detener = MDRaisedButton(
            text="DETENER",
            size_hint=(0.34, 1),
            font_size=sp(11),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            disabled=True,
            on_release=self._detener_seguimiento,
        )
        fila_acciones_2.add_widget(self.btn_renombrar)
        fila_acciones_2.add_widget(self.btn_eliminar)
        fila_acciones_2.add_widget(self.btn_detener)
        self.add_widget(fila_acciones_2)

        # Botón para grabar recorrido manual localmente
        self.btn_grabar_manual = MDRaisedButton(
            text="INICIAR GRABACIÓN LOCAL",
            size_hint=(1, None),
            height=dp(44),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self._alternar_grabacion_manual,
        )
        self.add_widget(self.btn_grabar_manual)

    def on_enter(self, *args):
        """Actualiza la lista al ingresar a la pantalla."""
        self.cargar_historial()
        self._actualizar_boton_grabacion()

    def _alternar_grabacion_manual(self, _instance=None):
        try:
            if not RutasService.esta_grabando():
                if not self.gps.activo:
                    self.gps.iniciar()
                nombre = f"Recorrido {datetime.now().strftime('%d/%m %H:%M')}"
                RutasService.iniciar_grabacion(nombre)
                ubicacion = self.gps.obtener_ubicacion()
                if ubicacion:
                    RutasService.registrar_punto(ubicacion["latitud"], ubicacion["longitud"])
                self.estado.text = f"Grabando «{nombre}» (puntos cada 8 m)."
                self.estado.text_color = VERDE_ESTADO
            else:
                ruta = RutasService.finalizar_grabacion()
                if ruta:
                    self.estado.text = f"Ruta «{ruta['nombre']}» guardada con éxito."
                    self.estado.text_color = VERDE_ESTADO
                else:
                    self.estado.text = "Grabación cancelada (menos de 2 puntos)."
                    self.estado.text_color = DORADO_APAGADO
                self.cargar_historial()
        except Exception as err:
            self.estado.text = f"Error al grabar: {err}"
        self._actualizar_boton_grabacion()

    def _actualizar_boton_grabacion(self):
        if RutasService.esta_grabando():
            self.btn_grabar_manual.text = "FINALIZAR Y GUARDAR GRABACIÓN"
            self.btn_grabar_manual.md_bg_color = BORDO_OSCURO
        else:
            self.btn_grabar_manual.text = "INICIAR GRABACIÓN LOCAL"
            self.btn_grabar_manual.md_bg_color = BORDO

    # ==================================================
    # CARGAR Y MOSTRAR HISTORIAL
    # ==================================================

    def cargar_historial(self):
        self.contenedor_lista.clear_widgets()
        self.botones_rutas = []
        self.ruta_seleccionada_idx = None
        self._deshabilitar_acciones()

        rutas = RutasService.cargar_rutas()
        if not rutas:
            lbl_vacio = MDLabel(
                text="No hay rutas guardadas.\nPodés iniciar una ruta en Navegación o pulsar Grabar.",
                halign="center",
                theme_text_color="Custom",
                text_color=DORADO_APAGADO,
                size_hint_y=None,
                height=dp(80),
            )
            self.contenedor_lista.add_widget(lbl_vacio)
            self.estado.text = "Historial vacío."
            return

        self.estado.text = f"{len(rutas)} ruta(s) guardada(s). Tocá una para gestionarla."

        for i, ruta in enumerate(rutas):
            nombre = ruta.get("nombre", f"Ruta {i+1}")
            destino = ruta.get("destino", "")
            cant_puntos = len(ruta.get("puntos", []))
            creada = ruta.get("creada", "")
            if creada and "T" in creada:
                creada = creada.replace("T", " ")

            detalle = f"Destino: {destino}" if destino else f"{cant_puntos} puntos GPS"
            if creada:
                detalle += f" • {creada[:16]}"

            tarjeta = MDCard(
                orientation="vertical",
                size_hint=(1, None),
                height=dp(64),
                padding=[dp(12), dp(8), dp(12), dp(8)],
                spacing=dp(2),
                md_bg_color=OSCURO,
                line_color=BORDO_OSCURO,
                line_width=dp(1),
                elevation=2,
                ripple_behavior=True,
            )
            lbl_nombre = MDLabel(
                text=nombre,
                bold=True,
                font_style="Subtitle1",
                theme_text_color="Custom",
                text_color=CREMA_CLARO,
                size_hint_y=None,
                height=dp(26),
            )
            lbl_detalle = MDLabel(
                text=detalle,
                font_style="Caption",
                theme_text_color="Custom",
                text_color=DORADO_APAGADO,
                size_hint_y=None,
                height=dp(20),
            )
            tarjeta.add_widget(lbl_nombre)
            tarjeta.add_widget(lbl_detalle)

            tarjeta.bind(on_release=lambda _card, idx=i: self._seleccionar_ruta(idx))
            self.botones_rutas.append(tarjeta)
            self.contenedor_lista.add_widget(tarjeta)

    def _seleccionar_ruta(self, idx):
        self.ruta_seleccionada_idx = idx
        rutas = RutasService.cargar_rutas()
        if idx >= len(rutas):
            return

        ruta = rutas[idx]
        self.estado.text = f"Seleccionada: «{ruta.get('nombre')}»"
        self.estado.text_color = VERDE_ESTADO

        # Resaltar la tarjeta seleccionada
        for i, card in enumerate(self.botones_rutas):
            if i == idx:
                card.md_bg_color = BORDO
                card.line_color = DORADO_APAGADO
            else:
                card.md_bg_color = OSCURO
                card.line_color = BORDO_OSCURO

        # Habilitar botones de acción
        self.btn_reanudar.disabled = False
        self.btn_invertir.disabled = False
        self.btn_renombrar.disabled = False
        self.btn_eliminar.disabled = False
        self.btn_detener.disabled = False

    def _deshabilitar_acciones(self):
        self.btn_reanudar.disabled = True
        self.btn_invertir.disabled = True
        self.btn_renombrar.disabled = True
        self.btn_eliminar.disabled = True
        self.btn_detener.disabled = True
        self._cerrar_panel_renombrar()

    # ==================================================
    # ACCIONES SOBRE LA RUTA SELECCIONADA
    # ==================================================

    def _reanudar_ruta_seleccionada(self, _instance=None):
        if self.ruta_seleccionada_idx is None:
            return
        rutas = RutasService.cargar_rutas()
        if self.ruta_seleccionada_idx >= len(rutas):
            return

        ruta = rutas[self.ruta_seleccionada_idx]
        app = MDApp.get_running_app()

        ubicacion = self.gps.obtener_ubicacion()
        if ubicacion is None and not self.gps.activo:
            self.gps.iniciar()
            ubicacion = self.gps.obtener_ubicacion()

        indicacion = RutasService.iniciar_seguimiento(ruta, ubicacion, auto_direccion=False)
        self.estado.text = indicacion or f"Retomando «{ruta.get('nombre')}»."
        if app and hasattr(app, "switch_screen"):
            app.switch_screen("navegacion")
            try:
                screen_nav = app.sm.get_screen("navegacion").children[-1]
                if hasattr(screen_nav, "_mostrar_indicacion") and indicacion:
                    screen_nav._mostrar_indicacion(indicacion)
            except Exception:
                pass

    def _invertir_y_reanudar_seleccionada(self, _instance=None):
        """Invierte la ruta seleccionada (trayecto de regreso) e inicia el seguimiento."""
        if self.ruta_seleccionada_idx is None:
            return
        rutas = RutasService.cargar_rutas()
        if self.ruta_seleccionada_idx >= len(rutas):
            return

        ruta_orig = rutas[self.ruta_seleccionada_idx]
        ruta_inv = RutasService.invertir_ruta(ruta_orig)
        app = MDApp.get_running_app()

        ubicacion = self.gps.obtener_ubicacion()
        if ubicacion is None and not self.gps.activo:
            self.gps.iniciar()
            ubicacion = self.gps.obtener_ubicacion()

        indicacion = RutasService.iniciar_seguimiento(ruta_inv, ubicacion, auto_direccion=False)
        self.estado.text = indicacion or f"Retomando regreso «{ruta_inv.get('nombre')}»."
        if app and hasattr(app, "switch_screen"):
            app.switch_screen("navegacion")
            try:
                screen_nav = app.sm.get_screen("navegacion").children[-1]
                if hasattr(screen_nav, "_mostrar_indicacion") and indicacion:
                    screen_nav._mostrar_indicacion(indicacion)
            except Exception:
                pass

    def _abrir_panel_renombrar(self, _instance=None):
        if self.ruta_seleccionada_idx is None:
            return
        rutas = RutasService.cargar_rutas()
        if self.ruta_seleccionada_idx >= len(rutas):
            return
        ruta = rutas[self.ruta_seleccionada_idx]
        self.input_nuevo_nombre.text = ruta.get("nombre", "")
        self.panel_renombrar.opacity = 1
        self.panel_renombrar.disabled = False
        self.panel_renombrar.height = dp(96)

    def _dictar_nuevo_nombre(self, _instance=None):
        self.estado.text = "Decí el nuevo nombre de la ruta…"
        self.voz.escuchar(self._al_dictar_nuevo_nombre)

    def _al_dictar_nuevo_nombre(self, texto, error):
        if error or not texto:
            self.estado.text = f"{error or 'No se entendió el nombre.'}"
            return
        self.input_nuevo_nombre.text = texto.strip()
        self._guardar_renombrado()

    def _guardar_renombrado(self, _instance=None):
        nuevo_nombre = self.input_nuevo_nombre.text.strip()
        if not nuevo_nombre:
            self.estado.text = "Escribí o dictá un nombre válido."
            return
        if RutasService.renombrar_ruta(self.ruta_seleccionada_idx, nuevo_nombre):
            self.estado.text = f"Nombre actualizado a «{nuevo_nombre}»."
            self.estado.text_color = VERDE_ESTADO
            self._cerrar_panel_renombrar()
            self.cargar_historial()

    def _cerrar_panel_renombrar(self, _instance=None):
        self.panel_renombrar.opacity = 0
        self.panel_renombrar.disabled = True
        self.panel_renombrar.height = 0

    def _eliminar_ruta_seleccionada(self, _instance=None):
        if self.ruta_seleccionada_idx is None:
            return
        if RutasService.eliminar_ruta_por_indice(self.ruta_seleccionada_idx):
            self.estado.text = "Ruta eliminada correctamente."
            self.estado.text_color = DORADO_APAGADO
            self.cargar_historial()

    def _detener_seguimiento(self, _instance=None):
        RutasService.detener_seguimiento()
        self.estado.text = "Seguimiento de ruta detenido."
        self.estado.text_color = DORADO_APAGADO
