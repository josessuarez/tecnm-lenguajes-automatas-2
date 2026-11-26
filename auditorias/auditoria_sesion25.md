# Bitácora de Requerimientos y Auditoría IA — Sesión 25
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Control de Flujo: Bucles while y for  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Estructura de ciclo en instrucciones TAC  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Aquí ya tenía que juntar etiquetas y saltos. Me dibujé en papel el esqueleto del while: `L0:`, `IFFALSE cond GOTO L1`, `GOTO L0` y `L1:`. Para el for es lo mismo pero con la inicialización antes y la actualización (`post`) justo antes del `GOTO` de regreso; si la pongo después del `GOTO` nunca se ejecuta y el ciclo es infinito. Me atoré con el caso de dos ciclos seguidos: tenía que seguir escribiendo donde iba `*n` y no en la posición 0, y usar el contador de etiquetas compartido para que el segundo empiece en `L2`.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Garantía de actualización de iterador |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo la parte de código intermedio (TAC) de un compilador
en C11, sesión 25: Control de Flujo: Bucles while y for. Se compila con gcc -std=c11 -Wall -Wextra -Werror
junto con un harness que ya tiene main(), así que no lleva main(). Solo
biblioteca estándar. Contrato (api_sesion25.h):

void emitir_while(char lineas[][64], int max, int *n, const char *cond, int *etiquetas);
void emitir_for(char lineas[][64], int max, int *n, const char *inicio, const char *cond, const char *post, int *etiquetas);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- while: "Lini:", "IFFALSE cond GOTO Lfin", "GOTO Lini", "Lfin:" (4 líneas).
- for: init, "Lini:", "IFFALSE cond GOTO Lfin", post, "GOTO Lini", "Lfin:"; si post es NULL se omite (5 líneas).
- Cada ciclo consume dos etiquetas de *etiquetas; dos ciclos seguidos no repiten nombres.
- Se escribe a partir de lineas[*n] y se actualiza *n. Nunca pasar de max.
- No cambies las firmas. Revisa todos los punteros NULL antes de usarlos.

OUTPUT: Solo el archivo ia_sesion25_c.c (con #include "api_sesion25.h"),
funciones auxiliares static y la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion25_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: revisa el espacio ANTES de gastar etiquetas)
 */
#include "api_sesion25.h"
#include <stddef.h>
#include <stdio.h>

/* Copia una linea en la siguiente posicion libre. O(longitud). */
static void pon(char lineas[][64], int *n, const char *texto) {
    snprintf(lineas[*n], 64, "%s", texto);
    (*n)++;
}

/* Reserva el par de etiquetas (inicio, fin) del ciclo. O(1). */
static void reserva(int *etiquetas, int *ini, int *fin) {
    *ini = (*etiquetas)++;
    *fin = (*etiquetas)++;
}

/* while: Lini: / IFFALSE cond GOTO Lfin / GOTO Lini / Lfin:  -> 4 lineas. O(1).
 * Si no caben las 4, no escribe nada ni gasta etiquetas. */
void emitir_while(char lineas[][64], int max, int *n, const char *cond, int *etiquetas) {
    if (lineas == NULL || n == NULL || cond == NULL || etiquetas == NULL) return;
    if (*n < 0 || max - *n < 4) return;
    int ini, fin;
    char buf[64];
    reserva(etiquetas, &ini, &fin);
    snprintf(buf, sizeof buf, "L%d:", ini);                    pon(lineas, n, buf);
    snprintf(buf, sizeof buf, "IFFALSE %s GOTO L%d", cond, fin); pon(lineas, n, buf);
    snprintf(buf, sizeof buf, "GOTO L%d", ini);                pon(lineas, n, buf);
    snprintf(buf, sizeof buf, "L%d:", fin);                    pon(lineas, n, buf);
}

/* for: init / Lini: / IFFALSE cond GOTO Lfin / post / GOTO Lini / Lfin:
 * 6 lineas, o 5 si post es NULL. O(1). */
void emitir_for(char lineas[][64], int max, int *n, const char *inicio,
                const char *cond, const char *post, int *etiquetas) {
    if (lineas == NULL || n == NULL || inicio == NULL || cond == NULL || etiquetas == NULL) return;
    int necesito = (post != NULL) ? 6 : 5;
    if (*n < 0 || max - *n < necesito) return;
    int ini, fin;
    char buf[64];
    reserva(etiquetas, &ini, &fin);
    pon(lineas, n, inicio);
    snprintf(buf, sizeof buf, "L%d:", ini);                    pon(lineas, n, buf);
    snprintf(buf, sizeof buf, "IFFALSE %s GOTO L%d", cond, fin); pon(lineas, n, buf);
    if (post != NULL) pon(lineas, n, post);                    /* va ANTES del GOTO */
    snprintf(buf, sizeof buf, "GOTO L%d", ini);                pon(lineas, n, buf);
    snprintf(buf, sizeof buf, "L%d:", fin);                    pon(lineas, n, buf);
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Bucles anidados y condiciones de parada

### Salida del harness
```text
=== AUDITORIA SESION 25 - Esqueleto TAC de while y for ===
Caso                               esperado:              obtenido:              resultado
[ 1] while: marca de inicio             esperado: L0:          obtenido: L0:          OK
[ 2] while: condicion
     esperado: IFFALSE a < b GOTO L1
     obtenido: IFFALSE a < b GOTO L1
     -> OK
[ 3] dos ciclos sin colision            esperado: L2:          obtenido: L2:          OK
[ 4] for: inicializacion                esperado: i = 0        obtenido: i = 0        OK
[ 5] for: actualizacion                 esperado: i = i + 1    obtenido: i = i + 1    OK
[ 6] for sin post (NULL)                esperado: 5            obtenido: 5            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion25.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA separó la parte de escribir (`pon()`) y la de pedir etiquetas (`reserva()`), y así el código se lee casi igual que el esqueleto. Lo que omitió: pedía las dos etiquetas antes de revisar si cabían las líneas, entonces cuando no había espacio el ciclo no se escribía pero las etiquetas sí se gastaban, y el siguiente ciclo saltaba de L1 a L4. Le moví la revisión de espacio antes de `reserva()`. También le pedí que dejara la actualización antes del `GOTO`, porque en un primer intento la puso después de la marca de fin. O(1) por ciclo.

---
**Firma digital:** josessuarez
