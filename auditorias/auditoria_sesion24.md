# Bitácora de Requerimientos y Auditoría IA — Sesión 24
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Control de Flujo: Sentencia if-else  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Saltos condicionales y evaluación booleana  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Primero tuve que entender el corto circuito: en un AND, si el primer operando es falso ya no se revisa el segundo, por eso se usa `IFFALSE ... GOTO salida`. En el OR es al revés, con `IFTRUE`. Lo implementé eligiendo la palabra según el `op` y emitiendo un salto por cada operando. Me atoré con el caso del segundo operando `NULL`: al principio emitía `IFFALSE (null) GOTO L0`; lo arreglé emitiendo el segundo salto solo si `b` no es `NULL`.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Cortocircuito lógico en expresiones |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo la parte de código intermedio (TAC) de un compilador
en C11, sesión 24: Control de Flujo: Sentencia if-else. Se compila con gcc -std=c11 -Wall -Wextra -Werror
junto con un harness que ya tiene main(), así que no lleva main(). Solo
biblioteca estándar. Contrato (api_sesion24.h):

int emitir_condicion(char lineas[][64], int max, const char *a, const char *b, char op, const char *etiqueta);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- op '&' usa IFFALSE y op '|' usa IFTRUE, siempre hacia la etiqueta recibida.
- Cada operando lleva su propio salto: "IFFALSE a GOTO L", "IFFALSE b GOTO L".
- Si b es NULL se emite un solo salto, sin inventar comparaciones.
- Nunca escribir más de max líneas. Regresa cuántas líneas escribió.
- No cambies las firmas. Revisa todos los punteros NULL antes de usarlos.

OUTPUT: Solo el archivo ia_sesion24_c.c (con #include "api_sesion24.h"),
funciones auxiliares static y la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion24_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: un operador que no es & ni | no emite nada)
 */
#include "api_sesion24.h"
#include <stddef.h>
#include <stdio.h>

/* Agrega "<salto> <operando> GOTO <etiqueta>" si hay espacio. O(1). */
static int agrega(char lineas[][64], int max, int *n, const char *salto,
                  const char *operando, const char *etiqueta) {
    if (*n >= max) return 0;
    snprintf(lineas[*n], 64, "%s %s GOTO %s", salto, operando, etiqueta);
    (*n)++;
    return 1;
}

/* Corto circuito: & -> IFFALSE (si uno es falso ya no sigue),
 * | -> IFTRUE (si uno es verdadero ya no sigue).
 * Cada operando lleva su propio salto. b NULL -> un solo salto.
 * Regresa cuantas lineas escribio. O(1). */
int emitir_condicion(char lineas[][64], int max, const char *a, const char *b,
                     char op, const char *etiqueta) {
    if (lineas == NULL || max <= 0 || a == NULL || etiqueta == NULL) return 0;
    const char *salto;
    if (op == '&')      salto = "IFFALSE";
    else if (op == '|') salto = "IFTRUE";
    else return 0;
    int n = 0;
    agrega(lineas, max, &n, salto, a, etiqueta);
    if (b != NULL) agrega(lineas, max, &n, salto, b, etiqueta);
    return n;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Condiciones compuestas con AND y OR

### Salida del harness
```text
=== AUDITORIA SESION 24 - Saltos condicionales con cortocircuito ===
Caso                               esperado:              obtenido:              resultado
[ 1] a && b (primer salto)
     esperado: IFFALSE a GOTO L9
     obtenido: IFFALSE a GOTO L9
     -> OK
[ 2] a && b (segundo salto)
     esperado: IFFALSE b GOTO L9
     obtenido: IFFALSE b GOTO L9
     -> OK
[ 3] a && b (dos saltos)                esperado: 2            obtenido: 2            OK
[ 4] a || b (primer salto)
     esperado: IFTRUE a GOTO L9
     obtenido: IFTRUE a GOTO L9
     -> OK
[ 5] a || b (segundo salto)
     esperado: IFTRUE b GOTO L9
     obtenido: IFTRUE b GOTO L9
     -> OK
[ 6] condicion simple (b NULL)
     esperado: IFFALSE a GOTO L0
     obtenido: IFFALSE a GOTO L0
     -> OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion24.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La versión de la IA se parecía a la mía, pero las dos tenían el mismo hueco: cualquier `op` que no fuera `|` se trataba como AND. Si llega por error un `'+'`, generaba saltos `IFFALSE` como si nada. Le pedí que si el operador no es `&` ni `|` regrese 0 sin emitir. También pasó la escritura a una función `agrega()` que revisa `max` antes de cada línea, así el segundo salto no se escribe si no cabe. Complejidad O(1) por condición.

---
**Firma digital:** josessuarez
