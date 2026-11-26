# Lenguajito · especificación del lenguaje

**Proyecto Integrador · Lenguajes y Autómatas II (SCD-1016)**

Este documento es el **contrato** del lenguaje que debe compilar tu equipo. Si tu
compilador y este documento no coinciden, el documento manda: los programas de
prueba (`proyecto_integrador/tests/`) están escritos contra esta especificación.

---

## 1. Un programa completo

```c
// suma de los primeros 5 enteros
int suma = 0;
int i = 1;

while (i <= 5) {
    suma = suma + i;
    i = i + 1;
}

print(suma);   // imprime 15
```

Un programa es una secuencia de sentencias. No hay función `main`: el programa
completo es el cuerpo de `main`.

---

## 2. Tokens

| Categoría | Tokens |
| :--- | :--- |
| Palabras reservadas | `int`, `if`, `else`, `while`, `print` |
| Identificadores | `[a-zA-Z_][a-zA-Z0-9_]*` |
| Enteros | `[0-9]+` (sin signo; el signo es un operador unario) |
| Operadores | `+` `-` `*` `/` `<` `<=` `>` `>=` `==` `!=` `=` |
| Delimitadores | `(` `)` `{` `}` `;` |
| Comentarios | `//` hasta el final de la línea |
| Espacios | espacio, tabulador, salto de línea (se ignoran) |

Todo lo demás es un **error léxico** (`S01`). Los identificadores distinguen
mayúsculas de minúsculas (`suma` y `Suma` son distintos).

Para `--tokens`, cada línea es `LINEA:COLUMNA TIPO lexema`, con `TIPO` en:

| Tipo | Qué incluye |
| :--- | :--- |
| `KW` | `int`, `if`, `else`, `while`, `print` |
| `ID` | identificadores |
| `NUM` | literales enteros |
| `OP` | operadores y `=` |
| `DELIM` | `(`, `)`, `{`, `}`, `;` |
| `EOF` | fin de archivo (una sola línea, al final) |

---

## 3. Gramática

Notación EBNF: `{ x }` = cero o más, `[ x ]` = opcional, `|` = alternativa.

```ebnf
programa        := { sentencia } EOF
sentencia       := declaracion
                 | asignacion
                 | seleccion
                 | iteracion
                 | impresion
                 | bloque
declaracion     := "int" IDENT [ "=" expresion ] ";"
asignacion      := IDENT "=" expresion ";"
seleccion       := "if" "(" expresion ")" bloque [ "else" bloque ]
iteracion       := "while" "(" expresion ")" bloque
impresion       := "print" "(" expresion ")" ";"
bloque          := "{" { sentencia } "}"

expresion       := comparacion
comparacion     := aditiva [ ( "<" | "<=" | ">" | ">=" | "==" | "!=" ) aditiva ]
aditiva         := multiplicativa { ( "+" | "-" ) multiplicativa }
multiplicativa  := unaria { ( "*" | "/" ) unaria }
unaria          := [ "-" | "+" ] primaria
primaria        := NUMERO | IDENT | "(" expresion ")"
```

Notas para el analizador:

- Es **LL(1)**: con un token de anticipación se decide siempre.
- Las llaves de `if` y `while` son obligatorias. Eso elimina el *dangling else* y
  simplifica el parser.
- La precedencia ya está en la gramática: paréntesis > unario > `* /` > `+ -` >
  comparaciones. Los operadores binarios son **asociativos a la izquierda**.
- `if` y `while` reciben una **sentencia**, que en la práctica es un bloque.

---

## 4. AST y formato `--ast`

El compilador debe poder imprimir el árbol en una sola línea, con paréntesis y
sin espacios de más. Ese formato es el que usan las pruebas del Hito 1.

| Construcción | Formato |
| :--- | :--- |
| Programa | `(programa SENTENCIA...)` |
| Declaración | `(declarar x)` o `(declarar x EXPR)` |
| Asignación | `(asignar x EXPR)` |
| `if` sin else | `(si COND (entonces SENT))` |
| `if` con else | `(si COND (entonces SENT) (si_no SENT))` |
| `while` | `(mientras COND SENT)` |
| Bloque | `(bloque SENTENCIA...)` |
| `print` | `(imprimir EXPR)` |
| Binaria | `(OP IZQ DER)` con `OP` = `+ - * / < <= > >= == !=` |
| Unaria | `(neg EXPR)` |
| Número | `(num 42)` |
| Variable | `(var x)` |

Ejemplos exactos:

```text
int x = 1 + 2 * 3;
```
```text
(programa (declarar x (+ (num 1) (* (num 2) (num 3)))))
```

```text
x = (1 + 2) * 3;
```
```text
(programa (asignar x (* (+ (num 1) (num 2)) (num 3))))
```

```text
if (x < 3) { print(1); } else { print(0); }
```
```text
(programa (si (< (var x) (num 3)) (entonces (bloque (imprimir (num 1)))) (si_no (bloque (imprimir (num 0))))))
```

Declaración sin inicializar: `int y;` → `(programa (declarar y))`.

---

## 5. Semántica

- **Tipo único:** todas las variables son `int` de 32 bits. Los valores de verdad
  son enteros: `0` es falso y cualquier otro valor es verdadero.
- **Comparaciones:** devuelven `1` (verdadero) o `0` (falso).
- **Declaración obligatoria:** una variable debe declararse antes de usarse.
- **Ámbitos:** el programa tiene un ámbito global; cada bloque `{ }` abre un ámbito
  nuevo. Al cerrarlo, sus variables dejan de existir.
- **Shadowing:** se permite redeclarar un nombre en un ámbito interno; la versión
  interna oculta a la externa mientras el bloque esté activo.
- **Redeclaración:** declarar dos veces el mismo nombre **en el mismo ámbito** es
  error (`E02`).
- **Inicialización:** una variable declarada sin `=` no se puede leer hasta que se
  le asigne un valor. Leerla antes es un error de compilación (`E01`). (Decisión
  simple: no hay valores basura.)
- **División entre cero:** si el divisor es una constante `0`, es error de
  compilación (`E04`). En tiempo de ejecución, los programas de prueba no dividen
  entre cero; no se exige manejarlo.
- **`print(e)`:** imprime el valor de `e` en decimal seguido de `\n` en `stdout`.
- **Resultado del programa:** termina con código 0 si llega al final.

---

## 6. Diagnósticos

Formato **exacto** de cada error (uno por línea):

```text
archivo:línea:columna: error EXX: mensaje
```

- `archivo` es la ruta tal como se pasó en la línea de comandos.
- `línea` y `columna` empiezan en 1.
- Si hay varios errores, se imprimen todos (máximo 20) y el compilador termina con
  código 1.

| Código | Cuándo |
| :--- | :--- |
| `S01` | error de sintaxis o carácter inválido (se esperaba otra cosa) |
| `E01` | variable no declarada o leída antes de tener valor |
| `E02` | redeclaración en el mismo ámbito |
| `E04` | división entre cero constante |

Ejemplos:

```text
programa.lj:1:7: error E01: variable no declarada 'x'
programa.lj:2:5: error E02: redeclaración de 'x' en el mismo ámbito
programa.lj:1:12: error E04: división entre cero constante
programa.lj:1:9: error S01: se esperaba una expresión
```

El texto después de `EXX:` es libre; lo que **no** es libre es el formato, el código
y la línea.

---

## 7. Interfaz de línea de comandos

```bash
./lenguajito ARCHIVO --tokens   # lista de tokens
./lenguajito ARCHIVO --ast      # AST en el formato de la sección 4
./lenguajito ARCHIVO --check    # solo análisis; 0 si no hay errores, 1 si hay
./lenguajito ARCHIVO --tac      # código de tres direcciones (optimizado)
./lenguajito ARCHIVO --asm      # ensamblador x86-64 (AT&T)
./lenguajito ARCHIVO --run      # compila, enlaza y ejecuta; reenvía la salida
```

- Sin modo, se asume `--check`.
- `--check` imprime los diagnósticos en `stdout` y no genera archivos.
- `--tokens` imprime un token por línea: `LINEA:COLUMNA TIPO lexema`.
- `--run` debe ejecutar el **binario generado** (no un intérprete) a partir del
  Hito 4; antes de eso puede interpretar el AST para probar la semántica.
- Códigos de salida del compilador: `0` todo bien, `1` errores de compilación,
  `2` uso incorrecto de la línea de comandos.

---

## 8. Código de tres direcciones (`--tac`)

Una instrucción por línea, en este formato:

| Forma | Significado |
| :--- | :--- |
| `x = a op b` | operación binaria (`op` es `+ - * / < <= > >= == !=`) |
| `x = a` | copia |
| `x = -a` | negación |
| `goto L1` | salto incondicional |
| `ifFalso a goto L1` | salta si `a` vale 0 |
| `L1:` | etiqueta de destino |
| `print a` | impresión |

- Los temporales se llaman `_t0`, `_t1`, ... y no se reutilizan.
- Las variables conservan su nombre.
- `--tac` imprime el código **después** de las optimizaciones.

Ejemplo para `if (x < 3) { print(1); } else { print(0); }`:

```text
_t0 = x < 3
ifFalso _t0 goto L1
print 1
goto L2
L1:
print 0
L2:
```

Ejemplo para `while (i < 3) { print(i); i = i + 1; }`:

```text
L1:
_t0 = i < 3
ifFalso _t0 goto L2
print i
_t1 = i + 1
i = _t1
goto L1
L2:
```

---

## 9. Lo que el lenguaje **no** incluye

Para que quepa en el semestre, la versión base **no** tiene: `float`, arreglos,
`struct`, funciones, `for`, `do-while`, `break`, `continue`, `return`, entrada de
usuario ni cadenas. Cualquiera de esas cosas es una **extensión** (sección 10 del
`PROYECTO.md`) y requiere autorización del docente antes de implementarse.

---

## 10. Programa de prueba comentado

```c
// ¿cuántas veces cabe 3 en 10?
int n = 10;
int veces = 0;

while (n >= 3) {
    n = n - 3;
    veces = veces + 1;
}

print(veces);   // 3
print(n);       // 1
```

Salida esperada:

```text
3
1
```
