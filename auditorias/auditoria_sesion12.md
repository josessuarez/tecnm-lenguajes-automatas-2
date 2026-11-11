# Bitácora de Requerimientos y Auditoría IA — Sesión 12
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Atributos de Símbolos y Memoria  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Estructura de registro de símbolos  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Al principio pensé que el offset era nada más ir sumando tamaños, pero con `char` y luego `int` eso da offset 1 para el int, y el harness espera 4. Ahí entendí lo de la alineación: cada tipo tiene que empezar en una dirección que sea múltiplo de su tamaño. Entonces si el cursor va en 1 y toca un `int`, lo subo al siguiente múltiplo de 4, que es 4, y dejo 3 bytes de relleno. Para redondear usé el residuo: si `cursor % ancho` no es 0, le sumo lo que le falta. La función regresa `offset + ancho`, que es donde va a empezar la siguiente variable. También valido que el registro no sea NULL, que el cursor no sea negativo y que el tipo exista, y en esos casos regreso -1 sin tocar nada.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Cálculo de desplazamientos y alineación |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy calculando el marco de pila en un compilador en C11. Se
compila con gcc -std=c11 -Wall -Wextra -Werror junto con un harness que ya
tiene main(), así que no lleva main(). Solo biblioteca estándar. Contrato
(api_sesion12.h):

typedef struct Simbolo { const char *nombre; const char *tipo;
                         int offset; } Simbolo;
int ancho_de_tipo(const char *tipo);
int asignar_offset(Simbolo *simbolo, int cursor);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- Anchos: char=1, short=2, int=4, float=4, double=8, puntero=8; cualquier
  otro tipo devuelve -1.
- asignar_offset: alinear el cursor al siguiente múltiplo del ancho del
  tipo, guardar ese offset en el registro y devolver offset + ancho.
- El cursor nunca retrocede.
- Registro NULL, cursor negativo o tipo desconocido devuelven -1 sin
  escribir en el registro.
- No cambies las firmas ni la estructura.

OUTPUT: Solo el archivo ia_sesion12_c.c (con #include "api_sesion12.h") y la
complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
#include "api_sesion12.h"
#include <limits.h>
#include <stddef.h>
#include <string.h>

/* Tabla de tipos con su ancho en bytes (alineacion natural = ancho). */
static const struct { const char *nombre; int ancho; } TIPOS[] = {
    { "char", 1 }, { "short", 2 }, { "int", 4 },
    { "float", 4 }, { "double", 8 }, { "puntero", 8 },
};

/* O(t), t = numero de tipos (constante, 6). */
int ancho_de_tipo(const char *tipo) {
    if (tipo == NULL) return -1;
    for (size_t i = 0; i < sizeof TIPOS / sizeof TIPOS[0]; i++)
        if (strcmp(TIPOS[i].nombre, tipo) == 0)
            return TIPOS[i].ancho;
    return -1;
}

/* Alinea el cursor al ancho del tipo, guarda el offset en el registro y
 * devuelve donde empieza la siguiente variable. O(1).
 * Devuelve -1 (sin tocar el registro) si algo es invalido o habria overflow. */
int asignar_offset(Simbolo *simbolo, int cursor) {
    if (simbolo == NULL || cursor < 0) return -1;

    int ancho = ancho_de_tipo(simbolo->tipo);
    if (ancho < 0) return -1;

    /* ancho siempre es potencia de 2: redondeo con mascara */
    if (cursor > INT_MAX - 2 * ancho) return -1;
    int offset = (cursor + ancho - 1) & ~(ancho - 1);

    simbolo->offset = offset;
    return offset + ancho;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Alineación de tipos y tamaños en bytes

### Salida del harness
```text
=== AUDITORIA SESION 12 - Registro de simbolo y offsets alineados ===
Caso                               esperado:              obtenido:              resultado
[ 1] ancho_de_tipo("int")               esperado: 4            obtenido: 4            OK
[ 2] ancho_de_tipo("double")            esperado: 8            obtenido: 8            OK
[ 3] int en cursor 0                    esperado: 4            obtenido: 4            OK
[ 4] char e int: offset de b            esperado: 4            obtenido: 4            OK
[ 5] double desde cursor 4              esperado: 8            obtenido: 8            OK
[ 6] registro NULL                      esperado: -1           obtenido: -1           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion12.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> El caso que no cuidaba la IA fue el desbordamiento del cursor. Para redondear usó la fórmula `(cursor + ancho - 1) & ~(ancho - 1)`, que funciona porque todos los anchos son potencias de 2, pero si el cursor viene muy cerca de `INT_MAX` la suma se pasa del rango de `int` y eso es comportamiento indefinido en C, el offset puede salir negativo y el cursor "retrocede", que es justo lo que el contrato prohíbe. El harness no lo prueba porque usa cursores chicos (0 y 4). Lo arreglé revisando `cursor > INT_MAX - 2 * ancho` antes de calcular y regresando -1 en ese caso. También comprobé que con tipo `NULL` no truene, porque `strcmp` con NULL rompe el programa. Las dos funciones son O(1): la tabla de tipos siempre tiene 6 elementos.

---
**Firma digital:** josessuarez
