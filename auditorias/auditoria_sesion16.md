# Bitácora de Requerimientos y Auditoría IA — Sesión 16
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Firmas de Función y Parámetros  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Registro de prototipos y aridad  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo primero fue el orden de las validaciones: si el catálogo o el nombre vienen en `NULL`, `strcmp` truena, así que eso se revisa antes del ciclo, junto con la aridad negativa (todos regresan -1). Donde sí me atoré fue en distinguir 0 de -1: al principio regresaba 0 en cuanto el nombre no coincidía, y eso hacía que una función inexistente saliera como "aridad incorrecta". Lo resolví con una bandera `encontrada`: si al terminar el recorrido nunca apareció el nombre es -1, y si apareció pero ninguna aridad coincidió es 0. También me fijé en que se compara contra `catalogo[i].aridad` y no contra `total`, que es el número de firmas, no de parámetros. Y como el contrato dice que se recorre el catálogo completo, no me salgo en la primera coincidencia del nombre, por si la misma función viene registrada dos veces con distinta aridad.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Coincidencia estricta de parámetros |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo el chequeo de llamadas a función de un compilador
en C11. Se compila con gcc -std=c11 -Wall -Wextra -Werror
junto con un harness que ya tiene main(), así que no lleva main(). Solo
biblioteca estándar. Contrato (api_sesion16.h):

typedef struct Funcion { const char *nombre; int aridad; } Funcion;
int verificar_llamada(const Funcion *catalogo, int total, const char *nombre, int nargs);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- Busca el nombre con comparación exacta (strcmp) y recorre el catálogo
  completo.
- Regresa 1 si existe una firma con ese nombre y esa aridad, 0 si el nombre
  existe pero ninguna aridad coincide, y -1 si el nombre no está.
- Una función registrada con 0 parámetros solo acepta 0 argumentos.
- catalogo NULL, nombre NULL o nargs negativo regresan -1 sin crashear.
- No cambies la firma ni la estructura.

OUTPUT: Solo el archivo ia_sesion16_c.c (con #include "api_sesion16.h") y la
complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion16_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: recorre todo el catalogo y salta nombres NULL)
 */
#include "api_sesion16.h"
#include <stddef.h>
#include <string.h>

/* Busca el nombre en todo el catalogo y compara la aridad. O(n).
 * 1 = llamada valida, 0 = existe pero la aridad no coincide,
 * -1 = no existe o argumentos invalidos. */
int verificar_llamada(const Funcion *catalogo, int total, const char *nombre, int nargs) {
    if (catalogo == NULL || nombre == NULL || nargs < 0 || total < 0) return -1;

    int existe = 0;
    for (int i = 0; i < total; i++) {
        const char *n = catalogo[i].nombre;
        if (n == NULL || strcmp(n, nombre) != 0) continue;
        existe = 1;
        if (catalogo[i].aridad == nargs) return 1;
    }
    return existe ? 0 : -1;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Llamadas con número incorrecto de args

### Salida del harness
```text
=== AUDITORIA SESION 16 - Registro de prototipos y aridad ===
Caso                               esperado:              obtenido:              resultado
[ 1] suma con 3 argumentos              esperado: 1            obtenido: 1            OK
[ 2] suma con 2 argumentos              esperado: 0            obtenido: 0            OK
[ 3] suma con 4 argumentos              esperado: 0            obtenido: 0            OK
[ 4] funcion no registrada              esperado: -1           obtenido: -1           OK
[ 5] main sin argumentos                esperado: 1            obtenido: 1            OK
[ 6] nombre NULL                        esperado: -1           obtenido: -1           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion16.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La primera versión que me dio la IA regresaba en la primera firma con el mismo nombre (`return catalogo[i].aridad == nargs;`). Con el harness pasa 6/6 porque cada caso solo tiene una firma, pero el contrato dice que se recorre el catálogo completo: si `suma` estuviera registrada con 2 y con 3 parámetros, una llamada con 3 argumentos saldría como aridad incorrecta. Se lo corregí para que siga buscando y solo regrese 0 al final. Además, la IA no revisaba que el nombre de una firma del catálogo pudiera venir en `NULL`, y `strcmp` con NULL es comportamiento indefinido; le agregué el `continue` para saltarla. También agregó `total < 0`, que yo no tenía, y lo dejé. Compilé con `-fsanitize=address,undefined` y salió limpio. La función es O(n) en el tamaño del catálogo y no usa memoria extra.

---
**Firma digital:** josessuarez
