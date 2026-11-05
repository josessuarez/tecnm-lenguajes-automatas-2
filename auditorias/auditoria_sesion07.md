# Bitácora de Requerimientos y Auditoría IA — Sesión 07
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Evaluación de Expresiones Aritméticas  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Jose Manuel Suarez Huex
**Matrícula:** 24531320 
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Evaluador recursivo de operaciones  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo más difícil fue el orden de las cosas. Al principio hacía la división y luego checaba si el divisor era 0, pero así ya truena antes de llegar al if, entonces tuve que checar el divisor primero. También me costó ver cómo hacer que el error se "suba" al padre: si un hijo falla, el padre ya no debe seguir calculando, por eso después de evaluar cada hijo reviso si `*error` ya no es 0 y me salgo. Y lo del `NULL` también, si no lo checo primero el programa se cae al hacer `n->tipo`.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C99 / GCC o Rust 2021 sin bibliotecas no estándar |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Manejo estricto de división entre cero |
| **Output (O)** | Solo código modular y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
Contexto: Estoy programando en C (C11 con GCC, compilando con -Wall -Wextra -Werror),
sin bibliotecas que no sean estándar. Tengo este header que no puedo cambiar:

typedef enum { NODO_NUM, NODO_BINOP } TipoNodo;
typedef struct ASTNode {
    TipoNodo tipo; int valor; char op;
    struct ASTNode *izq; struct ASTNode *der;
} ASTNode;
int evaluar(ASTNode *n, int *error);

Rol: Actúa como ingeniero de software de sistemas y compiladores.

Especificación:
- evaluar() recorre el árbol de forma recursiva y regresa el resultado.
- Operadores: + - * / %.
- Antes de dividir o sacar módulo, revisa que el divisor no sea 0. Si es 0,
  pon *error = 1 y no hagas la operación.
- Si el nodo es NULL o el operador no es válido, pon *error = 2 sin que se caiga.
- Si un hijo ya marcó error, el padre no debe seguir calculando, solo regresa.
- No cambies la firma de la función y no pongas main().

Output: Solo el código de la función (puede tener funciones auxiliares static)
y al final un análisis corto de complejidad en tiempo y memoria.
```

---

## 3. Código Devuelto por el Modelo
```c
#include "api_sesion07.h"
#include <limits.h>
#include <stddef.h>   /* para NULL (se lo agregue yo, la IA no lo puso) */

#define ERR_OK        0
#define ERR_DIV_CERO  1
#define ERR_INVALIDO  2

/* Marca el error solo si no habia uno antes (se conserva el primero) */
static int fallar(int *error, int codigo) {
    if (error != NULL && *error == ERR_OK) {
        *error = codigo;
    }
    return 0;
}

/* Complejidad: O(n) en tiempo (visita cada nodo una vez)
 *              O(h) en memoria por la pila de recursion (h = altura) */
int evaluar(ASTNode *n, int *error) {
    if (error != NULL && *error != ERR_OK) return 0;   /* ya hubo error */
    if (n == NULL) return fallar(error, ERR_INVALIDO);

    if (n->tipo == NODO_NUM) return n->valor;
    if (n->tipo != NODO_BINOP) return fallar(error, ERR_INVALIDO);

    int a = evaluar(n->izq, error);
    if (error != NULL && *error != ERR_OK) return 0;
    int b = evaluar(n->der, error);
    if (error != NULL && *error != ERR_OK) return 0;

    switch (n->op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) return fallar(error, ERR_DIV_CERO);
            if (a == INT_MIN && b == -1) return fallar(error, ERR_INVALIDO);
            return a / b;
        case '%':
            if (b == 0) return fallar(error, ERR_DIV_CERO);
            if (a == INT_MIN && b == -1) return fallar(error, ERR_INVALIDO);
            return a % b;
        default:
            return fallar(error, ERR_INVALIDO);
    }
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Expresiones complejas y división por 0

### Salida del harness
```text
=== AUDITORIA SESION 07 - Evaluador recursivo con deteccion de errores ===
Caso                               esperado:              obtenido:              resultado
[ 1] (2+3)*4                            esperado: 20           obtenido: 20           OK
[ 2] 7/0 -> error 1                     esperado: 1            obtenido: 1            OK
[ 3] 7%0 -> error 1                     esperado: 1            obtenido: 1            OK
[ 4] arbol NULL -> error 2              esperado: 2            obtenido: 2            OK
[ 5] 10-2-3                             esperado: 5            obtenido: 5            OK
[ 6] (8/2)+1                            esperado: 5            obtenido: 5            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion07.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La primera vez que compilé el código de la IA no jaló: usaba `NULL` pero no incluyó `<stddef.h>`, y como el harness compila con `-Werror` me tiró error. Le agregué el include y ya pasó las 6 pruebas.
>
> Luego hice unas pruebas extra que el harness no trae. `(7/0)+1` sí regresa error 1 sin seguir calculando, y un operador con un hijo `NULL` regresa error 2, así que eso está bien. Lo que la IA no cuidó es el desbordamiento en `+`, `-` y `*`: con `INT_MAX + 1` el resultado da `-2147483648` y no marca ningún error (compilando con `-fsanitize=undefined` sale "signed integer overflow"). En la división sí checó el caso `INT_MIN / -1`, pero en la suma, la resta y la multiplicación no. El contrato de la sesión solo pide división entre cero, así que no lo cambié, pero lo dejo anotado para cuando hagamos el evaluador completo del proyecto.

---
**Firma digital:** jossesuarez 
