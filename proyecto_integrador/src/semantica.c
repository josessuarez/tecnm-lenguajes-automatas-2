/* semantica.c - tabla de simbolos con ambitos y diagnosticos (--check). */
#include "semantica.h"
#include "parser.h"   /* MAX_ERRORES */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================
 * PARTE 2: ANÁLISIS SEMÁNTICO (--check)
 * ========================================================= */
typedef struct Symbol {
    char *name;
    bool initialized;
    struct Symbol *next;
} Symbol;

typedef struct Scope {
    Symbol *head;
    struct Scope *parent;
} Scope;

typedef struct {
    const char *filename;
    Scope *current_scope;
    int error_count;
} SemaCtx;

static void report_sema_error(SemaCtx *ctx, int line, int col, const char *code, const char *msg, const char *ident) {
    if (ctx->error_count >= MAX_ERRORES) return;
    if (ident) {
        printf("%s:%d:%d: error %s: %s '%s'\n", ctx->filename, line, col, code, msg, ident);
    } else {
        printf("%s:%d:%d: error %s: %s\n", ctx->filename, line, col, code, msg);
    }
    ctx->error_count++;
}

static void scope_push(SemaCtx *ctx) {
    Scope *s = (Scope *)calloc(1, sizeof(Scope));
    if (!s) exit(2);
    s->parent = ctx->current_scope;
    ctx->current_scope = s;
}

static void scope_pop(SemaCtx *ctx) {
    Scope *s = ctx->current_scope;
    if (!s) return;
    Symbol *sym = s->head;
    while (sym) {
        Symbol *next = sym->next;
        free(sym->name);
        free(sym);
        sym = next;
    }
    ctx->current_scope = s->parent;
    free(s);
}

static Symbol *scope_lookup_current(Scope *scope, const char *name) {
    for (Symbol *s = scope ? scope->head : NULL; s != NULL; s = s->next) {
        if (strcmp(s->name, name) == 0) return s;
    }
    return NULL;
}

static Symbol *scope_lookup_all(Scope *scope, const char *name) {
    for (Scope *sc = scope; sc != NULL; sc = sc->parent) {
        Symbol *found = scope_lookup_current(sc, name);
        if (found) return found;
    }
    return NULL;
}

/* ---------------------------------------------------------
 * Hito 2 (Héctor): inicialización sensible al flujo.
 * Foto del estado `initialized` de todas las variables visibles.
 * --------------------------------------------------------- */
typedef struct {
    Symbol **syms;
    bool *vals;
    int n;
} Snapshot;

static Snapshot snap_take(SemaCtx *ctx) {
    Snapshot s = {NULL, NULL, 0};
    for (Scope *sc = ctx->current_scope; sc; sc = sc->parent)
        for (Symbol *y = sc->head; y; y = y->next) s.n++;
    s.syms = (Symbol **)malloc(sizeof(Symbol *) * (s.n ? s.n : 1));
    s.vals = (bool *)malloc(sizeof(bool) * (s.n ? s.n : 1));
    if (!s.syms || !s.vals) exit(2);
    int i = 0;
    for (Scope *sc = ctx->current_scope; sc; sc = sc->parent)
        for (Symbol *y = sc->head; y; y = y->next) {
            s.syms[i] = y;
            s.vals[i] = y->initialized;
            i++;
        }
    return s;
}

/* Estado actual de las mismas variables de la foto */
static bool *snap_current(const Snapshot *s) {
    bool *v = (bool *)malloc(sizeof(bool) * (s->n ? s->n : 1));
    if (!v) exit(2);
    for (int i = 0; i < s->n; i++) v[i] = s->syms[i]->initialized;
    return v;
}

static void snap_restore(const Snapshot *s) {
    for (int i = 0; i < s->n; i++) s->syms[i]->initialized = s->vals[i];
}

static void snap_free(Snapshot *s) {
    free(s->syms);
    free(s->vals);
}

/* Evalúa si una subexpresión es una constante con valor 0 (para E04) */
static bool is_const_zero(const ASTNode *expr, int *out_val) {
    if (!expr) return false;
    if (expr->type == AST_NUM) {
        if (out_val) *out_val = expr->num_val;
        return true;
    }
    if (expr->type == AST_NEG) {
        int v = 0;
        if (is_const_zero(expr->left, &v)) {
            if (out_val) *out_val = -v;
            return true;
        }
        return false;
    }
    if (expr->type == AST_BINARIA) {
        int l = 0, r = 0;
        if (is_const_zero(expr->left, &l) && is_const_zero(expr->right, &r)) {
            if (strcmp(expr->text, "+") == 0) { if (out_val) *out_val = l + r; return true; }
            if (strcmp(expr->text, "-") == 0) { if (out_val) *out_val = l - r; return true; }
            if (strcmp(expr->text, "*") == 0) { if (out_val) *out_val = l * r; return true; }
            if (strcmp(expr->text, "/") == 0 && r != 0) { if (out_val) *out_val = l / r; return true; }
        }
    }
    return false;
}

static void sema_check_expr(SemaCtx *ctx, const ASTNode *expr) {
    if (!expr) return;

    switch (expr->type) {
        case AST_NUM:
            break;

        case AST_VAR: {
            Symbol *sym = scope_lookup_all(ctx->current_scope, expr->text);
            if (!sym) {
                report_sema_error(ctx, expr->line, expr->col, "E01", "variable no declarada", expr->text);
            } else if (!sym->initialized) {
                report_sema_error(ctx, expr->line, expr->col, "E01", "variable leída antes de tener valor", expr->text);
            }
            break;
        }

        case AST_NEG:
            sema_check_expr(ctx, expr->left);
            break;

        case AST_BINARIA:
            sema_check_expr(ctx, expr->left);
            sema_check_expr(ctx, expr->right);
            if (strcmp(expr->text, "/") == 0) {
                int rval = 1;
                if (is_const_zero(expr->right, &rval) && rval == 0) {
                    report_sema_error(ctx, expr->line, expr->col, "E04", "división entre cero constante", NULL);
                }
            }
            break;

        default:
            break;
    }
}

static void sema_check_stmt(SemaCtx *ctx, const ASTNode *stmt) {
    if (!stmt) return;

    switch (stmt->type) {
        case AST_DECLARAR: {
            /* 1. Primero validamos la expresión derecha (para detectar `int x = x + 1;`).
             *    Si la declaración vino con error de sintaxis, no se revisa (ya se reportó). */
            if (stmt->left && !stmt->con_error) {
                sema_check_expr(ctx, stmt->left);
            }
            /* 2. Verificamos si ya existe en el MISMO ámbito (E02) */
            Symbol *existing = scope_lookup_current(ctx->current_scope, stmt->text);
            if (existing) {
                report_sema_error(ctx, stmt->line, stmt->col, "E02", "redeclaración en el mismo ámbito de", stmt->text);
                if (stmt->left || stmt->con_error) {
                    existing->initialized = true;
                }
            } else {
                Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
                if (!sym) exit(2);
                sym->name = lj_strdup(stmt->text);
                /* con_error: se registra como inicializada para no dar E01 en cascada */
                sym->initialized = (stmt->left != NULL) || stmt->con_error;
                sym->next = ctx->current_scope->head;
                ctx->current_scope->head = sym;
            }
            break;
        }

        case AST_ASIGNAR: {
            sema_check_expr(ctx, stmt->left);
            Symbol *sym = scope_lookup_all(ctx->current_scope, stmt->text);
            if (!sym) {
                report_sema_error(ctx, stmt->line, stmt->col, "E01", "variable no declarada", stmt->text);
            } else {
                sym->initialized = true;
            }
            break;
        }

        case AST_IMPRIMIR:
            sema_check_expr(ctx, stmt->left);
            break;

        case AST_BLOQUE:
            scope_push(ctx);
            for (const ASTNode *s = stmt->extra; s != NULL; s = s->next) {
                sema_check_stmt(ctx, s);
            }
            scope_pop(ctx);
            break;

        case AST_SI: {
            sema_check_expr(ctx, stmt->left);
            Snapshot base = snap_take(ctx);

            sema_check_stmt(ctx, stmt->right);          /* rama then */
            bool *then_vals = snap_current(&base);
            snap_restore(&base);                        /* vuelve al estado original */

            if (stmt->extra) {
                sema_check_stmt(ctx, stmt->extra);      /* rama else */
                bool *else_vals = snap_current(&base);
                /* inicializada solo si se asignó en las DOS ramas */
                for (int i = 0; i < base.n; i++)
                    base.syms[i]->initialized = then_vals[i] && else_vals[i];
                free(else_vals);
            }
            /* sin else: ya quedó restaurado, lo de adentro no cuenta */

            free(then_vals);
            snap_free(&base);
            break;
        }

        case AST_MIENTRAS: {
            sema_check_expr(ctx, stmt->left);
            Snapshot base = snap_take(ctx);
            sema_check_stmt(ctx, stmt->right);          /* adentro sí cuenta */
            snap_restore(&base);                        /* después del ciclo no */
            snap_free(&base);
            break;
        }

        default:
            break;
    }
}

int sema_check_program(const char *filename, const ASTNode *root, int initial_errors) {
    if (!root) return initial_errors;

    SemaCtx ctx = {
        .filename = filename ? filename : "<entrada>",
        .current_scope = NULL,
        .error_count = initial_errors
    };

    scope_push(&ctx); /* Ámbito global */
    for (const ASTNode *s = root->extra; s != NULL; s = s->next) {
        sema_check_stmt(&ctx, s);
    }
    scope_pop(&ctx);

    return ctx.error_count;
}
