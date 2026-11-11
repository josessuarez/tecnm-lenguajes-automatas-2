# Bitácora de Requerimientos y Auditoría IA — Sesión 09
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Reporte Formal de Errores Semánticos  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Diagnóstico con línea y columna  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo que más me hizo pensar fue el buffer. El que reserva la memoria es el que llama la función, yo solo recibo `salida` y `cap`, así que no puedo usar `sprintf` porque si el mensaje es más largo que el buffer escribe fuera y truena (eso se ve en el caso 4, donde `cap=10`). Usé `snprintf` porque nunca escribe más de `cap` bytes contando el `'\0'` y siempre deja la cadena terminada, entonces con `cap=10` deja 9 caracteres: `main.c:14`. El otro detalle fue que `cap` viene como `int` pero `snprintf` pide `size_t`; si `cap` fuera negativo y lo convierto directo, se vuelve un número gigante y ya no protege nada. Por eso primero reviso `salida == NULL || cap <= 0` y hasta después hago el cast. También cuidé el formato exacto: sin espacio antes de los `:` y sin punto final, porque el harness compara con `strcmp` carácter por carácter.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, sin bibliotecas no estándar; se compila junto a un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Estructura estándar de errores GCC: `archivo:linea:columna: error: mensaje`, sin desbordar el buffer y siempre terminado en `'\0'` |
| **Output (O)** | Solo código modular (sin `main`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo el módulo de diagnósticos de un compilador en C11.
Mi archivo se compila con gcc -std=c11 -Wall -Wextra -Werror junto con un
harness que ya tiene main(), así que NO debe tener main(). Solo biblioteca
estándar. El contrato está en api_sesion09.h:

void reportar(const char *archivo, int linea, int columna,
              const char *mensaje, char *salida, int cap);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- Escribir en `salida` el diagnóstico con el formato exacto de GCC:
  archivo:linea:columna: error: mensaje
  (sin espacios extra ni punto final).
- `salida` es un buffer del llamador de tamaño `cap`: nunca escribir más
  de cap-1 caracteres y el resultado siempre termina en '\0'.
- Si el texto no cabe, se trunca (ej. cap=10 -> "main.c:14").
- Línea y columna pueden ser 0.
- No cambies la firma.

OUTPUT: Dame solo el archivo ia_sesion09_c.c completo (con
#include "api_sesion09.h") y al final la complejidad de la función.
```

---

## 3. Código Devuelto por el Modelo
```c
#include "api_sesion09.h"
#include <stdio.h>
#include <stddef.h>

/* Formatea un diagnostico estilo GCC:  archivo:linea:columna: error: mensaje
 * - Nunca escribe mas de cap-1 caracteres y siempre termina en '\0'.
 * - Si el buffer no es valido (NULL o cap <= 0) no hace nada.
 * - Si archivo o mensaje son NULL usa un texto por defecto.
 * Complejidad: O(n) en tiempo, n = longitud del diagnostico; O(1) memoria extra. */
void reportar(const char *archivo, int linea, int columna, const char *mensaje, char *salida, int cap) {
    if (salida == NULL || cap <= 0)
        return;

    if (archivo == NULL) archivo = "<desconocido>";
    if (mensaje == NULL) mensaje = "";

    int n = snprintf(salida, (size_t)cap, "%s:%d:%d: error: %s",
                     archivo, linea, columna, mensaje);
    if (n < 0)
        salida[0] = '\0';          /* error de formato: deja la cadena vacia */
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Múltiples errores sin abortar parser

### Salida del harness
```text
=== AUDITORIA SESION 09 - Diagnostico con linea y columna ===
Caso                               esperado:              obtenido:              resultado
[ 1] main.c:14:5
     esperado: main.c:14:5: error: tipo incompatible
     obtenido: main.c:14:5: error: tipo incompatible
     -> OK
[ 2] a.c:1:1
     esperado: a.c:1:1: error: x
     obtenido: a.c:1:1: error: x
     -> OK
[ 3] linea 0
     esperado: x.c:0:0: error: y
     obtenido: x.c:0:0: error: y
     -> OK
[ 4] cap=10 trunca                      esperado: main.c:14    obtenido: main.c:14    OK

RESULTADO: 4/4 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion09.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> El caso borde fue el valor de `cap`. La primera idea de la IA fue pasar `cap` directo a `snprintf`, pero `cap` es `int` y `snprintf` recibe `size_t`. Si alguien llama con `cap = 0` no pasa nada, pero con `cap` negativo la conversión da un número enorme y `snprintf` cree que tiene muchísimo espacio, o sea que se desborda el buffer, justo lo que el contrato dice que nunca debe pasar. El harness no prueba ese caso (solo prueba `cap=10`), así que el código pasaba 4/4 aunque estuviera mal. Lo detecté porque en la Fase 1 ya me había topado con el cast a `size_t`. Se arregló validando `salida == NULL || cap <= 0` antes de llamar a `snprintf`. También agregué que si `archivo` o `mensaje` vienen en `NULL` use un texto por defecto, porque pasar `NULL` a `%s` es comportamiento indefinido. La función es O(n) en tiempo (n = largo del diagnóstico) y no pide memoria dinámica, así que no hay fugas.

---
**Firma digital:** josessuarez
