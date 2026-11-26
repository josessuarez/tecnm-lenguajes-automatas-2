# Bitácora de Requerimientos y Auditoría IA — Sesión 23
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Etiquetas y Saltos Incondicionales  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Generador de etiquetas e instrucciones goto  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Esta fue la más corta. `nueva_etiqueta()` es igual a los temporales de la sesión 21 pero con `L`. En `emitir_flujo()` uso el `es_salto` para elegir entre `GOTO L2` y `L2:`. El detalle es el `NULL`: si no lo reviso antes, `snprintf` con `%s` y `NULL` truena o imprime basura, así que pongo `dst[0] = '\0'` y regreso.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Trazabilidad del flujo de control |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo la parte de código intermedio (TAC) de un compilador
en C11, sesión 23: Etiquetas y Saltos Incondicionales. Se compila con gcc -std=c11 -Wall -Wextra -Werror
junto con un harness que ya tiene main(), así que no lleva main(). Solo
biblioteca estándar. Contrato (api_sesion23.h):

void nueva_etiqueta(char *dst, int cap, int *contador);
void emitir_flujo(char *dst, int cap, const char *etiqueta, int es_salto);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- nueva_etiqueta() escribe "LK" con K = *contador y luego lo incrementa (la primera es L0).
- emitir_flujo(): es_salto != 0 escribe "GOTO etiqueta"; es_salto == 0 escribe "etiqueta:".
- Etiqueta NULL produce cadena vacía. Nunca escribir fuera de cap.
- El GOTO y la marca deben usar exactamente el mismo nombre.
- No cambies las firmas. Revisa todos los punteros NULL antes de usarlos.

OUTPUT: Solo el archivo ia_sesion23_c.c (con #include "api_sesion23.h"),
funciones auxiliares static y la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion23_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: etiqueta vacia "" tambien da cadena vacia)
 */
#include "api_sesion23.h"
#include <stddef.h>
#include <stdio.h>

/* Escribe LK con K = *contador y luego lo incrementa. O(1). */
void nueva_etiqueta(char *dst, int cap, int *contador) {
    if (dst == NULL || cap <= 0 || contador == NULL || *contador < 0) return;
    int w = snprintf(dst, (size_t)cap, "L%d", *contador);
    if (w > 0 && w < cap) (*contador)++;
}

/* es_salto != 0 -> "GOTO L"; es_salto == 0 -> "L:". O(longitud).
 * Etiqueta NULL o vacia -> cadena vacia. */
void emitir_flujo(char *dst, int cap, const char *etiqueta, int es_salto) {
    if (dst == NULL || cap <= 0) return;
    dst[0] = '\0';
    if (etiqueta == NULL || etiqueta[0] == '\0') return;
    snprintf(dst, (size_t)cap, es_salto ? "GOTO %s" : "%s:", etiqueta);
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Saltos adelante y atrás estructurados

### Salida del harness
```text
=== AUDITORIA SESION 23 - Etiquetas y saltos incondicionales ===
Caso                               esperado:              obtenido:              resultado
[ 1] nueva_etiqueta(L0)                 esperado: L0           obtenido: L0           OK
[ 2] nueva_etiqueta(L5)                 esperado: L5           obtenido: L5           OK
[ 3] emitir_flujo(GOTO L2)              esperado: GOTO L2      obtenido: GOTO L2      OK
[ 4] emitir_flujo(marca L2)             esperado: L2:          obtenido: L2:          OK
[ 5] emitir_flujo(NULL)                 esperado:              obtenido:              OK
[ 6] nueva_etiqueta(L10)                esperado: L10          obtenido: L10          OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion23.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA resolvió el `GOTO`/marca con un solo `snprintf` eligiendo el formato con `?:`. Lo que se le pasó es la etiqueta vacía `""`: con su versión salía `GOTO ` (con espacio y sin nombre), que es un salto inválido. Le agregué que una etiqueta vacía también dé cadena vacía. Además, igual que en la 21, que el contador solo suba si la etiqueta cupo completa, para que las etiquetas sigan siendo consecutivas y únicas. Todo O(1).

---
**Firma digital:** josessuarez
