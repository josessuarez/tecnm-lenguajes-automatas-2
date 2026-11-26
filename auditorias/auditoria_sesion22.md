# Bitácora de Requerimientos y Auditoría IA — Sesión 22
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Generación de TAC para Expresiones  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Traductor de nodos AST a TAC  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Aquí lo difícil fue entender que el orden de las líneas no es el de lectura: en `a + b * c` primero sale `_t0 = b * c`. Lo resolví con una función recursiva en postorden: primero visito el hijo izquierdo, luego el derecho y al final emito la línea del operador. Cada llamada me regresa el "nombre" del resultado (el valor si es hoja o el temporal nuevo). Me atoré un rato porque declaraba el contador de temporales como `static` y en la segunda prueba ya no empezaba en `_t0`; lo cambié a una variable local que paso por puntero.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Conservación de precedencia de operadores |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo la parte de código intermedio (TAC) de un compilador
en C11, sesión 22: Generación de TAC para Expresiones. Se compila con gcc -std=c11 -Wall -Wextra -Werror
junto con un harness que ya tiene main(), así que no lleva main(). Solo
biblioteca estándar. Contrato (api_sesion22.h):

typedef struct Nodo {
    char tipo;
    char op;
    char valor[16];
    struct Nodo *izq;
    struct Nodo *der;
} Nodo;

int generar_tac(const Nodo *raiz, char lineas[][64], int max);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- Recorrido postorden: hijos primero, luego una línea "_tK = izq op der" por operador.
- Las hojas (tipo 'H') no generan líneas.
- Cada llamada empieza en _t0 (nada de static ni globales).
- Nunca escribir más de max líneas ni más de 31 caracteres por línea.
- Regresa el número de líneas escritas; raíz NULL o max <= 0 regresa 0.
- No cambies las firmas. Revisa todos los punteros NULL antes de usarlos.

OUTPUT: Solo el archivo ia_sesion22_c.c (con #include "api_sesion22.h"),
funciones auxiliares static y la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion22_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: no pide temporal si ya no hay espacio
 * y respeta el limite de 31 caracteres por linea)
 */
#include "api_sesion22.h"
#include <stddef.h>
#include <stdio.h>

#define NOMBRE 16
#define LARGO_MAX 31

typedef struct {
    char (*lineas)[64];
    int max;
    int usadas;
    int temporal;
} Gen;

/* Postorden: hijos primero, luego una linea por operador.
 * Regresa 1 si pudo generar todo el subarbol. O(n). */
static int emitir(Gen *g, const Nodo *n, char *nombre) {
    if (n == NULL) return 0;
    if (n->tipo == 'H') {
        snprintf(nombre, NOMBRE, "%s", n->valor);
        return 1;
    }
    char a[NOMBRE], b[NOMBRE];
    if (!emitir(g, n->izq, a) || !emitir(g, n->der, b)) return 0;
    if (g->usadas >= g->max) return 0;          /* sin espacio: no se emite */
    snprintf(nombre, NOMBRE, "_t%d", g->temporal++);
    char linea[64];
    snprintf(linea, sizeof linea, "%s = %s %c %s", nombre, a, n->op, b);
    linea[LARGO_MAX] = '\0';                    /* nunca mas de 31 caracteres */
    snprintf(g->lineas[g->usadas++], 64, "%s", linea);
    return 1;
}

/* Genera TAC desde cero (_t0) y regresa cuantas lineas escribio. O(n). */
int generar_tac(const Nodo *raiz, char lineas[][64], int max) {
    if (raiz == NULL || lineas == NULL || max <= 0) return 0;
    Gen g = { lineas, max, 0, 0 };
    char res[NOMBRE];
    emitir(&g, raiz, res);
    return g.usadas;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Expresiones anidadas con paréntesis

### Salida del harness
```text
=== AUDITORIA SESION 22 - Traductor de AST a TAC con precedencia ===
Caso                               esperado:              obtenido:              resultado
[ 1] a + b * c                          esperado: _t0 = b * c  obtenido: _t0 = b * c  OK
[ 2] a + b * c (2a linea)
     esperado: _t1 = a + _t0
     obtenido: _t1 = a + _t0
     -> OK
[ 3] (a + b) * c
     esperado: _t1 = _t0 * c
     obtenido: _t1 = _t0 * c
     -> OK
[ 4] x * (y - z) + w
     esperado: _t2 = _t1 + w
     obtenido: _t2 = _t1 + w
     -> OK
[ 5] hoja sin operaciones               esperado: 0            obtenido: 0            OK
[ 6] (a + b) * (c - d)
     esperado: _t2 = _t0 * _t1
     obtenido: _t2 = _t0 * _t1
     -> OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion22.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA juntó todo el estado (líneas, max, usadas y temporal) en un `struct Gen`, que se ve más ordenado que mis cuatro parámetros sueltos. Lo que omitió en su primera versión fue el límite de `max`: pedía un temporal nuevo aunque ya no hubiera espacio y seguía recorriendo. Le agregué que si ya no cabe la línea, regrese 0 y deje de emitir. También le puse el recorte a 31 caracteres que pide el contrato y que ni ella ni yo habíamos respetado. Complejidad O(n) en el número de nodos; sin fugas porque no usa memoria dinámica.

---
**Firma digital:** josessuarez
