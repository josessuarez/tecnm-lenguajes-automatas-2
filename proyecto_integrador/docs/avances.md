# Avances del proyecto

**Equipo:** José Manuel, Oscar Mauricio, María Jesús, Jordi Geovany y Héctor Iván  
**Repositorio:** https://github.com/jordipech76-boop/tecnm-lenguajes-automatas-2  
**Tag actual:** `v0.1-hito1`

Completa una sección antes de cada clase de avance. Tres bullets y el commit.

---

## Avance 0 · Sesión 05 · Hito 0

1. **Qué compila hoy** (comando exacto): `cd proyecto_integrador && make` genera `./lenguajito` sin warnings.
2. **Qué falla y por qué**: nada del compilador todavía; solo estaba la estructura que da el curso.
3. **Qué sigue**: leer `docs/gramatica.md`, repartir roles y empezar el lexer.
4. **Commit**: `361f194`

---

## Avance 1 · Sesión 08

1. **Qué compila hoy** (comando exacto): `make` y `./lenguajito tests/propias/valido1.lj --tokens` imprime un token por línea con `línea:columna TIPO lexema` y `EOF` al final. Ignora espacios y comentarios `//`.
2. **Qué falla y por qué**: el lexer ya marca `S01` con línea y columna (`./lenguajito tests/propias/invalido1.lj --tokens`), pero `--ast`, `--check` y los demás modos todavía dicen "sin implementar" porque falta el parser.
3. **Qué sigue**: parser descendente recursivo de expresiones, declaraciones y asignaciones, y `--ast`, para tener `make test-hito1` en verde en la sesión 10.
4. **Commit**: `b066d71`

---

## Avance 2 · Sesión 15

1. **Qué compila hoy** (comando exacto): `make`; `./lenguajito tests/propias/valido3.lj --ast` imprime el árbol completo con `if/else`, `while` y una variable declarada dentro de un bloque, y `./lenguajito tests/propias/invalido2.lj --check` reporta `E01`, `E02` y `E04` en una sola pasada usando la tabla de símbolos con ámbitos anidados.
2. **Qué falla y por qué**: `--run` todavía responde "sin implementar", así que las pruebas de `tests/validos/` no pasan. Además, la inicialización no sigue el flujo del programa: una asignación dentro de un `if` marca la variable como inicializada aunque el `if` no se cumpla.
3. **Qué sigue**: un intérprete que recorra el AST para `--run` (valores por ámbito, comparaciones que dan 1 o 0 y `print`), para tener `make test-hito2` en verde en la sesión 20.
4. **Commit**: `79864aa`

---

## Avance 3 · Sesión 25

1. **Qué compila hoy** (comando exacto):
2. **Qué falla y por qué**:
3. **Qué sigue**:
4. **Commit**:

---

## Avance 4 · Sesión 35

1. **Qué compila hoy** (comando exacto):
2. **Qué falla y por qué**:
3. **Qué sigue**:
4. **Commit**:
