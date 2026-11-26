#!/usr/bin/env python3
"""Ejecuta las pruebas doradas de Lenguajito.

Uso (desde proyecto_integrador/):
    python3 tests/run.py            # todas
    python3 tests/run.py --hito1    # solo el Hito 1
    make test                       # equivalente a la primera
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parent
BIN = RAIZ.parent / "lenguajito"


def ejecutar(archivo: Path, modo: str) -> subprocess.CompletedProcess:
    return subprocess.run(
        [str(BIN), str(archivo), modo],
        capture_output=True,
        text=True,
    )


def revisar_validos(errores: list[str]) -> int:
    total = 0
    for prog in sorted((RAIZ / "validos").glob("*.lj")):
        total += 1
        esperado = prog.with_suffix(".out").read_text(encoding="utf-8")
        r = ejecutar(prog, "--run")
        if r.returncode != 0:
            errores.append(f"{prog.name}: --run terminó con {r.returncode}")
        elif r.stdout != esperado:
            errores.append(
                f"{prog.name}: salida distinta\n"
                f"  esperado: {esperado!r}\n  obtenido: {r.stdout!r}"
            )
    return total


def revisar_invalidos(errores: list[str]) -> int:
    total = 0
    for prog in sorted((RAIZ / "invalidos").glob("*.lj")):
        total += 1
        linea, codigo = prog.with_suffix(".err").read_text(encoding="utf-8").split()
        r = ejecutar(prog, "--check")
        patron = rf":{linea}:\d+: error {codigo}:"
        if r.returncode != 1:
            errores.append(f"{prog.name}: --check debió salir con 1, salió con {r.returncode}")
        elif not re.search(patron, r.stdout):
            errores.append(
                f"{prog.name}: se esperaba un diagnóstico '{codigo}' en la línea {linea}\n"
                f"  obtenido: {r.stdout!r}"
            )
    return total


def revisar_ast(errores: list[str]) -> int:
    total = 0
    for prog in sorted((RAIZ / "hito1").glob("*.lj")):
        total += 1
        if prog.with_suffix(".err").exists():
            linea, codigo = prog.with_suffix(".err").read_text(encoding="utf-8").split()
            r = ejecutar(prog, "--check")
            if r.returncode != 1 or not re.search(rf":{linea}:\d+: error {codigo}:", r.stdout):
                errores.append(f"{prog.name}: se esperaba {codigo} en la línea {linea}")
            continue
        esperado = prog.with_suffix(".ast").read_text(encoding="utf-8").strip()
        r = ejecutar(prog, "--ast")
        if r.returncode != 0:
            errores.append(f"{prog.name}: --ast terminó con {r.returncode}")
        elif r.stdout.strip() != esperado:
            errores.append(
                f"{prog.name}: AST distinto\n"
                f"  esperado: {esperado}\n  obtenido: {r.stdout.strip()}"
            )
    return total


def revisar_tac(errores: list[str]) -> int:
    total = 0
    patron = re.compile(r"=\s*\d+\s*[-+*/]\s*\d+")
    for prog in sorted((RAIZ / "tac").glob("*.lj")):
        total += 1
        r = ejecutar(prog, "--tac")
        if r.returncode != 0:
            errores.append(f"{prog.name}: --tac terminó con {r.returncode}")
        elif patron.search(r.stdout):
            errores.append(
                f"{prog.name}: quedó una operación entre constantes; falta plegado\n"
                f"  TAC: {r.stdout!r}"
            )
    return total


def revisar_asm(errores: list[str]) -> int:
    total = 0
    for prog in sorted((RAIZ / "validos").glob("*.lj"))[:1]:
        total += 1
        r = ejecutar(prog, "--asm")
        if r.returncode != 0:
            errores.append(f"{prog.name}: --asm terminó con {r.returncode}")
            continue
        if ".globl" not in r.stdout:
            errores.append(f"{prog.name}: el ensamblador no declara .globl")
        if not re.search(r"\bret\b", r.stdout):
            errores.append(f"{prog.name}: el ensamblador no termina con ret")
    return total


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--hito1", action="store_true")
    ap.add_argument("--hito2", action="store_true")
    ap.add_argument("--hito3", action="store_true")
    ap.add_argument("--hito4", action="store_true")
    args = ap.parse_args()

    if not BIN.exists():
        print(f"❌ No existe {BIN}. Ejecuta `make` primero.")
        return 1

    errores: list[str] = []
    total = 0
    modo = "todos"
    if args.hito1:
        modo = "Hito 1"
        total += revisar_ast(errores)
    elif args.hito2:
        modo = "Hito 2"
        total += revisar_validos(errores) + revisar_invalidos(errores)
    elif args.hito3:
        modo = "Hito 3"
        total += revisar_validos(errores) + revisar_invalidos(errores) + revisar_tac(errores)
    elif args.hito4:
        modo = "Hito 4"
        total += (revisar_validos(errores) + revisar_invalidos(errores)
                  + revisar_tac(errores) + revisar_asm(errores))
    else:
        total += (revisar_ast(errores) + revisar_validos(errores)
                  + revisar_invalidos(errores) + revisar_tac(errores)
                  + revisar_asm(errores))

    for error in errores:
        print(f"❌ {error}")
    print(f"\n{modo}: {total - len(errores)}/{total} pruebas superadas")
    return 0 if not errores else 1


if __name__ == "__main__":
    sys.exit(main())
