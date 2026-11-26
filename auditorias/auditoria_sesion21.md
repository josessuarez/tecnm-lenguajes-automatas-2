# Bitácora de Requerimientos y Auditoría IA — Sesión 21
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Fundamentos de Código de 3 Direcciones  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** JOSE MANUEL SUAREZ HUEX  
**Matrícula:** 24531320  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Representación en Cuádruplos (op,a,b,r)  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo de `nuevo_temporal()` salió rápido con `snprintf`: escribo `_t` más el contador y luego lo subo, así la primera vez sale `_t0`. Donde me atoré fue en `emitir_cuadruplo()` cuando faltan argumentos: al principio le pasaba el `NULL` directo a `%s` y eso es comportamiento indefinido (en mi máquina imprimía `(null)`). Lo arreglé con el operador `? :` para que cada `NULL` se escriba como `_`. También me di cuenta de que nunca debo escribir más de `cap`, por eso uso `snprintf` y no `sprintf`.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 / GCC con `-Wall -Wextra -Werror`, solo biblioteca estándar; se compila con un harness que ya trae `main()` |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Generación de temporales únicos (_t0..) |
| **Output (O)** | Solo código modular (sin `main`, auxiliares `static`) y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Estoy haciendo la parte de código intermedio (TAC) de un compilador
en C11, sesión 21: Fundamentos de Código de 3 Direcciones. Se compila con gcc -std=c11 -Wall -Wextra -Werror
junto con un harness que ya tiene main(), así que no lleva main(). Solo
biblioteca estándar. Contrato (api_sesion21.h):

void nuevo_temporal(char *dst, int cap, int *contador);
void emitir_cuadruplo(char *dst, int cap, const char *op, const char *a1, const char *a2, const char *res);

ROL: Actúa como ingeniero de software de sistemas y compiladores.

ESPECIFICACIÓN:
- nuevo_temporal() escribe "_tK" con K = *contador y después incrementa el contador
  (la primera llamada da _t0).
- emitir_cuadruplo() escribe "(op, a1, a2, res)"; cada argumento NULL se escribe "_".
- Nunca escribir más de cap bytes, sin malloc ni variables globales.
- Si dst, cap o contador no sirven, no se toca nada.
- No cambies las firmas. Revisa todos los punteros NULL antes de usarlos.

OUTPUT: Solo el archivo ia_sesion21_c.c (con #include "api_sesion21.h"),
funciones auxiliares static y la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
```c
/* ia_sesion21_c.c - Fase 2: El Cientifico
 * Propuesta de la IA (revisada: no avanza el contador si no pudo escribir)
 */
#include "api_sesion21.h"
#include <stddef.h>
#include <stdio.h>

/* Devuelve el texto o "_" si el campo viene vacio. O(1). */
static const char *o_guion(const char *s) {
    return (s == NULL) ? "_" : s;
}

/* Escribe _tK en dst con K = *contador y despues lo incrementa. O(1).
 * Si dst, cap o contador no sirven, no toca nada. */
void nuevo_temporal(char *dst, int cap, int *contador) {
    if (dst == NULL || cap <= 0 || contador == NULL || *contador < 0) return;
    int w = snprintf(dst, (size_t)cap, "_t%d", *contador);
    if (w > 0 && w < cap) (*contador)++;   /* solo si el nombre cupo completo */
}

/* Formatea (op, a1, a2, res); cada NULL se escribe como "_". O(longitud). */
void emitir_cuadruplo(char *dst, int cap, const char *op, const char *a1,
                      const char *a2, const char *res) {
    if (dst == NULL || cap <= 0) return;
    snprintf(dst, (size_t)cap, "(%s, %s, %s, %s)",
             o_guion(op), o_guion(a1), o_guion(a2), o_guion(res));
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Expresiones aritméticas compuestas

### Salida del harness
```text
=== AUDITORIA SESION 21 - Cuadruplos y temporales unicos ===
Caso                               esperado:              obtenido:              resultado
[ 1] nuevo_temporal(_t0)                esperado: _t0          obtenido: _t0          OK
[ 2] nuevo_temporal(_t7)                esperado: _t7          obtenido: _t7          OK
[ 3] cuadruplo de a + b
     esperado: (+, a, b, _t0)
     obtenido: (+, a, b, _t0)
     -> OK
[ 4] cuadruplo de _t0 * c
     esperado: (*, _t0, c, _t1)
     obtenido: (*, _t0, c, _t1)
     -> OK
[ 5] asignacion simple                  esperado: (=, a, _, x) obtenido: (=, a, _, x) OK
[ 6] goto sin operandos
     esperado: (goto, L1, _, _)
     obtenido: (goto, L1, _, _)
     -> OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion21.md
```

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA lo hizo casi igual que yo, pero metió una función `o_guion()` que queda más limpia que repetir el `? :` cuatro veces. El caso que no había pensado ninguno de los dos al principio es qué pasa si el buffer es muy chico: el contador se subía aunque el nombre del temporal saliera cortado, y entonces se "gastaba" un número. Le pedí que solo incremente si `snprintf` regresó menos que `cap`, o sea que sí cupo completo. También le agregué que un contador negativo no se use. Todo es O(1) y lo corrí con AddressSanitizer y UBSan sin errores.

---
**Firma digital:** josessuarez
