/* lexer.c - analizador lexico de Lenguajito (docs/gramatica.md, seccion 2). */
#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *slice_str(const char *start, size_t len) {
    char *s = (char *)malloc(len + 1);
    if (!s) exit(2);
    memcpy(s, start, len);
    s[len] = '\0';
    return s;
}

static bool is_keyword(const char *s) {
    return strcmp(s, "int") == 0   ||
           strcmp(s, "if") == 0    ||
           strcmp(s, "else") == 0  ||
           strcmp(s, "while") == 0 ||
           strcmp(s, "print") == 0;
}

static const char *token_type_str(TokenType t) {
    switch (t) {
        case TOK_KW:    return "KW";
        case TOK_ID:    return "ID";
        case TOK_NUM:   return "NUM";
        case TOK_OP:    return "OP";
        case TOK_DELIM: return "DELIM";
        case TOK_EOF:   return "EOF";
        default:        return "ERROR";
    }
}

static void push_token(Token **arr, size_t *count, size_t *cap, TokenType type, char *lexeme, int line, int col) {
    if (*count >= *cap) {
        *cap = (*cap == 0) ? 64 : (*cap * 2);
        *arr = (Token *)realloc(*arr, (*cap) * sizeof(Token));
        if (!*arr) exit(2);
    }
    (*arr)[*count].type = type;
    (*arr)[*count].lexeme = lexeme;
    (*arr)[*count].line = line;
    (*arr)[*count].col = col;
    (*count)++;
}

Token *lexer_scan_all(const char *filename, const char *src, size_t *out_count, bool print_tokens, int *out_lex_errors) {
    Token *tokens = NULL;
    size_t count = 0, cap = 0;
    int line = 1, col = 1;
    int errors = 0;
    size_t i = 0;

    while (src[i] != '\0') {
        char c = src[i];

        /* 1. Espacios y saltos de línea */
        if (c == ' ' || c == '\t' || c == '\r') {
            i++; col++;
            continue;
        }
        if (c == '\n') {
            i++; line++; col = 1;
            continue;
        }

        /* 2. Comentarios de línea // */
        if (c == '/' && src[i + 1] == '/') {
            while (src[i] != '\0' && src[i] != '\n') {
                i++; col++;
            }
            continue;
        }

        int start_line = line;
        int start_col = col;

        /* 3. Identificadores y palabras reservadas */
        if (isalpha((unsigned char)c) || c == '_') {
            size_t start = i;
            while (isalnum((unsigned char)src[i]) || src[i] == '_') {
                i++; col++;
            }
            char *lex = slice_str(src + start, i - start);
            TokenType t = is_keyword(lex) ? TOK_KW : TOK_ID;
            if (print_tokens) printf("%d:%d %s %s\n", start_line, start_col, token_type_str(t), lex);
            push_token(&tokens, &count, &cap, t, lex, start_line, start_col);
            continue;
        }

        /* 4. Enteros sin signo */
        if (isdigit((unsigned char)c)) {
            size_t start = i;
            while (isdigit((unsigned char)src[i])) {
                i++; col++;
            }
            char *lex = slice_str(src + start, i - start);
            if (print_tokens) printf("%d:%d NUM %s\n", start_line, start_col, lex);
            push_token(&tokens, &count, &cap, TOK_NUM, lex, start_line, start_col);
            continue;
        }

        /* 5. Operadores de 2 caracteres: <=, >=, ==, != */
        if ((c == '<' && src[i + 1] == '=') ||
            (c == '>' && src[i + 1] == '=') ||
            (c == '=' && src[i + 1] == '=') ||
            (c == '!' && src[i + 1] == '=')) {
            char *lex = slice_str(src + i, 2);
            i += 2; col += 2;
            if (print_tokens) printf("%d:%d OP %s\n", start_line, start_col, lex);
            push_token(&tokens, &count, &cap, TOK_OP, lex, start_line, start_col);
            continue;
        }

        /* 6. Operadores de 1 carácter: + - * / < > = */
        if (strchr("+-*/<>=", c) != NULL) {
            char *lex = slice_str(src + i, 1);
            i++; col++;
            if (print_tokens) printf("%d:%d OP %s\n", start_line, start_col, lex);
            push_token(&tokens, &count, &cap, TOK_OP, lex, start_line, start_col);
            continue;
        }

        /* 7. Delimitadores: ( ) { } ; */
        if (strchr("(){};", c) != NULL) {
            char *lex = slice_str(src + i, 1);
            i++; col++;
            if (print_tokens) printf("%d:%d DELIM %s\n", start_line, start_col, lex);
            push_token(&tokens, &count, &cap, TOK_DELIM, lex, start_line, start_col);
            continue;
        }

        /* 8. Cualquier otra cosa es error léxico S01 */
        char *bad = slice_str(src + i, 1);
        i++; col++;
        errors++;
        if (print_tokens) {
            printf("%s:%d:%d: error S01: carácter inválido '%s'\n", filename, start_line, start_col, bad);
        }
        push_token(&tokens, &count, &cap, TOK_ERROR, bad, start_line, start_col);
    }

    if (print_tokens) {
        printf("%d:%d EOF\n", line, col);   /* una sola linea al final */
    }
    push_token(&tokens, &count, &cap, TOK_EOF, slice_str("EOF", 3), line, col);

    *out_count = count;
    if (out_lex_errors) *out_lex_errors = errors;
    return tokens;
}

void lexer_free_tokens(Token *tokens, size_t count) {
    if (!tokens) return;
    for (size_t i = 0; i < count; i++) {
        free(tokens[i].lexeme);
    }
    free(tokens);
}