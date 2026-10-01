# 🦯 Proyecto Centinela v2 — Bastón Inteligente Asistivo

[![FreeCAD 1.1](https://img.shields.io/badge/CAD-FreeCAD_1.1-blue.svg?logo=freecad)](https://www.freecad.org/)
[![Hardware](https://img.shields.io/badge/Material-PA12--CF%20%7C%20ASA-orange.svg)](#-especificaciones-de-hardware)
[![Protection](https://img.shields.io/badge/Protección-IP65_Waterproof-green.svg)](#-especificaciones-de-hardware)
[![Firmware](https://img.shields.io/badge/Firmware-ESP32_C%2B%2B-red.svg?logo=espressif)](versiones/Centinela_v2/)
[![App](https://img.shields.io/badge/Android-Centinela_App-brightgreen.svg?logo=android)](versiones/Centinela_App/)
[![Power](https://img.shields.io/badge/Potencia-2S_Li--Ion_7.4V-yellow.svg)](#-sistema-de-alimentación-y-bms-2s)

Sistema integral de movilidad, orientación espacial y seguridad para personas con discapacidad visual o sordoceguera. Diseñado con estándares de ingeniería de producto para uso continuo en intemperie urbana severa (sol $>50^\circ\text{C}$, lluvia continua, impactos de asfalto y vibración de barrido).

---

## 🗺️ Mapa de Navegación del Repositorio

Para facilitar la exploración, el repositorio está estructurado en módulos independientes:

```
PROYECTO_CENTINELA/
│
├── 📁 versiones/
│   ├── 📁 Centinela_v2/              ──► [PRODUCCIÓN] Firmware ESP32, Arquitectura y CAD
│   │   ├── 📁 cad/                  ──► Modelos 3D, Macro FreeCAD, STEP y STLs
│   │   ├── 📁 documentacion/        ──► Mapeo de hardware y guías de exposición
│   │   ├── 📁 presentacion/         ──► Activos y panel interactivo web
│   │   └── 📄 Centinela_v2.ino      ──► Sketch maestro del ESP32
│   │
│   ├── 📁 Centinela_App/             ──► [MÓVIL] Aplicación Android y Folleto interactivo
│   │   ├── 📁 app/                  ──► Código fuente de la app Centinela
│   │   └── 📁 folleto/              ──► Documentación promocional y de difusión
│   │
│   └── 📁 historico_prototipos/     ──► [ARCHIVO] Prototipos evolutivos (v0.0 a v1 y backups)
│
├── 📁 documentacion/                 ──► [BASE DE CONOCIMIENTO]
│   ├── 📁 base_conocimiento_ia/     ──► Módulos técnicos (sensores, pinout, algoritmos)
│   └── 📁 historico/                ──► Carpetas de campo y listas de compras
│
└── 📄 README.md                      ──► Portada principal e índice del proyecto
```

---

## ⚡ Especificaciones de Hardware y Mecánica (Centinela v2)

El diseño del bastón fue desarrollado en **FreeCAD 1.1** implementando acoplamientos mecánicos de alto rendimiento:

* **Longitud total en servicio:** $1200\text{ mm}$ (4 tramos modulares axiales de $300\text{ mm}$).
* **Diámetro exterior:** $\varnothing 24.0\text{ mm}$ (tubo cilíndrico liso ergonómico).
* **Estructura Interna:** 4 nervaduras longitudinales en cruz que aumentan la rigidez a flexión ($I_{xx}$) en un $28\%$ con solo $11\%$ de masa adicional.
* **Uniones Taper-Lock:** Asiento cónico de autocentrado a $30^\circ$ que elimina por completo el juego mecánico (bamboleo) en la mano.
* **Bloqueo Torsional:** 4 chavetas longitudinales que absorben el $100\%$ del esfuerzo de torsión, protegiendo al pasador contra cizallamiento.
* **Sellado contra Lluvia (IP65):** Garganta maquinada para junta tórica O-Ring NBR ($\varnothing 15 \times 2.0\text{ mm}$) con efecto teja escurridor por gravedad.
* **Canal Central Seco:** Diámetro interior libre de $\varnothing 14.2\text{ mm}$ para pasaje continuo de las líneas de alimentación de $7.4\text{ V}$.

### ⚖️ Presupuesto de Masa Definitivo

| Subsistema | Componentes Principales | Masa Estimada |
| :--- | :--- | :---: |
| **Estructura Mecánica** | 4 Tramos + Puntera + 4 Pines + 4 Clips (PA12-CF / ASA) | **254.8 g** |
| **Alimentación (2S)** | 2x Celdas 18650 Li-Ion + Portapilas 2S + Placa BMS 2S | **109.0 g** |
| **Inteligencia y Sensores** | ESP32 DevKit + GPS NEO-6M + IMU MPU-6050 + 3x Sonar / Radar | **74.5 g** |
| **Interacción y Mango** | Empuñadura ergonómica + 3 Motores hápticos + Parlante I2S + Cables | **68.0 g** |
| **PESO TOTAL TERMINADO** | **Bastón inteligente completo listo para la calle** | **~506 g** |

> **Nota ergonómica:** El $72\%$ del peso total se concentra en el mango ($r \to 0$). El momento de inercia pendular durante el barrido es de apenas $0.14\text{ N}\cdot\text{m}$, previniendo fatiga en la muñeca durante jornadas de 8 a 12 horas.

---

## 📂 Acceso a los Archivos de Fabricación

Los archivos listos para fabricación aditiva se ubican en [`versiones/Centinela_v2/cad/`](versiones/Centinela_v2/cad/):

1. **Macro Paramétrico:** [`baston_centinela_v2_final.FCMacro`](versiones/Centinela_v2/cad/baston_centinela_v2_final.FCMacro) (Ejecutable en FreeCAD).
2. **Documento Nativo:** [`Centinela_v2_Ensamblado.FCStd`](versiones/Centinela_v2/cad/Centinela_v2_Ensamblado.FCStd) (Con hoja de cálculo editable).
3. **Ensamble Universal:** [`Baston_Centinela_v2_Completo.step`](versiones/Centinela_v2/cad/Baston_Centinela_v2_Completo.step) (Formato AP214 con colores).
4. **Mallas STL de Impresión:**
   * `Tramo_1_Superior.stl` (Blanco Técnico)
   * `Tramo_2_Intermedio_Alto.stl` (Azul Centinela)
   * `Tramo_3_Intermedio_Alerta.stl` (Rojo Tráfico RAL 3020)
   * `Tramo_4_Inferior_Base.stl` (Blanco Reflectante)
   * `Puntera_Bumper_Impacto.stl` (Grafito POM/TPU)
   * `Pasador_Pin_Seguridad.stl` (Naranja Seguridad)
   * `Clip_Detent_Retencion.stl` (Verde Técnico)

---

## 🚀 Guía Rápida de Uso

### 1. Visualizar o Modificar en FreeCAD
1. Abre FreeCAD (versión 1.1 o superior).
2. Abre el archivo `versiones/Centinela_v2/cad/Centinela_v2_Ensamblado.FCStd`.
3. Haz doble clic en la hoja **`Parametros_Centinela`** para ajustar cualquier cota (diámetros, tolerancias, espesores). Al cerrar, el modelo se recalculará automáticamente.

### 2. Cargar el Firmware en el ESP32
1. Abre Arduino IDE.
2. Abre el archivo [`versiones/Centinela_v2/Centinela_v2.ino`](versiones/Centinela_v2/Centinela_v2.ino).
3. Selecciona la placa **ESP32 Dev Module** y el puerto COM correspondiente.
4. Compila y flashea el microcontrolador.

### 3. Conectar la Aplicación Móvil
1. Navega a [`versiones/Centinela_App/app/`](versiones/Centinela_App/app/).
2. La app gestiona el enlace Bluetooth SPP automático, telemetría de batería, radar frontal en pantalla y botón de emergencia SOS con geolocalización SMS.
