from kivymd.app import MDApp
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.core.window import Window
from kivy.metrics import dp

from screens.inicio import InicioScreen
from screens.estado import EstadoScreen
from screens.navegacion import NavegacionScreen
from screens.emergencia import EmergenciaScreen
from screens.configuracion import ConfiguracionScreen
from screens.rutas import RutasScreen

from widgets.menu_lateral import MenuLateral
from widgets.top_bar import TopBar
from components.fondo_centinela import FondoCentinela
from components.estilos_centinela import BORDO
from services.bluetooth import get_bluetooth_service
from kivy.clock import Clock


class CentinelaApp(MDApp):

    def build(self):

        self.theme_cls.primary_palette = "Green"
        self.theme_cls.theme_style = "Dark"
        self.title = "Centinela"

        # Ajuste automático del teclado en Android para que el campo de texto
        # quede visible justo por encima del teclado virtual.
        Window.keyboard_anim_args = {"d": 0.2, "t": "in_out_quart"}
        Window.softinput_mode = "below_target"

        # ============================================================
        # CONTENEDOR PRINCIPAL
        # ============================================================

        root = FloatLayout()

        # ============================================================
        # FONDO
        # ============================================================

        fondo = FondoCentinela(
            size_hint=(1, 1),
            pos_hint={"x": 0, "y": 0}
        )
        root.add_widget(fondo)

        # ============================================================
        # PANTALLAS — ocupan el 100 % de la pantalla
        # ============================================================

        self.sm = ScreenManager(
            size_hint=(1, 1),
            pos_hint={"x": 0, "y": 0}
        )

        self._add_screen("inicio", InicioScreen())
        self._add_screen("estado", EstadoScreen())
        self._add_screen("navegacion", NavegacionScreen())
        self._add_screen("emergencia", EmergenciaScreen())
        self._add_screen("configuracion", ConfiguracionScreen())
        self._add_screen("rutas", RutasScreen())

        root.add_widget(self.sm)

        # ============================================================
        # BARRA SUPERIOR GLOBAL (Micrófono y Badge de Batería)
        # ============================================================
        self.top_bar = TopBar(on_mic_press=self._escuchar_comando_global)
        root.add_widget(self.top_bar)

        self.bluetooth_service = get_bluetooth_service()
        Clock.schedule_interval(self._actualizar_bateria_global, 1.0)

        # ============================================================
        # MENÚ LATERAL DESPLEGABLE (Esquina superior izquierda)
        # ============================================================
        self.menu = MenuLateral(
            size_hint=(1, 1),
            pos_hint={"x": 0, "y": 0},
        )
        root.add_widget(self.menu)

        return root

    def _actualizar_bateria_global(self, dt):
        """Actualiza el icono y porcentaje de la batería del bastón en todas las pantallas."""
        try:
            conectado = self.bluetooth_service.connected
            bateria_valor = self.bluetooth_service.state.get("BATERIA", "--")
            self.top_bar.actualizar_bateria(conectado, bateria_valor)

            # Monitoreo de eventos críticos del bastón (Caída o Pulsador SOS)
            evento = str(self.bluetooth_service.state.get("EVENTO", "")).strip().upper()
            if evento in ("CAIDA", "EMERGENCIA", "SOS") and self.sm.current != "emergencia":
                self.switch_screen("emergencia")
        except Exception:
            pass

    def _escuchar_comando_global(self, _instance=None):
        """Activa el reconocimiento de voz para comandos globales y navegación."""
        from services.voz import VozService
        voz = VozService()
        voz.escuchar(self._procesar_comando_global)

    def _procesar_comando_global(self, texto, error):
        if error or not texto:
            return
        comando = texto.lower().strip()

        # Si el menú está abierto y pide cerrarlo
        if any(w in comando for w in ("cerrar menú", "cerrar menu", "ocultar menú")):
            if hasattr(self, "menu"):
                self.menu.cerrar()
            return

        # Si pide abrir menú
        if any(w in comando for w in ("abrir menú", "abrir menu", "mostrar menú", "mostrar menu", "menú", "menu")):
            if hasattr(self, "menu"):
                self.menu.abrir()
            return

        # Cambios de pantalla (equivalente a tocar los ítems del menú lateral)
        if any(w in comando for w in ("inicio", "ir a inicio", "pantalla inicio", "pantalla principal")):
            if hasattr(self, "menu") and self.menu._abierto:
                self.menu.cerrar()
            self.switch_screen("inicio")
            return
        elif any(w in comando for w in ("estado", "ir a estado", "pantalla estado", "batería", "baston")):
            if hasattr(self, "menu") and self.menu._abierto:
                self.menu.cerrar()
            self.switch_screen("estado")
            return
        elif any(w in comando for w in ("navegacion", "navegación", "ir a navegación", "pantalla navegación", "mapa")):
            if hasattr(self, "menu") and self.menu._abierto:
                self.menu.cerrar()
            self.switch_screen("navegacion")
            return
        elif any(w in comando for w in ("emergencia", "ir a emergencia", "pantalla emergencia", "socorro", "auxilio", "alerta")):
            if hasattr(self, "menu") and self.menu._abierto:
                self.menu.cerrar()
            self.switch_screen("emergencia")
            return
        elif any(w in comando for w in ("configuracion", "configuración", "ir a configuración", "pantalla configuración", "ajustes")):
            if hasattr(self, "menu") and self.menu._abierto:
                self.menu.cerrar()
            self.switch_screen("configuracion")
            return
        elif any(w in comando for w in ("rutas", "ir a rutas", "pantalla rutas", "historial")):
            if hasattr(self, "menu") and self.menu._abierto:
                self.menu.cerrar()
            self.switch_screen("rutas")
            return

        # Acciones contextuales sobre los botones de la pantalla activa:
        pantalla_actual = self.sm.current
        try:
            screen_widget = self.sm.get_screen(pantalla_actual).children[-1]
        except Exception:
            screen_widget = None

        if pantalla_actual == "inicio" and screen_widget:
            if any(w in comando for w in ("conectar", "desconectar", "conectar bastón", "vincular")):
                if hasattr(screen_widget, "alternar_conexion"):
                    screen_widget.alternar_conexion(None)
                return

        elif pantalla_actual == "emergencia" and screen_widget:
            if any(w in comando for w in ("911", "llamar 911", "policía", "policia")):
                if hasattr(screen_widget, "iniciar_llamada"):
                    screen_widget.iniciar_llamada("911", "911")
                return
            elif any(w in comando for w in ("contacto", "llamar contacto", "familiar")):
                if hasattr(screen_widget, "llamar_contacto"):
                    screen_widget.llamar_contacto(None)
                return
            elif any(w in comando for w in ("ubicación", "ubicacion", "enviar ubicación", "enviar mensaje")):
                if hasattr(screen_widget, "enviar_ubicacion"):
                    screen_widget.enviar_ubicacion(None)
                return
            elif any(w in comando for w in ("cancelar", "detener llamada")):
                if hasattr(screen_widget, "cancelar_llamada"):
                    screen_widget.cancelar_llamada(None)
                return

        elif pantalla_actual == "rutas" and screen_widget:
            if any(w in comando for w in ("grabar", "iniciar grabación", "guardar ruta", "finalizar grabación")):
                if hasattr(screen_widget, "_alternar_grabacion_manual"):
                    screen_widget._alternar_grabacion_manual(None)
                return
            elif any(w in comando for w in ("reanudar", "seguir ruta", "iniciar ruta")):
                if hasattr(screen_widget, "_reanudar_ruta_seleccionada"):
                    screen_widget._reanudar_ruta_seleccionada(None)
                return
            elif any(w in comando for w in ("eliminar", "borrar ruta")):
                if hasattr(screen_widget, "_eliminar_ruta_seleccionada"):
                    screen_widget._eliminar_ruta_seleccionada(None)
                return

        # Si el comando es exploración del entorno (¿Dónde estoy?)
        if any(w in comando for w in ("dónde estoy", "donde estoy", "explorar", "mi ubicación", "mi ubicacion", "mi posición", "mi posicion")):
            self.switch_screen("navegacion")
            try:
                screen_nav = self.sm.get_screen("navegacion").children[-1]
                if hasattr(screen_nav, "explorar_entorno"):
                    screen_nav.explorar_entorno()
            except Exception:
                pass
            return

        # Si no fue comando de pantalla, y es un destino (ej. "ir a ...", "farmacia", etc.):
        self.switch_screen("navegacion")
        try:
            screen_nav = self.sm.get_screen("navegacion").children[-1]
            if hasattr(screen_nav, "input_destino"):
                destino = texto.strip()
                for p in ("ir a la", "ir al", "ir a", "navegar al", "navegar a", "cómo llegar a", "llegar a"):
                    if destino.lower().startswith(p):
                        destino = destino[len(p):].strip()
                        break
                screen_nav.input_destino.text = destino
                if hasattr(screen_nav, "_iniciar_navegacion_destino"):
                    screen_nav._iniciar_navegacion_destino(destino=destino)
        except Exception:
            pass

    def on_start(self):
        """El botón Atrás cierra el menú si está abierto, o vuelve al inicio."""
        Window.bind(on_keyboard=self._on_keyboard)
        # Iniciar auto-conexión Bluetooth con el bastón Centinela v2 en background
        Clock.schedule_once(self._iniciar_auto_conexion_bt, 0.6)

    def _iniciar_auto_conexion_bt(self, _dt=None):
        """Lanza el hilo de auto-conexión SPP con el ESP32."""
        try:
            if hasattr(self, "bluetooth_service") and self.bluetooth_service:
                self.bluetooth_service.conectar_automaticamente()
        except Exception as e:
            print(f"[CentinelaApp] Error en auto-conexión BT: {e}")

    def on_pause(self):
        """Mantiene los servicios de GPS, Bluetooth y notificaciones activos al suspender la pantalla."""
        return True

    def on_resume(self):
        """Al volver a la app (ej: tocando la notificación), ir a pantalla Navegación."""
        try:
            from kivy.utils import platform
            if platform == "android":
                from jnius import autoclass
                PythonActivity = autoclass("org.kivy.android.PythonActivity")
                activity = PythonActivity.mActivity
                intent = activity.getIntent()
                if intent and intent.getAction() == "OPEN_NAVEGACION":
                    self.switch_screen("navegacion")
        except Exception:
            pass

    def _on_keyboard(self, _window, key, *_args):
        if key == 27:
            # Si el menú está abierto, cerrarlo primero
            if hasattr(self, "menu") and self.menu._abierto:
                self.menu.cerrar()
                return True
            if getattr(self, "sm", None) and self.sm.current != "inicio":
                self.switch_screen("inicio", direction="right")
                return True
        return False

    # ================================================================
    # AGREGAR PANTALLA
    # ================================================================

    def _add_screen(self, name, widget):
        screen = Screen(name=name)
        screen.add_widget(widget)
        self.sm.add_widget(screen)

    # ================================================================
    # CAMBIAR PANTALLA
    # ================================================================

    def switch_screen(self, name, direction="left"):

        if name in self.sm.screen_names:
            if self.sm.current != name:
                self.sm.transition.direction = direction
                self.sm.current = name


# ====================================================================
# EJECUTAR
# ====================================================================

if __name__ == "__main__":
    CentinelaApp().run()
