#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd -P)
ARCHIVE=${1:-"$HOME/Downloads/Postal 2.zip"}
DEST="$ROOT/portmaster/postal2"
STAGE=$(mktemp -d)
cleanup() { rm -rf "$STAGE"; }
trap cleanup EXIT INT TERM
[ -f "$ARCHIVE" ] || { printf 'Archive not found: %s\n' "$ARCHIVE" >&2; exit 1; }
unzip -q "$ARCHIVE" -d "$STAGE"
SOURCE="$STAGE/postal2"
[ -f "$SOURCE/gamedata/System/postal2-bin" ] || { echo 'Expected postal2/gamedata/System/postal2-bin was not found' >&2; exit 2; }
mkdir -p "$DEST/gamedata"
cp -a "$SOURCE/gamedata/." "$DEST/gamedata/"
if [ -d "$SOURCE/conf/.lgp/postal2/System" ]; then
  mkdir -p "$DEST/conf/.lgp/postal2/System"
  cp -a "$SOURCE/conf/.lgp/postal2/System/." "$DEST/conf/.lgp/postal2/System/"
fi
echo "Staged local payload under $DEST"
