/* entorno.h - valores de las variables mientras corre el programa (--run).
 * Responsable: José Manuel.
 *
 * Es una pila de ambitos, como la tabla de simbolos de semantica.c, pero
 * guardando el valor de cada variable. Cada bloque { } abre un ambito nuevo
 * y al cerrarlo sus variables desaparecen. Leer y asignar buscan del ambito
 * actual hacia el global, asi que una variable interna oculta a la externa.
 */
#ifndef ENTORNO_H
#define ENTORNO_H

/* Tipo opaco: los campos solo se conocen dentro de entorno.c. */
typedef struct Entorno Entorno;

/* Crea un entorno con el ambito global ya abierto. NULL si no hay memoria. */
Entorno *entorno_nuevo(void);

/* Libera todos los ambitos y variables que queden. Acepta NULL. */
void entorno_liberar(Entorno *env);

/* Abre un ambito nuevo encima del actual (al entrar a un bloque). */
void entorno_abrir_ambito(Entorno *env);

/* Cierra el ambito actual y libera sus variables (al salir de un bloque).
 * El ambito global no se cierra aqui: se libera con entorno_liberar. */
void entorno_cerrar_ambito(Entorno *env);

/* Declara 'nombre' en el ambito actual con 'valor'. Guarda una copia del
 * nombre, no el puntero que recibe. */
void entorno_declarar(Entorno *env, const char *nombre, int valor);

/* Busca 'nombre' del ambito actual hacia el global. Si existe, escribe su
 * valor en *valor y devuelve 1. Si no existe, devuelve 0 y no toca *valor. */
int entorno_leer(const Entorno *env, const char *nombre, int *valor);

/* Cambia el valor de la variable visible mas cercana con ese nombre.
 * Devuelve 1 si existe y 0 si no. */
int entorno_asignar(Entorno *env, const char *nombre, int valor);

#endif /* ENTORNO_H */
