# Entrega · Hito 1

**Equipo:** José Manuel, Oscar Mauricio, María Jesús, Jordi Geovany y Héctor Iván  
**Repositorio:** https://github.com/jordipech76-boop/tecnm-lenguajes-automatas-2  
**Tag:** `v0.1-hito1`  
**Fecha:** 2026-11-05

---

## 1. Qué funciona

```bash
make
make test-hito1
./lenguajito tests/propias/valido1.lj --tokens
./lenguajito tests/propias/valido2.lj --ast
./lenguajito tests/propias/invalido2.lj --check
```

- `--tokens` imprime un token por línea (`línea:columna TIPO lexema`) y `EOF` al final.
- `--ast` imprime el árbol en una línea con el formato de la sección 4 de `gramatica.md`.
- `--check` reporta `S01`, `E01`, `E02` y `E04` en `stdout`, varios en una pasada, y sale con 1.

## 2. Qué no funciona (y por qué)

- [ ] `--run`, `--tac` y `--asm`: son de los Hitos 2, 3 y 4.
- [ ] Después de un error de sintaxis se descarta la sentencia completa, así que puede salir un `E01` extra sobre esa variable.

## 3. Pruebas

```text
Hito 1: 4/4 pruebas superadas
```

Pruebas propias en `tests/propias/` (detalle en su `README.md`):

- `valido1.lj`, `valido2.lj`, `valido3.lj`: `while`, precedencia y unario, `if/else` con bloques; su `--ast` esperado está en los `.ast`.
- `invalido1.lj`: caracteres inválidos (`S01`).
- `invalido2.lj`: `E01`, `E02` y `E04` en una sola pasada.

## 4. Reparto de trabajo

| Integrante | Archivos / etapas | Commits principales |
| :--- | :--- | :--- |
| María Jesús | `lexer.c`, `lexer.h`, `docs/avances.md` | `b066d71`, `b0681e0`, `79864aa` |
| Oscar Mauricio | `parser.c`: expresiones (`comparacion` a `primaria`) | `79864aa` |
| Jordi Geovany | `parser.c`: sentencias; `ast.c`, `ast.h`; `compilador.c` | `b066d71`, `79864aa` |
| José Manuel | `semantica.c`, `semantica.h` (`--check`) | `79864aa` |
| Héctor Iván | `tests/propias/`, `README.md`, esta bitácora | `b066d71`, `79864aa`, `d6ae8b3` |

Los commits del Hito 1 se subieron desde la cuenta de Jordi, cargando los archivos por la web de GitHub. El reparto real es el de esta tabla; a partir del Hito 2 cada integrante sube su parte desde su propia cuenta.

## 5. Bitácora C-R-E-O (IA)

| Contexto | Rol | Especificación | Output | ¿Qué aceptaron, corrigieron o rechazaron? |
| :--- | :--- | :--- | :--- | :--- |
| Avance 1: faltaba el lexer y conectarlo con `compilador.c` del curso | Programador de C y compiladores | Tokens de la sección 2 de `gramatica.md`, formato `línea:columna TIPO lexema`, `S01` con línea y columna | `lexer.c`, `lexer.h` y `compilador.c` (Claude) | **Aceptado** para el Avance 1. En el Hito 1 se cambió por la versión de la fila siguiente, que guarda los caracteres inválidos como `TOK_ERROR` para que el parser los pueda reportar en `--check`. |
| Hito 1: faltaba el parser, el AST y `--check` | Ingeniero de compiladores en C11 | Gramática de la sección 3, formato `--ast` de la sección 4, diagnósticos de la sección 6, solo `libc` | `ast.h`, `ast.c`, `parser.c`, un `lexer.c` nuevo y un `compilador.h` modificado (Claude) | **Aceptado:** parser LL(1) con ciclos `while` para la asociatividad a la izquierda, `+` unario sin nodo y recuperación en `;` y `}`. Pasa `make test-hito1` 4/4. |
| La propuesta cambiaba `compilador.h` a `compilar(const char *, ModoCompilador)` | — | `main.c` del curso "ya viene resuelto" y llama `compilar(ruta, modo)` con el modo como texto | — | **Rechazado:** con ese cambio `main.c` no compila. Se dejó `compilador.h` original y `compilador.c` compara el modo con `strcmp`. |
| El lexer propuesto imprimía `EOF EOF` y no imprimía `EOF` si había errores | — | La sección 2 pide `EOF` en una sola línea al final | — | **Corregido:** ahora siempre imprime `línea:columna EOF`. |
| Los modos sin implementar respondían con `printf` y salían con 0 | — | La sección 7: `0` es todo bien | — | **Corregido:** el mensaje va a `stderr` y sale con 1, para que nadie crea que funcionó. |
| Con un carácter inválido en medio de una expresión salían dos `S01` en el mismo lugar y ninguno decía cuál carácter | — | Un diagnóstico claro por error | — | **Corregido:** se agregó `report_invalid_char()` en `parser.c`, que reporta una vez el carácter (`'@'`) y lo salta. |
| Los tipos de token estaban en `ast.h` y la semántica dentro de `parser.c` | — | `PROYECTO.md` pide separar las etapas para poder explicarlas y probarlas por separado | — | **Modificado:** se separó en `lexer.h`, `parser.c` y `semantica.c` sin cambiar el comportamiento. |


