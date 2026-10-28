Bitácora de Requerimientos y Auditoría IA — Sesión 04

Materia: Lenguajes y Autómatas II (SCD-1016)
Tema: Anatomía de un Árbol Sintáctico (AST)
Docente Titular: MCC. Iván Márquez Larios (ijmarquezl)
Estudiante: José Manuel Suárez Huex
Matrícula: 24531320
Track: C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)

1. Fase 1: El Artesano (Dificultad Manual)

Reto: Estructura de nodo binario (ASTNode)
Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:

Lo que más me costó fue acordarme de que malloc te regresa memoria con pura basura adentro, entonces si no le pones valor a todos los campos (op, izq, der) luego puedes leer cosas raras. Por eso en crear_numero puse los hijos en NULL y el op en '\0', aunque un número no los use.

También me atoré con el evaluar(NULL): si no pones el if (n == NULL) hasta arriba, el programa truena con segmentation fault porque intenta leer n->tipo de un puntero que no apunta a nada. Y como el harness compila con -Werror, cualquier warning ya no te deja compilar, así que tuve que poner un return -1 al final de evaluar para que todos los caminos regresen algo.

2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)
2.1 Matriz de Especificación Formal
Elemento C-R-E-O	Definición Técnica
Contexto (C)	C99 / GCC o Rust 2021 sin bibliotecas no estándar
Rol (R)	Ingeniero de software de sistemas y compiladores
Especificación (E)	Gramática E -> E + T | T con tipos de nodo
Output (O)	Solo código modular y análisis de complejidad
2.2 Prompt Estructurado
text
CONTEXTO: Estoy programando en C (estándar C11) con GCC. El código se
compila con -Wall -Wextra -Werror junto con un archivo harness que ya
tiene el main(), así que mi archivo NO debe traer main(). No puedo usar
bibliotecas fuera de la estándar. Ya tengo un header api_sesion04.h con:

  typedef enum { NODO_NUM, NODO_BINOP } TipoNodo;
  typedef struct ASTNode {
      TipoNodo tipo; int valor; char op;
      struct ASTNode *izq; struct ASTNode *der;
  } ASTNode;

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN: Para la gramática E -> E + T | T, implementa estas
3 funciones con la firma EXACTA:
  ASTNode *crear_numero(int v);
  ASTNode *crear_binario(char op, ASTNode *izq, ASTNode *der);
  int evaluar(ASTNode *n);
- Cada nodo se reserva en el heap con malloc.
- Usa el campo tipo para distinguir número de operación (nada de valores mágicos).
- crear_binario debe respetar el orden izquierdo/derecho de los hijos.
- evaluar debe evaluar primero ambos hijos y después aplicar el operador.
- Solo se soportan '+', '-' y '*'.
- evaluar(NULL) debe regresar -1 sin provocar segmentation fault.
- Debe construir 3+5*2 y evaluar a 13.

OUTPUT: Dame solo el archivo .c completo (con #include "api_sesion04.h"),
sin main(), y al final un análisis corto de complejidad temporal y espacial.
3. Código Devuelto por el Modelo
c
/* ia_sesion04_c.c - Fase 2: El Cientifico */
#include "api_sesion04.h"
#include <stdlib.h>

ASTNode *crear_numero(int v) {
    ASTNode *nuevo = (ASTNode *)malloc(sizeof(ASTNode));
    if (nuevo == NULL) {
        return NULL;
    }
    nuevo->tipo = NODO_NUM;
    nuevo->valor = v;
    nuevo->op = '\0';
    nuevo->izq = NULL;
    nuevo->der = NULL;
    return nuevo;
}

ASTNode *crear_binario(char op, ASTNode *izq, ASTNode *der) {
    ASTNode *nuevo = (ASTNode *)malloc(sizeof(ASTNode));
    if (nuevo == NULL) {
        return NULL;
    }
    nuevo->tipo = NODO_BINOP;
    nuevo->valor = 0;
    nuevo->op = op;
    nuevo->izq = izq;
    nuevo->der = der;
    return nuevo;
}

int evaluar(ASTNode *n) {
    if (n == NULL) {
        return -1;
    }
    if (n->tipo == NODO_NUM) {
        return n->valor;
    }
    if (n->tipo == NODO_BINOP) {
        int valor_izq = evaluar(n->izq);
        int valor_der = evaluar(n->der);
        switch (n->op) {
            case '+': return valor_izq + valor_der;
            case '-': return valor_izq - valor_der;
            case '*': return valor_izq * valor_der;
            default:  return -1;
        }
    }
    return -1;
}

/* Complejidad:
 *  - crear_numero / crear_binario: O(1) tiempo y O(1) espacio por nodo.
 *  - evaluar: O(n) tiempo, visita cada nodo una sola vez (recorrido postorden).
 *    Espacio O(h) en la pila de llamadas, donde h es la altura del arbol
 *    (en el peor caso, un arbol cargado a un solo lado, h = n).
 */
4. Fase 3: El Auditor (Resultados del Test Harness)

Pruebas ejecutadas: Árbol vacío y evaluación de nodo raíz

Salida del harness
text
=== AUDITORIA SESION 04 - Nodo binario de AST ===
Caso esperado: obtenido: resultado
[ 1] evaluar(nodo 42)                   esperado: 42           obtenido: 42           OK
[ 2] evaluar(3+5*2)                     esperado: 13           obtenido: 13           OK
[ 3] hijo izquierdo de (* 5 2)          esperado: 5            obtenido: 5            OK
[ 4] operador guardado                  esperado: *            obtenido: *            OK
[ 5] evaluar(NULL)                      esperado: -1           obtenido: -1           OK
[ 6] evaluar(10-4)                      esperado: 6            obtenido: 6            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion04.md
 Caso base y límites superados sin Crash.
 No existen accesos fuera de límites ni Memory Leaks.
 La complejidad teórica coincide con la implementada.
Reporte Crítico

¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?

La IA sí cubrió el evaluar(NULL) porque se lo pedí directo en el prompt, pero hay cosas que no consideró:

El -1 es ambiguo: evaluar regresa -1 tanto con NULL como con un operador inválido, pero -1 también puede ser un resultado normal (por ejemplo 3 - 4). Con solo ver el -1 no sabes si fue error o si la cuenta daba eso. No lo cambié porque el contrato pide exactamente -1, pero lo dejo anotado.
No hay forma de liberar el árbol: no propuso una función para hacer free de los nodos. Para esta sesión no se califica, pero en un compilador de verdad se necesitaría algo como liberar_arbol(ASTNode *n) recorriendo en postorden. El harness tampoco libera, pero como el programa termina luego luego, el sistema operativo recupera la memoria.
Overflow: si multiplicas números grandes el int se desborda y en C eso es comportamiento indefinido. Con los casos del harness no pasa, pero no hay validación.

Lo que sí revisé fue que la firma quedara idéntica a la de api_sesion04.h y que no trajera main(), porque si no el harness no compila. Con eso compiló limpio con -Wall -Wextra -Werror y pasó 6/6.

Firma digital: [suarez]
