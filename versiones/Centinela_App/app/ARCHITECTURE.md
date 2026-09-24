# Arquitectura y Diseño - Centinela App

App accesible de asistencia para personas ciegas o con baja visión, vinculada a bastón inteligente ESP32 por Bluetooth SPP.

## 1. Estructura de Directorios

- **main.py**: Punto de entrada del ciclo de vida KivyMD (gestor de pantallas ScreenManager, eventos globales del sistema y ciclo de vida de Android).
- **widgets/**:
  - menu_lateral.py: Navegación accesible tipo cajón lateral con soporte TalkBack.
  - 	op_bar.py: Barra superior flotante unificada (botón de comando de voz y badge de estado de batería del bastón).
- **components/**:
  - estilos_centinela.py: Tokens de diseño, colorimetría WCAG AAA (alto contraste) y tipografía.
  - ondo_centinela.py: Fondo dinámico uniforme para todas las pantallas.
- **screens/**:
  - inicio.py: Estado general de conexión Bluetooth con el bastón y accesos rápidos.
  - estado.py: Telemetría en tiempo real del bastón (batería, inclinación, vibración, estado de sensores).
  - 
avegacion.py: Asistencia de navegación GPS, cálculo de rutas e indicaciones paso a paso.
  - emergencia.py: Marcación rápida (911, contactos) y envío de coordenadas por SMS/alerta.
  - utas.py: Grabación, persistencia y reproducción de rutas habituales.
  - configuracion.py: Ajustes de sensibilidad, voz TTS, volumen y contactos de emergencia.
- **services/**:
  - luetooth.py: Singleton de conexión y lectura/escritura serial Bluetooth SPP con el bastón.
  - oz.py: Síntesis de voz (TTS) y reconocimiento de voz (STT).
  - gps.py: Adquisición de geolocalización Android/Desktop y brújula.
  - 
otificaciones.py: Notificaciones en primer plano y alertas del sistema.
  - utas.py: Cálculo y parsing de caminos OSM/GraphHopper.
  - lmacenamiento.py: Persistencia local SQLite/JSON de preferencias y rutas.

## 2. Paleta de Colorimetría (Accesibilidad y Contraste)

- **CREMA (#D8C5A5)**: Base visual cálida para superficies secundarias.
- **CREMA_CLARO (#F2E6CC)**: Texto principal sobre fondos oscuros (cumple ratio WCAG AAA).
- **BORDO (#6E1F2B)**: Color de identidad institucional y botones primarios.
- **BORDO_OSCURO (#45121B)**: Sombra y superficies activas.
- **OSCURO (#211A16)**: Fondo principal de alto contraste y legibilidad.
- **DORADO_APAGADO (#B59A6A)**: Bordes, divisores y elementos de acento tenue.
- **VERDE_ESTADO (#4DBF73)**: Estado de batería óptimo y conexión activa.
- **ROJO_EMERGENCIA (#A6292E)**: Estados críticos, batería baja y alerta SOS.
