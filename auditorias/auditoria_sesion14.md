# Bitácora de Requerimientos y Auditoría IA — Sesión 14
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Declaración vs Uso de Variables  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Detección de variables no declaradas  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo que más cuidé fue el caso del prefijo: si comparo con `strncmp` o solo la primera letra, `"x"` y `"xy"` salen iguales y `usar("xy")` diría que sí está declarada. Con `strcmp` la comparación es completa y se resuelve. Luego `declarar()` reutiliza `usar()`: si ya existe regresa 0 y **no** sube `total`, que es lo que mide el caso 4 (el total tiene que seguir en 1 después de redeclarar). El nombre vacío lo detecté con `nombre[0] == '\0'` y regreso -1 sin insertar. También dejé el tope de `TABLA_MAX` (16) para no escribir fuera del arreglo cuando la tabla se llena.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Invariante de definición previa |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo el chequeo de declaración previa de un compilador
en C11. Se compila con gcc -std=c11 -Wall -Wextra -Werror junto con un
harness que ya tiene main(), así que no lleva main(). Solo biblioteca
estándar. Contrato (api_sesion14.h):

#define TABLA_MAX 16
typedef struct Tabla { const char *nombres[TABLA_MAX]; int total; } Tabla;
int declarar(Tabla *tabla, const char *nombre);
int usar(const Tabla *tabla, const char *nombre);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- declarar() regresa 1 si insertó, 0 si el nombre ya estaba (sin cambiar
  total) y -1 si recibe NULL o un nombre vacío.
- usar() regresa 1 solo si el nombre ya fue declarado; en cualquier otro
  caso 0.
- Comparación exacta: "x" y "xy" son nombres distintos.
- Nunca escribir ni leer fuera de nombres[].
- No cambies las firmas ni la estructura.

OUTPUT: Solo el archivo ia_sesion14_c.c (con #include "api_sesion14.h"),
auxiliares static, y la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion14_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: misma firma que api_sesion14.h)
 */
#include "api_sesion14.h"
#include <stddef.h>
#include <string.h>

/* Cuantas entradas se pueden leer sin salirse del arreglo. */
static int entradas_validas(const Tabla *tabla) {
    if (tabla->total < 0) return 0;
    if (tabla->total > TABLA_MAX) return TABLA_MAX;
    return tabla->total;
}

/* 1 si el nombre ya fue declarado (comparacion exacta), 0 en otro caso. O(n). */
int usar(const Tabla *tabla, const char *nombre) {
    if (tabla == NULL || nombre == NULL || nombre[0] == '\0') return 0;
    int n = entradas_validas(tabla);
    for (int i = 0; i < n; i++) {
        const char *s = tabla->nombres[i];
        if (s != NULL && strcmp(s, nombre) == 0) return 1;
    }
    return 0;
}

/* 1 = insertado, 0 = ya existia, -1 = argumento invalido o tabla llena. O(n). */
int declarar(Tabla *tabla, const char *nombre) {
    if (tabla == NULL || nombre == NULL || nombre[0] == '\0') return -1;
    if (tabla->total < 0 || tabla->total > TABLA_MAX) return -1;
    if (usar(tabla, nombre)) return 0;
    if (tabla->total == TABLA_MAX) return -1;
    tabla->nombres[tabla->total++] = nombre;
    return 1;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Uso antes de declarar y redeclaración

### Salida del harness
```text
=== AUDITORIA SESION 14 - Declaracion previa y redeclaracion ===
Caso                               esperado:              obtenido:              resultado
[ 1] usar sin declarar                  esperado: 0            obtenido: 0            OK
[ 2] usar despues de declarar           esperado: 1            obtenido: 1            OK
[ 3] redeclarar el mismo nombre         esperado: 0            obtenido: 0            OK
[ 4] total tras redeclarar              esperado: 1            obtenido: 1            OK
[ 5] prefijo no es el mismo nombre      esperado: 0            obtenido: 0            OK
[ 6] declarar nombre vacio              esperado: -1           obtenido: -1           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion14.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> El contrato no dice qué hacer cuando la tabla ya tiene `TABLA_MAX` nombres, y si solo se hace `nombres[total++] = nombre` sin revisar, el nombre 17 se escribiría fuera del arreglo y pisaría `total`. El harness no llega a ese caso porque declara uno o dos nombres nada más. Lo corregí regresando -1 cuando la tabla está llena, igual que con los argumentos inválidos, y además limité las lecturas con `entradas_validas()` por si `total` viene negativo o mayor que 16. Otro detalle: `usar(tabla, "")` regresa 0, porque un nombre vacío nunca se pudo declarar. Las dos funciones son O(n) con n ≤ 16, que para esta tabla es prácticamente constante; con muchas variables convendría una tabla hash.

---
**Firma digital:** josessuarez
