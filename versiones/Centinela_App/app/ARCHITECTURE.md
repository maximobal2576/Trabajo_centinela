# Arquitectura y Diseño - Centinela App

App accesible de asistencia para personas ciegas o con baja visión, vinculada a bastón inteligente ESP32 por Bluetooth SPP.

## 1. Estructura de Directorios

- **`main.py`**: Punto de entrada del ciclo de vida KivyMD (gestor de pantallas `ScreenManager`, eventos globales del sistema y ciclo de vida de Android).
- **`widgets/`**:
  - `menu_lateral.py`: Navegación accesible tipo cajón lateral con soporte TalkBack.
  - `top_bar.py`: Barra superior flotante unificada (botón de comando de voz y badge de batería en tiempo real).
  - `dialogo_ruta.py`: Tarjeta flotante interactiva para confirmación de guardado de rutas.
  - `tarjeta_ruta.py`: Elemento visual accesible individual del historial de rutas guardadas.
  - `tarjeta_metrica.py`: Tarjeta horizontal de visualización de telemetría y diagnósticos.
  - `panel_sos.py`: Botones accesibles de emergencia (911, contacto, SMS GPS) y cuenta regresiva de seguridad.
- **`components/`**:
  - `estilos_centinela.py`: Tokens de diseño, colorimetría WCAG AAA (alto contraste) y tipografía.
  - `fondo_centinela.py`: Fondo dinámico uniforme para todas las pantallas.
- **`screens/`**:
  - `inicio.py`: Estado general de conexión Bluetooth con el bastón y accesos rápidos.
  - `estado.py`: Telemetría en tiempo real del bastón (batería, inclinación, vibración, estado de sensores).
  - `navegacion.py`: Asistencia de navegación GPS, cálculo de rutas peatonales e indicaciones paso a paso.
  - `emergencia.py`: Marcación rápida con cuenta regresiva cancelable y envío de alertas GPS por SMS.
  - `rutas.py`: Grabación, persistencia y reproducción de rutas habituales (con inversión automática de ida/vuelta).
  - `configuracion.py`: Ajustes de sensibilidad, voz TTS, volumen y contactos de emergencia.
- **`services/`**:
  - `bluetooth.py`: Singleton y ciclo de vida de sockets Bluetooth SPP y reconexión automática.
  - `protocolo_baston.py`: Formato binario I2S PCM y parser de telemetría ASCII (`EVENTO:CAIDA`, `EVENTO:SOS`, etc.).
  - `calculos_geo.py`: Distancias geodésicas (Haversine), interpolación de puntos a 8 m y rumbos cardinales.
  - `geocodificador.py`: Consulta de mapa OSM / Nominatim, búsqueda difusa de lugares y reverse geocoding.
  - `rutas.py`: Orquestador de ruteo peatonal online OSRM, persistencia JSON y seguimiento en tiempo real.
  - `voz.py`: Síntesis de voz (TTS) y reconocimiento de voz (STT).
  - `gps.py`: Adquisición de geolocalización Android/Desktop y brújula.
  - `notificaciones.py`: Notificaciones en primer plano y alertas del sistema.
  - `almacenamiento.py`: Persistencia local SQLite/JSON de preferencias.

## 2. Paleta de Colorimetría (Accesibilidad y Contraste WCAG AAA)

- **CREMA (`#D8C5A5`)**: Base visual cálida para superficies secundarias.
- **CREMA_CLARO (`#F2E6CC`)**: Texto principal sobre fondos oscuros (cumple ratio WCAG AAA).
- **BORDO (`#6E1F2B`)**: Color de identidad institucional y botones primarios.
- **BORDO_OSCURO (`#45121B`)**: Sombra y superficies activas.
- **OSCURO (`#211A16`)**: Fondo principal de alto contraste y legibilidad.
- **DORADO_APAGADO (`#B59A6A`)**: Bordes, divisores y elementos de acento tenue.
- **VERDE_ESTADO (`#4DBF73`)**: Estado de batería óptimo y conexión activa.
- **ROJO_EMERGENCIA (`#A6292E`)**: Estados críticos, batería baja y alerta SOS.