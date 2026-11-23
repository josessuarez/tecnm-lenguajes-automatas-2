# Bitácora de Requerimientos y Auditoría IA — Sesión 17
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Comprobación Semántica de Retornos  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Validación de tipo de sentencia return  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Me costó decidir el orden. El criterio de aceptación dice que el tipo se revisa antes que las rutas, así que si alguien regresa un `double` en una función `int` con una ruta abierta, el diagnóstico es 1 y no 2. Para comparar tipos hice una función `rango()` que ordena los tipos de más angosto a más ancho (`char < short < int < long < float < double`); la conversión es válida si el rango de la expresión no es mayor que el de la función. El caso `void` lo saqué aparte: si la función es `void`, solo es válida si la expresión también es `void`, y en ese caso no importan las rutas, porque una función void puede terminar sin `return`. Al revés, si la función no es void pero la expresión sí, también es error de tipo. Las rutas incoherentes (cero rutas, más retornos que rutas o retornos negativos) las reviso al principio y regresan -1.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Rutas de ejecución con retorno garantizado |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo la validación de sentencias return de un compilador en
C11. Se compila con gcc -std=c11 -Wall -Wextra -Werror
junto con un harness que ya tiene main(), así que no lleva main(). Solo
biblioteca estándar. Contrato (api_sesion17.h):

int diagnosticar_retorno(const char *tipo_funcion, const char *tipo_expresion,
                         int rutas_totales, int rutas_con_return);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- Regresa 0 si todo está bien, 1 si el tipo es incompatible y 2 si queda
  una ruta sin return.
- La conversión solo se acepta si el tipo de la función es igual o más
  ancho que el de la expresión (char < short < int < long < float < double).
- En una función void, devolver un valor siempre es error de tipo.
- El tipo se revisa antes que las rutas.
- Tipos NULL, cero rutas, retornos negativos o más retornos que rutas
  regresan -1.
- No cambies la firma.

OUTPUT: Solo el archivo ia_sesion17_c.c (con #include "api_sesion17.h") y la
complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion17_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: la funcion void ya no exige return en cada ruta)
 */
#include "api_sesion17.h"
#include <stddef.h>
#include <string.h>

/* Rango de anchura de cada tipo numerico; -1 si no se conoce. O(1). */
static int rango_tipo(const char *t) {
    static const char *orden[] = {"char", "short", "int", "long", "float", "double"};
    for (int i = 0; i < 6; i++) {
        if (strcmp(t, orden[i]) == 0) return i;
    }
    return -1;
}

/* 0 = correcto, 1 = tipo incompatible, 2 = ruta sin return, -1 = entrada invalida.
 * El tipo se revisa antes que las rutas. O(1). */
int diagnosticar_retorno(const char *tipo_funcion, const char *tipo_expresion, int rutas_totales, int rutas_con_return) {
    if (tipo_funcion == NULL || tipo_expresion == NULL) return -1;
    if (rutas_totales <= 0 || rutas_con_return < 0 || rutas_con_return > rutas_totales) return -1;

    int f_void = (strcmp(tipo_funcion, "void") == 0);
    int e_void = (strcmp(tipo_expresion, "void") == 0);
    if (f_void || e_void) return (f_void && e_void) ? 0 : 1;

    int rf = rango_tipo(tipo_funcion);
    int re = rango_tipo(tipo_expresion);
    if (rf < 0 || re < 0) return -1;
    if (re > rf) return 1;

    return (rutas_con_return == rutas_totales) ? 0 : 2;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Funciones sin retorno o retornos mixtos

### Salida del harness
```text
=== AUDITORIA SESION 17 - Validacion de retornos y rutas ===
Caso                               esperado:              obtenido:              resultado
[ 1] int con int y dos rutas            esperado: 0            obtenido: 0            OK
[ 2] void devuelve valor                esperado: 1            obtenido: 1            OK
[ 3] ruta sin retorno                   esperado: 2            obtenido: 2            OK
[ 4] double acepta int                  esperado: 0            obtenido: 0            OK
[ 5] int no acepta double               esperado: 1            obtenido: 1            OK
[ 6] cero rutas de ejecucion            esperado: -1           obtenido: -1           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion17.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA revisaba las rutas también para las funciones `void`: con `diagnosticar_retorno("void", "void", 2, 1)` regresaba 2, como si faltara un return, cuando en C una función void puede terminar sin `return` y no es error. El harness no prueba ese caso, por eso pasaba 6/6. Lo corregí: si la función o la expresión es `void`, se decide solo por el tipo y se regresa antes de mirar las rutas. Otra cosa que revisé es que un tipo desconocido (por ejemplo `"string"`) no se confunda con incompatible: regresa -1 porque no hay forma de saber su ancho. Con `-fsanitize=address,undefined` no salió nada. Todo es O(1).

---
**Firma digital:** josessuarez
