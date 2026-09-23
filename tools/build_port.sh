#!/usr/bin/env bash
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd -P)
PORT="$ROOT/portmaster"
DIST="$PORT/dist"
ZIG=${ZIG:-zig}
mkdir -p "$DIST"
ZIG="$ZIG" "$ROOT/tools/build_tsps_bridge.sh"
chmod 0755 "$PORT/Postal 2.sh" "$PORT/postal2/setup.sh"
rm -f "$DIST/postal2.zip"
cd "$PORT"
zip -9 -r "$DIST/postal2.zip" "Postal 2.sh" postal2 \
  -x 'postal2/gamedata/*' 'postal2/logs/*' 'postal2/conf/*' \
     'postal2/Postal 2.zip' 'postal2/postal2.zip' \
     'postal2/.postal2-stage/*' 'postal2/*.log' 'postal2/*.ready' >/dev/null
if [ -f postal2/gamedata/README.txt ]; then
  zip -9 -r "$DIST/postal2.zip" postal2/gamedata/README.txt >/dev/null
fi
printf '%s\n' "$DIST/postal2.zip"
