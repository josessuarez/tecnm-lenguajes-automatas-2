# Compilador de Lenguajito · Proyecto Integrador

**Curso:** Lenguajes y Autómatas II (SCD-1016)  
**Equipo:** José Manuel, Oscar Mauricio, María Jesús, Jordi Geovany y Héctor Iván  
**Tag actual:** `v0.1-hito1` (el Hito 2 se cierra con `v0.2-hito2`)

Compilador escrito en C11 (`gcc -std=c11 -Wall -Wextra -Werror`, solo `libc`) con un parser descendente recursivo LL(1) hecho a mano. Las reglas están en [`../PROYECTO.md`](../PROYECTO.md) y el lenguaje en [`docs/gramatica.md`](docs/gramatica.md).

## Cómo se usa

```bash
cd proyecto_integrador
make                 # construye ./lenguajito
make test-hito1      # pruebas del Hito 1 (4/4)
make test-hito2      # pruebas del Hito 2 (14/14)

./lenguajito tests/propias/valido1.lj --tokens     # un token por línea
./lenguajito tests/propias/valido2.lj --ast        # árbol en una línea
./lenguajito tests/propias/invalido2.lj --check    # diagnósticos, sale con 1
./lenguajito tests/validos/05_shadowing.lj --run   # ejecuta el programa: imprime 2 y 1
```

## Qué funciona

### Hito 1

- `--tokens`: `línea:columna TIPO lexema`, ignora espacios y comentarios `//`, termina en `EOF` y marca `S01` en caracteres inválidos.
- `--ast`: declaraciones, asignaciones, `if/else`, `while`, `print`, bloques y expresiones con precedencia; el `-` unario es `(neg ...)` y el `+` unario no genera nodo.
- `--check`: `S01` de sintaxis con recuperación (varios errores por pasada, máximo 20), `E01`, `E02` y `E04`, en `stdout` con el formato `archivo:línea:columna: error EXX: mensaje`.

### Hito 2

- `--run`: ejecuta el programa interpretando el AST. Primero hace la misma revisión de `--check`; si hay errores los imprime y sale con 1 sin ejecutar nada. Si no, recorre las sentencias y cada `print` escribe el valor seguido de `\n`.
  - **Entorno** (`entorno.c`): pila de ámbitos con el valor de cada variable. Cada bloque abre un ámbito y al cerrarlo sus variables desaparecen; la variable interna oculta a la externa (shadowing).
  - **Evaluador** (`evaluador.c`): números, variables, `neg`, `+ - * /` y las comparaciones, que dan 1 o 0. Revisa el divisor antes de dividir y las cuentas dan la vuelta como un `int` de 32 bits en vez de ser comportamiento indefinido.
  - **Intérprete** (`interprete.c`): declaraciones (sin valor quedan en 0), asignaciones, `print`, bloques, `if/else` y `while`, que vuelve a evaluar la condición en cada vuelta.
- Errores en ejecución: una división entre cero que no es constante (por ejemplo, `int z = 0; print(10 / z);`) pasa `--check`, pero `--run` la detecta, escribe `línea:columna: error en ejecucion: division entre cero` en `stderr` y sale con 1.
- **Inicialización sensible al flujo** en `--check`: lo que se asigna dentro de un `if` sin `else` o dentro de un `while` no cuenta después; en un `if/else`, la variable cuenta como asignada solo si se asignó en las dos ramas. Leerla antes da `E01`.
- **Sin E01 en cascada**: si una declaración falla después de leer el nombre (como `int z = 5 # 2;`), el parser la conserva marcada con `con_error`, así que usar `z` después ya no da un `E01` de más; solo sale el `S01` real.

## Qué no funciona todavía

- `--tac` (Hito 3) y `--asm` (Hito 4): responden "todavía sin implementar" y salen con 1.
- `E04` solo detecta la división entre una **constante** cero, como pide la especificación; con una variable que vale 0, el error aparece hasta `--run`.

## Pruebas

- Pruebas del curso: `make test-hito1` (4/4) y `make test-hito2` (14/14: los 9 programas de `tests/validos/` con `--run` y los 5 de `tests/invalidos/` con `--check`).
- Pruebas propias en [`tests/propias/`](tests/propias/README.md): `valido1` a `valido3` (`--ast`), `invalido1` e `invalido2` (`--check`), `run1` a `run5` (`--run` con su `.out`) y los casos de flujo y de recuperación (`invalido3` a `invalido5` y `valido4`).

## Archivos

| Archivo | Qué hace | Responsable |
| :--- | :--- | :--- |
| `src/main.c`, `src/compilador.h` | interfaz de línea de comandos (vienen del curso) | — |
| `src/compilador.c` | conecta las etapas según el modo, incluido `--run` | Jordi |
| `src/lexer.h`, `src/lexer.c` | tokens con línea y columna | María |
| `src/parser.h`, `src/parser.c` | parser LL(1): expresiones (Oscar), sentencias (Jordi) y declaraciones con `con_error` (Héctor) | Oscar, Jordi y Héctor |
| `src/ast.h`, `src/ast.c` | nodos del AST, impresión `--ast` y liberación de memoria | Jordi |
| `src/semantica.h`, `src/semantica.c` | tabla de símbolos con ámbitos y `--check` (José Manuel); inicialización sensible al flujo (Héctor) | José Manuel y Héctor |
| `src/entorno.h`, `src/evaluador.h`, `src/interprete.h` | contratos de las piezas de `--run` | Oscar |
| `src/entorno.c` | valores de las variables por ámbito | José Manuel |
| `src/evaluador.c` | cálculo de expresiones | Oscar |
| `src/interprete.c` | ejecución de las sentencias | Jordi |
| `tests/propias/` | pruebas del equipo: Hito 1 (Héctor), `run1` a `run5` (María), flujo y recuperación (Héctor) | Héctor y María |
| `docs/avances.md`, `docs/entrega_hito1.md` | avances y bitácora de IA | María y Héctor |
