#!/usr/bin/env bash
set -euo pipefail
ROOT=${1:-"$(cd "$(dirname "$0")/.." && pwd)"}
PIN=e6448b04e5bfdabffbd650e1ccc53b82cd8c1c5d
SRC=${GL4ES_SRC:-/tmp/gl4es-tsps-source}
BUILD=${GL4ES_BUILD:-/tmp/gl4es-tsps-build}
OUT="$ROOT/portmaster/postal2/gl4es/libGL.so.1"
PATCH="$ROOT/patches/gl4es-tsps-safe-profile.patch"
if [ ! -d "$SRC/.git" ]; then git clone https://github.com/ptitSeb/gl4es.git "$SRC"; fi
git -C "$SRC" fetch --quiet origin "$PIN"
git -C "$SRC" reset --hard --quiet "$PIN"
git -C "$SRC" clean -ffdqx --quiet
git -C "$SRC" apply --check "$PATCH"
git -C "$SRC" apply "$PATCH"
cmake -S "$SRC" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DNOX11=ON -DGLX_STUBS=ON -DEGL_WRAPPER=ON -DGBM=ON
cmake --build "$BUILD" --target GL -j"${JOBS:-2}"
install -Dm755 "$SRC/lib/libGL.so.1" "$OUT"
file "$OUT"
sha256sum "$OUT"
