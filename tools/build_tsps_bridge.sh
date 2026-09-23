#!/usr/bin/env bash
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
SRC="$ROOT/src"
GL="$SRC/glbridge"
BUILD="$ROOT/build/tsps-bridge"
OUT="$ROOT/portmaster/postal2"
ZIG=${ZIG:-zig}

mkdir -p "$BUILD" "$OUT/glbridge"
common=(-O2 -s -fno-unwind-tables -fno-asynchronous-unwind-tables -I "$SRC" -I "$GL")

"$ZIG" cc -target arm-linux-gnueabihf -mcpu=cortex_a7 -shared -fPIC "${common[@]}" \
  -o "$BUILD/libEGL.so.1" "$GL/client.c" "$GL/client_xport.c" -lm

for name in libEGL.so libEGL.so.1 libGLESv1_CM.so libGLESv1_CM.so.1 \
            libGLESv2.so libGLESv2.so.2; do
  cp -f "$BUILD/libEGL.so.1" "$OUT/glbridge/$name"
done

"$ZIG" cc -target aarch64-linux-gnu.2.17 -O2 -s \
  -fno-unwind-tables -fno-asynchronous-unwind-tables \
  -I "$GL" -o "$OUT/postal2_present" "$GL/server.c" -ldl

chmod 0755 "$OUT/postal2_present" "$OUT/glbridge"/*
file "$OUT/postal2_present" "$OUT/glbridge/libEGL.so.1"
sha256sum "$OUT/postal2_present" "$OUT/glbridge/libEGL.so.1"
