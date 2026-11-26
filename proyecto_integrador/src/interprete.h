/* interprete.h - ejecuta el programa recorriendo el AST (--run, Hito 2).
 * Responsable: Jordi.
 *
 * compilador.c llama a interpretar() solo si la revision de --check no
 * encontro errores.
 */
#ifndef INTERPRETE_H
#define INTERPRETE_H

#include "ast.h"

/* Ejecuta 'programa' (la raiz AST_PROGRAMA) sentencia por sentencia, con un
 * entorno nuevo que libera al terminar. Cada print escribe el valor en
 * stdout seguido de '\n', y nada mas.
 * Devuelve 0 si el programa termino bien y 1 si hubo un error en ejecucion
 * (por ejemplo, division entre cero); el mensaje de error va a stderr. */
int interpretar(const ASTNode *programa);

#endif /* INTERPRETE_H */
