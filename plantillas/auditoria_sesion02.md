%%writefile auditorias/auditoria_sesion02.md
# Bitácora de Requerimientos y Auditoría IA — Sesión 02
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Gestión en GitHub Web y Nodos Dinámicos  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320 
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Lista enlazada de tokens en Heap  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo primero en lo que me atoré fue el campo `lexema`. Yo pensaba que era un `char *` y le quería hacer `malloc` aparte, pero en el `.h` es `char lexema[16]`, o sea que el texto ya vive dentro del nodo. Por eso lo tuve que copiar letra por letra cuidando no pasarme de 15 caracteres y dejar siempre el `'\0'` al final, porque si no me salgo del arreglo.
> Otra cosa: la plantilla no traía `#include <stdlib.h>`, y como se compila con `-Wall -Wextra -Werror`, sin eso `malloc` y `free` no compilan.
> En `agregar_token` me costó entender por qué recibe `NodoToken **cabeza`. Es porque, si la lista está vacía, la función tiene que cambiar la cabeza misma, no solo una copia.
> En `liberar_lista` hay que guardar el `siguiente` **antes** del `free`, porque si no estaría leyendo memoria ya liberada (use-after-free).
> Siendo honesto, me atoré y pedí ayuda para entender esto, no me salió 100% solo.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C99 / GCC o Rust 2021 sin bibliotecas no estándar |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Gestión de memoria dinámica C99/Rust |
| **Output (O)** | Solo código modular y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo una lista enlazada de tokens en el heap para un
compilador, en C (C99/C11 con GCC, solo biblioteca estandar). Un harness
compila mi archivo con: gcc -std=c11 -Wall -Wextra -Werror, asi que cualquier
warning cuenta como error. El struct ya esta definido en api_sesion02.h:

typedef struct NodoToken {
    char lexema[16];
    struct NodoToken *siguiente;
} NodoToken;

ROL: Actua como ingeniero de software de sistemas y compiladores que sabe
manejar memoria dinamica sin fugas.

ESPECIFICACION: Implementa estas 4 funciones SIN cambiar las firmas:
- NodoToken *crear_nodo(const char *lexema);  -> malloc del nodo, copiar el
  lexema (max 15 caracteres + '\0'), siguiente = NULL.
- void agregar_token(NodoToken **cabeza, const char *lexema); -> agregar al
  FINAL, funciona aunque la lista este vacia.
- int contar_tokens(NodoToken *cabeza); -> numero de nodos, 0 si es NULL.
- void liberar_lista(NodoToken *cabeza); -> liberar todo sin use-after-free,
  liberar_lista(NULL) no debe caerse.
Reglas: cada nodo con malloc, nada de arreglos globales, sin main().

OUTPUT: Solo el archivo .c que incluya "api_sesion02.h", modular, con
comentarios cortos, y al final la complejidad de cada funcion.
```

---

## 3. Código Devuelto por el Modelo
```c
#include "api_sesion02.h"
#include <stdio.h>
#include <stdlib.h>

NodoToken *crear_nodo(const char *lexema) {
    NodoToken *nodo = malloc(sizeof *nodo);
    if (!nodo) {
        return NULL;
    }
    /* snprintf corta solo si pasa de 15 y siempre pone '\0' */
    snprintf(nodo->lexema, sizeof nodo->lexema, "%s", lexema ? lexema : "");
    nodo->siguiente = NULL;
    return nodo;
}

void agregar_token(NodoToken **cabeza, const char *lexema) {
    NodoToken **p;
    NodoToken *nuevo;

    if (!cabeza) {
        return;
    }
    nuevo = crear_nodo(lexema);
    if (!nuevo) {
        return;
    }
    /* avanzar hasta el puntero NULL del final y colgar ahi el nodo */
    p = cabeza;
    while (*p) {
        p = &(*p)->siguiente;
    }
    *p = nuevo;
}

int contar_tokens(NodoToken *cabeza) {
    int n = 0;
    for (; cabeza; cabeza = cabeza->siguiente) {
        n++;
    }
    return n;
}

void liberar_lista(NodoToken *cabeza) {
    while (cabeza) {
        NodoToken *sig = cabeza->siguiente;
        free(cabeza);
        cabeza = sig;
    }
}

/* Complejidad:
 * crear_nodo    -> O(1)
 * agregar_token -> O(n)  (recorre hasta el final)
 * contar_tokens -> O(n)
 * liberar_lista -> O(n)
 */
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Prevención de Use-After-Free y cadenas nulas

### Salida del harness
**Versión Artesano:**
```text
=== AUDITORIA SESION 02 - Lista enlazada de tokens en heap ===
Caso                               esperado:              obtenido:              resultado
[ 1] crear_nodo("if")->lexema           esperado: if           obtenido: if           OK
[ 2] contar_tokens(3 tokens)            esperado: 3            obtenido: 3            OK
[ 3] orden: segundo token               esperado: dos          obtenido: dos          OK
[ 4] ultimo->siguiente == NULL          esperado: 1            obtenido: 1            OK
[ 5] contar_tokens(NULL)                esperado: 0            obtenido: 0            OK
[ 6] liberar_lista(NULL)                esperado: sin crash    obtenido: sin crash    OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion02.md
```

**Versión IA:**
```text
=== AUDITORIA SESION 02 - Lista enlazada de tokens en heap ===
Caso                               esperado:              obtenido:              resultado
[ 1] crear_nodo("if")->lexema           esperado: if           obtenido: if           OK
[ 2] contar_tokens(3 tokens)            esperado: 3            obtenido: 3            OK
[ 3] orden: segundo token               esperado: dos          obtenido: dos          OK
[ 4] ultimo->siguiente == NULL          esperado: 1            obtenido: 1            OK
[ 5] contar_tokens(NULL)                esperado: 0            obtenido: 0            OK
[ 6] liberar_lista(NULL)                esperado: sin crash    obtenido: sin crash    OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion02.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks* (en mi implementación: el lexema nunca pasa de 15 + '\0' y `liberar_lista` libera todos los nodos. Ojo: el harness no libera las listas de los casos 1 a 4, esas fugas son del harness, no de las funciones).
* [x] La complejidad teórica coincide con la implementada (crear O(1); agregar, contar y liberar O(n)).

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> **1. Lexemas de más de 15 caracteres:** la IA usa `snprintf`, que corta el texto **sin avisar**. Si llega un identificador largo como `contador_de_tokens`, se guarda recortado y nadie se entera. En un compilador eso es un error grave, porque dos variables distintas podrían terminar con el mismo nombre. No lo puedo arreglar sin cambiar el struct del contrato, así que lo dejo documentado como limitación. La solución real sería usar `char *lexema` con memoria dinámica, o al menos reportar error si `strlen(lexema) > 15`.
>
> **2. Fallo de `malloc`:** si `malloc` regresa NULL, `agregar_token` solo hace `return` y el token se pierde en silencio. Como la función es `void`, no tiene forma de avisarle a quien la llama. Esto tampoco se arregla sin cambiar la firma.
>
> **3. `agregar_token` es O(n):** cada vez recorre toda la lista para llegar al final, así que meter n tokens cuesta O(n²) en total. Para un archivo con muchos tokens eso es lento. Se arreglaría guardando un puntero al último nodo (`cola`), pero la firma del contrato no lo permite.
>
> **4. El harness no prueba todo:** no revisa fugas, porque el caso 6 solo checa que no truene, y en los casos 1 a 4 nunca se libera nada. Tampoco prueba lexemas largos ni `agregar_token(NULL, ...)`. Salir "en verde" no significa que el código esté perfecto.
>
> **Lo que evitó errores:** le pasé a la IA el struct exacto con `char lexema[16]`. Si no, lo más seguro es que asumiera `char *` y le hiciera `malloc`/`strcpy`, y no habría compilado. La IA respetó las firmas y no tuve que corregir nada antes de compilar.

---
**Firma digital:** JOSE MANUEL 
