/* compilador.h - contrato de la interfaz del compilador de Lenguajito. */
#ifndef COMPILADOR_H
#define COMPILADOR_H

/* Compila `ruta` en el modo indicado ("--tokens", "--ast", "--check",
 * "--tac", "--asm" o "--run"). Devuelve el código de salida del proceso:
 *   0 todo bien
 *   1 errores de compilación
 *   2 uso incorrecto
 */
int compilar(const char *ruta, const char *modo);

#endif /* COMPILADOR_H */
