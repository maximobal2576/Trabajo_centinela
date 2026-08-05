---
description: Describe when these instructions should be loaded by the agent based on task contextContexto del proyecto
Soy el desarrollador de un bastón inteligente llamado Centinela para personas con discapacidad visual.

El proyecto está desarrollado en Arduino Mega 2560 utilizando una arquitectura modular. El objetivo es construir un sistema estable, mantenible y escalable, priorizando la confiabilidad por encima de agregar funciones rápidamente.

Actualmente estamos desarrollando la versión 0.5, cuya finalidad es construir una base sólida. La versión 0.5 NO busca tener todas las funciones finales, sino una arquitectura limpia y estable.

Filosofía de desarrollo

Quiero trabajar como un proyecto profesional.

Reglas:

No hacer parches rápidos.
Cada módulo debe tener una única responsabilidad.
No modificar módulos que ya funcionan sin una justificación clara.
Antes de agregar funciones nuevas, la arquitectura debe permanecer estable.
Siempre priorizar estabilidad sobre nuevas características.
Los cambios deben ser pequeños, comprobables y fáciles de revertir.
Arquitectura

Los módulos principales son:

Configuracion
Estado
Scheduler
Sensores
Radar
Movimiento (MPU6050)
Decision
Motores
Audio
Bluetooth
Diagnostico

Cada módulo debe cumplir únicamente su función.

Hardware

Arduino Mega

Hardware:

HC-SR04 Radar (1 sensor montado sobre servo)
HC-SR04 Frontal
HC-SR04 Suelo
Servo SG90
MPU6050
DFPlayer Mini
HC-05 Bluetooth
4 motores vibradores
Buzzer
Baterías 18650
Estado actual

El proyecto compila.

La versión actual es estable.

El radar fue revertido a la versión estable porque los intentos de dividir las mediciones por zonas generaban problemas de arquitectura.

Actualmente el radar funciona como un único sensor.

La lógica de zonas será implementada más adelante en el módulo Decision, NO dentro de Radar.

Objetivo inmediato

Terminar completamente la base de Centinela v0.5.

No agregar funciones avanzadas hasta que todos los módulos sean estables.

Forma de trabajo esperada

Cuando propongas cambios:

Explicar la arquitectura.
Explicar por qué se hace el cambio.
Modificar únicamente el módulo necesario.
Evitar efectos secundarios.
Mantener compatibilidad con el resto del proyecto.

Nunca reescribir módulos completos si solo hace falta cambiar una parte.

Y agregaría una última instrucción, que creo que nos habría ahorrado varios dolores de cabeza:

Antes de modificar cualquier archivo, analizá cómo interactúa con el resto del proyecto. Si falta contexto, pedilo. No supongas estructuras ni reescribas módulos completos sin necesidad.
# applyTo: 'Describe when these instructions should be loaded by the agent based on task context' # when provided, instructions will automatically be added to the request context when the pattern matches an attached file
---

<!-- Tip: Use /create-instructions in chat to generate content with agent assistance -->

Provide project context and coding guidelines that AI should follow when generating code, answering questions, or reviewing changes.