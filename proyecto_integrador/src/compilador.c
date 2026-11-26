/* compilador.c - punto de entrada del compilador.
 *
 * Hecho (Hito 1): --tokens, --ast y --check (S01, E01, E02, E04).
 * Hecho (Hito 2): --run interpretando el AST (interprete.c).
 * TODO (Hito 3): generar TAC y optimizarlo.
 * TODO (Hito 4): generar ensamblador, ensamblar, enlazar y ejecutar.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compilador.h"
#include "lexer.h"
#include "parser.h"
#include "semantica.h"
#include "interprete.h"

/* Lee el archivo completo en memoria. Devuelve NULL si no se pudo abrir. */
static char *leer_archivo(const char *ruta) {
    FILE *f = fopen(ruta, "rb");
    if (f == NULL) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long tam = ftell(f);
    if (tam < 0) { fclose(f); return NULL; }
    rewind(f);

    char *buf = malloc((size_t)tam + 1);
    if (buf == NULL) { fclose(f); return NULL; }
    size_t leidos = fread(buf, 1, (size_t)tam, f);
    buf[leidos] = '\0';
    fclose(f);
    return buf;
}

int compilar(const char *ruta, const char *modo) {
    char *codigo = leer_archivo(ruta);
    if (codigo == NULL) {
        fprintf(stderr, "%s: no se pudo abrir el archivo\n", ruta);
        return 2;
    }

    size_t num_tokens = 0;
    int errores_lexicos = 0;
    bool solo_tokens = (strcmp(modo, "--tokens") == 0);
    Token *tokens = lexer_scan_all(ruta, codigo, &num_tokens, solo_tokens, &errores_lexicos);

    int resultado;
    if (solo_tokens) {
        resultado = (errores_lexicos > 0) ? 1 : 0;
    } else {
        int errores_sintaxis = 0;
        ASTNode *raiz = parse_program(ruta, tokens, num_tokens, &errores_sintaxis);

        if (strcmp(modo, "--ast") == 0) {
            /* --ast no revisa la semantica: tests/hito1/02 asigna a una
             * variable sin declarar y aun asi espera el arbol. */
            if (errores_sintaxis > 0) {
                resultado = 1;
            } else {
                ast_print(raiz);
                resultado = 0;
            }
        } else if (strcmp(modo, "--check") == 0) {
            int total = sema_check_program(ruta, raiz, errores_sintaxis);
            resultado = (total > 0) ? 1 : 0;
        } else if (strcmp(modo, "--run") == 0) {
            /* Hito 2: primero la misma revision de --check. Si hay errores
             * se imprimen y no se ejecuta nada. */
            int total = sema_check_program(ruta, raiz, errores_sintaxis);
            if (total > 0) {
                resultado = 1;
            } else {
                resultado = interpretar(raiz);
            }
        } else {
            fprintf(stderr, "%s: modo %s todavia sin implementar\n", ruta, modo);
            resultado = 1;
        }
        ast_free(raiz);
    }

    lexer_free_tokens(tokens, num_tokens);
    free(codigo);
    return resultado;
}
