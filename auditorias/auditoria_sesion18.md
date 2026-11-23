# Bitácora de Requerimientos y Auditoría IA — Sesión 18
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Arreglos y Verificación de Límites  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Cálculo de tamaño de arreglo en memoria  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> El cálculo es `elementos * ancho`, pero lo que cuidé fue el tipo del resultado: si multiplico dos `int` y el arreglo es enorme, se desborda antes de convertirse a `long`. Por eso convierto los dos a `long` antes de multiplicar. Los anchos los puse con `strcmp` uno por uno (char 1, short 2, int 4, float 4, double 8 y puntero 8), y cualquier otro nombre regresa -1. Cero elementos es válido y pesa 0 bytes; solo los negativos son error. En `indice_en_rango()` el detalle es el borde: el último índice válido es `elementos - 1`, así que la comparación es `indice < elementos` y no `<=`. Los índices negativos los rechazo primero.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Indexación entera no negativa |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy calculando el tamaño de arreglos y validando índices en un
compilador en C11. Se compila con gcc -std=c11 -Wall -Wextra -Werror
junto con un harness que ya tiene main(), así que no lleva main(). Solo
biblioteca estándar. Contrato (api_sesion18.h):

long bytes_arreglo(const char *tipo, int elementos);
int indice_en_rango(int indice, int elementos);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- bytes_arreglo() regresa elementos * ancho del tipo. Tipos válidos: char
  (1), short (2), int (4), float (4), double (8) y puntero (8). Otro nombre,
  tipo NULL o elementos negativos regresan -1. Cero elementos pesa 0.
- La multiplicación no se debe desbordar en int.
- indice_en_rango() regresa 1 si 0 <= indice < elementos y 0 si no. El
  índice igual a elementos y los negativos se rechazan.
- No cambies las firmas.

OUTPUT: Solo el archivo ia_sesion18_c.c (con #include "api_sesion18.h") y la
complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion18_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: la multiplicacion se hace en long)
 */
#include "api_sesion18.h"
#include <stddef.h>
#include <string.h>

typedef struct { const char *nombre; int bytes; } TipoBase;

static const TipoBase TIPOS[] = {
    {"char", 1}, {"short", 2}, {"int", 4},
    {"float", 4}, {"double", 8}, {"puntero", 8}
};

/* Ancho en bytes del tipo; -1 si no se conoce. O(1). */
static int ancho_tipo(const char *tipo) {
    for (size_t i = 0; i < sizeof TIPOS / sizeof TIPOS[0]; i++) {
        if (strcmp(TIPOS[i].nombre, tipo) == 0) return TIPOS[i].bytes;
    }
    return -1;
}

/* elementos * ancho; -1 si el tipo no existe o la dimension es negativa. O(1). */
long bytes_arreglo(const char *tipo, int elementos) {
    if (tipo == NULL || elementos < 0) return -1;
    int w = ancho_tipo(tipo);
    if (w < 0) return -1;
    return (long)elementos * (long)w;
}

/* 1 si 0 <= indice < elementos, 0 en otro caso. O(1). */
int indice_en_rango(int indice, int elementos) {
    return (indice >= 0 && indice < elementos) ? 1 : 0;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Límites estáticos y dimensiones inválidas

### Salida del harness
```text
=== AUDITORIA SESION 18 - Tamano de arreglo y limites ===
Caso                               esperado:              obtenido:              resultado
[ 1] int[10]                            esperado: 40           obtenido: 40           OK
[ 2] double[3]                          esperado: 24           obtenido: 24           OK
[ 3] int[-4]                            esperado: -1           obtenido: -1           OK
[ 4] indice 0 de 5                      esperado: 1            obtenido: 1            OK
[ 5] indice 5 de 5                      esperado: 0            obtenido: 0            OK
[ 6] indice negativo                    esperado: 0            obtenido: 0            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion18.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La primera respuesta multiplicaba `elementos * w` como `int` y después lo convertía a `long` en el `return`. Con los casos del harness no se nota, pero con algo como `double[300000000]` el producto ya se desbordó antes de la conversión y sale un número negativo. Lo corregí haciendo el cast antes de multiplicar. También revisé que `indice_en_rango` no use resta (`indice <= elementos - 1`), porque con `elementos = INT_MIN` la resta se desborda; la comparación directa `indice >= 0 && indice < elementos` no tiene ese problema. Lo pasé por `-fsanitize=address,undefined` y quedó limpio. Las dos funciones son O(1).

---
**Firma digital:** josessuarez
