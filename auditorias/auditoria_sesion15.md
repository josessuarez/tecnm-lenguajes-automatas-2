# Bitácora de Requerimientos y Auditoría IA — Sesión 15
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Bloques Anidados y Ciclo de Vida  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Operaciones Push y Pop de ámbito  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Aquí ya entra la memoria dinámica. `push_ambito()` hace `malloc` de un `Ambito`, le pone el nombre y lo engancha al actual con `padre = actual`. Lo difícil fue `pop_ambito()`: si hago `free(actual)` y después leo `actual->padre`, estoy leyendo memoria ya liberada. Por eso primero guardo el padre en una variable y luego libero. El otro problema fue la raíz: en el harness la raíz es una variable local (`Ambito raiz = {"global", NULL}`), o sea, no salió de `malloc`, y si le hago `free` el programa truena. Como la raíz es la única sin padre, si `actual->padre == NULL` regreso NULL y no libero nada. Lo revisé con valgrind y no quedaron fugas: cada push del harness tiene su pop.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Liberación de memoria al cerrar bloque |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy manejando la apertura y cierre de bloques en un compilador
en C11. Se compila con gcc -std=c11 -Wall -Wextra -Werror junto con un
harness que ya tiene main(), así que no lleva main(). Solo biblioteca
estándar. Contrato (api_sesion15.h):

typedef struct Ambito { const char *nombre; struct Ambito *padre; } Ambito;
Ambito *push_ambito(Ambito *actual, const char *nombre);
Ambito *pop_ambito(Ambito *actual);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- push_ambito() reserva con malloc un ámbito nuevo, guarda el nombre y lo
  enlaza al actual como padre. Regresa NULL si actual o nombre son NULL o si
  malloc falla.
- pop_ambito() libera el ámbito que recibe y regresa su padre. Sobre la raíz
  (padre == NULL) regresa NULL y no libera nada, porque la raíz puede no
  venir de malloc.
- No leer memoria después de liberarla; cada push se empareja con un pop.
- No cambies las firmas ni la estructura.

OUTPUT: Solo el archivo ia_sesion15_c.c (con #include "api_sesion15.h") y la
complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion15_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: misma firma que api_sesion15.h)
 */
#include "api_sesion15.h"
#include <stddef.h>
#include <stdlib.h>

/* Abre un bloque: reserva un ambito nuevo enlazado al actual. O(1).
 * El nombre no se copia: debe vivir al menos lo mismo que el ambito. */
Ambito *push_ambito(Ambito *actual, const char *nombre) {
    if (actual == NULL || nombre == NULL) return NULL;
    Ambito *nuevo = malloc(sizeof *nuevo);
    if (nuevo == NULL) return NULL;
    nuevo->nombre = nombre;
    nuevo->padre = actual;
    return nuevo;
}

/* Cierra el bloque actual: guarda el padre, libera el ambito y regresa
 * el padre. La raiz (padre == NULL) no se libera y regresa NULL. O(1).
 * OJO: solo se le debe pasar un ambito creado con push_ambito(). */
Ambito *pop_ambito(Ambito *actual) {
    if (actual == NULL || actual->padre == NULL) return NULL;
    Ambito *padre = actual->padre;
    free(actual);
    return padre;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Aislamiento de variables fuera de bloque

### Salida del harness
```text
=== AUDITORIA SESION 15 - Push y pop de ambitos ===
Caso                               esperado:              obtenido:              resultado
[ 1] nombre del ambito nuevo            esperado: if           obtenido: if           OK
[ 2] profundidad con dos push           esperado: 3            obtenido: 3            OK
[ 3] pop devuelve el padre              esperado: 1            obtenido: 1            OK
[ 4] pop sobre la raiz                  esperado: 1            obtenido: 1            OK
[ 5] push con padre NULL                esperado: 1            obtenido: 1            OK
[ 6] profundidad tras pop               esperado: 2            obtenido: 2            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion15.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA resolvió bien el orden (guardar el padre antes del `free`), pero hay dos cosas que el contrato deja abiertas y el harness no prueba. Primero, `pop_ambito()` no tiene forma de saber si el ámbito salió de `malloc`: si alguien le pasa un ámbito hijo que sea variable local, le haría `free` y el programa truena. La única regla segura con este contrato es "solo se hace pop de lo que se hizo push", así que lo dejé documentado en el comentario y en esta bitácora. Segundo, el nombre no se copia, solo se guarda el puntero: si el nombre viene de un buffer temporal (por ejemplo, el token que va leyendo el analizador), el ámbito se queda apuntando a basura. Para el harness funciona porque se usan literales como `"if"` y `"while"`, pero en el compilador real habría que usar `strdup` y liberarlo en el pop. Lo revisé con valgrind (`--leak-check=full`) y salió limpio, sin fugas ni accesos inválidos. Las dos funciones son O(1).

---
**Firma digital:** josessuarez 
