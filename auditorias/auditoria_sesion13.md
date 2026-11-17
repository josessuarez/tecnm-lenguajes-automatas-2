# Bitácora de Requerimientos y Auditoría IA — Sesión 13
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Manejo de Ámbitos (Scope Global/Local)  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Pila de tablas de símbolos (Stack Scopes)  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo primero que me confundió fue que `declarar()` NO tiene que revisar los ámbitos padre. Mi idea era usar `buscar()` antes de insertar, pero así nunca se podría declarar una `x` en un bloque si ya existe una `x` global, y eso es justo el shadowing que pide el caso 4. Entonces `declarar()` solo recorre el arreglo del ámbito actual con `strcmp`, y si no está y todavía hay espacio (menos de `AMBITO_MAX`, que son 8) lo guarda en `nombres[total]` y sube `total`. `buscar()` es un `while` que empieza en el ámbito que le pasan y sube por `padre` hasta que se acaba la cadena (NULL); lo que regresa es el **nombre del ámbito** donde lo encontró, no el símbolo. Como los arreglos son fijos no hay `malloc`, pero sí hay que cuidar no pasarse de las 8 posiciones.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Ocultamiento de variables (Shadowing) |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo la tabla de símbolos de un compilador en C11. Se
compila con gcc -std=c11 -Wall -Wextra -Werror junto con un harness que ya
tiene main(), así que no lleva main(). Solo biblioteca estándar. Contrato
(api_sesion13.h):

#define AMBITO_MAX 8
typedef struct Ambito { const char *nombre;
                        const char *nombres[AMBITO_MAX];
                        int total; struct Ambito *padre; } Ambito;
int declarar(Ambito *ambito, const char *nombre);
const char *buscar(const Ambito *ambito, const char *nombre);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- declarar() inserta solo en el ámbito que recibe, sin revisar los padres
  (se permite shadowing). Regresa 1 si insertó, 0 si el nombre ya estaba en
  ese mismo ámbito y -1 si el ámbito está lleno, es NULL o el nombre es NULL
  o vacío.
- buscar() empieza en el ámbito actual y sube por padre hasta la raíz.
  Regresa el campo nombre del ámbito más cercano que declara el
  identificador, o NULL si no existe en ninguno.
- La comparación de nombres es exacta (strcmp).
- No leer fuera del arreglo nombres[] aunque total venga mal.
- No cambies las firmas ni la estructura.

OUTPUT: Solo el archivo ia_sesion13_c.c (con #include "api_sesion13.h"),
auxiliares static, y la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion13_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: misma firma que api_sesion13.h)
 */
#include "api_sesion13.h"
#include <stddef.h>
#include <string.h>

/* Busca el nombre solo en este ambito. O(k), k = simbolos del ambito.
 * Ignora huecos NULL y no lee mas alla de AMBITO_MAX aunque total venga mal. */
static int esta_en(const Ambito *ambito, const char *nombre) {
    int limite = ambito->total;
    if (limite < 0) limite = 0;
    if (limite > AMBITO_MAX) limite = AMBITO_MAX;
    for (int i = 0; i < limite; i++) {
        const char *s = ambito->nombres[i];
        if (s != NULL && strcmp(s, nombre) == 0) return 1;
    }
    return 0;
}

/* Inserta solo en el ambito actual (permite shadowing). O(k). */
int declarar(Ambito *ambito, const char *nombre) {
    if (ambito == NULL || nombre == NULL || nombre[0] == '\0') return -1;
    if (ambito->total < 0 || ambito->total > AMBITO_MAX) return -1;
    if (esta_en(ambito, nombre)) return 0;
    if (ambito->total == AMBITO_MAX) return -1;
    ambito->nombres[ambito->total++] = nombre;
    return 1;
}

/* Recorre la cadena actual -> raiz y regresa el nombre del ambito mas
 * cercano que declara el identificador. O(p * k), p = profundidad. */
const char *buscar(const Ambito *ambito, const char *nombre) {
    if (nombre == NULL || nombre[0] == '\0') return NULL;
    for (const Ambito *a = ambito; a != NULL; a = a->padre) {
        if (esta_en(a, nombre)) return a->nombre;
    }
    return NULL;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Búsqueda jerárquica de identificadores

### Salida del harness
```text
=== AUDITORIA SESION 13 - Pila de ambitos y busqueda jerarquica ===
Caso                               esperado:              obtenido:              resultado
[ 1] declarar(&g, "x")                  esperado: 1            obtenido: 1            OK
[ 2] buscar en el mismo ambito          esperado: global       obtenido: global       OK
[ 3] buscar subiendo al padre           esperado: global       obtenido: global       OK
[ 4] shadowing en el hijo               esperado: bloque       obtenido: bloque       OK
[ 5] tres niveles hasta la raiz         esperado: raiz         obtenido: raiz         OK
[ 6] nombre no declarado                esperado: (nulo)       obtenido: (nulo)       OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion13.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> Al revisar el código vi que el riesgo estaba en confiar en que `total` siempre viene bien: si se recorre `for (i = 0; i < total; i++)` y se escribe en `nombres[total]` sin revisar nada, y alguien le pasa un ámbito con basura en `total` (por ejemplo uno sin `memset`), se lee y se escribe fuera del arreglo. El harness no lo prueba porque siempre limpia los ámbitos con `memset`. Por eso el recorrido quedó acotado a `AMBITO_MAX` en el auxiliar `esta_en()` y `declarar()` regresa -1 si `total` está fuera de rango. También agregué que se salte entradas NULL para que `strcmp` no truene, y que `buscar()` con nombre vacío regrese NULL. Complejidad: `declarar()` es O(k) con k ≤ 8 símbolos del ámbito, y `buscar()` es O(p·k), donde p es la profundidad de la cadena.

---
**Firma digital:** josessuarez
