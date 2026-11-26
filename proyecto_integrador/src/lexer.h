/* lexer.h - analizador lexico de Lenguajito (docs/gramatica.md, seccion 2). */
#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    TOK_KW,     /* int, if, else, while, print */
    TOK_ID,     /* identificadores */
    TOK_NUM,    /* literales enteros */
    TOK_OP,     /* + - * / < <= > >= == != = */
    TOK_DELIM,  /* ( ) { } ; */
    TOK_EOF,    /* fin de archivo */
    TOK_ERROR   /* caracter invalido (error lexico S01) */
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int line;
    int col;
} Token;

/* Escanea todo el codigo fuente y devuelve un arreglo de tokens que
 * termina en TOK_EOF. Si print_tokens es true imprime "linea:columna TIPO
 * lexema" (modo --tokens) y los errores S01 en stdout. */
Token *lexer_scan_all(const char *filename, const char *src, size_t *out_count,
                      bool print_tokens, int *out_lex_errors);

void lexer_free_tokens(Token *tokens, size_t count);

#endif /* LEXER_H */
