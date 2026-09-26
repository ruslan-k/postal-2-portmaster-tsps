#!/bin/bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
obj=$(mktemp)
trap 'rm -f "$obj"' EXIT
gcc -m32 -fPIC -O2 -Wall -Wextra -Werror -c "$root/src/postal2_sdl_input_trace.c" -o "$obj"
ld -m elf_i386 -shared "$obj" -o "$root/portmaster/postal2/postal2_sdl_input_trace.so"
file "$root/portmaster/postal2/postal2_sdl_input_trace.so"
