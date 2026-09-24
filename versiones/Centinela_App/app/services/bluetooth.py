# =============================================================================
# services/bluetooth.py — Centinela v2.0
# =============================================================================
# Cambios respecto a v1:
#   - Busca el ESP32 por nombre "Centinela-Baston" (o variantes)
#     ademas de los nombres originales (centinela, baston, etc.)
#   - Conexion AUTOMATICA al iniciar la app: conectar_automaticamente()
#     lanza un hilo daemon que intenta conectar en segundo plano sin
#     bloquear la UI. Si falla, reintenta cada 5 segundos.
#   - Reconexion automatica: si se pierde la conexion, el hilo la
#     detecta y vuelve a intentar sin intervención del usuario.
#   - Nuevo metodo enviar_audio_pcm(datos_pcm): empaqueta bytes PCM
#     con magic header (0xA5 0x5A + longitud 2B little-endian) y los
#     envia por el socket BT SPP al ESP32 para reproduccion via I2S.
#   - Mensajes de error actualizados: ya no dice "HC-05", dice "baston".
#   - Eliminadas referencias a AUDIO_OCUPADO (la v2 usa AUDIO streaming).
# =============================================================================

import threading
import time

from kivy.utils import platform


if platform == "android":
    from android.permissions import Permission, request_permissions
    from jnius import autoclass
else:
    try:
        import serial
        from serial.tools import list_ports
    except ImportError:
        serial = None
        list_ports = None


from services.protocolo_baston import (
    empaquetar_audio_pcm,
    interpretar_linea_telemetria,
    CABECERA_AUDIO_MAGIC,
)

# Intervalo de reintento de conexion automatica (segundos)
_RECONECT_INTERVAL = 5.0


class BluetoothService:
    """Gestiona el vinculo BT SPP con el ESP32 del baston Centinela."""

    # UUID estandar SPP RFCOMM — igual para HC-05 y ESP32 BluetoothSerial
    ESP32_UUID = "00001101-0000-1000-8000-00805F9B34FB"

    # Nombres que puede tener el dispositivo ESP32 del baston
    NOMBRES_BASTON = ("centinela-baston", "centinela", "baston", "esp32", "hc-05", "hc-06")

    def __init__(self, port=None, baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.serial = None
        self.socket = None
        self.input_stream = None
        self.output_stream = None
        self.connected = False
        self.last_error = ""
        self._buffer = ""
        self._audio_mic_buffer = bytearray()
        self._mic_grabando = False
        self.state = {
            "EVENTO":    "---",
            "DIRECCION": "---",
            "BATERIA":   "--",
            "AUDIO":     "0",
            "BLUETOOTH": "0",
            "ESTADO":    "---",
            "AURICULAR": "0",
        }

        # Hilo de conexion automatica en background
        self._auto_thread = None
        self._auto_stop   = False

    # ==================================================================
    # CONEXION AUTOMATICA EN BACKGROUND
    # ==================================================================

    def conectar_automaticamente(self):
        """
        Lanza un hilo daemon que intenta conectar al baston automaticamente.
        Si la conexion falla, reintenta cada _RECONECT_INTERVAL segundos.
        Seguro para llamar multiples veces: ignora si ya esta corriendo.
        """
        if self._auto_thread and self._auto_thread.is_alive():
            return  # Ya esta corriendo

        self._auto_stop = False
        self._auto_thread = threading.Thread(
            target=self._bucle_conexion_auto,
            daemon=True,
            name="CentinelaBluetoothAuto"
        )
        self._auto_thread.start()

    def _bucle_conexion_auto(self):
        """Hilo daemon: conecta y reconecta automaticamente en background."""
        while not self._auto_stop:
            if not self.connected:
                self.last_error = "Buscando baston..."
                try:
                    self.connect()
                except Exception as e:
                    self.last_error = f"Error de conexion: {e}"

            # Esperar antes del proximo intento (en intervalos cortos
            # para poder detectar self._auto_stop rapidamente)
            for _ in range(int(_RECONECT_INTERVAL / 0.5)):
                if self._auto_stop or self.connected:
                    break
                time.sleep(0.5)

            # Si ya esta conectado, seguir en bucle para detectar desconexion
            if self.connected:
                time.sleep(1.0)

    def detener_auto(self):
        """Detiene el hilo de reconexion automatica."""
        self._auto_stop = True

    # ==================================================================
    # CONEXION PRINCIPAL
    # ==================================================================

    def connect(self):
        """Conecta al ESP32 del baston. Busca por nombre automaticamente."""
        if self.connected:
            return True

        if platform == "android":
            return self._connect_android()

        return self._connect_windows()

    def _connect_windows(self):
        """Conexion por puerto COM en Windows (pruebas con cable USB-serial)."""
        if serial is None or list_ports is None:
            self.last_error = (
                "Falta pyserial. Ejecuta: python -m pip install pyserial"
            )
            return False

        port = self._find_windows_port()
        if port is None:
            self.last_error = "No se encontro el baston en ningun puerto COM."
            return False

        try:
            self.serial = serial.Serial(port, self.baudrate, timeout=0)
            self.connected = True
            self.last_error = ""
        except Exception as e:
            self.last_error = f"No se pudo abrir {port}: {e}"

        return self.connected

    def _find_windows_port(self):
        if list_ports is None:
            return None
        for p in list_ports.comports():
            desc = (p.description or "").lower()
            if any(n in desc for n in ("cp210", "ch340", "uart", "usb serial", "esp32")):
                return p.device
        return None

    def _connect_android(self):
        """Conexion BT SPP en Android. Busca el baston por nombre y conecta."""
        try:
            BluetoothAdapter = autoclass("android.bluetooth.BluetoothAdapter")
            adapter = BluetoothAdapter.getDefaultAdapter()

            if adapter is None:
                self.last_error = "Este dispositivo no tiene Bluetooth."
                return False

            if not adapter.isEnabled():
                self.last_error = "Activa el Bluetooth del celular."
                return False

            # 1. Buscar en dispositivos ya vinculados
            device = self._find_paired_esp32(adapter)

            # 2. Si no esta vinculado, hacer discovery y vincular automaticamente
            if device is None:
                self.last_error = "Buscando baston cerca..."
                device = self._discover_and_bond_esp32(adapter)

            if device is None:
                self.last_error = "No se encontro el baston encendido cerca."
                return False

            # 3. Conectar socket RFCOMM (SPP)
            self._connect_to_device_socket(device)
            self.connected = True
            self.last_error = ""

        except Exception as error:
            self.disconnect()
            self.last_error = f"No se pudo conectar al baston: {error}"

        return self.connected

    def _discover_and_bond_esp32(self, adapter):
        """
        Busca dispositivos BT cercanos y vincula automaticamente el ESP32
        sin necesidad de ir a Ajustes de Android.
        """
        from jnius import PythonJavaClass, java_method

        PythonActivity = autoclass("org.kivy.android.PythonActivity")
        IntentFilter   = autoclass("android.content.IntentFilter")
        BluetoothDevice = autoclass("android.bluetooth.BluetoothDevice")
        activity = PythonActivity.mActivity

        encontrado = {"device": None}

        class DiscoveryReceiver(PythonJavaClass):
            __javainterfaces__ = ["android/content/BroadcastReceiver"]
            __javacontext__ = "app"

            @java_method("(Landroid/content/Context;Landroid/content/Intent;)V")
            def onReceive(self, context, intent):
                action = intent.getAction()
                if action == BluetoothDevice.ACTION_FOUND:
                    dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                    if dev:
                        name = (dev.getName() or "").lower()
                        if any(n in name for n in BluetoothService.NOMBRES_BASTON):
                            encontrado["device"] = dev
                            try:
                                dev.createBond()
                            except Exception:
                                pass
                elif action == "android.bluetooth.device.action.PAIRING_REQUEST":
                    try:
                        dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                        if dev:
                            try:
                                dev.setPin(bytearray(b"1234"))
                            except Exception:
                                dev.setPin(bytearray(b"0000"))
                    except Exception:
                        pass

        receiver = DiscoveryReceiver()
        filtro = IntentFilter()
        filtro.addAction(BluetoothDevice.ACTION_FOUND)
        filtro.addAction("android.bluetooth.device.action.PAIRING_REQUEST")
        filtro.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED)

        try:
            activity.registerReceiver(receiver, filtro)
            if adapter.isDiscovering():
                adapter.cancelDiscovery()
            adapter.startDiscovery()

            inicio = time.time()
            while time.time() - inicio < 10.0:
                if encontrado["device"] is not None:
                    break
                dev_bond = self._find_paired_esp32(adapter)
                if dev_bond is not None:
                    encontrado["device"] = dev_bond
                    break
                time.sleep(0.4)

            try:
                adapter.cancelDiscovery()
            except Exception:
                pass
        except Exception:
            pass
        finally:
            try:
                activity.unregisterReceiver(receiver)
            except Exception:
                pass

        return encontrado["device"]

    def _connect_to_device_socket(self, device):
        """Crea y conecta el socket RFCOMM con soporte para canal estandar y fallback."""
        UUID = autoclass("java.util.UUID")

        # Intento 1: RFCOMM seguro por UUID SPP estandar
        try:
            self.socket = device.createRfcommSocketToServiceRecord(
                UUID.fromString(self.ESP32_UUID)
            )
            self.socket.connect()
        except Exception:
            # Intento 2: RFCOMM inseguro por UUID
            try:
                self.socket = device.createInsecureRfcommSocketToServiceRecord(
                    UUID.fromString(self.ESP32_UUID)
                )
                self.socket.connect()
            except Exception:
                # Intento 3: reflexion directa en canal 1
                Integer = autoclass("java.lang.Integer")
                method = device.getClass().getMethod("createRfcommSocket", Integer.TYPE)
                self.socket = method.invoke(device, 1)
                self.socket.connect()

        self.input_stream  = self.socket.getInputStream()
        self.output_stream = self.socket.getOutputStream()

    @staticmethod
    def _find_paired_esp32(adapter):
        """Busca el baston ESP32 en la lista de dispositivos ya vinculados."""
        devices = adapter.getBondedDevices().iterator()
        while devices.hasNext():
            device = devices.next()
            name = (device.getName() or "").lower()
            if any(n in name for n in BluetoothService.NOMBRES_BASTON):
                return device
        return None

    # ==================================================================
    # DESCONEXION
    # ==================================================================

    def disconnect(self):
        if platform == "android":
            if self.socket is not None:
                try:
                    self.socket.close()
                except Exception:
                    pass
            self.socket       = None
            self.input_stream  = None
            self.output_stream = None
        elif self.serial and self.serial.is_open:
            self.serial.close()

        self.connected = False
        self._buffer   = ""

    # ==================================================================
    # ENVIAR COMANDO DE TEXTO
    # ==================================================================

    def send_command(self, command):
        """Envia un comando de texto al ESP32 (protocolo SPP simple)."""
        if not self.connect():
            return False

        try:
            data = command.encode("ascii")
            if platform == "android":
                self.output_stream.write(data)
                self.output_stream.flush()
            else:
                self.serial.write(data)
                self.serial.flush()
            return True
        except Exception as error:
            self.disconnect()
            self.last_error = f"No se pudo enviar el comando: {error}"
            return False

    # ==================================================================
    # ENVIAR AUDIO PCM AL ESP32 (v2)
    # ==================================================================

    def enviar_audio_pcm(self, datos_pcm: bytes) -> bool:
        """
        Envia un chunk de audio PCM al ESP32 para reproduccion via I2S.

        Formato del paquete:
          [0xA5][0x5A][len_lo][len_hi][...bytes PCM...]

        El ESP32 detecta el magic header, lee la longitud y almacena
        los bytes en su buffer circular que luego escribe al bus I2S
        (MAX98357A -> altavoz/auricular).

        datos_pcm: bytes crudos PCM 16-bit signed, 16kHz, mono.
        """
        if not self.connected:
            return False

        if not datos_pcm:
            return True

        # Empaquetar con header I2S utilizando el módulo de protocolo
        paquete = empaquetar_audio_pcm(datos_pcm)

        try:
            if platform == "android":
                # Convertir a bytearray para la API Java
                self.output_stream.write(bytearray(paquete))
                self.output_stream.flush()
            else:
                if self.serial:
                    self.serial.write(paquete)
                    self.serial.flush()
            return True
        except Exception as error:
            self.disconnect()
            self.last_error = f"Error enviando audio: {error}"
            return False

    # ==================================================================
    # LECTURA DE DATOS (estado del baston)
    # ==================================================================

    def update(self, dt=None):
        """Lee mensajes disponibles sin bloquear la interfaz."""
        if not self.connected:
            return

        try:
            if platform == "android":
                self._read_android()
            else:
                self._read_windows()
        except Exception as error:
            self.disconnect()
            self.last_error = f"Se perdio la conexion: {error}"

    def _read_windows(self):
        while self.serial and self.serial.in_waiting:
            line = self.serial.readline().decode("ascii", errors="ignore").strip()
            if line:
                self._parse_line(line)

    def _read_android(self):
        while self.input_stream and self.input_stream.available() > 0:
            byte = self.input_stream.read()
            if byte == -1:
                raise OSError("El baston cerro la conexion.")
            if byte == 10:   # \n
                line = self._buffer.strip()
                self._buffer = ""
                if line:
                    self._parse_line(line)
            elif byte != 13:  # ignorar \r
                self._buffer += chr(byte)

    def _parse_line(self, line):
        """Extrae pares clave:valor o eventos de micrófono enviados por el ESP32."""
        evento, datos = interpretar_linea_telemetria(line, self.state, self._audio_mic_buffer)
        if evento == "MIC_START":
            self._mic_grabando = True
        elif evento == "MIC_STOP":
            self._mic_grabando = False
            if datos:
                self._procesar_audio_mic_remoto(datos)

    def _procesar_audio_mic_remoto(self, pcm_data: bytes):
        """Envía el stream de audio capturado por el bastón al motor de voz para ejecutar la orden."""
        try:
            from services.voz import VozService
            from kivymd.app import MDApp
            voz = VozService()

            def al_reconocer_texto(texto, error):
                if texto and not error:
                    app = MDApp.get_running_app()
                    if app and hasattr(app, "_procesar_comando_global"):
                        print(f"[Voz Bastón] Orden reconocida: «{texto}»")
                        app._procesar_comando_global(texto, None)
                elif error:
                    print(f"[Voz Bastón] Aviso: {error}")

            voz.procesar_audio_pcm(pcm_data, al_reconocer_texto)
        except Exception as e:
            print(f"[Bluetooth] Error procesando voz remota del bastón: {e}")


# ==================================================================
# SINGLETON GLOBAL
# ==================================================================

_servicio_compartido = None


def get_bluetooth_service():
    """Mantiene una unica conexion fisica aunque se cambie de pantalla."""
    global _servicio_compartido
    if _servicio_compartido is None:
        _servicio_compartido = BluetoothService()
    return _servicio_compartido
