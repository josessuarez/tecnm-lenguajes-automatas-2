# Entrega · Hito 2

**Equipo:** José Manuel, Oscar Mauricio, María Jesús, Jordi Geovany y Héctor Iván  
**Repositorio:** https://github.com/jordipech76-boop/tecnm-lenguajes-automatas-2  
**Tag:** `v0.2-hito2`  
**Fecha:** 2026-11-24

---

## 1. Qué funciona

```bash
make
make test-hito2
./lenguajito tests/validos/05_shadowing.lj --run    # imprime 2 y 1
./lenguajito tests/propias/run1.lj --run            # factorial de 5: 120
./lenguajito tests/propias/invalido3.lj --check     # E01 por flujo, sale con 1
./lenguajito tests/propias/invalido5.lj --check     # solo el S01, sin E01 de más
```

- `--run` ejecuta el programa interpretando el AST: primero hace la revisión de `--check` y, si no hay errores, recorre las sentencias (`declarar`, `asignar`, `print`, bloques, `if/else` y `while`).
- Las piezas de `--run` están separadas por contratos (`entorno.h`, `evaluador.h`, `interprete.h`): el **entorno** guarda los valores por ámbito con shadowing, el **evaluador** calcula las expresiones (las comparaciones dan 1 o 0 y el divisor se revisa antes de dividir) y el **intérprete** ejecuta las sentencias.
- Errores en ejecución: una división entre una variable que vale 0 se reporta en `stderr` como `línea:columna: error en ejecucion: division entre cero` y el programa sale con 1.
- La semántica es **sensible al flujo**: lo asignado dentro de un `if` sin `else` o de un `while` no cuenta después; en un `if/else` cuenta solo si se asignó en las dos ramas.
- Un error de sintaxis en una declaración ya no produce un `E01` en cascada: la declaración se conserva marcada con `con_error`.
- Todo lo del Hito 1 (`--tokens`, `--ast`, `--check` con `S01`, `E01`, `E02` y `E04`, varios errores por pasada) sigue funcionando.

## 2. Qué no funciona (y por qué)

- [ ] `--tac` y `--asm`: son de los Hitos 3 y 4.
- [ ] `E04` solo detecta la división entre una **constante** cero, como pide la especificación; con una variable, el error aparece hasta `--run`.
- [ ] El análisis de flujo es conservador: lo asignado dentro de un `while` no cuenta después del ciclo, aunque la condición sea siempre verdadera, porque `--check` no evalúa las condiciones.
- [ ] Un desbordamiento (por ejemplo `INT_MAX + 1`) no se reporta: el resultado da la vuelta como un `int` de 32 bits.
- [ ] Los errores en ejecución salen sin el nombre del archivo (`3:9: error en ejecucion: division entre cero`): `interpretar()` solo recibe el AST, y agregar la ruta cambiaría el contrato de `interprete.h`.

## 3. Pruebas

Salida de las pruebas del curso, corridas en Google Colab el 23 de noviembre de 2026 sobre el commit `38cc2c4` (el código de `src/` no ha cambiado después de ese commit):

```text
Hito 1: 4/4 pruebas superadas
Hito 2: 14/14 pruebas superadas
```

Pruebas propias que agregamos en este hito (detalle en [`tests/propias/README.md`](../tests/propias/README.md)):

- `run1.lj` a `run5.lj`: factorial con `while`, `if/else` con comparaciones, shadowing en bloques anidados, precedencia y menos unario, y una tabla con `<=`; cada una con su `.out`, que coincide exacto con la salida de `--run`.
- `invalido3.lj` e `invalido4.lj`: `E01` por inicialización sensible al flujo (`if` sin `else` y `while`).
- `invalido5.lj`: solo el `S01` de `int z = 5 # 2;`, sin `E01` de más al usar `z`.
- `valido4.lj`: `if/else` que asigna en las dos ramas; `--check` sin errores y `--run` imprime `1`.

## 4. Reparto de trabajo

| Integrante | Archivos / etapas | Commits principales |
| :--- | :--- | :--- |
| José Manuel | `entorno.c`: valores de las variables por ámbito | `91f76e4`, `55ea7f8` |
| Oscar Mauricio | contratos `entorno.h`, `evaluador.h` e `interprete.h`; `evaluador.c`; README del proyecto; salidas esperadas de las pruebas de flujo y esta entrega | `585665b`, `f070dcc`, `c054f1d` |
| Jordi Geovany | `interprete.c` y `--run` en `compilador.c` | `37680d4`, `92977a4` |
| Héctor Iván | inicialización sensible al flujo en `semantica.c`; `con_error` en `ast.h` y `parser.c`; pruebas de flujo y recuperación | `2d65686`, `479c02f`, `ab3e986`, `9541680` |
| María Jesús | pruebas propias de `--run` (`run1` a `run5`) y su documentación | `38cc2c4` |

## 5. Bitácora C-R-E-O (IA)

| Contexto | Rol | Especificación | Output | ¿Qué aceptaron, corrigieron o rechazaron? |
| :--- | :--- | :--- | :--- | :--- |
| Hito 2: había que dividir `--run` en piezas para que tres personas programaran al mismo tiempo | Ingeniero de compiladores en C11 | Firmas de `entorno.h`, `evaluador.h` e `interprete.h` según el plan del equipo, compatibles con el `ASTNode` existente, que compilen con `-Wall -Wextra -Werror -Wpedantic` | Los tres `.h` (Claude) | **Aceptado**, con dos decisiones del equipo: `entorno_declarar` guarda una **copia** del nombre (en la sesión 13 vimos que guardar el puntero falla si el texto cambia) y `evaluar_expr` pone `*error` en 0 al empezar. Se agregó `EVAL_VAR_DESCONOCIDA` solo como protección. |
| Hito 2: faltaba `evaluador.c` para que `--run` calculara las expresiones | Ingeniero de compiladores en C11 | Contrato de `evaluador.h`: `AST_NUM`, `AST_VAR`, `AST_NEG` y `AST_BINARIA`; comparaciones que dan 1 o 0; revisar el divisor antes de dividir; propagar el primer error; sin comportamiento indefinido | `evaluador.c` (Claude) | **Aceptado:** evaluación recursiva con una función auxiliar. La propuesta hace las cuentas en `long long`, así un desbordamiento da la vuelta como un `int` de 32 bits y `INT_MIN / -1` no truena. Lo probamos solo con 19 casos y con `make test-hito2` (14/14) en Colab. |
| Hito 2: `--run` necesita guardar el valor de cada variable por ámbito | Programador de C | Contrato de `entorno.h` (de Oscar): abrir y cerrar ámbito, declarar, leer, asignar y liberar; shadowing; regresar 0 si la variable no existe; sin fugas de memoria | `entorno.c` (Claude) | **Aceptado** después de probarlo con un programa de prueba y AddressSanitizer: la variable interna tapa a la externa, al cerrar el ámbito vuelve la externa, regresa 0 si no existe y no hay fugas. Ya con todo el equipo, `make test-hito1` da 4/4 y `make test-hito2` 14/14. **Corregido:** se ajustó al contrato de Oscar (el ámbito global no se cierra con `entorno_cerrar_ambito` y `entorno_leer` no toca el valor si la variable no existe). |
| Hito 2: faltaba ejecutar el programa con `--run` (`interpretar()` y conectarlo en `compilador.c`) | Programador de C y compiladores | Contrato de `interprete.h`: recorrer el AST, `print` con `"%d\n"`, cada bloque abre y cierra un ámbito, `while` vuelve a evaluar la condición, 0 si todo bien y 1 si hay error en ejecución (mensaje a `stderr`); usar solo `entorno.h` y `evaluador.h`; antes de ejecutar, correr la revisión de `--check` | `interprete.c` y el caso `--run` de `compilador.c` (Claude) | **Aceptado:** un `switch` por tipo de sentencia; el bloque cierra su ámbito aunque falle algo adentro; las declaraciones sin valor quedan en 0. Se comprobó con `make test-hito2` 14/14 y `valgrind` sin fugas. |
| Como `evaluador.c` de Oscar todavía no estaba, la IA hizo un evaluador provisional para probar | — | El evaluador es de Oscar y se programa por separado | `evaluador` de prueba (Claude) | **Rechazado para el repo:** solo se usó para probar en local y no se subió. Con el `evaluador.c` real de Oscar se volvió a probar: 14/14. |
| Los errores en ejecución salen sin el nombre del archivo (`3:9: error en ejecucion: division entre cero`) | — | `interpretar()` solo recibe el AST, no la ruta | — | **Aceptado como limitación:** para poner el archivo habría que cambiar el contrato de `interprete.h`, y eso afecta a los demás. |
| Hito 2: la inicialización de variables no seguía el flujo del programa; una variable asignada dentro de un `if` o un `while` quedaba como inicializada para siempre en `--check` | Programador de C y compiladores | Reglas: `if` sin `else`, lo asignado adentro no cuenta después; `if` con `else`, cuenta solo si se asignó en las dos ramas; `while`, lo asignado adentro no cuenta después del ciclo; guardar el estado `initialized` antes de cada rama y restaurarlo; tocar solo `semantica.c` | `semantica.c` con `snap_take`, `snap_current`, `snap_restore` y los casos nuevos de `AST_SI` y `AST_MIENTRAS` (Claude) | **Aceptado sin cambios:** la lógica de guardar y restaurar el estado y la regla del AND entre las dos ramas del `if/else`. Se usó el código tal cual lo propuso la IA. Comprobado en Colab: `make test-hito1` 4/4, `make test-hito2` 14/14; `invalido3` e `invalido4` dan `E01` en la línea 4 y `valido4` pasa `--check` sin errores. |
| Un error de sintaxis en una declaración (`int z = 5 # 2;`) descartaba la sentencia completa, la variable nunca se declaraba y salía un E01 de más | Programador de C y compiladores | `parse_declaracion` debe devolver el nodo marcado como "con error" en vez de `NULL`; la semántica registra la variable como inicializada sin revisar su expresión; no reportar nada extra | Campo `con_error` en `ast.h`, cambio en `parse_declaracion` de `parser.c` y en el caso `AST_DECLARAR` de `semantica.c` (Claude) | **Aceptado sin cambios:** el campo `con_error` y su uso en el parser y la semántica. Si falla el nombre (`int = 5;`) se sigue devolviendo `NULL`, porque no hay variable que registrar. Con `int z = 5 # 2; print(z);` solo sale `invalido5.lj:2:11: error S01: carácter inválido '#'`, sin `E01`. |
| Faltaban pruebas propias para las dos correcciones | — | Cuatro casos de la sección 5 del plan: `if` sin `else`, `if/else`, `while` y declaración con error, todos empezando con `int x = 5;` | `valido4.lj`, `invalido3.lj`, `invalido4.lj` e `invalido5.lj` (Claude) | **Aceptado con un cambio:** los 4 programas son los de la IA; solo renombré los archivos para seguir la numeración de `tests/propias/`. Sus salidas esperadas (`invalido3.check`, `invalido4.check`, `invalido5.check` y `valido4.out`) se generaron con el compilador (commit `b18024e`). |
| Por error cambié el contenido de `invalido1.lj`, una prueba del Hito 1, y dejó de coincidir con `invalido1.check` | — | Restaurar el archivo desde el historial de Git (commit `b066d71`), sin regenerar el `.check` | Pasos para restaurar desde el historial de GitHub (Claude) | **Corregido:** `invalido1.lj` quedó otra vez con `@` y `#` (commit `b18024e`) y vuelve a coincidir con `invalido1.check`. Error mío, no de la IA. |
| La IA no pudo compilar ni correr el código porque no tenía el repo completo | — | — | — | **Aceptado como limitación:** el código se compila y se prueba en el repo del equipo con `make` y `make test-hito1`, no en el entorno de la IA. Resultado en Google Colab sobre el tag `v0.2-hito2`: Hito 1 4/4, Hito 2 14/14 y las 14 pruebas propias OK. |
| Hito 2: faltaban pruebas propias de `--run` en `tests/propias/` | Programador de pruebas para un compilador en C | 4 o 5 programas que cubran `while`, `if/else`, comparaciones, shadowing, precedencia y menos unario; cada uno debe pasar `--check` sin errores; su `.out` es la salida exacta de `--run`, un número por línea | `run1.lj` a `run5.lj` y los comandos para generar los `.out` (Claude) | **Aceptado:** los 5 programas. Antes de guardar cada `.out` se corrió `--check` (sin errores) y se revisó a mano que los números fueran los correctos: factorial 120; `4` y `0`; `10`, `15` y `1`; `14`, `20`, `-20`, `5` y `3`; tabla del 3. |
| Al crear los archivos desde el editor, el traductor del navegador cambió el código (`if` → "si", `print` → "imprimir") y un bloque pegado en la terminal se cortó | — | El `.lj` tiene que tener exactamente la sintaxis del lenguaje | — | **Corregido:** se descartaron esas versiones y los archivos se crearon con `printf` de una sola línea en la terminal. Se revisaron con `cat` antes de generar los `.out`. |
| La IA también preparó un `README.md` del proyecto actualizado al Hito 2 | — | El README del proyecto se actualiza una sola vez para todo el equipo | `README.md` (Claude) | **Rechazado:** Oscar ya había subido el README del Hito 2, así que no se subió para no reemplazar el suyo. Solo se agregó la tabla de `run1` a `run5` en `tests/propias/README.md`. |

## 6. Demo

- [ ] Preparada para 3 minutos.
- [ ] Cualquier integrante puede explicar cualquier parte.

Orden propuesto (unos 30 segundos por punto):

1. `make test-hito2`: 14/14 en verde.
2. `--run` con `tests/validos/05_shadowing.lj`: la `x` del bloque tapa a la de afuera y al cerrar el bloque vuelve la de afuera (entorno).
3. `--run` con `tests/propias/run1.lj`: un `while` que calcula el factorial (intérprete y evaluador).
4. `--check` con `tests/propias/invalido3.lj`: el `E01` sale porque el `if` no tiene `else` (análisis de flujo).
5. `--check` con `tests/propias/invalido5.lj`: solo sale el `S01`, sin `E01` en cascada.
6. Cerrar con cómo se dividió el trabajo usando contratos `.h`, igual que en los laboratorios.
