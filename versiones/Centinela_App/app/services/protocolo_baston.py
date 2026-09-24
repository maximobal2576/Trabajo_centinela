# -*- coding: utf-8 -*-
"""
services/protocolo_baston.py - Protocolo de comunicacion y telemetria para el baston inteligente.

Define la estructura de datos intercambiada por Bluetooth SPP entre el ESP32 del baston y el telefono:

1. Telemetria ASCII entrante (del baston a la app):
   - "EVENTO:CAIDA"       -> Deteccion de impacto o caida del usuario mediante acelerometro/giroscopio.
   - "EVENTO:SOS"         -> Pulsador de panico activado fisicamente en la empuñadura.
   - "BATERIA:85"         -> Porcentaje de carga restante de la bateria Li-Ion del baston.
   - "DIRECCION:NORTE"    -> Brujula digital interna integrada en el baston.
   - "MIC_START"          -> Inicio de captura de voz desde el microfono I2S del baston.
   - "MIC_CHUNK:<b64>"    -> Fragmentos de audio digital comprimidos en Base64.
   - "MIC_STOP"           -> Fin de transmision de voz remota para ejecutar la orden en el telefono.

2. Paquetes binarios salientes (de la app al baston):
   - Audio streaming PCM hacia el parlante/auricular I2S (MAX98357A):
     Cabecera magica: [0xA5, 0x5A] + Longitud (2 bytes little-endian) + Muestras PCM (16 kHz, 16-bit mono).
   - Comandos simples de control de buzzer/vibrador (ej: "B:1\n" para baliza en esquina, "B:0\n" apagar).
"""

import base64

# Encabezado magico para paquetes de audio PCM enviados al ESP32 (debe coincidir con firmware en C++)
CABECERA_AUDIO_MAGIC = bytes([0xA5, 0x5A])


def empaquetar_audio_pcm(datos_pcm: bytes) -> bytes:
    """
    Empaqueta un chunk de audio PCM de 16-bit 16kHz mono con la cabecera magica
    y la longitud de 2 bytes para que el ESP32 lo envie directamente al amplificador I2S.
    """
    if not datos_pcm:
        return b""
    longitud = len(datos_pcm)
    longitud_bytes = bytes([longitud & 0xFF, (longitud >> 8) & 0xFF])
    return CABECERA_AUDIO_MAGIC + longitud_bytes + datos_pcm


def interpretar_linea_telemetria(linea: str, estado_dict: dict, buffer_mic: bytearray) -> tuple:
    """
    Analiza una linea de texto recibida desde el baston ESP32 por puerto serie o Bluetooth.
    
    Retorna una tupla: (tipo_evento, datos)
    - ("ESTADO", (clave, valor)) -> actualizacion de sensor o bateria.
    - ("MIC_START", None)       -> inicio de dictado remoto.
    - ("MIC_CHUNK", None)       -> fragmento de audio agregado al buffer.
    - ("MIC_STOP", bytes_pcm)   -> audio completo listo para reconocimiento de voz.
    - (None, None)              -> linea vacia o no reconocida.
    """
    linea = linea.strip()
    if not linea:
        return None, None

    # 1. Evento: Inicio de captura de microfono en baston
    if linea == "MIC_START":
        buffer_mic.clear()
        return "MIC_START", None

    # 2. Evento: Fin de captura de microfono
    if linea == "MIC_STOP":
        datos_pcm = bytes(buffer_mic)
        buffer_mic.clear()
        return "MIC_STOP", datos_pcm

    # 3. Fragmento intermedio de audio de microfono en Base64
    if linea.startswith("MIC_CHUNK:"):
        chunk_b64 = linea[len("MIC_CHUNK:"):].strip()
        try:
            chunk = base64.b64decode(chunk_b64)
            buffer_mic.extend(chunk)
            return "MIC_CHUNK", None
        except Exception:
            return None, None

    # 4. Telemetria clave:valor (BATERIA:xx, EVENTO:xx, etc.)
    if ":" in linea:
        clave, valor = linea.split(":", 1)
        clave = clave.strip()
        valor = valor.strip()
        estado_dict[clave] = valor
        return "ESTADO", (clave, valor)

    return None, None