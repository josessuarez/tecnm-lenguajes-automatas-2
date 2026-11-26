/* ast.c - construccion, impresion (--ast) y liberacion del AST. */
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *lj_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *copy = (char *)malloc(len);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}

ASTNode *ast_new_node(ASTNodeType type, int line, int col) {
    ASTNode *node = (ASTNode *)calloc(1, sizeof(ASTNode));
    if (!node) {
        perror("calloc ASTNode");
        exit(2);
    }
    node->type = type;
    node->line = line;
    node->col = col;
    return node;
}

ASTNode *ast_new_num(int val, int line, int col) {
    ASTNode *node = ast_new_node(AST_NUM, line, col);
    node->num_val = val;
    return node;
}

ASTNode *ast_new_var(const char *name, int line, int col) {
    ASTNode *node = ast_new_node(AST_VAR, line, col);
    node->text = lj_strdup(name);
    return node;
}

ASTNode *ast_new_neg(ASTNode *expr, int line, int col) {
    ASTNode *node = ast_new_node(AST_NEG, line, col);
    node->left = expr;
    return node;
}

ASTNode *ast_new_binaria(const char *op, ASTNode *left, ASTNode *right, int line, int col) {
    ASTNode *node = ast_new_node(AST_BINARIA, line, col);
    node->text = lj_strdup(op);
    node->left = left;
    node->right = right;
    return node;
}

ASTNode *ast_new_declarar(const char *name, ASTNode *init_expr, int line, int col) {
    ASTNode *node = ast_new_node(AST_DECLARAR, line, col);
    node->text = lj_strdup(name);
    node->left = init_expr;
    return node;
}

ASTNode *ast_new_asignar(const char *name, ASTNode *expr, int line, int col) {
    ASTNode *node = ast_new_node(AST_ASIGNAR, line, col);
    node->text = lj_strdup(name);
    node->left = expr;
    return node;
}

ASTNode *ast_new_imprimir(ASTNode *expr, int line, int col) {
    ASTNode *node = ast_new_node(AST_IMPRIMIR, line, col);
    node->left = expr;
    return node;
}

ASTNode *ast_new_si(ASTNode *cond, ASTNode *then_b, ASTNode *else_b, int line, int col) {
    ASTNode *node = ast_new_node(AST_SI, line, col);
    node->left = cond;
    node->right = then_b;
    node->extra = else_b;
    return node;
}

ASTNode *ast_new_mientras(ASTNode *cond, ASTNode *body, int line, int col) {
    ASTNode *node = ast_new_node(AST_MIENTRAS, line, col);
    node->left = cond;
    node->right = body;
    return node;
}

void ast_append_child(ASTNode *parent, ASTNode *child) {
    if (!parent || !child) return;
    if (!parent->extra) {
        parent->extra = child;
    } else {
        ASTNode *curr = parent->extra;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = child;
    }
}

static void ast_print_rec(const ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAMA:
            printf("(programa");
            for (const ASTNode *s = node->extra; s != NULL; s = s->next) {
                printf(" ");
                ast_print_rec(s);
            }
            printf(")");
            break;

        case AST_BLOQUE:
            printf("(bloque");
            for (const ASTNode *s = node->extra; s != NULL; s = s->next) {
                printf(" ");
                ast_print_rec(s);
            }
            printf(")");
            break;

        case AST_DECLARAR:
            if (node->left) {
                printf("(declarar %s ", node->text);
                ast_print_rec(node->left);
                printf(")");
            } else {
                printf("(declarar %s)", node->text);
            }
            break;

        case AST_ASIGNAR:
            printf("(asignar %s ", node->text);
            ast_print_rec(node->left);
            printf(")");
            break;

        case AST_SI:
            printf("(si ");
            ast_print_rec(node->left);
            printf(" (entonces ");
            ast_print_rec(node->right);
            printf(")");
            if (node->extra) {
                printf(" (si_no ");
                ast_print_rec(node->extra);
                printf(")");
            }
            printf(")");
            break;

        case AST_MIENTRAS:
            printf("(mientras ");
            ast_print_rec(node->left);
            printf(" ");
            ast_print_rec(node->right);
            printf(")");
            break;

        case AST_IMPRIMIR:
            printf("(imprimir ");
            ast_print_rec(node->left);
            printf(")");
            break;

        case AST_BINARIA:
            printf("(%s ", node->text);
            ast_print_rec(node->left);
            printf(" ");
            ast_print_rec(node->right);
            printf(")");
            break;

        case AST_NEG:
            printf("(neg ");
            ast_print_rec(node->left);
            printf(")");
            break;

        case AST_NUM:
            printf("(num %d)", node->num_val);
            break;

        case AST_VAR:
            printf("(var %s)", node->text);
            break;
    }
}

void ast_print(const ASTNode *root) {
    if (!root) return;
    ast_print_rec(root);
    printf("\n");
}

void ast_free(ASTNode *node) {
    if (!node) return;
    ast_free(node->left);
    ast_free(node->right);

    if (node->type == AST_PROGRAMA || node->type == AST_BLOQUE) {
        ASTNode *curr = node->extra;
        while (curr) {
            ASTNode *next = curr->next;
            ast_free(curr);
            curr = next;
        }
    } else {
        ast_free(node->extra);
    }

    free(node->text);
    free(node);
}