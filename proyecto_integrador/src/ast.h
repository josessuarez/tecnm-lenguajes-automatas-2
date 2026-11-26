/* ast.h - nodos del arbol sintactico abstracto (docs/gramatica.md, seccion 4). */
#ifndef AST_H
#define AST_H

typedef enum {
    AST_PROGRAMA,
    AST_DECLARAR,
    AST_ASIGNAR,
    AST_SI,
    AST_MIENTRAS,
    AST_IMPRIMIR,
    AST_BLOQUE,
    AST_BINARIA,
    AST_NEG,
    AST_NUM,
    AST_VAR
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    int line;
    int col;
    char *text;             /* nombre de variable u operador binario */
    int num_val;            /* valor para AST_NUM */
    int con_error;          /* 1 si la declaración tuvo error de sintaxis (Hito 2) */
    struct ASTNode *left;   /* expr izq, condicion o valor a asignar */
    struct ASTNode *right;  /* expr der, cuerpo entonces o cuerpo mientras */
    struct ASTNode *extra;  /* lista de hijos en PROGRAMA/BLOQUE o rama si_no */
    struct ASTNode *next;   /* siguiente sentencia en una lista enlazada */
} ASTNode;

/* Constructores y utilidades */
char *lj_strdup(const char *s);
ASTNode *ast_new_node(ASTNodeType type, int line, int col);
ASTNode *ast_new_num(int val, int line, int col);
ASTNode *ast_new_var(const char *name, int line, int col);
ASTNode *ast_new_neg(ASTNode *expr, int line, int col);
ASTNode *ast_new_binaria(const char *op, ASTNode *left, ASTNode *right, int line, int col);
ASTNode *ast_new_declarar(const char *name, ASTNode *init_expr, int line, int col);
ASTNode *ast_new_asignar(const char *name, ASTNode *expr, int line, int col);
ASTNode *ast_new_imprimir(ASTNode *expr, int line, int col);
ASTNode *ast_new_si(ASTNode *cond, ASTNode *then_b, ASTNode *else_b, int line, int col);
ASTNode *ast_new_mientras(ASTNode *cond, ASTNode *body, int line, int col);
void ast_append_child(ASTNode *parent, ASTNode *child);

/* Impresion (--ast) y liberacion de memoria */
void ast_print(const ASTNode *root);
void ast_free(ASTNode *node);

#endif /* AST_H */
