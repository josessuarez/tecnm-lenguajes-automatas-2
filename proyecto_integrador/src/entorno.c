/* entorno.c - valores de las variables mientras corre el programa (--run).
 * Responsable: José Manuel.
 *
 * Cada ambito es una lista de variables (nombre y valor). Los ambitos se
 * apilan: el actual apunta a su padre. Para leer o asignar se busca del
 * ambito actual hacia el global, asi la variable de adentro tapa a la de
 * afuera (shadowing). Al cerrar un ambito se liberan sus variables y la
 * de afuera vuelve a verse.
 */
#include "entorno.h"

#include <stdlib.h>
#include <string.h>

typedef struct Variable {
    char *nombre;
    int valor;
    struct Variable *sig;
} Variable;

typedef struct Ambito {
    Variable *vars;
    struct Ambito *padre;
} Ambito;

struct Entorno {
    Ambito *actual;
};

static char *copiar_texto(const char *s) {
    size_t n = strlen(s) + 1;
    char *copia = malloc(n);
    if (copia == NULL) exit(2);
    memcpy(copia, s, n);
    return copia;
}

static Variable *buscar_en(const Ambito *amb, const char *nombre) {
    for (Variable *v = amb->vars; v != NULL; v = v->sig) {
        if (strcmp(v->nombre, nombre) == 0) return v;
    }
    return NULL;
}

static Variable *buscar(const Entorno *env, const char *nombre) {
    if (env == NULL || nombre == NULL) return NULL;
    for (const Ambito *a = env->actual; a != NULL; a = a->padre) {
        Variable *v = buscar_en(a, nombre);
        if (v != NULL) return v;
    }
    return NULL;
}

/* Libera un ambito con todas sus variables y devuelve su padre. */
static Ambito *liberar_ambito(Ambito *a) {
    Ambito *padre = a->padre;
    Variable *v = a->vars;
    while (v != NULL) {
        Variable *sig = v->sig;
        free(v->nombre);
        free(v);
        v = sig;
    }
    free(a);
    return padre;
}

Entorno *entorno_nuevo(void) {
    Entorno *env = calloc(1, sizeof(Entorno));
    if (env == NULL) return NULL;
    env->actual = calloc(1, sizeof(Ambito));   /* ambito global */
    if (env->actual == NULL) {
        free(env);
        return NULL;
    }
    return env;
}

void entorno_liberar(Entorno *env) {
    if (env == NULL) return;
    while (env->actual != NULL) {
        env->actual = liberar_ambito(env->actual);
    }
    free(env);
}

void entorno_abrir_ambito(Entorno *env) {
    if (env == NULL) return;
    Ambito *a = calloc(1, sizeof(Ambito));
    if (a == NULL) exit(2);
    a->padre = env->actual;
    env->actual = a;
}

void entorno_cerrar_ambito(Entorno *env) {
    if (env == NULL || env->actual == NULL) return;
    if (env->actual->padre == NULL) return;   /* el global no se cierra aqui */
    env->actual = liberar_ambito(env->actual);
}

void entorno_declarar(Entorno *env, const char *nombre, int valor) {
    if (env == NULL || env->actual == NULL || nombre == NULL) return;

    /* si ya existe en este mismo ambito solo se actualiza el valor */
    Variable *v = buscar_en(env->actual, nombre);
    if (v != NULL) {
        v->valor = valor;
        return;
    }

    v = malloc(sizeof(Variable));
    if (v == NULL) exit(2);
    v->nombre = copiar_texto(nombre);
    v->valor = valor;
    v->sig = env->actual->vars;
    env->actual->vars = v;
}

int entorno_leer(const Entorno *env, const char *nombre, int *valor) {
    Variable *v = buscar(env, nombre);
    if (v == NULL) return 0;          /* no se toca *valor */
    if (valor != NULL) *valor = v->valor;
    return 1;
}

int entorno_asignar(Entorno *env, const char *nombre, int valor) {
    Variable *v = buscar(env, nombre);
    if (v == NULL) return 0;
    v->valor = valor;
    return 1;
}
