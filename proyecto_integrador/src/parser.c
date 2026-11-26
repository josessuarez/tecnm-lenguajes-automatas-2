/* parser.c - parser descendente recursivo LL(1) de Lenguajito. */
#include "parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================
 * PARTE 1: PARSER DESCENDENTE RECURSIVO LL(1)
 * ========================================================= */
typedef struct {
    const char *filename;
    const Token *tokens;
    size_t count;
    size_t pos;
    int error_count;
} Parser;

static void report_syntax_error(Parser *p, int line, int col, const char *msg) {
    if (p->error_count >= MAX_ERRORES) return;
    printf("%s:%d:%d: error S01: %s\n", p->filename, line, col, msg);
    p->error_count++;
}

/* Un TOK_ERROR es un caracter que el lexer no reconocio: se reporta una
 * sola vez, diciendo cual fue, y se salta. */
static void report_invalid_char(Parser *p, const Token *tok) {
    char msg[64];
    snprintf(msg, sizeof msg, "carácter inválido '%s'", tok->lexeme);
    report_syntax_error(p, tok->line, tok->col, msg);
}

static const Token *peek(const Parser *p) {
    if (p->pos < p->count) {
        return &p->tokens[p->pos];
    }
    return &p->tokens[p->count - 1]; /* TOK_EOF */
}

static const Token *advance(Parser *p) {
    const Token *tok = peek(p);
    if (tok->type != TOK_EOF && p->pos + 1 < p->count) {
        p->pos++;
    }
    return tok;
}

static bool check_lexeme(const Parser *p, TokenType type, const char *lexeme) {
    const Token *tok = peek(p);
    if (tok->type != type) return false;
    if (!lexeme) return true;
    return tok->lexeme && strcmp(tok->lexeme, lexeme) == 0;
}

static bool match_lexeme(Parser *p, TokenType type, const char *lexeme) {
    if (check_lexeme(p, type, lexeme)) {
        advance(p);
        return true;
    }
    return false;
}

static const Token *expect(Parser *p, TokenType type, const char *lexeme, const char *err_msg) {
    if (check_lexeme(p, type, lexeme)) {
        return advance(p);
    }
    const Token *tok = peek(p);
    if (tok->type == TOK_ERROR) {
        report_invalid_char(p, tok);   /* la causa real es el caracter */
        advance(p);
    } else {
        report_syntax_error(p, tok->line, tok->col, err_msg);
    }
    return NULL;
}

/* Sincronización modo pánico hasta ';' o '}' para seguir buscando errores */
static void synchronize(Parser *p) {
    while (peek(p)->type != TOK_EOF) {
        const Token *tok = peek(p);
        if (tok->type == TOK_ERROR) {
            report_invalid_char(p, tok);
            advance(p);
            continue;
        }
        if (check_lexeme(p, TOK_DELIM, ";")) {
            advance(p);
            return;
        }
        if (check_lexeme(p, TOK_DELIM, "}")) {
            return;
        }
        if (check_lexeme(p, TOK_KW, "int") ||
            check_lexeme(p, TOK_KW, "if") ||
            check_lexeme(p, TOK_KW, "while") ||
            check_lexeme(p, TOK_KW, "print")) {
            return;
        }
        advance(p);
    }
}

/* Prototipos de la gramática de expresiones y sentencias */
static ASTNode *parse_expresion(Parser *p);
static ASTNode *parse_sentencia(Parser *p);
static ASTNode *parse_bloque(Parser *p);

/* primaria := NUMERO | IDENT | "(" expresion ")" */
static ASTNode *parse_primaria(Parser *p) {
    const Token *tok = peek(p);

    if (tok->type == TOK_ERROR) {
        report_invalid_char(p, tok);
        advance(p);
        return NULL;
    }

    if (tok->type == TOK_NUM) {
        advance(p);
        int val = (int)strtol(tok->lexeme, NULL, 10);
        return ast_new_num(val, tok->line, tok->col);
    }

    if (tok->type == TOK_ID) {
        advance(p);
        return ast_new_var(tok->lexeme, tok->line, tok->col);
    }

    if (match_lexeme(p, TOK_DELIM, "(")) {
        ASTNode *inside = parse_expresion(p);
        if (!inside) return NULL;
        if (!expect(p, TOK_DELIM, ")", "se esperaba ')'")) {
            ast_free(inside);
            return NULL;
        }
        return inside;
    }

    report_syntax_error(p, tok->line, tok->col, "se esperaba una expresión");
    return NULL;
}

/* unaria := [ "-" | "+" ] primaria */
static ASTNode *parse_unaria(Parser *p) {
    const Token *tok = peek(p);
    if (match_lexeme(p, TOK_OP, "-")) {
        ASTNode *operand = parse_primaria(p);
        if (!operand) return NULL;
        return ast_new_neg(operand, tok->line, tok->col);
    }
    if (match_lexeme(p, TOK_OP, "+")) {
        /* Según sección 4 de gramatica.md, '+' unario no genera nodo extra */
        return parse_primaria(p);
    }
    return parse_primaria(p);
}

/* multiplicativa := unaria { ( "*" | "/" ) unaria } */
static ASTNode *parse_multiplicativa(Parser *p) {
    ASTNode *left = parse_unaria(p);
    if (!left) return NULL;

    while (check_lexeme(p, TOK_OP, "*") || check_lexeme(p, TOK_OP, "/")) {
        const Token *op = advance(p);
        ASTNode *right = parse_unaria(p);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        left = ast_new_binaria(op->lexeme, left, right, op->line, op->col);
    }
    return left;
}

/* aditiva := multiplicativa { ( "+" | "-" ) multiplicativa } */
static ASTNode *parse_aditiva(Parser *p) {
    ASTNode *left = parse_multiplicativa(p);
    if (!left) return NULL;

    while (check_lexeme(p, TOK_OP, "+") || check_lexeme(p, TOK_OP, "-")) {
        const Token *op = advance(p);
        ASTNode *right = parse_multiplicativa(p);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        left = ast_new_binaria(op->lexeme, left, right, op->line, op->col);
    }
    return left;
}

static bool is_comp_op(const Token *tok) {
    if (tok->type != TOK_OP || !tok->lexeme) return false;
    return strcmp(tok->lexeme, "<") == 0  ||
           strcmp(tok->lexeme, "<=") == 0 ||
           strcmp(tok->lexeme, ">") == 0  ||
           strcmp(tok->lexeme, ">=") == 0 ||
           strcmp(tok->lexeme, "==") == 0 ||
           strcmp(tok->lexeme, "!=") == 0;
}

/* comparacion := aditiva [ ( "<" | "<=" | ">" | ">=" | "==" | "!=" ) aditiva ] */
static ASTNode *parse_comparacion(Parser *p) {
    ASTNode *left = parse_aditiva(p);
    if (!left) return NULL;

    if (is_comp_op(peek(p))) {
        const Token *op = advance(p);
        ASTNode *right = parse_aditiva(p);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        left = ast_new_binaria(op->lexeme, left, right, op->line, op->col);
    }
    return left;
}

static ASTNode *parse_expresion(Parser *p) {
    return parse_comparacion(p);
}

/* declaracion := "int" IDENT [ "=" expresion ] ";" */
static ASTNode *parse_declaracion(Parser *p) {
    const Token *kw = advance(p); /* consume "int" */
    const Token *id = expect(p, TOK_ID, NULL, "se esperaba un identificador después de 'int'");
    if (!id) {
        synchronize(p);
        return NULL;
    }

    /* Hito 2: desde aquí el nombre ya se leyó. Si algo falla, no se descarta
     * la declaración: se devuelve marcada con_error para que la semántica
     * registre la variable y no salga un E01 en cascada. */
    ASTNode *init_expr = NULL;
    if (match_lexeme(p, TOK_OP, "=")) {
        init_expr = parse_expresion(p);
        if (!init_expr) {
            synchronize(p);
            ASTNode *bad = ast_new_declarar(id->lexeme, NULL, kw->line, kw->col);
            bad->con_error = 1;
            return bad;
        }
    }

    if (!expect(p, TOK_DELIM, ";", "se esperaba ';' al final de la declaración")) {
        ast_free(init_expr);
        synchronize(p);
        ASTNode *bad = ast_new_declarar(id->lexeme, NULL, kw->line, kw->col);
        bad->con_error = 1;
        return bad;
    }

    return ast_new_declarar(id->lexeme, init_expr, kw->line, kw->col);
}

/* asignacion := IDENT "=" expresion ";" */
static ASTNode *parse_asignacion(Parser *p) {
    const Token *id = advance(p); /* consume IDENT */
    if (!expect(p, TOK_OP, "=", "se esperaba '=' en la asignación")) {
        synchronize(p);
        return NULL;
    }

    ASTNode *expr = parse_expresion(p);
    if (!expr) {
        synchronize(p);
        return NULL;
    }

    if (!expect(p, TOK_DELIM, ";", "se esperaba ';' al final de la asignación")) {
        ast_free(expr);
        synchronize(p);
        return NULL;
    }

    return ast_new_asignar(id->lexeme, expr, id->line, id->col);
}

/* impresion := "print" "(" expresion ")" ";" */
static ASTNode *parse_impresion(Parser *p) {
    const Token *kw = advance(p); /* consume "print" */
    if (!expect(p, TOK_DELIM, "(", "se esperaba '(' después de 'print'")) {
        synchronize(p);
        return NULL;
    }

    ASTNode *expr = parse_expresion(p);
    if (!expr) {
        synchronize(p);
        return NULL;
    }

    if (!expect(p, TOK_DELIM, ")", "se esperaba ')' después de la expresión")) {
        ast_free(expr);
        synchronize(p);
        return NULL;
    }

    if (!expect(p, TOK_DELIM, ";", "se esperaba ';' al final de print")) {
        ast_free(expr);
        synchronize(p);
        return NULL;
    }

    return ast_new_imprimir(expr, kw->line, kw->col);
}

/* bloque := "{" { sentencia } "}" */
static ASTNode *parse_bloque(Parser *p) {
    const Token *lbrace = expect(p, TOK_DELIM, "{", "se esperaba '{' para abrir bloque");
    if (!lbrace) {
        synchronize(p);
        return NULL;
    }

    ASTNode *blk = ast_new_node(AST_BLOQUE, lbrace->line, lbrace->col);
    while (peek(p)->type != TOK_EOF && !check_lexeme(p, TOK_DELIM, "}")) {
        ASTNode *stmt = parse_sentencia(p);
        if (stmt) {
            ast_append_child(blk, stmt);
        }
    }

    if (!expect(p, TOK_DELIM, "}", "se esperaba '}' para cerrar bloque")) {
        ast_free(blk);
        return NULL;
    }
    return blk;
}

/* seleccion := "if" "(" expresion ")" bloque [ "else" bloque ] */
static ASTNode *parse_seleccion(Parser *p) {
    const Token *kw = advance(p); /* consume "if" */
    if (!expect(p, TOK_DELIM, "(", "se esperaba '(' después de 'if'")) {
        synchronize(p);
        return NULL;
    }

    ASTNode *cond = parse_expresion(p);
    if (!cond) {
        synchronize(p);
        return NULL;
    }

    if (!expect(p, TOK_DELIM, ")", "se esperaba ')' después de la condición")) {
        ast_free(cond);
        synchronize(p);
        return NULL;
    }

    ASTNode *then_b = parse_bloque(p);
    if (!then_b) {
        ast_free(cond);
        return NULL;
    }

    ASTNode *else_b = NULL;
    if (match_lexeme(p, TOK_KW, "else")) {
        else_b = parse_bloque(p);
        if (!else_b) {
            ast_free(cond);
            ast_free(then_b);
            return NULL;
        }
    }

    return ast_new_si(cond, then_b, else_b, kw->line, kw->col);
}

/* iteracion := "while" "(" expresion ")" bloque */
static ASTNode *parse_iteracion(Parser *p) {
    const Token *kw = advance(p); /* consume "while" */
    if (!expect(p, TOK_DELIM, "(", "se esperaba '(' después de 'while'")) {
        synchronize(p);
        return NULL;
    }

    ASTNode *cond = parse_expresion(p);
    if (!cond) {
        synchronize(p);
        return NULL;
    }

    if (!expect(p, TOK_DELIM, ")", "se esperaba ')' después de la condición")) {
        ast_free(cond);
        synchronize(p);
        return NULL;
    }

    ASTNode *body = parse_bloque(p);
    if (!body) {
        ast_free(cond);
        return NULL;
    }

    return ast_new_mientras(cond, body, kw->line, kw->col);
}

static ASTNode *parse_sentencia(Parser *p) {
    const Token *tok = peek(p);

    if (tok->type == TOK_ERROR) {
        report_invalid_char(p, tok);
        advance(p);
        return NULL;
    }

    if (check_lexeme(p, TOK_KW, "int"))   return parse_declaracion(p);
    if (check_lexeme(p, TOK_KW, "if"))    return parse_seleccion(p);
    if (check_lexeme(p, TOK_KW, "while")) return parse_iteracion(p);
    if (check_lexeme(p, TOK_KW, "print")) return parse_impresion(p);
    if (check_lexeme(p, TOK_DELIM, "{"))  return parse_bloque(p);
    if (tok->type == TOK_ID)              return parse_asignacion(p);

    report_syntax_error(p, tok->line, tok->col, "se esperaba el inicio de una sentencia");
    advance(p);
    synchronize(p);
    return NULL;
}

ASTNode *parse_program(const char *filename, const Token *tokens, size_t count, int *out_errors) {
    Parser p = {
        .filename = filename ? filename : "<entrada>",
        .tokens = tokens,
        .count = count,
        .pos = 0,
        .error_count = 0
    };

    ASTNode *prog = ast_new_node(AST_PROGRAMA, 1, 1);

    while (peek(&p)->type != TOK_EOF && p.error_count < MAX_ERRORES) {
        ASTNode *stmt = parse_sentencia(&p);
        if (stmt) {
            ast_append_child(prog, stmt);
        }
    }

    if (out_errors) {
        *out_errors = p.error_count;
    }
    return prog;
}
