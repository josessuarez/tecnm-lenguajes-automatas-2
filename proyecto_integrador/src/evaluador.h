/* evaluador.h - calcula el valor de una expresion del AST (--run).
 * Responsable: Oscar.
 */
#ifndef EVALUADOR_H
#define EVALUADOR_H

#include "ast.h"
#include "entorno.h"

/* Valores que puede dejar evaluar_expr en *error. */
#define EVAL_OK              0
#define EVAL_DIV_CERO        1  /* division entre cero en ejecucion */
#define EVAL_VAR_DESCONOCIDA 2  /* variable que no esta en el entorno;
                                   no deberia pasar si --check paso */

/* Evalua 'e' con los valores de 'env' y devuelve el resultado.
 * Nodos que acepta: AST_NUM, AST_VAR, AST_NEG y AST_BINARIA.
 * Las comparaciones (< <= > >= == !=) dan 1 o 0.
 * Pone *error en EVAL_OK al empezar. Si encuentra un error deja el codigo
 * en *error, deja de calcular y devuelve 0. Con '/' revisa el divisor
 * ANTES de dividir. */
int evaluar_expr(const ASTNode *e, Entorno *env, int *error);

#endif /* EVALUADOR_H */
