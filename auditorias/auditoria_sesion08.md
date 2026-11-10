# Bitácora de Requerimientos y Auditoría IA — Sesión 08
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Inferencia y Promoción de Tipos  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX 
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Reglas de casteo implícito  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo primero fue darme cuenta de que el enum ya viene en orden (ERROR=0, CHAR=1, INT=2, FLOAT=3), entonces podía comparar los tipos con `<` y `>` como si fueran el rango. Pero ahí mismo está la trampa: ERROR vale 0, o sea que si no lo reviso primero, `promover(ERROR, INT)` me regresa INT porque 2 > 0 y se pierde el error. Por eso puse el `if` de ERROR antes que todo. En `hay_perdida` pasa lo mismo: ERROR < INT daría 1 y diría que hay pérdida cuando ni siquiera es una conversión válida, así que también lo filtré. En `nombre_tipo` usé un `switch` y tuve que dejar un `return` al final porque con `-Wall -Wextra -Werror` el compilador se queja si la función puede terminar sin regresar nada.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, sin bibliotecas no estándar; se compila junto a un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Promoción entera y ensanchamiento: jerarquía CHAR < INT < FLOAT, `promover` regresa el más ancho y propaga ERROR, `hay_perdida` es 1 solo si el destino es más angosto |
| **Output (O)** | Solo código modular (sin `main`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo la parte de tipos de un compilador en C11. Mi archivo
se compila con gcc -std=c11 -Wall -Wextra -Werror junto con un harness que ya
tiene main(), así que mi archivo NO debe tener main(). Solo puedo usar la
biblioteca estándar. Tengo este contrato en api_sesion08.h:

typedef enum { TIPO_ERROR = 0, TIPO_CHAR = 1, TIPO_INT = 2, TIPO_FLOAT = 3 } Tipo;
Tipo promover(Tipo a, Tipo b);
int hay_perdida(Tipo destino, Tipo origen);
const char *nombre_tipo(Tipo t);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- Jerarquía de promoción: CHAR < INT < FLOAT.
- promover(a, b) devuelve el tipo más ancho. Si alguno es TIPO_ERROR devuelve TIPO_ERROR.
- hay_perdida(destino, origen) devuelve 1 cuando el destino es más angosto que el origen y 0 en otro caso.
- nombre_tipo devuelve "char", "int", "float" o "error".
- No cambies las firmas ni el enum.

OUTPUT: Dame solo el archivo ia_sesion08_c.c completo (con #include "api_sesion08.h")
y al final una línea con la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
#include "api_sesion08.h"

/* Rango de cada tipo en la jerarquia CHAR < INT < FLOAT.
 * Devuelve -1 si el tipo no es valido. O(1). */
static int rango(Tipo t) {
    switch (t) {
        case TIPO_CHAR:  return 1;
        case TIPO_INT:   return 2;
        case TIPO_FLOAT: return 3;
        default:         return -1;
    }
}

Tipo promover(Tipo a, Tipo b) {
    int ra = rango(a);
    int rb = rango(b);
    if (ra < 0 || rb < 0)
        return TIPO_ERROR;          /* el centinela ERROR se propaga */
    return (ra >= rb) ? a : b;
}

int hay_perdida(Tipo destino, Tipo origen) {
    int rd = rango(destino);
    int ro = rango(origen);
    if (rd < 0 || ro < 0)
        return 0;                   /* con ERROR no hay conversion valida */
    return rd < ro;
}

const char *nombre_tipo(Tipo t) {
    switch (t) {
        case TIPO_ERROR: return "error";
        case TIPO_CHAR:  return "char";
        case TIPO_INT:   return "int";
        case TIPO_FLOAT: return "float";
        default:         return "desconocido";
    }
}
/* Complejidad: todas las funciones son O(1) en tiempo y memoria. */
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Mezcla de tipos enteros y flotantes

### Salida del harness
```text
=== AUDITORIA SESION 08 - Reglas de promocion y perdida ===
Caso                               esperado:              obtenido:              resultado
[ 1] promover(CHAR,INT)                 esperado: int          obtenido: int          OK
[ 2] promover(INT,FLOAT)                esperado: float        obtenido: float        OK
[ 3] promover(FLOAT,FLOAT)              esperado: float        obtenido: float        OK
[ 4] promover(ERROR,INT)                esperado: error        obtenido: error        OK
[ 5] hay_perdida(CHAR,INT)              esperado: 1            obtenido: 1            OK
[ 6] hay_perdida(INT,CHAR)              esperado: 0            obtenido: 0            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion08.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> El caso borde es el centinela ERROR en `hay_perdida`. El contrato solo dice "1 cuando el destino es más angosto", y como ERROR vale 0 en el enum, si comparas directo los valores `hay_perdida(ERROR, INT)` da 1, como si convertir a "error" perdiera datos, lo cual no tiene sentido. El harness no prueba ese caso (solo prueba ERROR en `promover`), así que pasaría 6/6 aunque estuviera mal. Lo detecté porque a mano ya me había topado con lo mismo en la Fase 1. La versión final usa una función `rango()` que regresa -1 para tipos no válidos; en ese caso `hay_perdida` regresa 0 y `promover` regresa ERROR, así tampoco truena si llega un valor fuera del enum. Además revisé que la IA no metiera `main()` ni cambiara las firmas, que es lo que haría fallar la compilación con el harness.

---
**Firma digital:** jossesuarez
