# Bitácora de Requerimientos y Auditoría IA — Sesión 27
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Indexación de Arreglos en TAC  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Cálculo de desplazamientos base + offset  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Aquí lo importante es no olvidar multiplicar por el ancho del elemento: `A[i]` con enteros de 4 bytes es `_t0 = i * 4` y luego `_t1 = A[_t0]`. En 2D me costó más: primero saco el índice lineal `i * filas + j` (en ese orden, como dice el contrato), después lo multiplico por el ancho y al final leo `M[_t2]`. Cada paso pide un temporal nuevo, así que voy subiendo `*temporales` cuatro veces. Al principio intercambié `i` y `j` y salía otra celda; con el harness lo vi luego luego.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Aplanado de direcciones unidimensionales |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo la parte de código intermedio (TAC) de un compilador
en C11, sesión 27: Indexación de Arreglos en TAC. Se compila con gcc -std=c11 -Wall -Wextra -Werror
junto con un harness que ya tiene main(), así que no lleva main(). Solo
biblioteca estándar. Contrato (api_sesion27.h):

int emitir_acceso(char lineas[][64], int max, const char *base, const char *indice, int ancho, int *temporales);
int emitir_acceso_2d(char lineas[][64], int max, const char *base, const char *i, const char *j, int filas, int ancho, int *temporales);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- 1D: "_ta = i * ancho" y "_tb = A[_ta]" (2 líneas).
- 2D: "_ta = i * filas", "_tb = _ta + j", "_tc = _tb * ancho", "_td = M[_tc]" (4 líneas), en ese orden.
- Cada temporal nuevo incrementa *temporales; nunca repetir nombres.
- Base NULL (o cualquier puntero NULL) no emite nada y regresa 0.
- Regresa cuántas líneas escribió; nunca pasar de max.
- No cambies las firmas. Revisa todos los punteros NULL antes de usarlos.

OUTPUT: Solo el archivo ia_sesion27_c.c (con #include "api_sesion27.h"),
funciones auxiliares static y la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion27_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: ancho o filas <= 0 no emite nada)
 */
#include "api_sesion27.h"
#include <stddef.h>
#include <stdio.h>

/* Pide el siguiente temporal libre. O(1). */
static int nuevo(int *temporales) {
    return (*temporales)++;
}

/* A[i] con elementos de 'ancho' bytes:
 *   _ta = i * ancho
 *   _tb = A[_ta]
 * Regresa las lineas escritas (2) o 0 si no se puede. O(1). */
int emitir_acceso(char lineas[][64], int max, const char *base,
                  const char *indice, int ancho, int *temporales) {
    if (lineas == NULL || base == NULL || indice == NULL || temporales == NULL) return 0;
    if (max < 2 || ancho <= 0) return 0;
    int off = nuevo(temporales);
    int val = nuevo(temporales);
    snprintf(lineas[0], 64, "_t%d = %s * %d", off, indice, ancho);
    snprintf(lineas[1], 64, "_t%d = %s[_t%d]", val, base, off);
    return 2;
}

/* M[i][j], indice lineal = i * filas + j (en ese orden, como pide el contrato):
 *   _ta = i * filas
 *   _tb = _ta + j
 *   _tc = _tb * ancho
 *   _td = M[_tc]
 * Regresa 4 o 0 si no se puede. O(1). */
int emitir_acceso_2d(char lineas[][64], int max, const char *base, const char *i,
                     const char *j, int filas, int ancho, int *temporales) {
    if (lineas == NULL || base == NULL || i == NULL || j == NULL || temporales == NULL) return 0;
    if (max < 4 || filas <= 0 || ancho <= 0) return 0;
    int a = nuevo(temporales);
    int b = nuevo(temporales);
    int c = nuevo(temporales);
    int d = nuevo(temporales);
    snprintf(lineas[0], 64, "_t%d = %s * %d", a, i, filas);
    snprintf(lineas[1], 64, "_t%d = _t%d + %s", b, a, j);
    snprintf(lineas[2], 64, "_t%d = _t%d * %d", c, b, ancho);
    snprintf(lineas[3], 64, "_t%d = %s[_t%d]", d, base, c);
    return 4;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Arreglos multidimensionales en TAC

### Salida del harness
```text
=== AUDITORIA SESION 27 - Desplazamientos para arreglos 1D y 2D ===
Caso                               esperado:              obtenido:              resultado
[ 1] 1D: i * 4                          esperado: _t0 = i * 4  obtenido: _t0 = i * 4  OK
[ 2] 1D: A[_t0]                         esperado: _t1 = A[_t0] obtenido: _t1 = A[_t0] OK
[ 3] 2D: i * 3                          esperado: _t0 = i * 3  obtenido: _t0 = i * 3  OK
[ 4] 2D: offset por ancho
     esperado: _t2 = _t1 * 4
     obtenido: _t2 = _t1 * 4
     -> OK
[ 5] 2D: M[_t2]                         esperado: _t3 = M[_t2] obtenido: _t3 = M[_t2] OK
[ 6] base NULL (sin acceso)             esperado: 0            obtenido: 0            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion27.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA sacó una función `nuevo()` para pedir temporales, que queda más claro que el `(*temporales)++` repetido. El caso que omitió (y yo también) es un `ancho` o `filas` en 0 o negativo: generaba `_t0 = i * 0`, que siempre apunta al primer elemento y no marca ningún error. Le agregué que si `ancho <= 0` o `filas <= 0` no emita nada y regrese 0. Revisé que respetara el orden `i * filas + j` y no lo cambiara por `j * filas + i`. O(1) por acceso, sin memoria dinámica.

---
**Firma digital:** josessuarez
