/* evaluador.c - calcula el valor de una expresion del AST (--run).
 * Responsable: Oscar.
 *
 * Es el mismo evaluador recursivo del laboratorio 07, pero con variables:
 *   numero    -> su valor
 *   variable  -> el valor que tenga en el Entorno (entorno.c)
 *   neg       -> el valor del hijo con el signo cambiado
 *   binaria   -> primero los dos hijos, luego el operador
 * Las comparaciones (< <= > >= == !=) dan 1 si se cumplen y 0 si no.
 *
 * Las cuentas se hacen en long long y al final se pasan a int: asi un
 * desbordamiento (por ejemplo INT_MAX + 1) da la vuelta como en un int de
 * 32 bits en lugar de ser comportamiento indefinido de C.
 */
#include "evaluador.h"

#include <string.h>

/* Aplica el operador 'op' a 'a' y 'b'. Con '/' revisa el divisor ANTES. */
static int aplicar(const char *op, int a, int b, int *error) {
    long long x = a, y = b;

    if (strcmp(op, "+") == 0) return (int)(x + y);
    if (strcmp(op, "-") == 0) return (int)(x - y);
    if (strcmp(op, "*") == 0) return (int)(x * y);
    if (strcmp(op, "/") == 0) {
        if (y == 0) {
            *error = EVAL_DIV_CERO;
            return 0;
        }
        return (int)(x / y);            /* INT_MIN / -1 cabe en long long: no truena */
    }

    if (strcmp(op, "<") == 0)  return a < b;
    if (strcmp(op, "<=") == 0) return a <= b;
    if (strcmp(op, ">") == 0)  return a > b;
    if (strcmp(op, ">=") == 0) return a >= b;
    if (strcmp(op, "==") == 0) return a == b;
    if (strcmp(op, "!=") == 0) return a != b;

    return 0;                           /* el parser solo genera los operadores de arriba */
}

/* Hace el trabajo recursivo. *error ya viene en EVAL_OK desde evaluar_expr;
 * en cuanto un hijo marca error se deja de calcular y se regresa 0. */
static int calcular(const ASTNode *e, Entorno *env, int *error) {
    if (e == NULL) return 0;            /* no pasa si el parser no tuvo errores */

    switch (e->type) {
    case AST_NUM:
        return e->num_val;

    case AST_VAR: {
        int v = 0;
        if (!entorno_leer(env, e->text, &v)) {
            *error = EVAL_VAR_DESCONOCIDA;
            return 0;
        }
        return v;
    }

    case AST_NEG: {
        int v = calcular(e->left, env, error);
        if (*error != EVAL_OK) return 0;
        return (int)(-(long long)v);
    }

    case AST_BINARIA: {
        int a = calcular(e->left, env, error);
        if (*error != EVAL_OK) return 0;    /* el error sube sin calcular el otro lado */
        int b = calcular(e->right, env, error);
        if (*error != EVAL_OK) return 0;
        return aplicar(e->text, a, b, error);
    }

    default:
        return 0;                       /* las sentencias no son expresiones */
    }
}

int evaluar_expr(const ASTNode *e, Entorno *env, int *error) {
    int sin_uso = EVAL_OK;
    if (error == NULL) error = &sin_uso;   /* si no me dan donde avisar, no me caigo */
    *error = EVAL_OK;
    return calcular(e, env, error);
}
