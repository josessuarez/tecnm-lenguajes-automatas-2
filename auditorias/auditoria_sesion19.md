# Bitácora de Requerimientos y Auditoría IA — Sesión 19
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Estructuras (struct) y Miembros  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Offsets de miembros en estructuras  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Aquí tuve que acordarme de cómo acomoda la memoria la arquitectura: cada campo empieza en un múltiplo de su propio tamaño. Para `char a; int b;`, `a` queda en 0 y `b` no puede empezar en 1, se recorre hasta 4. Hice una función `alinear(pos, a)` que redondea hacia arriba con `(pos + a - 1) / a * a`. Lo que se me olvidó la primera vez fue el padding del final: `char, int, short` termina en 10, pero el tamaño real es 12, porque se redondea al múltiplo del campo más grande (4). Eso lo arreglé guardando el ancho mayor y redondeando al final. Lo comprobé contra `sizeof` y `offsetof` en gcc y dio lo mismo. El struct sin campos regresa 0, y si un tipo no se conoce se regresa -1.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Alineación de memoria por palabras |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy calculando la disposición en memoria de structs en un compilador
en C11 para x86-64. Se compila con gcc -std=c11 -Wall -Wextra -Werror
junto con un harness que ya tiene main(), así que no lleva main(). Solo
biblioteca estándar. Contrato (api_sesion19.h):

typedef struct Campo { const char *nombre; const char *tipo; } Campo;
int offset_campo(const Campo *campos, int total, const char *nombre);
int tamano_struct(const Campo *campos, int total);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- Cada campo se alinea a su propio ancho (char 1, short 2, int 4, float 4,
  long 8, double 8, puntero 8) antes de darle offset.
- offset_campo() regresa el offset del campo pedido después del relleno.
- tamano_struct() regresa el total redondeado al múltiplo del campo más
  ancho. Sin campos regresa 0.
- Campo inexistente, lista NULL o tipo desconocido regresan -1.
- No cambies las firmas ni la estructura.

OUTPUT: Solo el archivo ia_sesion19_c.c (con #include "api_sesion19.h") y la
complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion19_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: un tipo desconocido regresa -1 en lugar de
 * seguir sumando con ancho 0)
 */
#include "api_sesion19.h"
#include <stddef.h>
#include <string.h>

/* Ancho (= alineacion) de cada tipo en x86-64; -1 si no se conoce. O(1). */
static int ancho_tipo(const char *tipo) {
    static const struct { const char *n; int w; } tabla[] = {
        {"char", 1}, {"short", 2}, {"int", 4}, {"float", 4},
        {"long", 8}, {"double", 8}, {"puntero", 8}
    };
    if (tipo == NULL) return -1;
    for (size_t i = 0; i < sizeof tabla / sizeof tabla[0]; i++) {
        if (strcmp(tabla[i].n, tipo) == 0) return tabla[i].w;
    }
    return -1;
}

/* Redondea pos al siguiente multiplo de a (a > 0). */
static int redondear(int pos, int a) {
    int resto = pos % a;
    return resto == 0 ? pos : pos + (a - resto);
}

/* Offset del campo pedido despues de aplicar el relleno. O(n). */
int offset_campo(const Campo *campos, int total, const char *nombre) {
    if (campos == NULL || nombre == NULL || total < 0) return -1;
    int pos = 0;
    for (int i = 0; i < total; i++) {
        int w = ancho_tipo(campos[i].tipo);
        if (w < 0) return -1;
        pos = redondear(pos, w);
        if (campos[i].nombre != NULL && strcmp(campos[i].nombre, nombre) == 0) return pos;
        pos += w;
    }
    return -1;
}

/* Tamano total con padding final al multiplo del campo mas ancho. O(n). */
int tamano_struct(const Campo *campos, int total) {
    if (campos == NULL || total < 0) return -1;
    int pos = 0, max_al = 1;
    for (int i = 0; i < total; i++) {
        int w = ancho_tipo(campos[i].tipo);
        if (w < 0) return -1;
        pos = redondear(pos, w) + w;
        if (w > max_al) max_al = w;
    }
    return total == 0 ? 0 : redondear(pos, max_al);
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Acceso a miembros inexistentes

### Salida del harness
```text
=== AUDITORIA SESION 19 - Offsets de miembros y tamano de struct ===
Caso                               esperado:              obtenido:              resultado
[ 1] struct char e int: b               esperado: 4            obtenido: 4            OK
[ 2] struct de tres campos: c           esperado: 8            obtenido: 8            OK
[ 3] tamano total con padding           esperado: 12           obtenido: 12           OK
[ 4] double seguido de char             esperado: 16           obtenido: 16           OK
[ 5] miembro inexistente                esperado: -1           obtenido: -1           OK
[ 6] struct sin campos                  esperado: 0            obtenido: 0            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion19.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA, en `offset_campo()`, se saltaba los tipos desconocidos y seguía sumando con ancho 0, así que un struct con un tipo inventado daba offsets que no existen en lugar de -1. El contrato dice que un tipo desconocido regresa -1, así que lo cambié para que se detenga ahí. En `tamano_struct()` tenía el caso de cero campos bien, pero si le pasabas `total = 0` con `campos = NULL` regresaba 0; lo dejé en -1 porque la lista NULL es inválida según el contrato. Comparé los cuatro casos del harness contra `sizeof` y `offsetof` reales de gcc en x86-64 y coinciden (4, 8, 12 y 16). Sanitizers limpios. Las dos funciones son O(n) en el número de campos.

---
**Firma digital:** josessuarez
