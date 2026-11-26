# Bitácora de Requerimientos y Auditoría IA — Sesión 26
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** TAC para Llamadas a Funciones  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Instrucciones param, call y return  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo primero fue el orden: cada argumento que no es `NULL` genera un `PARAM`, primero `a1` y luego `a2`, y el número del `CALL` es cuántos `PARAM` salieron de verdad, no siempre 2. Para la llamada anidada `f(g(x))` me di cuenta de que solo tengo que pasar el temporal de la primera llamada (`_t0`) como argumento de la segunda, y como escribo a partir de `*n` no se encima nada. En `emitir_return()` lo único es no imprimir un `NULL`.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Convención de paso de argumentos TAC |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo la parte de código intermedio (TAC) de un compilador
en C11, sesión 26: TAC para Llamadas a Funciones. Se compila con gcc -std=c11 -Wall -Wextra -Werror
junto con un harness que ya tiene main(), así que no lleva main(). Solo
biblioteca estándar. Contrato (api_sesion26.h):

int emitir_llamada(char lineas[][64], int max, int *n, const char *funcion, const char *a1, const char *a2, const char *resultado);
void emitir_return(char *dst, int cap, const char *valor);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- Un PARAM por cada argumento no NULL, en el orden a1, a2.
- Después "res = CALL f, k" con k = número real de PARAM; si resultado es NULL, solo "CALL f, k".
- emitir_return(): "RETURN v", o solo "RETURN" si v es NULL.
- Se escribe desde lineas[*n] y se actualiza *n. Nunca pasar de max.
- No cambies las firmas. Revisa todos los punteros NULL antes de usarlos.

OUTPUT: Solo el archivo ia_sesion26_c.c (con #include "api_sesion26.h"),
funciones auxiliares static y la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion26_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: si no cabe la llamada completa no escribe
 * nada, para no dejar PARAM sueltos sin su CALL)
 */
#include "api_sesion26.h"
#include <stddef.h>
#include <stdio.h>

/* Emite PARAM a1, PARAM a2 (solo los que no son NULL, en ese orden) y
 * luego "res = CALL f, k" o "CALL f, k" si resultado es NULL.
 * k es el numero real de PARAM emitidos. Regresa las lineas escritas. O(1). */
int emitir_llamada(char lineas[][64], int max, int *n, const char *funcion,
                   const char *a1, const char *a2, const char *resultado) {
    if (lineas == NULL || n == NULL || funcion == NULL || *n < 0) return 0;
    const char *args[2] = { a1, a2 };
    int k = (a1 != NULL) + (a2 != NULL);
    if (max - *n < k + 1) return 0;            /* todo o nada */
    int antes = *n;
    for (int i = 0; i < 2; i++) {
        if (args[i] != NULL) snprintf(lineas[(*n)++], 64, "PARAM %s", args[i]);
    }
    if (resultado != NULL)
        snprintf(lineas[(*n)++], 64, "%s = CALL %s, %d", resultado, funcion, k);
    else
        snprintf(lineas[(*n)++], 64, "CALL %s, %d", funcion, k);
    return *n - antes;
}

/* "RETURN v" o solo "RETURN" si valor es NULL. O(1). */
void emitir_return(char *dst, int cap, const char *valor) {
    if (dst == NULL || cap <= 0) return;
    if (valor == NULL || valor[0] == '\0') snprintf(dst, (size_t)cap, "RETURN");
    else snprintf(dst, (size_t)cap, "RETURN %s", valor);
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Llamadas anidadas f(g(x))

### Salida del harness
```text
=== AUDITORIA SESION 26 - Llamadas, parametros y retorno ===
Caso                               esperado:              obtenido:              resultado
[ 1] PARAM x                            esperado: PARAM x      obtenido: PARAM x      OK
[ 2] PARAM y                            esperado: PARAM y      obtenido: PARAM y      OK
[ 3] CALL con dos argumentos
     esperado: _t0 = CALL f, 2
     obtenido: _t0 = CALL f, 2
     -> OK
[ 4] llamada anidada f(g(x))
     esperado: _t1 = CALL f, 1
     obtenido: _t1 = CALL f, 1
     -> OK
[ 5] CALL sin argumentos
     esperado: _t0 = CALL p, 0
     obtenido: _t0 = CALL p, 0
     -> OK
[ 6] RETURN con valor                   esperado: RETURN _t0   obtenido: RETURN _t0   OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion26.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> En mi versión, si no cabía todo, podían quedar los `PARAM` escritos sin su `CALL`, y eso deja la pila de parámetros desbalanceada para la siguiente llamada. La IA tampoco lo cuidaba al principio. Le pedí que fuera "todo o nada": calcula cuántas líneas necesita (`k + 1`) y si no caben no escribe ninguna. Además usé un arreglo `args[2]` para no repetir el `if`. Una diferencia: mi función regresa cuántos `PARAM` emitió y la de la IA cuántas líneas escribió; el harness no lo revisa, pero lo dejé documentado en el comentario. O(1).

---
**Firma digital:** josessuarez
