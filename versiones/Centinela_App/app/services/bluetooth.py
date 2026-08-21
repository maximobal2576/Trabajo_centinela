from kivy.utils import platform


if platform == "android":
    from android.permissions import Permission, request_permissions
    from jnius import autoclass
else:
    # pyserial solo se utiliza para probar el HC-05 desde Windows.  No debe
    # impedir abrir la interfaz cuando todavía no está instalado en VS Code.
    try:
        import serial
        from serial.tools import list_ports
    except ImportError:
        serial = None
        list_ports = None


class BluetoothService:
    """Gestiona el vínculo con el HC-05 en Android o durante pruebas en Windows."""

    HC05_UUID = "00001101-0000-1000-8000-00805F9B34FB"

    def __init__(self, port=None, baudrate=9600):
        self.port = port
        self.baudrate = baudrate
        self.serial = None
        self.socket = None
        self.input_stream = None
        self.output_stream = None
        self.connected = False
        self.last_error = ""
        self._buffer = ""
        self.state = {
            "EVENTO": "---",
            "DIRECCION": "---",
            "BATERIA": "--",
            "AUDIO_OCUPADO": "0",
            "BLUETOOTH": "0",
            "ESTADO": "---",
        }

    def connect(self):
        """Conecta al HC-05 emparejado en el dispositivo actual."""
        if self.connected:
            return True

        if platform == "android":
            return self._connect_android()

        return self._connect_windows()

    def _connect_windows(self):
        if serial is None or list_ports is None:
            self.last_error = (
                "Falta pyserial en el Python de VS Code. Ejecutá: "
                "python -m pip install pyserial"
            )
            return False

        port = self._find_windows_port()
        if port is None:
            self.last_error = "No se encontró el puerto Bluetooth."
            return False

        try:
            self.serial = serial.Serial(port, self.baudrate, timeout=0.2)
            self.connected = self.serial.is_open
            self.last_error = ""
        except (serial.SerialException, OSError) as error:
            self.serial = None
            self.connected = False
            self.last_error = f"No se pudo abrir {port}: {error}"

        return self.connected

    def _find_windows_port(self):
        if self.port:
            return self.port

        for port_info in list_ports.comports():
            name = (port_info.description or "").lower()
            if "hc-05" in name or "hc-06" in name or "bluetooth" in name:
                return port_info.device

        return None

    def _connect_android(self):
        request_permissions([
            Permission.BLUETOOTH_CONNECT,
            Permission.BLUETOOTH_SCAN,
        ])

        try:
            from android.permissions import check_permission

            if not check_permission(Permission.BLUETOOTH_CONNECT):
                self.last_error = "Autorizá Bluetooth y presioná Conectar nuevamente."
                return False

            BluetoothAdapter = autoclass("android.bluetooth.BluetoothAdapter")
            UUID = autoclass("java.util.UUID")
            adapter = BluetoothAdapter.getDefaultAdapter()

            if adapter is None:
                self.last_error = "Este teléfono no dispone de Bluetooth."
                return False

            if not adapter.isEnabled():
                self.last_error = "Activá el Bluetooth del teléfono."
                return False

            device = self._find_paired_hc05(adapter)
            if device is None:
                self.last_error = "Emparejá el HC-05 desde Ajustes de Android."
                return False

            self.socket = device.createRfcommSocketToServiceRecord(
                UUID.fromString(self.HC05_UUID)
            )
            self.socket.connect()
            self.input_stream = self.socket.getInputStream()
            self.output_stream = self.socket.getOutputStream()
            self.connected = True
            self.last_error = ""
        except Exception as error:
            self.disconnect()
            self.last_error = f"No se pudo conectar al HC-05: {error}"

        return self.connected

    @staticmethod
    def _find_paired_hc05(adapter):
        devices = adapter.getBondedDevices().iterator()

        while devices.hasNext():
            device = devices.next()
            name = (device.getName() or "").lower()

            if "hc-05" in name or "hc-06" in name:
                return device

        return None

    def disconnect(self):
        if platform == "android":
            if self.socket is not None:
                try:
                    self.socket.close()
                except Exception:
                    pass

            self.socket = None
            self.input_stream = None
            self.output_stream = None
        elif self.serial and self.serial.is_open:
            self.serial.close()

        self.connected = False
        self._buffer = ""

    def send_command(self, command):
        """Envía un comando simple al Arduino."""
        if not self.connect():
            return False

        try:
            if platform == "android":
                self.output_stream.write(command.encode("ascii"))
                self.output_stream.flush()
            else:
                self.serial.write(command.encode("ascii"))
                self.serial.flush()
            return True
        except Exception as error:
            self.disconnect()
            self.last_error = f"No se pudo enviar el comando: {error}"
            return False

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
            self.last_error = f"Se perdió la conexión: {error}"

    def _read_windows(self):
        while self.serial and self.serial.in_waiting:
            line = self.serial.readline().decode("ascii", errors="ignore").strip()
            if line:
                self._parse_line(line)

    def _read_android(self):
        while self.input_stream and self.input_stream.available() > 0:
            byte = self.input_stream.read()

            if byte == -1:
                raise OSError("El HC-05 cerró la conexión.")

            if byte == 10:
                line = self._buffer.strip()
                self._buffer = ""
                if line:
                    self._parse_line(line)
            elif byte != 13:
                self._buffer += chr(byte)

    def _parse_line(self, line):
        """Extrae pares clave:valor enviados por Arduino."""
        if ":" not in line:
            return

        key, value = line.split(":", 1)
        self.state[key.strip()] = value.strip()


_servicio_compartido = None


def get_bluetooth_service():
    """Mantiene una única conexión física aunque se cambie de pantalla."""
    global _servicio_compartido
    if _servicio_compartido is None:
        _servicio_compartido = BluetoothService()
    return _servicio_compartido
