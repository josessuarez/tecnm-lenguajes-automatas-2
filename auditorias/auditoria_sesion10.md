# Bitácora de Requerimientos y Auditoría IA — Sesión 10
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Hito 1: Mini-Evaluador de Expresiones  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Integración AST y Tipado Estático  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> La neta lo más difícil fue entender que la precedencia sale de la forma de las funciones y no de un `if`. Al principio quería leer todo de izquierda a derecha y con `2+3*4` me daba 20. Ya cuando separé en tres niveles (`expr` para + y -, `termino` para * y /, `primaria` para números y paréntesis) salió 14 solito, porque `termino` se "come" la multiplicación antes de regresar a la suma. Los paréntesis fueron fáciles porque `primaria` vuelve a llamar a `expr`, o sea es recursivo. Lo que sí me costó fue el manejo de errores: si encuentro un error en medio, tengo que dejar de calcular pero sin perder cuál fue el primero, porque `10/0` tiene que dar error 1 y no 2. Por eso solo guardo el error si todavía no había otro. También metí el menos unario en `primaria` por si viene algo como `-5+10`; el caso `5-10` del harness sí es resta normal y tiene que dar -5. Y al final, si después de evaluar todavía sobra texto (como en `2+3)`), lo marco como error 2.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Analizador descendente recursivo para enteros con `+ - * /`, paréntesis, espacios y menos unario; error 1 = división entre cero, error 2 = sintaxis inválida |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy armando el Hito 1 de un compilador en C11: un mini-evaluador
de expresiones. Se compila con gcc -std=c11 -Wall -Wextra -Werror junto con
un harness que ya tiene main(), así que mi archivo NO lleva main(). No puedo
usar eval ni bibliotecas de expresiones. Contrato (api_sesion10.h):

long evaluar_expresion(const char *fuente, int *error);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- Entrada: enteros, + - * /, paréntesis y espacios. Sin variables.
- Respetar precedencia (* y / antes que + y -) y paréntesis.
- Aceptar menos unario al inicio o después de un operador (ej. "-5+10" = 5).
- *error = 0 si todo bien, 1 si hay división entre cero, 2 si la sintaxis
  es inválida (operador sin operando, paréntesis sin cerrar, texto sobrante).
- Si hay error, regresar 0. Si error es NULL no debe tronar.
- No cambies la firma.

OUTPUT: Solo el archivo ia_sesion10_c.c (con #include "api_sesion10.h"),
funciones auxiliares static y un comentario con la complejidad.
```

---

## 3. Código Devuelto por el Modelo
```c
#include "api_sesion10.h"
#include <ctype.h>
#include <stddef.h>

/* Estado del analizador: posicion actual y primer error encontrado. */
typedef struct {
    const char *pos;
    int err;            /* 0 = ok, 1 = division entre cero, 2 = sintaxis */
} Analizador;

static long parse_suma(Analizador *a);

static void espacios(Analizador *a) {
    while (isspace((unsigned char)*a->pos)) a->pos++;
}

static void fallar(Analizador *a, int codigo) {
    if (a->err == 0) a->err = codigo;   /* conservamos el primer error */
}

/* factor := ['-'] ( entero | '(' suma ')' ) */
static long parse_factor(Analizador *a) {
    espacios(a);
    if (*a->pos == '-') {
        a->pos++;
        return -parse_factor(a);
    }
    if (*a->pos == '(') {
        a->pos++;
        long v = parse_suma(a);
        espacios(a);
        if (*a->pos == ')') a->pos++;
        else fallar(a, 2);
        return v;
    }
    if (!isdigit((unsigned char)*a->pos)) {
        fallar(a, 2);
        return 0;
    }
    long v = 0;
    while (isdigit((unsigned char)*a->pos)) {
        v = v * 10 + (*a->pos - '0');
        a->pos++;
    }
    return v;
}

/* producto := factor (('*' | '/') factor)* */
static long parse_producto(Analizador *a) {
    long v = parse_factor(a);
    for (;;) {
        espacios(a);
        char op = *a->pos;
        if (a->err || (op != '*' && op != '/')) return v;
        a->pos++;
        long d = parse_factor(a);
        if (a->err) return 0;
        if (op == '/') {
            if (d == 0) { fallar(a, 1); return 0; }
            v /= d;
        } else {
            v *= d;
        }
    }
}

/* suma := producto (('+' | '-') producto)* */
static long parse_suma(Analizador *a) {
    long v = parse_producto(a);
    for (;;) {
        espacios(a);
        char op = *a->pos;
        if (a->err || (op != '+' && op != '-')) return v;
        a->pos++;
        long d = parse_producto(a);
        if (a->err) return 0;
        v = (op == '+') ? v + d : v - d;
    }
}

/* Complejidad: O(n) en tiempo, n = longitud de la cadena (cada caracter se
 * lee una vez); O(p) de pila, p = profundidad de parentesis. */
long evaluar_expresion(const char *fuente, int *error) {
    int basura;
    if (error == NULL) error = &basura;
    if (fuente == NULL) { *error = 2; return 0; }

    Analizador a = { fuente, 0 };
    long v = parse_suma(&a);
    espacios(&a);
    if (a.err == 0 && *a.pos != '\0') fallar(&a, 2);   /* sobra texto */

    *error = a.err;
    return a.err ? 0 : v;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Precedencia, paréntesis, división entre cero y sintaxis inválida

### Salida del harness
```text
=== AUDITORIA SESION 10 - Mini-evaluador de expresiones (Hito 1) ===
Caso                               esperado:              obtenido:              resultado
[ 1] "2+3*4"                            esperado: 14           obtenido: 14           OK
[ 2] "(2+3)*4"                          esperado: 20           obtenido: 20           OK
[ 3] "10/0" -> error 1                  esperado: 1            obtenido: 1            OK
[ 4] "2+" -> error 2                    esperado: 2            obtenido: 2            OK
[ 5] "  7 "                             esperado: 7            obtenido: 7            OK
[ 6] "5-10"                             esperado: -5           obtenido: -5           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion10.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> El problema que encontré fue con el orden de los errores. En la primera versión que me dio la IA, cuando había división entre cero seguía analizando el resto y si después venía algo mal de sintaxis lo sobrescribía, entonces algo como `1/0+` regresaba error 2 en vez de 1. El harness no lo detecta porque `10/0` y `2+` vienen por separado. Lo arreglé con la función `fallar()` que solo guarda el código si todavía no hay error, y cortando el ciclo en cuanto aparece uno. Otra cosa que revisé fue `5-10`: el resultado tiene que salir negativo (-5) y como regresamos `long` no hay problema con el signo. La complejidad es O(n) porque cada carácter se lee una sola vez, y la recursión solo crece con los paréntesis anidados.

---
**Firma digital:** josessuarez
