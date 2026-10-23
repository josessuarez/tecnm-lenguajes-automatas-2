# Bitácora de Requerimientos y Auditoría IA — Sesión 01

**Materia:** Lenguajes y Autómatas II (SCD-1016)
**Tema:** Encuadre, Diagnóstico de Punteros y Metodología IA
**Docente Titular:** MCC. Iván Márquez Larios (ijmarquezl)
**Estudiante:** JOSE MANUEL SUAREZ HUEX
**Matrícula:** 24531320

---

## 1. Fase 1: El Artesano (Dificultad Manual)

**Reto:** Inversión de buffer in-place (`char*`)

**¿Qué problema o restricción de memoria/sintaxis identifiqué al codificar a mano?**

La idea general sí la tenía clara: un índice al inicio, otro al final, ir intercambiando con un `char` temporal hasta que se crucen. Pero me atoré con los casos borde y terminé apoyándome en la IA para cerrar la implementación.

Lo que más me costó o que no había considerado:

- **El `NULL`:** si no lo checas antes, `strlen(NULL)` truena con segfault.
- **La cadena vacía:** `strlen("")` da 0, y como `size_t` no tiene negativos, `n - 1` no da -1 sino un número enorme. Por eso hay que salir antes si `n < 2`.
- **In-place:** no se puede crear otro arreglo ni usar `malloc`; todo se hace sobre el mismo buffer.

También me pasó que corrí el test **antes** de guardar la celda con mi código, y me salió 3/6 porque estaba probando la versión vacía con el `TODO`. Después de volver a ejecutar la celda del `.c` y luego el test, salió **6/6**.

Código final:

```c
#include "api_sesion01.h"
#include <stdio.h>
#include <string.h>

void invertir(char *s) {
    if (s == NULL) return;

    size_t n = strlen(s);
    if (n < 2) return;

    size_t i = 0;
    size_t j = n - 1;

    while (i < j) {
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        i++;
        j--;
    }
}
```

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal

| Elemento C-R-E-O | Definición Técnica |
|---|---|
| Contexto (C) | C99 / GCC sin bibliotecas no estándar |
| Rol (R) | Ingeniero de software de sistemas y compiladores |
| Especificación (E) | Punteros, O(n) tiempo, O(1) memoria |
| Salida (O) | Solo código modular y análisis de complejidad |

### 2.2 Prompt Estructurado

> **Contexto:** Estoy haciendo una práctica en C (C99/C11) con GCC, sin bibliotecas fuera del estándar. Tengo un archivo `api_sesion01.h` que declara `void invertir(char *s);` y un harness que lo compila con `-std=c11 -Wall -Wextra -Werror`.
>
> **Rol:** Actúa como ingeniero de software de sistemas y compiladores.
>
> **Especificación:** Implementa `invertir` usando punteros para que invierta la cadena in-place, en O(n) de tiempo y O(1) de memoria extra (sin copias ni `malloc`). Debe funcionar con `"HOLA"`, `"abc"`, `""`, `"AB"`, `"a"` y `NULL` sin crashear. No cambies la firma y no incluyas `main()`.
>
> **Output:** Solo el código de la función listo para pegar en `ia_sesion01_c.c`, y abajo una línea con su complejidad.

---

## 3. Código Devuelto por el Modelo

```c
// Código devuelto por el LLM
void invertir(char *s) {
    if (s == NULL || *s == '\0') return;

    char *ini = s;
    char *fin = s;
    while (*(fin + 1) != '\0') {
        fin++;
    }

    while (ini < fin) {
        char tmp = *ini;
        *ini = *fin;
        *fin = tmp;
        ini++;
        fin--;
    }
}
// Complejidad: O(n) tiempo, O(1) memoria.
```

**Corrección que tuve que hacer:** agregar `#include <stddef.h>` arriba de la función, porque sin eso no compilaba (ver reporte crítico).

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)

**Pruebas ejecutadas:** valores `NULL`, buffers vacíos y longitudes pares e impares.

- [x] Caso base y límites superados sin crash.
- [x] No existen accesos fuera de límites ni memory leaks.
- [x] La complejidad teórica coincide con la implementada.

**Resultado del harness:** las dos versiones (artesano e IA) dieron **6/6 pruebas superadas**.
