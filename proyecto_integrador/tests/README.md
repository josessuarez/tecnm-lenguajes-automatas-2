# Pruebas doradas de Lenguajito

Estos archivos son el **contrato de comportamiento** del compilador. No se
modifican: si cambias un programa o su salida esperada, el criterio "Pruebas" del
hito vale 0.

## Cómo se ejecutan

Desde `proyecto_integrador/`:

```bash
make            # construye ./lenguajito
make test       # todas las pruebas
make test-hito1 # solo las del Hito 1
make test-hito2 # Hito 2
make test-hito3 # Hito 3
make test-hito4 # Hito 4
```

## Qué hay aquí

| Carpeta | Qué prueba | Cómo |
| :--- | :--- | :--- |
| `hito1/*.lj` | AST de expresiones, declaraciones y asignaciones | `--ast` comparado con `.ast` |
| `hito1/*.err` | error de sintaxis en el Hito 1 | `--check` con código y línea |
| `validos/*.lj` | programas correctos | `--run` comparado con `.out` |
| `invalidos/*.lj` | errores semánticos | `--check` con código y línea |
| `tac/*.lj` | plegado de constantes | `--tac` sin operaciones entre constantes |

Los códigos de error (`S01`, `E01`, `E02`, `E04`) están definidos en
[`docs/gramatica.md`](../docs/gramatica.md).

## Agregar pruebas propias

Crea tus programas en otra carpeta (por ejemplo `tests/propias/`) y ejecuta tu
propio script. Las carpetas `validos/`, `invalidos/`, `hito1/` y `tac/` son
doradas: se comparan contra la especificación, no contra tu implementación.
