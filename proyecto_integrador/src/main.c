/* main.c - interfaz de línea de comandos del compilador.
 *
 * Este archivo ya viene resuelto. Tu trabajo vive en compilar() y en los
 * módulos que agregues (lexer, parser, semantica, tac, codegen).
 */
#include <stdio.h>
#include <string.h>

#include "compilador.h"

static int modo_valido(const char *modo) {
    static const char *const modos[] = {
        "--tokens", "--ast", "--check", "--tac", "--asm", "--run"
    };
    for (size_t i = 0; i < sizeof modos / sizeof modos[0]; i++) {
        if (strcmp(modo, modos[i]) == 0) return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr,
                "uso: %s ARCHIVO [--tokens|--ast|--check|--tac|--asm|--run]\n",
                argv[0]);
        return 2;
    }

    const char *modo = (argc == 3) ? argv[2] : "--check";
    if (!modo_valido(modo)) {
        fprintf(stderr, "modo desconocido: %s\n", modo);
        return 2;
    }

    return compilar(argv[1], modo);
}
