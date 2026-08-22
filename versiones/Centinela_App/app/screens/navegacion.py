import urllib.parse
import webbrowser
from datetime import datetime

from kivy.clock import Clock
from kivy.metrics import dp, sp
from kivy.utils import platform
from kivy.uix.widget import Widget
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
from services.bluetooth import get_bluetooth_service
from services.gps import get_ubicacion_service
from services.rutas import RutasService
from services.voz import VozService


class NavegacionScreen(MDBoxLayout):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.orientation = "vertical"
        self.padding = [dp(20), dp(100), dp(20), dp(24)]
        self.spacing = dp(10)

        # =========================
        # SERVICIOS
        # =========================
        self.gps = get_ubicacion_service()
        self.voz = VozService()
        self.bluetooth = get_bluetooth_service()
        self.ultima_indicacion = ""
        self.destino_pendiente = ""

        # Registrar callback de ubicación para actualización inmediata en UI
        self.gps.on_location = self._on_nueva_ubicacion

        # =========================
        # TÍTULO E INDICACIONES (SUPERIOR)
        # =========================
        self.titulo = MDLabel(
            text="NAVEGACIÓN",
            halign="center",
            font_style="H4",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(52),
        )
        self.add_widget(self.titulo)

        self.indicaciones = MDLabel(
            text="Indicaciones: elegí o dictá un destino con el micrófono superior.",
            halign="center",
            font_style="Subtitle1",
            bold=True,
            theme_text_color="Custom",
            text_color=VERDE_ESTADO,
            size_hint_y=None,
            height=dp(68),
        )
        self.add_widget(self.indicaciones)

        # Espaciador para separar el título y empujar los controles abajo
        self.add_widget(Widget(size_hint_y=1))

        # =========================
        # ESTADO GPS
        # =========================
        self.estado_gps = MDLabel(
            text="GPS: Iniciando…",
            halign="center",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(38),
        )
        self.add_widget(self.estado_gps)

        # =========================
        # COORDENADAS
        # =========================
        fila_coords = MDBoxLayout(
            orientation="horizontal",
            size_hint_y=None,
            height=dp(36),
            spacing=dp(8),
        )
        self.latitud = MDLabel(
            text="Lat: --",
            halign="center",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )
        self.longitud = MDLabel(
            text="Lon: --",
            halign="center",
            bold=True,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
        )
        fila_coords.add_widget(self.latitud)
        fila_coords.add_widget(self.longitud)
        self.add_widget(fila_coords)

        # =========================
        # BUSCAR DESTINO
        # =========================
        self.input_destino = MDTextField(
            hint_text="Destino (ej. Farmacia Central, Plaza...)",
            mode="rectangle",
            size_hint=(1, None),
            height=dp(52),
        )
        self.add_widget(self.input_destino)

        # =========================
        # BOTÓN IR
        # =========================
        self.boton_ir = MDRaisedButton(
            text="BUSCAR Y NAVEGAR",
            pos_hint={"center_x": 0.5},
            size_hint=(0.95, None),
            height=dp(48),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self._iniciar_navegacion_destino,
        )
        self.add_widget(self.boton_ir)

        # =========================
        # BOTONES GPS (INICIAR / DETENER)
        # =========================
        fila_gps_botones = MDBoxLayout(
            orientation="horizontal",
            size_hint_y=None,
            height=dp(46),
            spacing=dp(10),
            pos_hint={"center_x": 0.5},
        )
        self.boton_iniciar = MDRaisedButton(
            text="INICIAR GPS",
            size_hint=(0.5, 1),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self.iniciar_ubicacion,
        )
        self.boton_detener = MDRaisedButton(
            text="DETENER GPS",
            size_hint=(0.5, 1),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self.detener_ubicacion,
        )
        fila_gps_botones.add_widget(self.boton_iniciar)
        fila_gps_botones.add_widget(self.boton_detener)
        self.add_widget(fila_gps_botones)

        # =========================
        # PANEL DIÁLOGO / PREGUNTA GUARDAR RUTA
        # =========================
        self.tarjeta_dialogo = MDCard(
            orientation="vertical",
            padding=dp(14),
            spacing=dp(10),
            size_hint=(1, None),
            height=dp(200),
            md_bg_color=OSCURO,
            line_color=DORADO_APAGADO,
            line_width=dp(1.2),
            elevation=4,
        )
        self.lbl_dialogo_titulo = MDLabel(
            text="¿Desea guardar esta ruta?",
            halign="center",
            bold=True,
            font_style="Subtitle1",
            theme_text_color="Custom",
            text_color=DORADO_APAGADO,
            size_hint_y=None,
            height=dp(28),
        )
        self.lbl_dialogo_desc = MDLabel(
            text="Destino: --",
            halign="center",
            font_style="Caption",
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            size_hint_y=None,
            height=dp(24),
        )
        self.input_nombre_ruta = MDTextField(
            hint_text="Nombre de la ruta",
            mode="rectangle",
            size_hint_y=None,
            height=dp(44),
        )
        self.fila_dialogo_botones = MDBoxLayout(
            orientation="horizontal",
            size_hint_y=None,
            height=dp(42),
            spacing=dp(8),
        )
        self.btn_dialogo_si = MDRaisedButton(
            text="SÍ (GUARDAR)",
            size_hint=(0.5, 1),
            font_size=sp(12),
            md_bg_color=BORDO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self._confirmar_guardar_ruta,
        )
        self.btn_dialogo_no = MDRaisedButton(
            text="NO (SOLO NAVEGAR)",
            size_hint=(0.5, 1),
            font_size=sp(12),
            md_bg_color=BORDO_OSCURO,
            theme_text_color="Custom",
            text_color=CREMA_CLARO,
            on_release=self._confirmar_solo_navegar,
        )
        self.fila_dialogo_botones.add_widget(self.btn_dialogo_si)
        self.fila_dialogo_botones.add_widget(self.btn_dialogo_no)

        self.tarjeta_dialogo.add_widget(self.lbl_dialogo_titulo)
        self.tarjeta_dialogo.add_widget(self.lbl_dialogo_desc)
        self.tarjeta_dialogo.add_widget(self.input_nombre_ruta)
        self.tarjeta_dialogo.add_widget(self.fila_dialogo_botones)

        # Oculto por defecto
        self.tarjeta_dialogo.opacity = 0
        self.tarjeta_dialogo.disabled = True
        self.tarjeta_dialogo.size_hint_y = None
        self.tarjeta_dialogo.height = 0
        self.add_widget(self.tarjeta_dialogo)

        # Clock de respaldo: refresca la UI si el callback no llegó aún
        Clock.schedule_interval(self.actualizar_gps, 1)

    # ==================================================
    # CICLO DE VIDA — AUTO-INICIO GPS
    # ==================================================

    def on_enter(self, *args):
        """Inicia el GPS automáticamente al entrar a la pantalla."""
        if not self.gps.activo:
            self.iniciar_ubicacion(None)

    # ==================================================
    # CALLBACK DIRECTO DESDE EL LISTENER JAVA
    # ==================================================

    def _on_nueva_ubicacion(self, lat, lon):
        """Recibe coordenadas directamente desde onLocationChanged (sin polling)."""
        self.latitud.text = f"Lat: {lat:.5f}"
        self.longitud.text = f"Lon: {lon:.5f}"
        self.estado_gps.text = "GPS: UBICACIÓN DISPONIBLE"
        self.estado_gps.text_color = VERDE_ESTADO

        ubicacion = {"latitud": lat, "longitud": lon}
        if RutasService.esta_grabando():
            RutasService.registrar_punto(lat, lon)

        indicacion = RutasService.indicacion_actual(ubicacion)
        if indicacion:
            self._mostrar_indicacion(indicacion)

    # ==================================================
    # INICIAR Y DETENER GPS
    # ==================================================

    def iniciar_ubicacion(self, instance=None):
        try:
            if self.gps.iniciar():
                self.estado_gps.text = "GPS: Buscando ubicación…"
                self.estado_gps.text_color = VERDE_ESTADO
            else:
                self.estado_gps.text = f"GPS: {self.gps.ultimo_error}"
                self.estado_gps.text_color = CREMA_CLARO
        except Exception as e:
            self.estado_gps.text = f"GPS: Error - {e}"

    def detener_ubicacion(self, instance=None):
        try:
            self.gps.detener()
            self.estado_gps.text = "GPS: Detenido"
            self.estado_gps.text_color = CREMA_CLARO
        except Exception:
            self.estado_gps.text = "GPS: Detenido"
            self.estado_gps.text_color = CREMA_CLARO

    # ==================================================
    # FLUJO DE NAVEGACIÓN Y GUARDADO DENTRO DE LA APP
    # ==================================================

    def _dictar_destino(self, _instance=None):
        self.estado_gps.text = "GPS: Decí a dónde querés ir…"
        self.voz.escuchar(self._al_dictar_destino)

    def _al_dictar_destino(self, texto, error):
        if error or not texto:
            self.estado_gps.text = f"GPS: {error or 'No se entendió el destino.'}"
            return
        destino = texto.strip()
        for prefijo in ("ir a", "ir al", "ir a la", "navegar a", "navegar al", "cómo llegar a", "llegar a"):
            if destino.lower().startswith(prefijo):
                destino = destino[len(prefijo):].strip()
                break
        self.input_destino.text = destino
        self._iniciar_navegacion_destino(destino=destino)

    def _iniciar_navegacion_destino(self, _instance=None, destino=None):
        destino = destino or self.input_destino.text.strip()
        if not destino:
            self.estado_gps.text = "Escribí o dictá un destino para navegar."
            return

        self.destino_pendiente = destino
        self.lbl_dialogo_desc.text = f"Destino: «{destino}»"
        self.input_nombre_ruta.text = f"Ruta a {destino}"

        # Mostrar panel de diálogo
        self.tarjeta_dialogo.opacity = 1
        self.tarjeta_dialogo.disabled = False
        self.tarjeta_dialogo.height = dp(200)
        self.estado_gps.text = f"¿Desea guardar la ruta a «{destino}»? Decí Sí o No."

    def _escuchar_respuesta_dialogo(self, _instance=None):
        self.estado_gps.text = "Escuchando: decí «Sí» para guardar o «No» para solo navegar…"
        self.voz.escuchar(self._procesar_respuesta_dialogo_voz)

    def _procesar_respuesta_dialogo_voz(self, texto, error):
        if error or not texto:
            self.estado_gps.text = f"GPS: {error or 'No se entendió la respuesta.'}"
            return
        respuesta = texto.lower().strip()
        if any(w in respuesta for w in ("sí", "si", "guardar", "grabar", "correcto")):
            self._confirmar_guardar_ruta()
        elif any(w in respuesta for w in ("no", "solo", "navegar", "abrir")):
            self._confirmar_solo_navegar()
        else:
            self.estado_gps.text = f"Entendí «{texto}». Decí «Sí» para guardar o «No» para solo navegar."

    def _confirmar_guardar_ruta(self, _instance=None):
        nombre = self.input_nombre_ruta.text.strip() or f"Ruta a {self.destino_pendiente}"
        self._ocultar_dialogo()
        self._iniciar_recorrido_interno(self.destino_pendiente, guardar_nombre=nombre)

    def _confirmar_solo_navegar(self, _instance=None):
        self._ocultar_dialogo()
        self._iniciar_recorrido_interno(self.destino_pendiente, guardar_nombre=None)

    def _ocultar_dialogo(self):
        self.tarjeta_dialogo.opacity = 0
        self.tarjeta_dialogo.disabled = True
        self.tarjeta_dialogo.height = 0

    def _iniciar_recorrido_interno(self, destino, guardar_nombre=None):
        """Calcula el recorrido en segundo plano y muestra indicaciones dentro de la app."""
        if not self.gps.activo:
            self.iniciar_ubicacion()

        ubicacion = self.gps.obtener_ubicacion()
        self.estado_gps.text = f"Calculando indicaciones hacia «{destino}»…"
        self.estado_gps.text_color = VERDE_ESTADO

        def _al_recibir_ruta(ruta, error):
            Clock.schedule_once(lambda dt: self._procesar_ruta_recibida(ruta, error, destino, guardar_nombre), 0)

        RutasService.obtener_ruta_online(destino, ubicacion, _al_recibir_ruta)

    def _procesar_ruta_recibida(self, ruta, error, destino, guardar_nombre):
        ubicacion = self.gps.obtener_ubicacion()
        if error or not ruta:
            # Fallback si no hay conexión a internet: crear ruta directa basada en coordenadas
            self.estado_gps.text = f"GPS: {error or 'Sin conexión para mapa online'}. Usando navegación directa."
            ruta = RutasService.guardar_ruta_destino(guardar_nombre or f"Ruta a {destino}", destino, ubicacion)
            if ubicacion:
                indicacion = f"Indicaciones: rumbo hacia «{destino}». Avanzá manteniendo tu dirección."
                self._mostrar_indicacion(indicacion)
            return

        if guardar_nombre:
            ruta["nombre"] = guardar_nombre
            rutas = RutasService.cargar_rutas()
            rutas.append(ruta)
            RutasService._guardar_rutas(rutas)
            puntos_cant = len(ruta.get("puntos", []))
            self.estado_gps.text = f"Ruta «{guardar_nombre}» guardada ({puntos_cant} puntos cada 8m)."
            # Activar grabación para seguir sumando puntos reales del usuario
            RutasService.iniciar_grabacion(guardar_nombre)
        else:
            self.estado_gps.text = f"Navegando hacia «{destino}»."

        # Iniciar seguimiento paso a paso directamente dentro de la app
        indicacion = RutasService.iniciar_seguimiento(ruta, ubicacion)
        if ruta.get("pasos") and len(ruta["pasos"]) > 0:
            primer_paso = ruta["pasos"][0].get("instruccion", "")
            if primer_paso:
                self._mostrar_indicacion(f"Indicaciones: {primer_paso}")
        elif indicacion:
            self._mostrar_indicacion(indicacion)

        self.estado_gps.text_color = VERDE_ESTADO

    # ==================================================
    # COMANDOS DE VOZ GENERALES EN NAVEGACIÓN
    # ==================================================

    def escuchar_comando(self, _instance=None):
        self.estado_gps.text = "GPS: Escuchando comando o destino…"
        self.voz.escuchar(self._procesar_comando)

    def _procesar_comando(self, texto, error):
        if error or not texto:
            self.estado_gps.text = f"GPS: {error or 'No se reconoció comando.'}"
            return
        comando = texto.lower().strip()

        # Si el comando es un destino hacia donde ir
        if any(comando.startswith(p) for p in ("ir a", "ir al", "ir a la", "navegar a", "navegar al", "cómo llegar a", "llegar a")):
            destino = comando
            for p in ("ir a la", "ir al", "ir a", "navegar al", "navegar a", "cómo llegar a", "llegar a"):
                if destino.startswith(p):
                    destino = destino[len(p):].strip()
                    break
            self.input_destino.text = destino
            self._iniciar_navegacion_destino(destino=destino)
            return

        if any(frase in comando for frase in ("retomar ruta", "seguir ruta", "volver por")):
            self._retomar_ruta_por_voz(texto)
        elif any(frase in comando for frase in ("finalizar ruta", "terminar ruta", "detener guardado")):
            self._finalizar_ruta_por_voz()
        elif any(frase in comando for frase in ("guardar ruta", "grabar ruta", "iniciar ruta")):
            self._iniciar_ruta_por_voz(texto)
        elif any(palabra in comando for palabra in ("iniciar", "activar", "prender")):
            self.iniciar_ubicacion(None)
        elif any(palabra in comando for palabra in ("detener", "parar", "apagar")):
            self.detener_ubicacion(None)
        else:
            # Interpretar directamente como destino
            self.input_destino.text = texto.strip()
            self._iniciar_navegacion_destino(destino=texto.strip())

    def _iniciar_ruta_por_voz(self, texto):
        """Inicia la misma grabación que usa la pantalla Rutas."""
        if not self.gps.activo:
            self.iniciar_ubicacion()
        if RutasService.esta_grabando():
            self.estado_gps.text = "GPS: Ya se está grabando una ruta. Decí finalizar ruta."
            return
        nombre = ""
        for frase in ("guardar ruta", "grabar ruta", "iniciar ruta"):
            if frase in texto.lower():
                nombre = texto.lower().split(frase, 1)[1].strip()
                break
        nombre = nombre or datetime.now().strftime("Ruta por voz %d-%m %H:%M")
        RutasService.iniciar_grabacion(nombre)
        ubicacion = self.gps.obtener_ubicacion()
        if ubicacion:
            RutasService.registrar_punto(**ubicacion)
        self.estado_gps.text = f"GPS: Grabando «{nombre}». Decí finalizar ruta al terminar."
        self.estado_gps.text_color = VERDE_ESTADO

    def _finalizar_ruta_por_voz(self):
        if not RutasService.esta_grabando():
            self.estado_gps.text = "GPS: No hay una ruta en grabación."
            return
        ruta = RutasService.finalizar_grabacion()
        if ruta is None:
            self.estado_gps.text = "GPS: No hubo suficientes puntos para guardar la ruta."
        else:
            self.estado_gps.text = f"GPS: Ruta «{ruta['nombre']}» guardada."
            self.estado_gps.text_color = VERDE_ESTADO

    def _retomar_ruta_por_voz(self, texto):
        if not self.gps.activo:
            self.iniciar_ubicacion()
        nombre = texto.lower()
        for frase in ("retomar ruta", "seguir ruta", "volver por"):
            if frase in nombre:
                nombre = nombre.split(frase, 1)[1].strip()
                break
        ruta = RutasService.buscar_ruta(nombre)
        if ruta is None:
            self.estado_gps.text = "GPS: No encontré esa ruta guardada."
            return

        # Si la ruta guardada tiene destino de Maps, abrirlo
        if ruta.get("destino"):
            self._abrir_en_google_maps(ruta["destino"])
            self.estado_gps.text = f"Abriendo Maps para «{ruta['nombre']}»."
            return

        indicacion = RutasService.iniciar_seguimiento(ruta, self.gps.obtener_ubicacion())
        if indicacion is None:
            self.estado_gps.text = "GPS: Esperando una ubicación para retomar la ruta."
            return
        self.estado_gps.text = f"GPS: Retomando «{ruta['nombre']}»."
        self._mostrar_indicacion(indicacion)

    def _mostrar_indicacion(self, texto):
        self.indicaciones.text = texto
        if texto != self.ultima_indicacion:
            self.ultima_indicacion = texto
            self.bluetooth.send_command(f"I:{texto}\n")

    # ==================================================
    # ACTUALIZACIÓN DE UI (Clock fallback — 1 seg)
    # ==================================================

    def actualizar_gps(self, dt):
        """Respaldo de UI: actualiza etiquetas si el callback directo no llegó."""
        try:
            datos = self.gps.obtener_ubicacion()
            if not datos:
                if self.gps.activo and self.gps.ultimo_error:
                    self.estado_gps.text = f"GPS: {self.gps.ultimo_error}"
                return

            lat = datos.get("latitud")
            lon = datos.get("longitud")

            if lat is not None:
                self.latitud.text = f"Lat: {lat:.5f}"
                if lon is not None and RutasService.esta_grabando():
                    RutasService.registrar_punto(lat, lon)

            if lon is not None:
                self.longitud.text = f"Lon: {lon:.5f}"

            if lat is not None and lon is not None:
                indicacion = RutasService.indicacion_actual(datos)
                if indicacion:
                    self._mostrar_indicacion(indicacion)
                self.estado_gps.text = "GPS: UBICACIÓN DISPONIBLE"
                self.estado_gps.text_color = VERDE_ESTADO

        except Exception:
            pass

