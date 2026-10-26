Bitácora de Requerimientos y Auditoría IA — Sesión 03
Materia: Lenguajes y Autómatas II (SCD-1016)
Tema: Google Colab, Mágicos y Flujo de Entrega
Docente Titular: MCC. Iván Márquez Larios (ijmarquezl)
Estudiante: JOSE MANUEL SUAREXZ HUEX 
Matrícula: 24531320
Track: C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)



1. Fase 1: El Artesano (Dificultad Manual)
Reto: Manipulación de archivos y llamadas OS

Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:

Durante la implementación manual identifiqué dificultades relacionadas con la manipulación segura de archivos mediante fopen(), fclose() y la validación de errores de entrada/salida. Además, fue necesario controlar correctamente el tamaño de los búferes para evitar accesos fuera de límites de memoria. También encontré problemas con la sintaxis de punteros y el manejo de códigos de retorno, aspectos fundamentales para garantizar la compatibilidad con GCC y evitar fallos de ejecución.



2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

2.1 Matriz de Especificación Formal
Elemento C-R-E-O	Definición Técnica
Contexto (C)	C99 / GCC o Rust 2021 sin bibliotecas no estándar
Rol (R)	Ingeniero de software de sistemas y compiladores
Especificación (E)	Salida estándar y códigos de retorno ABI
Output (O)	Solo código modular y análisis de complejidad

2.2 Prompt Estructurado
Actúa como un ingeniero de software especializado en sistemas y compiladores.

Genera una solución en lenguaje C99 compatible con GCC.
No utilices bibliotecas externas ni funciones no estándar.
Implementa un programa modular que gestione archivos y llamadas al sistema de forma segura.
Valida todas las operaciones de entrada y salida.
Maneja adecuadamente los errores mediante códigos de retorno estándar.
Incluye comentarios descriptivos, análisis de complejidad temporal y espacial.
La salida debe consistir exclusivamente en código C y su análisis técnico.

###*Reporte Crítico
La IA generó una solución funcional y compatible con GCC; sin embargo, fue necesario revisar manualmente el manejo de errores al abrir archivos y verificar que los recursos se liberaran correctamente. Durante la auditoría se confirmó que no existían accesos fuera de rango ni fugas de memoria.

Asimismo, se comprobó que la complejidad temporal y espacial coincidía con la estimación teórica. Esta práctica me permitió observar que la IA puede agilizar considerablemente el desarrollo inicial de una solución, pero sigue siendo importante realizar una revisión técnica para validar casos límite, comprobar la calidad del código y asegurar el cumplimiento de los requisitos establecidos por el harness.

**Firma digital:** [josessuarez]
