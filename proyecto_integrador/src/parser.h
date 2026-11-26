/* parser.h - parser descendente recursivo LL(1) (docs/gramatica.md, seccion 3). */
#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#include "ast.h"
#include "lexer.h"

#define MAX_ERRORES 20   /* la especificacion pide imprimir maximo 20 errores */

/* Recibe los tokens del lexer (terminados en TOK_EOF) y devuelve la raiz
 * AST_PROGRAMA. Los errores S01 se imprimen en stdout y se cuentan en
 * *out_errors. */
ASTNode *parse_program(const char *filename, const Token *tokens, size_t count, int *out_errors);

#endif /* PARSER_H */
