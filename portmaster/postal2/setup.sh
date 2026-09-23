#!/bin/bash
set -eu

GAMEDIR=${1:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)}
CONTROL=${PORTMASTER_CONTROLFOLDER:-}
if [ -z "$CONTROL" ]; then
  CONTROL=/mnt/SDCARD/Tools/PortMaster
fi

if [ -f "$GAMEDIR/gamedata/System/postal2-bin" ]; then
  echo "Postal 2 game data is already installed"
  exit 0
fi

ARCHIVE=
for candidate in "$GAMEDIR/Postal 2.zip" "$GAMEDIR/postal2.zip"; do
  if [ -f "$candidate" ]; then
    ARCHIVE="$candidate"
    break
  fi
done
if [ -z "$ARCHIVE" ]; then
  echo "Place Postal 2.zip next to the postal2 directory before first launch" >&2
  exit 1
fi

SEVEN=
for candidate in \
  "$CONTROL/7zzs.${DEVICE_ARCH:-}" \
  "$CONTROL/7zzs.aarch64" \
  "$CONTROL/7zzs.armhf" \
  "$CONTROL/7zzs"; do
  if [ -n "$candidate" ] && [ -x "$candidate" ]; then
    SEVEN="$candidate"
    break
  fi
done
if [ -z "$SEVEN" ]; then
  echo "PortMaster 7zzs extractor not found under $CONTROL" >&2
  exit 2
fi

STAGE="$GAMEDIR/.postal2-stage.$$"
cleanup() { rm -rf "$STAGE"; }
trap cleanup EXIT INT TERM
mkdir -p "$STAGE"
"$SEVEN" x -tzip -y "$ARCHIVE" "-o$STAGE" >/dev/null

SOURCE=
for candidate in "$STAGE/postal2/gamedata" "$STAGE/gamedata"; do
  if [ -f "$candidate/System/postal2-bin" ]; then
    SOURCE="$candidate"
    break
  fi
done
if [ -z "$SOURCE" ]; then
  echo "The archive does not contain postal2/gamedata/System/postal2-bin" >&2
  exit 3
fi

DEST="$GAMEDIR/gamedata"
mkdir -p "$DEST"
STAMP=$(date +%s 2>/dev/null || printf 'setup')
for item in "$SOURCE"/*; do
  [ -e "$item" ] || continue
  base=${item##*/}
  if [ -e "$DEST/$base" ]; then
    mv "$DEST/$base" "$DEST/$base.pre-setup.$STAMP"
  fi
  mv "$item" "$DEST/$base"
done

CONF_SOURCE="$STAGE/postal2/conf/.lgp/postal2/System"
CONF_DEST="$GAMEDIR/conf/.lgp/postal2/System"
if [ -d "$CONF_SOURCE" ]; then
  mkdir -p "$CONF_DEST"
  for file in "$CONF_SOURCE"/*.ini; do
    [ -f "$file" ] || continue
    base=${file##*/}
    [ -f "$CONF_DEST/$base" ] || cp -p "$file" "$CONF_DEST/$base"
  done
fi

chmod 0755 "$DEST/System/postal2-bin"
touch "$GAMEDIR/.postal2-data-ready"
echo "Postal 2 game data installed from $ARCHIVE"
