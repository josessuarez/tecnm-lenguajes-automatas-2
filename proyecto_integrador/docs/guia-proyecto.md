# Guía del Proyecto Integrador · Lenguajito

**Lenguajes y Autómatas II (SCD-1016)** · 30% de la calificación

Esta guía explica **cómo se trabaja** el proyecto, **qué se entrega** en cada hito y
**cómo se califica**. Las reglas formales están en
[`PROYECTO.md`](../../PROYECTO.md) y el lenguaje en
[`gramatica.md`](gramatica.md). Si algo no coincide, manda `PROYECTO.md`.

---

## 1. En 60 segundos

- Vas a construir un **compilador** para *Lenguajito*: variables `int`, asignaciones,
  `if/else`, `while` y `print`.
- En **equipos de 3 o 4**, con un repositorio por equipo (fork del repo del curso).
- Son **4 hitos** de 7.5 puntos: sesiones **10, 20, 30 y 40**.
- Entre hito y hito hay **avances** de 2 minutos (sesiones 05, 08, 15, 25 y 35).
- Todo se revisa **en tu repositorio**: `make test-hitoN` tiene que estar en verde.

---

## 2. Cómo se organiza el trabajo

### 2.1 Repositorio

1. Un integrante hace **fork** del repo del curso y agrega a los demás como
   colaboradores (*Settings → Collaborators*).
2. Todo el proyecto vive en `proyecto_integrador/`. No se aceptan enlaces a repos
   externos.
3. Cada hito se cierra con un **tag**: `v0.1-hito1`, `v0.2-hito2`, `v0.3-hito3`,
   `v1.0-hito4`.

```bash
git tag v0.2-hito2
git push origin v0.2-hito2
```

### 2.2 Roles sugeridos

Pueden rotar en cada hito. Lo importante es que **todos programen** y que el
`README` del proyecto diga quién hizo qué.

| Rol | Se ocupa de |
| :--- | :--- |
| Léxico y sintaxis | `lexer.c`, `parser.c`, AST |
| Semántica | tabla de símbolos, ámbitos, diagnósticos |
| TAC y optimización | `tac.c`, plegado, código muerto |
| Backend y pruebas | `codegen.c`, `as`/`gcc`, `tests/` |

### 2.3 Lo que ya está hecho y lo que es tuyo

| Ya viene | Es tuyo |
| :--- | :--- |
| `Makefile` | lexer, parser y AST |
| `main.c` (interfaz de línea de comandos) | análisis semántico |
| `compilador.h` y un `compilar()` vacío | TAC y optimizaciones |
| `tests/` con las pruebas del curso | backend x86-64 y enlace |

---

## 3. Calendario

| Sesión | Entregable | Qué se revisa |
| :---: | :--- | :--- |
| 05 | **Hito 0** (requisito, sin puntos) | repo, `make`, gramática leída, un programa de prueba |
| 08 | Avance 1 | tokens de un programa |
| **10** | **Hito 1** | `--tokens`, `--ast` y `--check` de expresiones |
| 15 | Avance 2 | parser de sentencias y tabla de símbolos inicial |
| **20** | **Hito 2** | gramática completa, ámbitos y diagnósticos |
| 25 | Avance 3 | TAC de expresiones y de un `if` |
| **30** | **Hito 3** | TAC de todo, plegado y código muerto |
| 35 | Avance 4 | `--asm` de una expresión y primer binario |
| **40** | **Hito 4** | backend, ELF y demo final |

---

## 4. Qué se entrega en cada hito

Marca la lista antes de la clase. Si algo no está, no cuenta como entregado.

### Hito 1 · Sesión 10 · Léxico, sintaxis y AST

- [ ] `--tokens` imprime `linea:columna TIPO lexema`, un token por línea.
- [ ] `--ast` produce el formato de la especificación para 3 programas.
- [ ] `--check` acepta los válidos y reporta `S01`/`E01` en los inválidos.
- [ ] `make test-hito1` en verde.
- [ ] `README` del proyecto con "qué funciona / qué no".
- [ ] Tag `v0.1-hito1`.
- [ ] Demo de 3 minutos.

### Hito 2 · Sesión 20 · Sentencias, ámbitos y diagnósticos

- [ ] Parser completo: bloques, `if/else`, `while`, `print`.
- [ ] Tabla de símbolos con ámbitos anidados y shadowing.
- [ ] Errores `E01` (no declarada / sin valor), `E02` (redeclaración) y `S01`.
- [ ] Se reportan varios errores en una pasada.
- [ ] `--run` ejecuta los programas (interpretar el AST está permitido aquí).
- [ ] `make test-hito2` en verde.
- [ ] Tag `v0.2-hito2` y demo.

### Hito 3 · Sesión 30 · TAC y optimización

- [ ] `--tac` cubre expresiones, asignaciones, `if/else`, `while` y `print`.
- [ ] Temporales únicos `_t0`, `_t1`, ...
- [ ] Plegado de constantes: no quedan operaciones entre constantes.
- [ ] Eliminación de código muerto: no quedan temporales que nadie usa.
- [ ] `make test-hito3` en verde (incluye la prueba de plegado).
- [ ] `--tac` de dos programas explicado en la demo.
- [ ] Tag `v0.3-hito3` y demo.

### Hito 4 · Sesión 40 · Backend y binario ELF

- [ ] `--asm` genera AT&T con `.text`, `.globl main`, prólogo, cuerpo y epílogo.
- [ ] `--run` ejecuta el **binario generado** (no un intérprete).
- [ ] `file` o `readelf` muestran un ELF ejecutable.
- [ ] `make test` completo en verde.
- [ ] Tag `v1.0-hito4`.
- [ ] Demo final de 5 minutos: un programa anidado, uno con errores y uno con la
      optimización visible.

---

## 5. Cómo se piden los avances

Cada avance se documenta en **`proyecto_integrador/docs/avances.md`** (ya viene el
formato en el repo) y se presenta en 2 minutos en clase. Tres bullets:

```text
1. Qué compila hoy (comando exacto).
2. Qué falla y por qué.
3. Qué sigue antes del hito.
```

El archivo debe tener el **commit** del avance. Si el avance falta, el hito
siguiente vale **10% menos**. Los avances no llevan puntos propios: son el
termómetro para que no llegues al hito sin nada.

---

## 6. Cómo se evalúa

### 6.1 Cómo se califica, paso a paso

El docente clona el fork, se va al tag del hito y corre:

```bash
git checkout v0.2-hito2
cd proyecto_integrador
make
make test-hito2
```

Esa salida es la evidencia de "Funciona" y "Pruebas". Después revisa el `README`,
el reparto de trabajo y la bitácora de IA, y pide la demo.

### 6.2 Rúbrica (7.5 puntos por hito)

| Criterio | Puntos | Evidencia en el repo |
| :--- | :---: | :--- |
| **Funciona** | 3.0 | `make test-hitoN` en verde + demo con el comando exacto |
| **Pruebas** | 1.5 | todas las pruebas de `tests/` + las tuyas en `tests/propias/` |
| **Claridad y demo** | 1.5 | `README`, código modular, sin warnings, cualquier integrante explica |
| **Auditoría e IA** | 1.5 | `docs/entrega_hitoN.md` con la tabla C-R-E-O |

### 6.3 Ejemplo de cálculo

Un equipo entrega el Hito 2:

| Criterio | Puntos obtenidos | Por qué |
| :--- | :---: | :--- |
| Funciona | 3.0 | `make test-hito2` en verde y demo correcta |
| Pruebas | 1.5 | doradas + 3 pruebas propias |
| Claridad y demo | 1.0 | un integrante no supo explicar el parser |
| Auditoría IA | 1.5 | bitácora completa, con dos respuestas rechazadas |
| **Total** | **7.0 / 7.5** | |

Si además faltó el avance de la sesión 15: `7.0 × 0.9 = 6.3`.

### 6.4 Penalizaciones

| Situación | Efecto |
| :--- | :--- |
| Sin demo | máximo 50% del hito |
| `make test` en rojo | 0 en "Funciona" y máximo 0.5 en "Pruebas" |
| Sin bitácora de IA | −1.5 |
| Avance faltante | −10% del hito siguiente |
| Entrega tardía | −10% por día, hasta 3 días; después 0 |
| `tests/` modificado | 0 en "Pruebas" |
| Código copiado de otro equipo | 0 en el proyecto |

### 6.5 Reparto individual

En la demo cualquier integrante puede ser cuestionado sobre cualquier parte. Si no
la explica, esa parte no se considera entregada. El `README` y los commits deben
reflejar el reparto; si alguien no participó, se documenta en el avance y se revisa
con evidencia.

---

## 7. Requisitos que no se negocian

1. **C11 y `gcc`**, con `-Wall -Wextra -Werror`, como en los laboratorios.
2. **Parser descendente recursivo escrito a mano.** Nada de `flex`, `bison`,
   `yacc` ni `ANTLR`.
3. **Solo `libc`.** Sin bibliotecas de terceros.
4. **`tests/` no se toca.** Las pruebas doradas son el contrato.
5. **La gramática base no se cambia.** Las extensiones se autorizan antes.
6. **IA con contrato C-R-E-O y auditoría.** Sin bitácora, se pierden 1.5 puntos.
7. **Un repo por equipo**, todo dentro de `proyecto_integrador/`.

---

## 8. Antes de cada entrega: checklist de 10

1. `make clean && make` compila sin warnings.
2. `make test-hitoN` está en verde.
3. El tag del hito está subido (`git push origin v0.N-hitoN`).
4. El `README` dice qué funciona y qué no, con comandos exactos.
5. El reparto de trabajo está actualizado.
6. La bitácora C-R-E-O está en `docs/entrega_hitoN.md`.
7. `docs/avances.md` tiene el último avance con su commit.
8. No modificaste `tests/`.
9. La demo está ensayada (3 o 5 minutos según el hito).
10. Cualquier integrante puede explicar cualquier archivo.

---

## 9. Preguntas frecuentes

**¿Puedo usar el parser del laboratorio 10?**
Sí, es tuyo. Tendrás que ampliarlo para la gramática completa.

**¿Tengo que hacer un intérprete antes del backend?**
No es obligatorio, pero es la forma más rápida de probar la semántica en el Hito 2.
En el Hito 4, `--run` debe usar el binario generado.

**¿Qué significa exactamente "Funciona"?**
Que `make test-hitoN` termina en verde **y** que en la demo ejecutas el comando y se
ve el resultado. No alcanza con decir que funciona.

**¿Puedo cambiar los programas de `tests/`?**
No. Si un programa está mal, avísale al docente y se corrige en el repo del curso
para todos.

**¿Y si mi equipo va muy bien y quiere más?**
Pide una extensión antes del Hito 3 (`for`, arreglos, funciones, `float`...). Suma
hasta 1 punto sobre 30 y se demuestra en la entrega final.

**¿Qué pasa si un integrante no aparece?**
Documéntalo en el avance con evidencia (commits, reparto). Se revisa caso por caso;
la nota individual puede ajustarse.

---

## 10. Enlaces

- Reglas completas: [`PROYECTO.md`](../../PROYECTO.md)
- Lenguaje, AST, diagnósticos y TAC: [`gramatica.md`](gramatica.md)
- Pruebas doradas: [`../tests/`](../tests/)
- Plantilla de entrega: [`../../plantillas/entrega_hito.md`](../../plantillas/entrega_hito.md)
- Avances: [`avances.md`](avances.md)
