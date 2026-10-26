Bitácora de Requerimientos y Auditoría IA — Sesión 03

Materia: Lenguajes y Autómatas II (SCD-1016)
Tema: Google Colab, Mágicos y Flujo de Entrega
Docente Titular: MCC. Iván Márquez Larios (ijmarquezl)
Estudiante: Jose Manuel Suarez Huex 
Matrícula: 24531320
Track: C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)

1. Fase 1: El Artesano (Dificultad Manual)

Reto: Manipulación de archivos y llamadas OS
Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:

No alcancé a terminar la versión a mano, se quedó con los TODO y el harness me dio 1/6 (el único que pasó fue el del archivo inexistente, pero nomás porque todo regresaba -1 jaja). Donde más me atoré fue en ejecutar: sabía que había que usar system(), pero no me acordaba que lo que regresa no es el código de salida directo sino un "estado" que hay que desempacar con WEXITSTATUS, y tampoco que para eso hay que incluir <sys/wait.h>. También me trabé pensando cómo contar la última línea cuando el archivo no termina en \n (caso "a\nb"), porque si nomás cuentas los saltos te sale 1 en vez de 2. Y otra cosa es que el harness compila con -Werror, entonces cualquier warning (un parámetro sin usar, una variable de más) ya no te deja compilar, hay que ser más cuidadoso.

2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)
2.1 Matriz de Especificación Formal
Elemento C-R-E-O	Definición Técnica
Contexto (C)	C99 / GCC o Rust 2021 sin bibliotecas no estándar
Rol (R)	Ingeniero de software de sistemas y compiladores
Especificación (E)	Salida estándar y códigos de retorno ABI
Output (O)	Solo código modular y análisis de complejidad
2.2 Prompt Estructurado
text
C (Contexto): Lenguaje C (C11), compilador gcc 13.3.0. Se compila con
-std=c11 -Wall -Wextra -Werror, o sea que cualquier warning cuenta como error.
Solo biblioteca estándar y POSIX.

R (Rol): Eres un programador senior de C que trabaja en herramientas de
compilación.

E (Especificación): Implementa estas tres funciones respetando EXACTAMENTE
las firmas de api_sesion03.h:

    #ifndef API_SESION03_H
    #define API_SESION03_H

    int escribir_texto(const char *ruta, const char *texto);
    int contar_lineas(const char *ruta);
    int ejecutar(const char *comando);

    #endif /* API_SESION03_H */

- escribir_texto: escribe el texto en el archivo con fopen/fputs, regresa 0
  si todo sale bien y -1 si hay error.
- contar_lineas: cuenta las líneas del archivo. Si el último renglón no
  termina en '\n' también cuenta. Si el archivo no existe regresa -1 (no
  abortar).
- ejecutar: corre el comando con system() y regresa el código de salida
  real usando WEXITSTATUS, no el estado crudo de system().
- Los archivos se cierran en todos los caminos.

O (Output): Un solo archivo .c que incluya "api_sesion03.h", sin main(),
solo el código.
3. Código Devuelto por el Modelo
c
/* ia_sesion03_c.c - Fase 2: El Cientifico */
#include "api_sesion03.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int escribir_texto(const char *ruta, const char *texto) {
    if (ruta == NULL || texto == NULL) {
        return -1;
    }

    FILE *archivo = fopen(ruta, "w");
    if (archivo == NULL) {
        return -1;
    }

    if (fputs(texto, archivo) == EOF) {
        fclose(archivo);
        return -1;
    }

    fclose(archivo);
    return 0;
}

int contar_lineas(const char *ruta) {
    if (ruta == NULL) {
        return -1;
    }

    FILE *archivo = fopen(ruta, "r");
    if (archivo == NULL) {
        return -1;
    }

    int lineas = 0;
    int ch;
    int ultimo_ch = EOF;

    while ((ch = fgetc(archivo)) != EOF) {
        if (ch == '\n') {
            lineas++;
        }
        ultimo_ch = ch;
    }

    // Caso borde: El archivo tiene contenido pero no termina en salto de línea
    if (ultimo_ch != EOF && ultimo_ch != '\n') {
        lineas++;
    }

    fclose(archivo);
    return lineas;
}

int ejecutar(const char *comando) {
    if (comando == NULL) {
        return -1;
    }

    int estado = system(comando);
    if (estado == -1) {
        return -1;
    }

    // Retornamos el código de salida real usando WEXITSTATUS si terminó normalmente
    if (WIFEXITED(estado)) {
        return WEXITSTATUS(estado);
    }

    return -1;
}
4. Fase 3: El Auditor (Resultados del Test Harness)

Pruebas ejecutadas: Detección de errores de compilación GCC

Comando: gcc -std=c11 -Wall -Wextra -Werror harness_sesion03_c.c ia_sesion03_c.c -o test_ia && ./test_ia

Salida del harness
text
=== AUDITORIA SESION 03 - Archivos y codigos de salida ===
Caso                               esperado:              obtenido:              resultado
[ 1] escribir_texto(...)==0             esperado: 0            obtenido: 0            OK
[ 2] contar_lineas(3 lineas)            esperado: 3            obtenido: 3            OK
[ 3] contar_lineas(sin salto final)     esperado: 2            obtenido: 2            OK
[ 4] contar_lineas(inexistente)         esperado: -1           obtenido: -1           OK
[ 5] ejecutar("true")                   esperado: 0            obtenido: 0            OK
[ 6] ejecutar("false")                  esperado: 1            obtenido: 1            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion03.md
 Caso base y límites superados sin Crash.
 No existen accesos fuera de límites ni Memory Leaks.
 La complejidad teórica coincide con la implementada.
Reporte Crítico

¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?

Comparación artesano vs IA: mi versión a mano sacó 1/6 y la de la IA 6/6. Compiló a la primera con -Wall -Wextra -Werror sin ningún warning y respetó las firmas del .h tal cual, no tuve que corregirle nada.

Lo que hizo bien:

Revisa NULL en los parámetros antes de usarlos y un archivo que no existe lo reporta con -1 en vez de tronar.
El archivo se cierra en todos los caminos (incluso cuando falla fputs), y como no usa malloc no hay memory leaks posibles.
El caso borde de contar_lineas sí lo cubrió: guarda el último carácter leído y si no fue \n suma una línea más. Así "a\nb" da 2. Un archivo vacío da 0, que también está bien.
En ejecutar usa WIFEXITED antes de WEXITSTATUS, que es lo correcto, porque si el proceso murió por una señal WEXITSTATUS no significa nada.

Caso borde que omitió la IA: en escribir_texto nunca revisa lo que regresa fclose(). Como fputs escribe en un buffer, el texto realmente se manda al disco hasta que se cierra el archivo, entonces si el disco está lleno o hay un error de escritura, ese error aparece en el fclose y la función igual regresaría 0 como si todo hubiera salido bien. Lo detecté leyendo el código línea por línea, porque el harness no lo prueba (sus casos siempre escriben en un archivo normal). Para el contexto de la sesión eso es justo lo que no queremos: que un paso del pipeline diga "todo bien" cuando en realidad falló.

Cómo lo solucioné: cambié el final de escribir_texto para que también revise el fclose:

c
if (fclose(archivo) != 0) {
    return -1;
}
return 0;

Volví a correr el harness con el cambio y sigue dando 6/6, o sea que no rompe nada y ahora sí reporta el error si la escritura falla al cerrar. Para comprobarlo probé escribir_texto("/dev/full", "hola") (/dev/full es un archivo de Linux que siempre dice "disco lleno"): con el código original de la IA regresa 0, y con la corrección regresa -1.

Otros detalles menores que vi:

Si el comando no existe, system() regresa 127 (lo pone el shell), no -1. No está mal, pero hay que saber que -1 solo sale si system() ni siquiera pudo crear el proceso.
Si el comando muere por una señal regresa -1, entonces se pierde la info de cuál señal fue (la convención del shell es 128 + número de señal).
contar_lineas no revisa ferror(), así que si hay un error de lectura a medio archivo regresaría un conteo incompleto en vez de -1.

Complejidad:

Función	Tiempo	Memoria
escribir_texto	O(n), n = largo del texto	O(1)
contar_lineas	O(n), n = bytes del archivo (lee carácter por carácter)	O(1)
ejecutar	O(1) por parte de la función, más lo que tarde el comando	O(1)

Coincide con lo implementado: solo hay un ciclo en contar_lineas que recorre el archivo una vez, y no se reserva memoria dinámica en ninguna función.

Conclusión: la IA sacó el código correcto y más rápido que yo, pero el harness solo garantiza que pasan los 6 casos que prueba, no que el código esté completo. Lo del fclose solo se ve si lees el código y entiendes cómo funciona el buffer de stdio.

Firma digital: josessuarez
