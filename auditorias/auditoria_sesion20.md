# Bitácora de Requerimientos y Auditoría IA — Sesión 20
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Hito 2: Analizador Semántico y Scopes  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Integración de Tabla de Símbolos y AST  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> `profundidad_de()` fue lo fácil: subo por `padre` con un contador de nivel y regreso el primero que declara el nombre, así el ámbito más interno gana el shadowing. Lo primero es revisar `NULL` y regresar -2, antes de leer cualquier campo. Donde me atoré fue en `contar_visibles()`: si `x` está en el bloque y en el global, solo debe contar una vez. Lo resolví guardando los nombres que ya vi en un arreglo local de 64 punteros y comparando con `strcmp` antes de contar uno nuevo. Funciona para el harness, pero tiene un límite fijo: si hay más de 64 nombres distintos, deja de contar. También limité el ciclo a `AMBITO_MAX` para no leer fuera del arreglo `simbolos` aunque `total` venga mal.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Contrato de Validación Semántica Integral |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy integrando la tabla de símbolos con los ámbitos de un compilador
en C11 (Hito 2). Se compila con gcc -std=c11 -Wall -Wextra -Werror
junto con un harness que ya tiene main(), así que no lleva main(). Solo
biblioteca estándar. Contrato (api_sesion20.h):

#define AMBITO_MAX 8
typedef struct Ambito {
    const char *nombre;
    const char *simbolos[AMBITO_MAX];
    int total;
    struct Ambito *padre;
} Ambito;
int profundidad_de(const Ambito *actual, const char *nombre);
int contar_visibles(const Ambito *actual);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- profundidad_de() regresa el nivel del ámbito más cercano que declara el
  nombre (0 = actual, 1 = padre...), -1 si no se ve y -2 si algún
  argumento es NULL.
- contar_visibles() cuenta cada nombre una sola vez aunque esté en varios
  ámbitos. Regresa -2 si actual es NULL.
- El ámbito más interno gana el shadowing.
- Sin malloc ni arreglos de tamaño fijo para los nombres vistos.
- Nunca leer simbolos[i] con i >= AMBITO_MAX, aunque total diga otra cosa.
- No cambies las firmas ni la estructura.

OUTPUT: Solo el archivo ia_sesion20_c.c (con #include "api_sesion20.h") y la
complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion20_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: respeta AMBITO_MAX aunque total venga mal)
 */
#include "api_sesion20.h"
#include <stddef.h>
#include <string.h>

/* Numero de simbolos que se pueden leer sin salir del arreglo. */
static int limite(const Ambito *a) {
    if (a->total < 0) return 0;
    return a->total > AMBITO_MAX ? AMBITO_MAX : a->total;
}

/* 1 si el ambito declara el nombre. O(k). */
static int declara(const Ambito *a, const char *nombre) {
    for (int i = 0; i < limite(a); i++) {
        if (a->simbolos[i] != NULL && strcmp(a->simbolos[i], nombre) == 0) return 1;
    }
    return 0;
}

/* Nivel del ambito mas cercano que declara el nombre (0 = actual).
 * -1 si no se ve, -2 si algun argumento es NULL. O(n). */
int profundidad_de(const Ambito *actual, const char *nombre) {
    if (actual == NULL || nombre == NULL) return -2;
    int nivel = 0;
    for (const Ambito *a = actual; a != NULL; a = a->padre, nivel++) {
        if (declara(a, nombre)) return nivel;
    }
    return -1;
}

/* Cuenta nombres distintos visibles. Un simbolo cuenta solo si no aparece
 * antes en su mismo ambito ni en un ambito mas interno. Sin memoria extra.
 * O(n^2) con n = simbolos de toda la cadena. -2 si actual es NULL. */
int contar_visibles(const Ambito *actual) {
    if (actual == NULL) return -2;
    int cuenta = 0;
    for (const Ambito *a = actual; a != NULL; a = a->padre) {
        for (int i = 0; i < limite(a); i++) {
            const char *s = a->simbolos[i];
            if (s == NULL) continue;
            int repetido = 0;
            for (int j = 0; j < i && !repetido; j++) {
                if (a->simbolos[j] != NULL && strcmp(a->simbolos[j], s) == 0) repetido = 1;
            }
            for (const Ambito *b = actual; b != a && !repetido; b = b->padre) {
                if (declara(b, s)) repetido = 1;
            }
            if (!repetido) cuenta++;
        }
    }
    return cuenta;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Casos complejos de shadowing y ámbito

### Salida del harness
```text
=== AUDITORIA SESION 20 - Resolucion de nombres y visibilidad ===
Caso                               esperado:              obtenido:              resultado
[ 1] nombre en el ambito actual         esperado: 0            obtenido: 0            OK
[ 2] nombre dos niveles arriba          esperado: 2            obtenido: 2            OK
[ 3] shadowing gana el interno          esperado: 0            obtenido: 0            OK
[ 4] nombre no visible                  esperado: -1           obtenido: -1           OK
[ 5] visibles sin repetir               esperado: 3            obtenido: 3            OK
[ 6] ambito NULL                        esperado: -2           obtenido: -2           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion20.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> Le pedí a la IA que no usara un arreglo fijo como el mío. Su propuesta cuenta un símbolo solo si no aparece antes en su mismo ámbito ni en un ámbito más interno, así que no necesita memoria extra y no tiene el límite de 64. Lo que omitió fue el caso de un nombre repetido dentro del **mismo** ámbito: solo revisaba los ámbitos internos, y si `simbolos` traía `x` dos veces en el global lo contaba doble. Le agregué el ciclo que revisa las posiciones anteriores del mismo ámbito. Tampoco respetaba `AMBITO_MAX` si `total` venía más grande; lo puse en la función `limite()`. El costo es O(n²) en el número total de símbolos, a cambio de O(1) de memoria; para los tamaños de un bloque normal no se nota. Lo pasé por sanitizers y no hubo accesos inválidos.

---
**Firma digital:** josessuarez
