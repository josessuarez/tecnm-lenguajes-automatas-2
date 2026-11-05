# Bitácora de Requerimientos y Auditoría IA — Sesión 06
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Sistema de Tipos Primitivos  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Chequeo de compatibilidad de tipos  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> - **Entender la dirección:** al principio me confundí con el orden de los parámetros. `compatible(destino, origen)` pregunta si puedo meter el `origen` en el `destino`, así que pasar de `int` a `float` sí se puede (no se pierde nada) pero de `float` a `int` no, porque se pierden los decimales y ocupa un cast explícito. La regla quedó como `destino >= origen`.
> - **No confiar en los números del enum:** el enum ya trae `CHAR = 1`, `INT = 2`, `FLOAT = 3`, pero preferí hacer una función `rango()` con un `switch` que diga el rango de cada tipo. Así, si el enum cambia de orden o le agregan otro tipo, la jerarquía no se rompe.
> - **El tipo ERROR y valores raros:** `rango()` regresa 0 para `TIPO_ERROR` y para cualquier valor que no sea del enum, y si alguno de los dos es 0, `compatible` regresa 0.
> - **`-Werror` y el `switch`:** en `nombre_tipo` puse los cuatro casos sin `default` para que el compilador avise si falta alguno, y el `return "desconocido"` va después del `switch` para que la función siempre regrese algo y no salga el warning de *control reaches end of non-void function*.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C99 / GCC o Rust 2021 sin bibliotecas no estándar |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Matriz de compatibilidad estática |
| **Output (O)** | Solo código modular y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
C: C11, gcc -std=c11 -Wall -Wextra -Werror, Linux. El archivo api_sesion06.h
   ya existe y no se puede modificar; define
   typedef enum { TIPO_ERROR = 0, TIPO_CHAR = 1, TIPO_INT = 2, TIPO_FLOAT = 3 } Tipo;
R: Ingeniero de compiladores encargado del análisis semántico (chequeo de tipos).
E: Implementa exactamente
     int compatible(Tipo destino, Tipo origen);
     const char *nombre_tipo(Tipo t);
   Reglas:
   1. La jerarquía es CHAR < INT < FLOAT.
   2. compatible regresa 1 si origen se puede asignar a destino sin perder
      datos (destino de igual o mayor rango) y 0 si requiere cast explícito.
   3. TIPO_ERROR nunca es compatible con nada, ni como destino ni como origen.
   4. nombre_tipo regresa "char", "int", "float" o "error".
O: Solo el contenido del .c, con #include "api_sesion06.h", sin main()
   y sin printf.
```

---

## 3. Código Devuelto por el Modelo
```c
#include "api_sesion06.h"

int compatible(Tipo destino, Tipo origen) {
    if (destino == TIPO_ERROR || origen == TIPO_ERROR) {
        return 0;
    }
    /* La jerarquia CHAR < INT < FLOAT coincide con el orden del enum */
    return destino >= origen;
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
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Rechazo de asignaciones incompatibles

### Salida del harness
```text
=== AUDITORIA SESION 06 - Tabla de compatibilidad de tipos ===
Caso                               esperado:              obtenido:              resultado
[ 1] compatible(INT,INT)                esperado: 1            obtenido: 1            OK
[ 2] compatible(FLOAT,INT)              esperado: 1            obtenido: 1            OK
[ 3] compatible(INT,FLOAT)              esperado: 0            obtenido: 0            OK
[ 4] compatible(INT,ERROR)              esperado: 0            obtenido: 0            OK
[ 5] nombre_tipo(INT)                   esperado: int          obtenido: int          OK
[ 6] nombre_tipo(ERROR)                 esperado: error        obtenido: error        OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion06.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> Las dos versiones sacaron 6/6. La IA cumple todas las reglas del prompt, pero **solo rechaza `TIPO_ERROR` y no revisa que el tipo sea uno de los que existen**. Como en C un `enum` es un entero, se le puede pasar cualquier número con un cast. El harness no lo prueba porque siempre usa los cuatro tipos válidos; lo vi leyendo el `return destino >= origen;` y lo comprobé con un `main` de prueba:
>
> | Llamada | IA | Mi artesano |
> | :--- | :---: | :---: |
> | `compatible((Tipo)7, TIPO_INT)` | **1** | 0 |
> | `nombre_tipo((Tipo)7)` | `desconocido` | `desconocido` |
>
> La IA dice que un `int` se puede asignar a un tipo que no existe, nada más porque 7 es mayor que 2. En un compilador real ese 7 podría venir de un nodo del AST mal construido, y el chequeo semántico lo dejaría pasar.
>
> La IA también depende de que el orden del enum sea igual al de la jerarquía (lo dice en su comentario). Si mañana agregan `TIPO_DOUBLE` en otra posición o reordenan el enum, `destino >= origen` deja de servir sin que nadie se dé cuenta.
>
> Lo solucioné con la función `rango()` de mi versión, que le da a cada tipo válido su rango con un `switch` y regresa 0 para `TIPO_ERROR` y para cualquier valor fuera del enum:
>
> ```c
> int d = rango(destino);
> int o = rango(origen);
> if (d == 0 || o == 0) return 0;
> return d >= o;
> ```
>
> **Complejidad:** O(1) en tiempo y memoria para las dos funciones; solo son comparaciones y un `switch`, sin ciclos ni memoria dinámica.

---
**Firma digital:** jossesuarez

