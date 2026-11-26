/* semantica.h - tabla de simbolos con ambitos y diagnosticos (--check). */
#ifndef SEMANTICA_H
#define SEMANTICA_H

#include "ast.h"

/* Recorre el AST y reporta E01 (no declarada / sin valor), E02
 * (redeclaracion en el mismo ambito) y E04 (division entre 0 constante).
 * Devuelve el total de errores (los que ya traia + los semanticos). */
int sema_check_program(const char *filename, const ASTNode *root, int initial_errors);

#endif /* SEMANTICA_H */
