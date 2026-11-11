# Bitácora de Requerimientos y Auditoría IA — Sesión 11
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Arquitectura de la Tabla de Símbolos  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Tabla Hash con encadenamiento  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo primero fue la fórmula de djb2: empieza en 5381 y por cada letra hace `h*33 + c`. Tuve cuidado de usar `unsigned int` porque con números grandes se desborda, y en unsigned eso está definido (da la vuelta) pero en `int` sería comportamiento indefinido. Con eso `hash_djb2("abc")` me dio 193485963. En la inserción lo que más pensé fue lo de los duplicados: el contrato dice que un nombre no se puede repetir en *ninguna* cubeta, entonces recorro toda la tabla antes de crear el nodo y si lo encuentro regreso 0. Para las colisiones meto el nodo nuevo al inicio de la lista de esa cubeta (`nuevo->sig = tabla[pos]`), así nunca piso al que ya estaba y es O(1). El nombre lo copio con `snprintf` porque el campo es de 16 bytes y si el nombre es más largo no quiero escribir fuera del arreglo.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Función de dispersión y colisiones O(1) |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo la tabla de símbolos de un compilador en C11. Se
compila con gcc -std=c11 -Wall -Wextra -Werror junto con un harness que ya
tiene main(), así que no lleva main(). Solo biblioteca estándar. Contrato
(api_sesion11.h):

typedef struct Simbolo { char nombre[16]; int tipo; int offset;
                         struct Simbolo *sig; } Simbolo;
unsigned int hash_djb2(const char *clave);
int insertar_simbolo(Simbolo *tabla[], unsigned int cubetas,
                     const char *nombre, int tipo, int offset);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- hash_djb2: h = 5381 y por cada byte h = h * 33 + c (unsigned).
- insertar_simbolo: índice = hash % cubetas; encadenar en colisión (nunca
  sobrescribir). Devuelve 1 si insertó, 0 si el nombre ya existía en
  cualquier cubeta, -1 si tabla es NULL, nombre es NULL o cubetas es 0.
- El nombre cabe en 16 bytes; no escribir fuera del arreglo.
- No cambies las firmas ni la estructura.

OUTPUT: Solo el archivo ia_sesion11_c.c (con #include "api_sesion11.h") y la
complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
#include "api_sesion11.h"
#include <stdlib.h>
#include <string.h>

#define MAX_NOMBRE ((int)sizeof(((Simbolo *)0)->nombre) - 1)   /* 15 */

/* djb2 de Dan Bernstein: h = h * 33 + c, empezando en 5381.
 * Complejidad: O(k), k = largo de la clave. */
unsigned int hash_djb2(const char *clave) {
    unsigned int h = 5381u;
    if (clave == NULL) return h;
    for (const unsigned char *c = (const unsigned char *)clave; *c != '\0'; c++)
        h = ((h << 5) + h) + *c;       /* (h << 5) + h == h * 33 */
    return h;
}

/* Devuelve 1 si inserto, 0 si el nombre ya existia, -1 si los argumentos
 * no son validos (o el nombre no cabe en el campo de 16 bytes).
 * Complejidad: O(n) por la busqueda de duplicados en toda la tabla;
 * la insercion en si es O(1) porque se agrega al frente de la cadena. */
int insertar_simbolo(Simbolo *tabla[], unsigned int cubetas, const char *nombre, int tipo, int offset) {
    if (tabla == NULL || nombre == NULL || cubetas == 0 || nombre[0] == '\0')
        return -1;
    if ((int)strlen(nombre) > MAX_NOMBRE)
        return -1;                     /* truncarlo crearia nombres repetidos */

    for (unsigned int i = 0; i < cubetas; i++)
        for (const Simbolo *s = tabla[i]; s != NULL; s = s->sig)
            if (strcmp(s->nombre, nombre) == 0)
                return 0;

    Simbolo *nodo = (Simbolo *)calloc(1, sizeof *nodo);
    if (nodo == NULL) return -1;
    memcpy(nodo->nombre, nombre, strlen(nombre) + 1);
    nodo->tipo = tipo;
    nodo->offset = offset;

    unsigned int idx = hash_djb2(nombre) % cubetas;
    nodo->sig = tabla[idx];            /* encadenar, nunca sobrescribir */
    tabla[idx] = nodo;
    return 1;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Manejo de claves duplicadas y colisiones

### Salida del harness
```text
=== AUDITORIA SESION 11 - Tabla hash con encadenamiento ===
Caso                               esperado:              obtenido:              resultado
[ 1] hash_djb2("")                      esperado: 5381         obtenido: 5381         OK
[ 2] hash_djb2("abc")                   esperado: 193485963    obtenido: 193485963    OK
[ 3] insertar x dos veces               esperado: 0            obtenido: 0            OK
[ 4] dos claves, una cubeta             esperado: 2            obtenido: 2            OK
[ 5] offset de beta en cadena           esperado: 4            obtenido: 4            OK
[ 6] insertar en tabla NULL             esperado: -1           obtenido: -1           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion11.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> El caso borde que la IA no cuidaba al principio fue el de nombres largos. El campo `nombre` es de 16 bytes, así que si el nombre tiene más de 15 letras se trunca al copiarlo. El problema es que dos nombres distintos como `contador_total_1` y `contador_total_2` se cortan igual y quedan guardados como el mismo nombre, pero el hash se calcula con el nombre completo, así que caen en cubetas diferentes y la tabla termina con dos símbolos "iguales". El harness no prueba eso porque todos sus nombres son cortos. Lo solucioné rechazando con -1 los nombres que no caben (y el vacío), en vez de truncarlos sin avisar. También revisé que use `calloc` y verifique si regresa NULL. La búsqueda de duplicados es O(n) porque recorre toda la tabla, como pide el contrato; la inserción en sí es O(1).

---
**Firma digital:** josessuarez
