from kivy.clock import Clock
from kivy.utils import platform
from kivymd.uix.boxlayout import MDBoxLayout
from kivymd.uix.button import MDRaisedButton
from kivymd.uix.label import MDLabel

from services.almacenamiento import AlmacenamientoService
from services.gps import UbicacionService

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
        self.padding = [30, 5, 70, 90]
        self.spacing = 16

        self.numero_pendiente = ""
        self.destino_pendiente = ""
        self.segundos_restantes = 0
        self.evento_cuenta_regresiva = None

        # ==========================================
        # TÍTULO
        # ==========================================

        self.titulo = MDLabel(
            text="EMERGENCIA",
            halign="center",
            font_style="H4",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=60,
        )

        self.add_widget(self.titulo)

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
            height=60,
        )

        self.add_widget(self.estado_llamada)

        # ==========================================
        # BOTÓN 911
        # ==========================================

        self.boton_911 = MDRaisedButton(
            text="LLAMAR 911",
            md_bg_color=ROJO_EMERGENCIA,
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=55,
            on_release=lambda instance: self.iniciar_llamada(
                "911",
                "911"
            ),
        )

        self.add_widget(self.boton_911)

        # ==========================================
        # BOTÓN CONTACTO
        # ==========================================

        self.boton_contacto = MDRaisedButton(
            text="LLAMAR CONTACTO DE EMERGENCIA",
            md_bg_color=BORDO,
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=55,
            on_release=self.llamar_contacto,
        )

        self.add_widget(self.boton_contacto)

        # ==========================================
        # BOTÓN UBICACIÓN
        # ==========================================

        self.boton_ubicacion = MDRaisedButton(
            text="ENVIAR UBICACIÓN AL CONTACTO",
            md_bg_color=BORDO_OSCURO,
            pos_hint={"center_x": 0.5},
            size_hint=(0.85, None),
            height=55,
            on_release=self.enviar_ubicacion,
        )

        self.add_widget(self.boton_ubicacion)

        # ==========================================
        # BOTÓN CANCELAR
        # ==========================================

        self.boton_cancelar = MDRaisedButton(
            text="CANCELAR",
            md_bg_color=OSCURO,
            pos_hint={"center_x": 0.5},
            size_hint=(0.70, None),
            height=50,
            disabled=True,
            opacity=0,
            on_release=self.cancelar_llamada,
        )

        self.add_widget(self.boton_cancelar)

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

        self.boton_cancelar.disabled = False
        self.boton_cancelar.opacity = 1

        self.boton_911.disabled = True
        self.boton_contacto.disabled = True
        self.boton_ubicacion.disabled = True

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

        from android.permissions import (
            Permission,
            request_permissions,
        )

        request_permissions([
            Permission.SEND_SMS,
            Permission.ACCESS_FINE_LOCATION,
            Permission.ACCESS_COARSE_LOCATION,
        ])

        self.estado_llamada.text = (
            self._enviar_alerta_ubicacion()
        )

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

        self.boton_cancelar.disabled = True
        self.boton_cancelar.opacity = 0

        self.boton_911.disabled = False
        self.boton_contacto.disabled = False
        self.boton_ubicacion.disabled = False