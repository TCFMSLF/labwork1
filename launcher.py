#!/usr/bin/env python3

import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent

PROGRAMS = [
    {
        "name": "geomfun",
        "source": "geomfun.c",
        "build": "gcc -I/usr/local/include geomfun.c -L/usr/local/lib -lraylib -lm -lX11 -lxcb -lXau -o geomfun",
    },
    {
        "name": "Falling_words",
        "source": "Falling_words.cpp",
        "build": "g++ -std=c++11 Falling_words.cpp -lncurses -o Falling_words",
    },
]


def needs_rebuild(prog):
    src = ROOT / prog["source"]
    bin_ = ROOT / prog["name"]
    if not src.exists():
        return False
    if not bin_.exists():
        return True
    return src.stat().st_mtime > bin_.stat().st_mtime


def build(prog):
    print(f"\n>>> {prog['build']}")
    res = subprocess.run(prog["build"], shell=True, cwd=ROOT)
    if res.returncode != 0:
        print("\n\033[31mОшибка сборки.\033[0m")
        sys.exit(1)
    print("\033[32mГотово.\033[0m")


def run(prog):
    print(f"\n\033[32mЗапуск {prog['name']}...\033[0m\n")
    res = subprocess.run([str(ROOT / prog["name"])])
    print(f"\n\033[36mПрограмма завершилась с кодом {res.returncode}\033[0m")


def menu():
    while True:
        print("\033[36m=== Лаунчер ===\033[0m\n")
        for i, prog in enumerate(PROGRAMS):
            status = "\033[32m[built]\033[0m" if not needs_rebuild(prog) else "\033[33m[needs rebuild]\033[0m"
            print(f"  {i}) {prog['source']}  {status}")
        print("\n  a) Собрать всё")
        print("  q) Выход\n")

        choice = input("Выберите номер программы (0-{}, a/q): ".format(len(PROGRAMS) - 1)).strip()

        if choice == "q":
            print("Выход.")
            break
        if choice == "a":
            for prog in PROGRAMS:
                build(prog)
            print("\n\033[32mВсе программы собраны.\033[0m\n")
            continue

        if not choice.isdigit() or int(choice) >= len(PROGRAMS):
            print("\033[31mНеверный выбор.\033[0m\n")
            continue

        prog = PROGRAMS[int(choice)]
        if needs_rebuild(prog):
            build(prog)
        run(prog)
        print()


if __name__ == "__main__":
    try:
        menu()
    except KeyboardInterrupt:
        print("\nВыход.")
        sys.exit(0)
