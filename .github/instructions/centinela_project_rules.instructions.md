---
description: Reglas persistentes del proyecto Centinela para la asistencia de IA.
applyTo: "**/*"
---

# Reglas de proyecto Centinela

Estas reglas deben aplicarse durante todo el proyecto.

- Trabajar siempre en una copia o rama cuando se van a aplicar cambios mayores.
- No modificar la versión estable sin autorización explícita.
- Cada cambio debe ser pequeño, reversible y claramente justificado.
- Antes de editar un módulo, analizar cómo interactúa con el resto del proyecto.
- No hacer parches rápidos ni soluciones temporales si pueden desestabilizar la base.
- Preservar la responsabilidad única de cada módulo.
- Priorizar estabilidad por encima de nuevas funcionalidades.
- No reescribir módulos completos sin necesidad.
- Explicar la arquitectura y el motivo de cada cambio propuesto.
- Evitar efectos secundarios y mantener compatibilidad con el resto del código.
- Si no hay contexto suficiente, pedir más información 
antes de avanzar.
-si crees que otra ia puede relizar mejor el trabajo ,avisar y recomendar que ia lo haria mejor a eso .
-si se necesita mas contexto ,consultar

# Contexto adicional

- La versión estable actual es `Centinela_v0.5 - copia`.
- El área de trabajo de desarrollo principal debe ser `Centinela_v0.5\Centinela_v1` o una carpeta separada similar.
-El mencionado radar estara formado por un servomotor con el sensor ultrasonico pegado y la zona se definira por el grado de giro del servomotor
- El proyecto está orientado a Arduino Mega 2560 y hardware específico como HC-SR04, MPU6050, DFPlayer Mini, HC-05, motores vibradores y buzzer.
