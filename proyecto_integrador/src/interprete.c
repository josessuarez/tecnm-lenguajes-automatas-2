/* interprete.c - ejecuta el programa recorriendo el AST (--run, Hito 2).
 * Responsable: Jordi.
 *
 * Recorre las sentencias en orden y hace lo que dice cada nodo:
 *   declarar  -> guarda la variable en el ambito actual (sin valor = 0)
 *   asignar   -> cambia el valor de la variable visible
 *   imprimir  -> printf("%d\n")
 *   bloque    -> abre un ambito, ejecuta sus sentencias y lo cierra
 *   si        -> ejecuta el bloque "entonces" o el "si_no"
 *   mientras  -> repite el cuerpo mientras la condicion no sea 0
 * Los valores de las expresiones los calcula evaluar_expr() (evaluador.c)
 * y las variables viven en el Entorno (entorno.c).
 */
#include "interprete.h"

#include <stdio.h>

#include "entorno.h"
#include "evaluador.h"

/* Calcula una expresion. Si hay error lo reporta en stderr y regresa 1. */
static int calcular(const ASTNode *e, Entorno *env, int *valor) {
    int error = EVAL_OK;
    *valor = evaluar_expr(e, env, &error);
    if (error == EVAL_DIV_CERO) {
        fprintf(stderr, "%d:%d: error en ejecucion: division entre cero\n", e->line, e->col);
        return 1;
    }
    if (error != EVAL_OK) {
        fprintf(stderr, "%d:%d: error en ejecucion: variable desconocida\n", e->line, e->col);
        return 1;
    }
    return 0;
}

static int ejecutar(const ASTNode *s, Entorno *env);

/* Ejecuta una lista de sentencias (extra -> next -> next...).
 * Se detiene en la primera que falle. */
static int ejecutar_lista(const ASTNode *primera, Entorno *env) {
    for (const ASTNode *s = primera; s != NULL; s = s->next) {
        if (ejecutar(s, env) != 0) return 1;
    }
    return 0;
}

/* Ejecuta una sentencia. Regresa 0 si todo bien y 1 si hubo error. */
static int ejecutar(const ASTNode *s, Entorno *env) {
    int v = 0;

    switch (s->type) {
    case AST_DECLARAR:
        /* int x;  -> vale 0.  int x = e;  -> primero se calcula e */
        if (s->left != NULL && calcular(s->left, env, &v) != 0) return 1;
        entorno_declarar(env, s->text, v);
        return 0;

    case AST_ASIGNAR:
        if (calcular(s->left, env, &v) != 0) return 1;
        entorno_asignar(env, s->text, v);
        return 0;

    case AST_IMPRIMIR:
        if (calcular(s->left, env, &v) != 0) return 1;
        printf("%d\n", v);
        return 0;

    case AST_BLOQUE: {
        /* cada bloque es un ambito nuevo: lo de adentro tapa a lo de afuera */
        entorno_abrir_ambito(env);
        int r = ejecutar_lista(s->extra, env);
        entorno_cerrar_ambito(env);   /* se cierra aunque haya fallado */
        return r;
    }

    case AST_SI:
        if (calcular(s->left, env, &v) != 0) return 1;
        if (v != 0) return ejecutar(s->right, env);       /* entonces */
        if (s->extra != NULL) return ejecutar(s->extra, env);  /* si_no */
        return 0;

    case AST_MIENTRAS:
        /* se vuelve a calcular la condicion en cada vuelta */
        for (;;) {
            if (calcular(s->left, env, &v) != 0) return 1;
            if (v == 0) return 0;
            if (ejecutar(s->right, env) != 0) return 1;
        }

    default:
        return 0;   /* las expresiones sueltas no son sentencias */
    }
}

int interpretar(const ASTNode *programa) {
    if (programa == NULL) return 1;

    Entorno *env = entorno_nuevo();
    if (env == NULL) {
        fprintf(stderr, "error: no hay memoria para el entorno\n");
        return 1;
    }

    /* el programa es como un bloque, pero usa el ambito global */
    int r = ejecutar_lista(programa->extra, env);

    fflush(stdout);
    entorno_liberar(env);
    return r;
}
