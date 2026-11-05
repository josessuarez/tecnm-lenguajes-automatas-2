# Bitácora de Requerimientos y Auditoría IA — Sesión 05
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Recorridos del AST y Notación Polaca  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** José Manuel Suárez Huex  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Impresión en Postorden / RPN  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> - **No se puede usar `printf`:** todo se tiene que ir pegando en el buffer. Primero convierto cada nodo a texto con `snprintf` en un arreglo `token[32]` y luego lo agrego con `strcat`.
> - **El espacio de separación:** el harness compara el texto exacto, así que no puede quedar un espacio al final. Lo resolví con una bandera `primer_token` que paso por puntero: si es el primer token no pongo espacio, y para los demás pongo el espacio antes del token.
> - **No pasarme del buffer:** antes de escribir calculo cuánto ocupa el token más su espacio y solo lo agrego si `len_actual + espacio_requerido < cap`, para que siempre quede lugar para el `'\0'`.
> - **La recursión:** `postorden` limpia el buffer con `buf[0] = '\0'`, así que si la llamaba recursivamente se borraba todo en cada nodo. Por eso hice una función auxiliar `postorden_recursivo` que recorre izquierdo, derecho y raíz.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C99 / GCC o Rust 2021 sin bibliotecas no estándar |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Invariantes de recorrido recursivo |
| **Output (O)** | Solo código modular y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
Contexto: Estoy programando en C11 y compilo con gcc -std=c11 -Wall -Wextra
-Werror. Tengo el archivo api_sesion05.h que no puedo cambiar, donde está el
struct ASTNode con los campos tipo (NODO_NUM o NODO_BINOP), valor, op, izq y der.

Rol: Actúa como ingeniero de compiladores que programa en C de forma segura.

Especificación: Escribe la función void postorden(ASTNode *n, char *buf, int cap)
que recorra el árbol en postorden (primero izquierdo, luego derecho y al final
la raíz) y guarde en buf la notación polaca inversa. Los tokens van separados
por un solo espacio, sin espacio al inicio ni al final (por ejemplo
"3 5 2 * +"). Si el árbol es NULL, buf queda como cadena vacía. Nunca escribas
más de cap - 1 caracteres y deja siempre el '\0' al final. No uses printf.

Output: Dame solo el archivo .c que incluya "api_sesion05.h", sin main().
```

---

## 3. Código Devuelto por el Modelo
```c
#include "api_sesion05.h"
#include <stdio.h>
#include <string.h>

static void postorden_recursivo(ASTNode *n, char *buf, int cap, int *primer_token) {
    if (n == NULL) return;
    postorden_recursivo(n->izq, buf, cap, primer_token);
    postorden_recursivo(n->der, buf, cap, primer_token);

    char token[32];
    if (n->tipo == NODO_NUM) {
        snprintf(token, sizeof(token), "%d", n->valor);
    } else if (n->tipo == NODO_BINOP) {
        snprintf(token, sizeof(token), "%c", n->op);
    } else {
        return;
    }

    int len_actual = strlen(buf);
    int len_token = strlen(token);
    int espacio_requerido = (*primer_token ? 0 : 1) + len_token;

    if (len_actual + espacio_requerido < cap) {
        if (!(*primer_token)) {
            strcat(buf, " ");
        }
        strcat(buf, token);
        *primer_token = 0;
    }
}

void postorden(ASTNode *n, char *buf, int cap) {
    if (buf == NULL || cap <= 0) return;
    buf[0] = '\0';
    int primer_token = 1;
    postorden_recursivo(n, buf, cap, &primer_token);
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Validación contra desbordamiento de pila

### Salida del harness
```text
=== AUDITORIA SESION 05 - Recorrido postorden y RPN ===
Caso                               esperado:              obtenido:              resultado
[ 1] hoja 42                            esperado: 42           obtenido: 42           OK
[ 2] 3+(5*2)                            esperado: 3 5 2 * +    obtenido: 3 5 2 * +    OK
[ 3] arbol NULL                         esperado:              obtenido:              OK
[ 4] (1+2)*(3+4)
     esperado: 1 2 + 3 4 + *
     obtenido: 1 2 + 3 4 + *
     -> OK
[ 5] 8-3                                esperado: 8 3 -        obtenido: 8 3 -        OK

RESULTADO: 5/5 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion05.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La versión de la IA pasó las 5 pruebas y nunca se sale del buffer, pero tiene un problema cuando **un token no cabe**: en vez de detenerse, se lo salta y sigue escribiendo los que vienen después si esos sí caben. El harness no lo detecta porque siempre usa buffers de 32 o 64 caracteres.
>
> Lo probé con `9*12345` en un buffer de 7. La respuesta correcta sería `"9 12345 *"`, que no cabe. La IA devuelve **`"9 *"`**: se brincó el `12345` y dejó un `*` con un solo operando, o sea, una RPN que parece válida pero está mal y no avisa nada.
>
> Lo solucioné marcando la bandera con -1 cuando un token no cabe y revisándola antes de escribir el siguiente, para cortar la escritura ahí:
>
> ```c
> postorden_recursivo(n->der, buf, cap, primer_token);
> if (*primer_token == -1) return;   /* ya no cupo un token: no seguir */
> ...
> } else {
>     *primer_token = -1;
> }
> ```
>
> Con eso el mismo caso da `"9"`: queda incompleto, pero sin tokens saltados. Volví a correr el harness con el cambio y sigue en 5/5.
>
> **Complejidad:** O(n) en tiempo porque visita cada nodo una vez (aunque `strlen` y `strcat` recorren el buffer en cada token, y eso en el peor caso lo hace más lento) y O(h) de pila por la recursión, donde h es la altura del árbol.

---
**Firma digital:** josessuarez

