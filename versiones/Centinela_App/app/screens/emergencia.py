from kivy.clock import Clock
from kivy.metrics import dp
from kivy.utils import platform
from kivy.uix.widget import Widget
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDRaisedButton
from kivymd.uix.label import MDLabel

from services.almacenamiento import AlmacenamientoService
from services.gps import UbicacionService
from widgets.panel_sos import PanelBotonesSOS

from components.estilos_centinela import (
    CREMA_CLARO,
    BORDO,
    BORDO_OSCURO,
    OSCURO,
    DORADO_APAGADO,
    ROJO_EMERGENCIA,
)


class EmergenciaScreen(MDBoxLayout):
    """Coordina llamadas de emergencia con una cuenta regresiva cancelable."""

    SEGUNDOS_CONFIRMACION = 5

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = [dp(20), dp(100), dp(20), dp(24)]
        self.spacing = dp(12)

        self.numero_pendiente = ""
        self.destino_pendiente = ""
        self.segundos_restantes = 0
        self.evento_cuenta_regresiva = None

        # ==========================================
        # TÍTULO (SUPERIOR)
        # ==========================================

        self.titulo = MDLabel(
            text="EMERGENCIA",
            halign="center",
            font_style="H4",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(56),
        )

        self.add_widget(self.titulo)

        # Espaciador para separar el título y empujar los botones abajo
        self.add_widget(Widget(size_hint_y=1))

        # ==========================================
        # ESTADO
        # ==========================================

        self.estado_llamada = MDLabel(
            text="Seleccioná una llamada de emergencia.",
            halign="center",
            font_style="H5",
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=dp(56),
        )

        self.add_widget(self.estado_llamada)

        # ==========================================
        # PANEL DE BOTONES SOS (MODULAR)
        # ==========================================
        self.panel_sos = PanelBotonesSOS(
            on_call_911=lambda instance: self.iniciar_llamada("911", "911"),
            on_call_contacto=self.llamar_contacto,
            on_send_gps=self.enviar_ubicacion,
            on_cancelar=self.cancelar_llamada,
        )
        self.add_widget(self.panel_sos)

        # Referencias de compatibilidad retrocompatible directa
        self.boton_911 = self.panel_sos.btn_911
        self.boton_contacto = self.panel_sos.btn_contacto
        self.boton_ubicacion = self.panel_sos.btn_ubicacion
        self.boton_cancelar = self.panel_sos.btn_cancelar

    # ==================================================
    # LLAMAR CONTACTO
    # ==================================================

    def llamar_contacto(self, instance):

        configuracion = (
            AlmacenamientoService.cargar_configuracion()
        )

        numero = configuracion[
            "contacto_emergencia"
        ]

        if not numero:

            self.estado_llamada.text = (
                "Configurá primero el contacto de emergencia."
            )

            return

        self.iniciar_llamada(
            numero,
            "contacto de emergencia"
        )

    # ==================================================
    # INICIAR LLAMADA
    # ==================================================

    def iniciar_llamada(self, numero, destino):

        if self.evento_cuenta_regresiva is not None:
            return

        self.numero_pendiente = numero
        self.destino_pendiente = destino
        self.segundos_restantes = (
            self.SEGUNDOS_CONFIRMACION
        )

        self._solicitar_permiso_llamada()
        self.panel_sos.activar_modo_cuenta_regresiva()
        self._mostrar_cuenta_regresiva()

        self.evento_cuenta_regresiva = (
            Clock.schedule_interval(
                self._actualizar_cuenta_regresiva,
                1,
            )
        )

    # ==================================================
    # CUENTA REGRESIVA
    # ==================================================

    def _actualizar_cuenta_regresiva(self, dt):

        self.segundos_restantes -= 1

        if self.segundos_restantes <= 0:

            self.evento_cuenta_regresiva.cancel()
            self.evento_cuenta_regresiva = None

            self._realizar_llamada()

            return False

        self._mostrar_cuenta_regresiva()

        return True

    def _mostrar_cuenta_regresiva(self):

        self.estado_llamada.text = (
            f"Llamar {self.destino_pendiente} "
            f"en {self.segundos_restantes} segundos."
        )

        self.estado_llamada.text_color = (
            ROJO_EMERGENCIA
        )

    # ==================================================
    # CANCELAR
    # ==================================================

    def cancelar_llamada(self, instance):

        if self.evento_cuenta_regresiva is not None:

            self.evento_cuenta_regresiva.cancel()
            self.evento_cuenta_regresiva = None

        self.estado_llamada.text = (
            "Llamada cancelada."
        )

        self.estado_llamada.text_color = (
            DORADO_APAGADO
        )

        self._restablecer_controles()

    # ==================================================
    # REALIZAR LLAMADA
    # ==================================================

    def _realizar_llamada(self):

        if platform != "android":

            self.estado_llamada.text = (
                "La llamada directa se realiza desde Android."
            )

            self._restablecer_controles()

            return

        try:

            from android.permissions import (
                Permission,
                check_permission,
                request_permissions,
            )

            from jnius import autoclass

            if not check_permission(
                Permission.CALL_PHONE
            ):

                request_permissions(
                    [Permission.CALL_PHONE]
                )

                self.estado_llamada.text = (
                    "Permití las llamadas y "
                    "presioná nuevamente."
                )

                self._restablecer_controles()

                return

            Intent = autoclass(
                "android.content.Intent"
            )

            Uri = autoclass(
                "android.net.Uri"
            )

            PythonActivity = autoclass(
                "org.kivy.android.PythonActivity"
            )

            intent = Intent(
                Intent.ACTION_CALL
            )

            intent.setData(
                Uri.parse(
                    f"tel:{self.numero_pendiente}"
                )
            )

            PythonActivity.mActivity.startActivity(
                intent
            )

            self.estado_llamada.text = (
                f"Llamando a "
                f"{self.destino_pendiente}."
            )

            self.estado_llamada.text_color = (
                ROJO_EMERGENCIA
            )

        except Exception as error:

            self.estado_llamada.text = (
                f"No se pudo iniciar la llamada: "
                f"{error}"
            )

        self._restablecer_controles()

    # ==================================================
    # PERMISO DE LLAMADA
    # ==================================================

    @staticmethod
    def _solicitar_permiso_llamada():

        if platform != "android":
            return

        from android.permissions import (
            Permission,
            request_permissions,
        )

        request_permissions(
            [Permission.CALL_PHONE]
        )

    # ==================================================
    # ENVIAR UBICACIÓN
    # ==================================================

    def enviar_ubicacion(self, instance):

        if platform != "android":

            self.estado_llamada.text = (
                "El envío directo se realiza desde Android."
            )

            return

        from android.permissions import Permission, check_permission, request_permissions

        permisos = [
            Permission.SEND_SMS,
            Permission.ACCESS_FINE_LOCATION,
            Permission.ACCESS_COARSE_LOCATION,
        ]
        if not check_permission(Permission.SEND_SMS):
            self.estado_llamada.text = "Autorizá SMS y ubicación para enviar la alerta."
            request_permissions(permisos, self._al_resolver_permisos_ubicacion)
            return

        self.estado_llamada.text = self._enviar_alerta_ubicacion()

    def _al_resolver_permisos_ubicacion(self, _permisos, _resultados):
        """Android responde de forma asíncrona; recién entonces se envía el SMS."""
        if platform != "android":
            return
        from android.permissions import Permission, check_permission
        if check_permission(Permission.SEND_SMS):
            self.estado_llamada.text = self._enviar_alerta_ubicacion()
        else:
            self.estado_llamada.text = "No se autorizó el envío de SMS."

    @staticmethod
    def _enviar_alerta_ubicacion():

        configuracion = (
            AlmacenamientoService.cargar_configuracion()
        )

        contacto = configuracion[
            "contacto_emergencia"
        ]

        if not contacto:
            return (
                "No hay contacto para enviar la alerta."
            )

        from android.permissions import (
            Permission,
            check_permission,
        )

        if not check_permission(
            Permission.SEND_SMS
        ):
            return (
                "No se autorizó el envío de SMS."
            )

        enlace = (
            UbicacionService.obtener_enlace_mapa()
        )

        mensaje = (
            "Alerta Centinela: se solicitó ayuda."
        )

        if enlace:

            mensaje += (
                f" Ubicación: {enlace}"
            )

        else:

            mensaje += (
                " Ubicación no disponible."
            )

        try:

            from jnius import autoclass

            SmsManager = autoclass(
                "android.telephony.SmsManager"
            )

            SmsManager.getDefault().sendTextMessage(
                contacto,
                None,
                mensaje,
                None,
                None,
            )

            return (
                "Alerta enviada al contacto."
            )

        except Exception as error:

            return (
                f"No se pudo enviar la alerta: "
                f"{error}"
            )

    # ==================================================
    # RESTABLECER CONTROLES
    # ==================================================

    def _restablecer_controles(self):
        self.numero_pendiente = ""
        self.destino_pendiente = ""
        self.segundos_restantes = 0
        self.panel_sos.restablecer_botones()
