#!/bin/bash
# PORTMASTER: postal2.zip, Postal 2.sh

XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}

if [ -d "/opt/system/Tools/PortMaster/" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
elif [ -d "$XDG_DATA_HOME/PortMaster/" ]; then
  controlfolder="$XDG_DATA_HOME/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi

source "$controlfolder/control.txt"
[ -f "${controlfolder}/mod_${CFW_NAME}.txt" ] && source "${controlfolder}/mod_${CFW_NAME}.txt"
get_controls

GAMEDIR=
for candidate in \
  "${directory:+${directory%/}/postal2}" \
  "${directory:+/${directory#/}/ports/postal2}" \
  /mnt/SDCARD/Data/ports/postal2 \
  /roms/ports/postal2 /sdcard/ports/postal2; do
  if [ -d "$candidate" ]; then
    GAMEDIR="$candidate"
    break
  fi
done
[ -n "$GAMEDIR" ] || { echo "Postal 2 data directory not found" >&2; exit 1; }
cd "$GAMEDIR" || exit 1
mkdir -p "$GAMEDIR/conf" "$GAMEDIR/logs" "$GAMEDIR/gamedata"
LOG="$GAMEDIR/logs/postal2.log"
[ -f "$LOG" ] && mv -f "$LOG" "$LOG.1" 2>/dev/null || true
exec >>"$LOG" 2>&1

echo "===== postal2 start ====="
date 2>/dev/null || true
echo "uname=$(uname -a)"
echo "platform=${PLATFORM:-unset} arch=${PLATFORM_ARCHITECTURE:-unset} cfw=${CFW_NAME:-unset}"

export PORTMASTER_CONTROLFOLDER="$controlfolder"
if [ ! -f "$GAMEDIR/gamedata/System/postal2-bin" ]; then
  if ! "$GAMEDIR/setup.sh" "$GAMEDIR"; then
    echo "Game data setup failed"
    sync
    pm_finish
    exit 1
  fi
fi

export XDG_DATA_HOME="$GAMEDIR/conf"
export XDG_CONFIG_HOME="$GAMEDIR/conf"
export HOME="$GAMEDIR/conf"
export SDL_GAMECONTROLLERCONFIG="${sdl_controllerconfig:-${SDL_GAMECONTROLLERCONFIG:-}}"
export SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS=1
export SDL_JOYSTICK_HIDAPI=0
export SDL_JOYSTICK_DISABLE=1
export SDL_NO_SIGNAL_HANDLERS=1
export MALLOC_ARENA_MAX=2

for ini in "$GAMEDIR/gamedata/System/Postal2.ini" \
           "$GAMEDIR/gamedata/System/Postal2MP.ini" \
           "$GAMEDIR/conf/.lgp/postal2/System/Postal2.ini" \
           "$GAMEDIR/conf/.lgp/postal2/System/Postal2MP.ini"; do
  if [ -f "$ini" ]; then
    sed -i 's/master.gamespy.com/master.333networks.com/g' "$ini"
    sed -i 's/ViewportX=800/ViewportX=640/g' "$ini"
    sed -i 's/ViewportY=600/ViewportY=480/g' "$ini"
  fi
done

BRIDGE=0
case "${POSTAL2_TSPS_BRIDGE:-0}" in
  1|yes|true|on) BRIDGE=1 ;;
  0|no|false|off) BRIDGE=0 ;;
  auto)
    if [ "${PLATFORM:-}" = "SmartProS" ] || \
       { [ "$(uname -m 2>/dev/null)" = "aarch64" ] && [ -d /mnt/SDCARD/spruce ]; }; then
      BRIDGE=1
    fi
    ;;
esac

PRES=0
INPUT_PID=0
XVFB_PID=0
XORG_CONFIG_TMP=0
XVFB_DISPLAY="${POSTAL2_XVFB_DISPLAY:-99}"
cleanup() {
  if [ "$INPUT_PID" -ne 0 ]; then
    kill "$INPUT_PID" 2>/dev/null || true
    wait "$INPUT_PID" 2>/dev/null || true
    INPUT_PID=0
  fi
  if [ "$XVFB_PID" -ne 0 ]; then
    kill "$XVFB_PID" 2>/dev/null || true
    wait "$XVFB_PID" 2>/dev/null || true
    XVFB_PID=0
  fi
  if [ "$PRES" -ne 0 ]; then
    kill "$PRES" 2>/dev/null || true
    wait "$PRES" 2>/dev/null || true
    PRES=0
  fi
  if [ "$XORG_CONFIG_TMP" -ne 0 ]; then
    rm -f /tmp/postal2-xorg.conf
    XORG_CONFIG_TMP=0
  fi
  rm -f /tmp/postal2.present.ready /tmp/tsp-glbridge.sock /tmp/tspgl-xport /tmp/postal2.frame "/tmp/.X${XVFB_DISPLAY}-lock" "/tmp/.X11-unix/X${XVFB_DISPLAY}"
}
trap cleanup EXIT INT TERM

find_input_event_by_name() {
  wanted="$1"
  for namefile in /sys/class/input/event*/device/name; do
    [ -f "$namefile" ] || continue
    name=$(cat "$namefile" 2>/dev/null || true)
    if [ "$name" = "$wanted" ]; then
      event=${namefile#/sys/class/input/}
      event=${event%%/*}
      printf '/dev/input/%s' "$event"
      return 0
    fi
  done
  return 1
}

start_input_helper() {
  cd "$GAMEDIR/gamedata/System" || return 1
  $GPTOKEYB2 "postal2-bin" -c "$GAMEDIR/postal2.ini" >>"$LOG" 2>&1 &
  INPUT_PID=$!
  n=0
  while [ "$n" -lt 10 ]; do
    POSTAL2_INPUT_EVENT=$(find_input_event_by_name "Fake Keyboard Mouse" || true)
    if [ -n "$POSTAL2_INPUT_EVENT" ]; then
      export POSTAL2_INPUT_EVENT
      echo "input_helper_pid=$INPUT_PID input_event=$POSTAL2_INPUT_EVENT input_wait=$n"
      return 0
    fi
    if ! kill -0 "$INPUT_PID" 2>/dev/null; then
      break
    fi
    n=$((n + 1))
    sleep 1
  done
  echo "input_helper_failed pid=$INPUT_PID input_wait=$n"
  return 1
}

run_hybrid_backend() {
  echo "backend=tsps-bridge-weston-x11"
  SYS="$GAMEDIR/armhf"
  LD="$SYS/lib/ld-linux-armhf.so.3"
  GLBRIDGE="$GAMEDIR/glbridge"
  PRESENTER="$GAMEDIR/postal2_present"
  GAMEBIN="$GAMEDIR/gamedata/System/postal2-bin"
  if [ ! -x "$PRESENTER" ] || [ ! -f "$LD" ] || [ ! -f "$GLBRIDGE/libEGL.so.1" ] || [ ! -x "$GAMEDIR/box86/box86" ] || [ ! -f "$GAMEDIR/gl4es/libGL.so.1" ]; then
    echo "Hybrid bridge files are incomplete"
    return 2
  fi

  export POSTAL2_WIDTH="${POSTAL2_WIDTH:-640}"
  export POSTAL2_HEIGHT="${POSTAL2_HEIGHT:-480}"
  export TSPGL_WIDTH="$POSTAL2_WIDTH"
  export TSPGL_HEIGHT="$POSTAL2_HEIGHT"
  export TSPGL_PRESENT="${POSTAL2_PRESENT:-letterbox}"
  rm -f /tmp/postal2.present.ready /tmp/tsp-glbridge.sock /tmp/tspgl-xport /tmp/postal2.frame

  (
    unset LD_PRELOAD
    unset LIBGL_ALWAYS_SOFTWARE GALLIUM_DRIVER MESA_LOADER_DRIVER_OVERRIDE
    unset LIBGL_DRIVERS_PATH __EGL_VENDOR_LIBRARY_FILENAMES EGL_PLATFORM
    export LD_LIBRARY_PATH="/usr/trimui/lib:/usr/lib:/lib:/mnt/SDCARD/spruce/flip/lib"
    export SDL_VIDEO_GL_DRIVER=libGLESv2.so
    export SDL_OPENGL_ES_DRIVER=1
    export XDG_RUNTIME_DIR=/tmp
    export TMPDIR=/tmp
    exec "$PRESENTER"
  ) &
  PRES=$!
  n=0
  while [ "$n" -lt 15 ]; do
    [ -f /tmp/postal2.present.ready ] && break
    kill -0 "$PRES" 2>/dev/null || break
    n=$((n + 1))
    sleep 1
  done
  echo "present_ready_wait=$n pid=$PRES"
  if [ ! -f /tmp/postal2.present.ready ]; then
    echo "Hybrid presenter failed to become ready"
    return 3
  fi

  WESTON_DIR=/tmp/postal2-weston
  WESTON_RUNTIME=weston_pkg_0.2
  if [ ! -f "$controlfolder/libs/${WESTON_RUNTIME}.squashfs" ]; then
    echo "Weston runtime is unavailable"
    return 4
  fi
  ${ESUDO:-} mkdir -p "$WESTON_DIR"
  if [ "${PM_CAN_MOUNT:-Y}" != "N" ]; then
    ${ESUDO:-} umount "$WESTON_DIR" >/dev/null 2>&1 || true
  fi
  ${ESUDO:-} mount "$controlfolder/libs/${WESTON_RUNTIME}.squashfs" "$WESTON_DIR"
  if [ ! -x "$WESTON_DIR/westonwrap32.sh" ]; then
    echo "westonwrap32.sh is missing from the runtime"
    return 5
  fi

  export PORT_32BIT=Y
  export SDL_VIDEODRIVER=x11
  export SDL_VIDEO_GL_DRIVER="$GAMEDIR/gl4es/libGL.so.1"
  export SDL_VIDEO_EGL_DRIVER="$GLBRIDGE/libEGL.so.1"
  export LIBGL_GLES="$GLBRIDGE/libGLESv2.so.2"
  export LIBGL_EGL="$GLBRIDGE/libEGL.so.1"
  export LIBGL_ES="${POSTAL2_LIBGL_ES:-2}"
  export LIBGL_GL="${POSTAL2_LIBGL_GL:-15}"
  export LIBGL_NOTEST="${POSTAL2_LIBGL_NOTEST:-1}"
  export LIBGL_SHRINK="${POSTAL2_LIBGL_SHRINK:-4}"
  export LIBGL_FBOFORCETEX="${POSTAL2_LIBGL_FBOFORCETEX:-1}"
  export LIBGL_FB="${POSTAL2_LIBGL_FB:-0}"
  export LIBGL_GETPROCADDRESS=1
  export LIBGL_DEPTH="${POSTAL2_LIBGL_DEPTH:-24}"
  export BOX86_DYNAREC="${POSTAL2_BOX86_DYNAREC:-1}"
  export BOX86_DYNAREC_STRONG="${POSTAL2_BOX86_DYNAREC_STRONG:-1}"
  export BOX86_DYNAREC_SAFE="${POSTAL2_BOX86_DYNAREC_SAFE:-1}"
  export BOX86_NOPROT="${POSTAL2_BOX86_NOPROT:-1}"
  export BOX86_ALLOWMISSINGLIBS="${POSTAL2_BOX86_ALLOWMISSINGLIBS:-1}"
  export BOX86_LD_LIBRARY_PATH="$GAMEDIR/box86/native:$GAMEDIR/box86/x86:$GAMEDIR/gamedata/System"
  export CRUSTY_BLOCK_INPUT=1
  GAME_LD="$GLBRIDGE:$SYS/lib/arm-linux-gnueabihf:$SYS/lib:$WESTON_DIR/lib_armhf:$WESTON_DIR/lib_armhf/graphics/gl4es_glxpass:$GAMEDIR/gl4es:/usr/lib/arm-linux-gnueabihf:/usr/lib32:$GAMEDIR/box86/native${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
  export LD_LIBRARY_PATH="$GAME_LD"

  cd "$GAMEDIR/gamedata/System" || return 6
  $GPTOKEYB2 "postal2-bin" -c "$GAMEDIR/postal2.ini" >>"$LOG" 2>&1 &
  INPUT_PID=$!
  pm_platform_helper "$GAMEDIR/box86/box86" >/dev/null &
  MESA_ROOT="/mnt/sdcard/mmcblk1p1/totono-mesa-llvmpipe"
  MESA_LIB="$MESA_ROOT/usr/lib/aarch64-linux-gnu"
  mkdir -p /tmp/mesa/lib
  ln -sfn "$MESA_LIB" /tmp/mesa/lib/aarch64-linux-gnu
  HOST_LD="/tmp/mesa/lib/aarch64-linux-gnu:$WESTON_DIR/lib_aarch64:$WESTON_DIR/lib_aarch64/libweston-14:/mnt/SDCARD/Persistent/portmaster/lib:/usr/lib:/lib"
  env -u LD_PRELOAD -u EGL_PLATFORM -u WP_32BIT \
    LIBGL_ALWAYS_SOFTWARE=1 \
    MESA_LOADER_DRIVER_OVERRIDE=llvmpipe \
    GALLIUM_DRIVER=llvmpipe \
    LIBGL_DRIVERS_PATH="/tmp/mesa/lib/aarch64-linux-gnu/dri" \
    __EGL_VENDOR_LIBRARY_FILENAMES="$MESA_ROOT/usr/share/glvnd/egl_vendor.d/50_mesa.json" \
    LD_LIBRARY_PATH="$HOST_LD" \
    "$WESTON_DIR/westonwrap.sh" headless gl kiosk llvmpipe \
    HOME="$HOME" \
    XDG_DATA_HOME="$XDG_DATA_HOME" \
    XDG_CONFIG_HOME="$XDG_CONFIG_HOME" \
    DISPLAY=:0 \
    SDL_VIDEODRIVER=x11 \
    SDL_VIDEO_GL_DRIVER="$SDL_VIDEO_GL_DRIVER" \
    SDL_VIDEO_EGL_DRIVER="$SDL_VIDEO_EGL_DRIVER" \
    LIBGL_GLES="$LIBGL_GLES" \
    LIBGL_EGL="$LIBGL_EGL" \
    LIBGL_ES="$LIBGL_ES" \
    LIBGL_GL="$LIBGL_GL" \
    LIBGL_NOTEST="$LIBGL_NOTEST" \
    LIBGL_SHRINK="$LIBGL_SHRINK" \
    LIBGL_FBOFORCETEX="$LIBGL_FBOFORCETEX" \
    LIBGL_FB="$LIBGL_FB" \
    LIBGL_GETPROCADDRESS="$LIBGL_GETPROCADDRESS" \
    LIBGL_DEPTH="$LIBGL_DEPTH" \
    env -u LD_PRELOAD "$LD" --library-path "$LD_LIBRARY_PATH" "$GAMEDIR/box86/box86" \
    ./postal2-bin -windowed
  result=$?
  echo "hybrid_game_exit=$result"
  "$WESTON_DIR/westonwrap.sh" cleanup >/dev/null 2>&1 || true
  if [ "${PM_CAN_MOUNT:-Y}" != "N" ]; then
    ${ESUDO:-} umount "$WESTON_DIR" >/dev/null 2>&1 || true
  fi
  return "$result"
}

run_xvfb_backend() {
  echo "backend=tsps-kms-presenter-xvfb"
  SYS="$GAMEDIR/armhf"
  LD="$SYS/lib/ld-linux-armhf.so.3"
  GLBRIDGE="$GAMEDIR/glbridge"
  PRESENTER="$GAMEDIR/postal2_present"
  XVFB_ROOT="${POSTAL2_XVFB_ROOT:-$GAMEDIR/xvfb}"
  XSERVER_KIND="${POSTAL2_XSERVER:-xvfb}"
  if [ "$XSERVER_KIND" = "xorg" ]; then
    XSERVER="$XVFB_ROOT/usr/lib/xorg/Xorg"
    if [ -n "${POSTAL2_XORG_CONFIG:-}" ]; then
      XSERVER_CONFIG="$POSTAL2_XORG_CONFIG"
    else
      XSERVER_CONFIG=/tmp/postal2-xorg.conf
      XORG_CONFIG_TMP=1
    fi
  else
    XSERVER="$XVFB_ROOT/usr/bin/Xvfb"
    XSERVER_CONFIG=
  fi
  GAMEBIN="$GAMEDIR/gamedata/System/postal2-bin"
  if [ ! -x "$PRESENTER" ] || [ ! -f "$LD" ] || [ ! -f "$GLBRIDGE/libEGL.so.1" ] || [ ! -x "$GAMEDIR/box86/box86" ] || [ ! -f "$GAMEDIR/gl4es/libGL.so.1" ]; then
    echo "X server backend game files are incomplete"
    return 2
  fi
  if [ ! -x "$XSERVER" ]; then
    echo "X server runtime is unavailable: $XSERVER"
    return 3
  fi
  if [ "$XSERVER_KIND" = "xorg" ] && [ "$XORG_CONFIG_TMP" -eq 0 ] && [ ! -f "$XSERVER_CONFIG" ]; then
    echo "Xorg config is unavailable: $XSERVER_CONFIG"
    return 3
  fi

  export POSTAL2_WIDTH="${POSTAL2_WIDTH:-640}"
  export POSTAL2_HEIGHT="${POSTAL2_HEIGHT:-480}"
  export TSPGL_WIDTH="$POSTAL2_WIDTH"
  export TSPGL_HEIGHT="$POSTAL2_HEIGHT"
  export TSPGL_PRESENT="${POSTAL2_PRESENT:-letterbox}"
  rm -f /tmp/postal2.present.ready /tmp/tsp-glbridge.sock /tmp/tspgl-xport /tmp/postal2.frame

  (
    unset LD_PRELOAD
    unset LIBGL_ALWAYS_SOFTWARE GALLIUM_DRIVER MESA_LOADER_DRIVER_OVERRIDE
    unset LIBGL_DRIVERS_PATH __EGL_VENDOR_LIBRARY_FILENAMES EGL_PLATFORM
    export LD_LIBRARY_PATH="/usr/trimui/lib:/usr/lib:/lib:/mnt/SDCARD/spruce/flip/lib"
    export SDL_VIDEODRIVER="${POSTAL2_PRESENTER_SDL_VIDEODRIVER:-kmsdrm}"
    export SDL_VIDEO_GL_DRIVER=libGLESv2.so
    export SDL_OPENGL_ES_DRIVER=1
    export XDG_RUNTIME_DIR=/tmp
    export TMPDIR=/tmp
    exec "$PRESENTER"
  ) >"$LOG.presenter" 2>&1 &
  PRES=$!
  n=0
  while [ "$n" -lt 20 ]; do
    [ -f /tmp/postal2.present.ready ] && break
    kill -0 "$PRES" 2>/dev/null || break
    n=$((n + 1))
    sleep 1
  done
  echo "present_ready_wait=$n pid=$PRES"
  if [ ! -f /tmp/postal2.present.ready ]; then
    echo "Xvfb backend presenter failed to become ready"
    return 4
  fi

  if [ "$XSERVER_KIND" = "xorg" ]; then
    if ! start_input_helper; then
      echo "gptokeyb2 failed to create a virtual input device"
      return 5
    fi
    if [ "$XORG_CONFIG_TMP" -ne 0 ]; then
      if [ ! -f "$XVFB_ROOT/xorg-dummy.conf" ]; then
        echo "Xorg config template is unavailable: $XVFB_ROOT/xorg-dummy.conf"
        return 5
      fi
      sed \
        -e "s|/tmp/xvfb-postal2/usr/lib/xorg/modules|$XVFB_ROOT/usr/lib/xorg/modules|g" \
        -e "s|__POSTAL2_INPUT_EVENT__|$POSTAL2_INPUT_EVENT|g" \
        "$XVFB_ROOT/xorg-dummy.conf" >"$XSERVER_CONFIG"
    fi
  fi

  XVFB_LD="$XVFB_ROOT/usr/lib/aarch64-linux-gnu:$XVFB_ROOT/lib/aarch64-linux-gnu:/usr/lib:/lib:/mnt/SDCARD/spruce/flip/lib"
  XVFB_SCREEN="${POSTAL2_XVFB_WIDTH:-1280}x${POSTAL2_XVFB_HEIGHT:-720}x${POSTAL2_XVFB_DEPTH:-24}"
  XSERVER_LOG="$LOG.xserver"
  if [ "$XSERVER_KIND" = "xorg" ]; then
    MESA_ROOT="${POSTAL2_MESA_ROOT:-/mnt/sdcard/mmcblk1p1/totono-mesa-llvmpipe}"
    MESA_LIB="$MESA_ROOT/usr/lib/aarch64-linux-gnu"
    XSERVER_LD="$MESA_LIB:$MESA_LIB/dri:$XVFB_LD"
    (
      export LD_LIBRARY_PATH="$XSERVER_LD"
      export XORG_MODULE_PATH="$XVFB_ROOT/usr/lib/xorg/modules"
      export LIBGL_ALWAYS_SOFTWARE=1
      export MESA_LOADER_DRIVER_OVERRIDE=llvmpipe
      export GALLIUM_DRIVER=llvmpipe
      export LIBGL_DRIVERS_PATH="$MESA_LIB/dri"
      export __GLX_VENDOR_LIBRARY_NAME=mesa
      exec "$XSERVER" ":$XVFB_DISPLAY" -config "$XSERVER_CONFIG" -noreset -nolisten tcp +extension GLX +iglx -logfile "$XSERVER_LOG"
    ) >"$LOG.xorg.stdout" 2>&1 &
  else
    (
      export LD_LIBRARY_PATH="$XVFB_LD"
      exec "$XSERVER" ":$XVFB_DISPLAY" -screen 0 "$XVFB_SCREEN" +extension GLX +iglx -ac -nolisten tcp -noreset
    ) >"$LOG.xvfb" 2>&1 &
  fi
  XVFB_PID=$!
  n=0
  while [ "$n" -lt 15 ]; do
    if [ -e "/tmp/.X11-unix/X${XVFB_DISPLAY}" ] && kill -0 "$XVFB_PID" 2>/dev/null; then
      break
    fi
    kill -0 "$XVFB_PID" 2>/dev/null || break
    n=$((n + 1))
    sleep 1
  done
  echo "xserver_kind=$XSERVER_KIND xserver_wait=$n pid=$XVFB_PID display=:$XVFB_DISPLAY"
  if [ ! -e "/tmp/.X11-unix/X${XVFB_DISPLAY}" ] || ! kill -0 "$XVFB_PID" 2>/dev/null; then
    echo "X server failed to become ready"
    return 5
  fi

  export PORT_32BIT=Y
  export DISPLAY=":$XVFB_DISPLAY"
  export SDL_VIDEODRIVER=x11
  export SDL_VIDEO_GL_DRIVER="$GAMEDIR/gl4es/libGL.so.1"
  export SDL_VIDEO_EGL_DRIVER="$GLBRIDGE/libEGL.so.1"
  export LIBGL_GLES="$GLBRIDGE/libGLESv2.so.2"
  export LIBGL_EGL="$GLBRIDGE/libEGL.so.1"
  export LIBGL_ES="${POSTAL2_LIBGL_ES:-2}"
  export LIBGL_GL="${POSTAL2_LIBGL_GL:-15}"
  export LIBGL_NOTEST="${POSTAL2_LIBGL_NOTEST:-1}"
  export LIBGL_SHRINK="${POSTAL2_LIBGL_SHRINK:-4}"
  export LIBGL_FBOFORCETEX="${POSTAL2_LIBGL_FBOFORCETEX:-1}"
  if [ "$XSERVER_KIND" = "xorg" ]; then
    export LIBGL_FB="${POSTAL2_LIBGL_FB:-0}"
  else
    export LIBGL_FB="${POSTAL2_LIBGL_FB:-1}"
  fi
  export LIBGL_GETPROCADDRESS=1
  export LIBGL_DEPTH="${POSTAL2_LIBGL_DEPTH:-24}"
  export BOX86_DYNAREC="${POSTAL2_BOX86_DYNAREC:-1}"
  export BOX86_DYNAREC_STRONG="${POSTAL2_BOX86_DYNAREC_STRONG:-1}"
  export BOX86_DYNAREC_SAFE="${POSTAL2_BOX86_DYNAREC_SAFE:-1}"
  export BOX86_NOPROT="${POSTAL2_BOX86_NOPROT:-1}"
  export BOX86_ALLOWMISSINGLIBS="${POSTAL2_BOX86_ALLOWMISSINGLIBS:-1}"
  export BOX86_LD_LIBRARY_PATH="$GAMEDIR/box86/native:$GAMEDIR/box86/x86:$GAMEDIR/gamedata/System"
  export CRUSTY_BLOCK_INPUT=1
  GAME_LD="$GLBRIDGE:$SYS/lib/arm-linux-gnueabihf:$SYS/lib:$GAMEDIR/gl4es:$GAMEDIR/box86/native:$GAMEDIR/gamedata/System:/usr/lib/arm-linux-gnueabihf:/usr/lib32${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
  export LD_LIBRARY_PATH="$GAME_LD"

  cd "$GAMEDIR/gamedata/System" || return 6
  if [ "$INPUT_PID" -eq 0 ]; then
    start_input_helper || return 6
  fi
  pm_platform_helper "$GAMEDIR/box86/box86" >/dev/null &
  # Guest i386 SDL classifier: next physical A/B changes only relative deltas.
  # POSTAL2_MOUSE_COORD_MODE=passive is the no-op comparator.
  if [ "${POSTAL2_INPUT_TRACE:-1}" = 1 ] && [ -f "$GAMEDIR/postal2_sdl_input_trace.so" ]; then
    export BOX86_LD_PRELOAD="$GAMEDIR/postal2_sdl_input_trace.so"
    export POSTAL2_MOUSE_COORD_MODE="${POSTAL2_MOUSE_COORD_MODE:-relative}"
    export POSTAL2_FORCE_CURSOR="${POSTAL2_FORCE_CURSOR:-1}"
    echo "input_trace=$BOX86_LD_PRELOAD coord_mode=$POSTAL2_MOUSE_COORD_MODE force_cursor=$POSTAL2_FORCE_CURSOR"
  else
    unset BOX86_LD_PRELOAD
    echo "input_trace=off"
  fi
  env -u LD_PRELOAD -u EGL_PLATFORM "$LD" --library-path "$GAME_LD" "$GAMEDIR/box86/box86" ./postal2-bin -windowed
  result=$?
  echo "xvfb_game_exit=$result"
  return "$result"
}

BACKEND="${POSTAL2_BACKEND:-xorg}"
if [ "$BACKEND" = "xvfb" ]; then
  run_xvfb_backend
  result=$?
elif [ "$BACKEND" = "xorg" ]; then
  POSTAL2_XSERVER=xorg run_xvfb_backend
  result=$?
elif [ "$BACKEND" = "hybrid" ]; then
  run_hybrid_backend
  result=$?
elif [ "$BRIDGE" -ne 1 ]; then
  echo "backend=legacy-weston-x11"
  WESTON_DIR=/tmp/postal2-weston
  WESTON_RUNTIME=weston_pkg_0.2
  if [ ! -f "$controlfolder/libs/${WESTON_RUNTIME}.squashfs" ]; then
    if [ ! -f "$controlfolder/harbourmaster" ]; then
      echo "Weston runtime and HarbourMaster are unavailable"
      result=2
    else
      ${ESUDO:-} "$controlfolder/harbourmaster" --quiet --no-check runtime_check "${WESTON_RUNTIME}.squashfs"
    fi
  fi
  if [ "${result:-0}" -eq 0 ] && [ ! -f "$controlfolder/libs/${WESTON_RUNTIME}.squashfs" ]; then
    echo "Weston runtime is unavailable after runtime_check"
    result=2
  fi
  if [ "${result:-0}" -eq 0 ]; then
    ${ESUDO:-} mkdir -p "$WESTON_DIR"
    if [ "${PM_CAN_MOUNT:-Y}" != "N" ]; then
      ${ESUDO:-} umount "$WESTON_DIR" >/dev/null 2>&1 || true
    fi
    ${ESUDO:-} mount "$controlfolder/libs/${WESTON_RUNTIME}.squashfs" "$WESTON_DIR"
    if [ ! -x "$WESTON_DIR/westonwrap32.sh" ]; then
      echo "westonwrap32.sh is missing from the runtime"
      result=2
    else
      export PORT_32BIT=Y
      export LIBGL_GL=15
      export LIBGL_FB=0
      export LIBGL_GETPROCADDRESS=1
      export LIBGL_DEPTH="${POSTAL2_LIBGL_DEPTH:-24}"
      export BOX86_DYNAREC="${POSTAL2_BOX86_DYNAREC:-1}"
      export BOX86_DYNAREC_STRONG="${POSTAL2_BOX86_DYNAREC_STRONG:-1}"
      export BOX86_DYNAREC_SAFE="${POSTAL2_BOX86_DYNAREC_SAFE:-1}"
      export BOX86_NOPROT="${POSTAL2_BOX86_NOPROT:-1}"
      export BOX86_ALLOWMISSINGLIBS="${POSTAL2_BOX86_ALLOWMISSINGLIBS:-1}"
      export CRUSTY_BLOCK_INPUT=1
      unset SDL_VIDEODRIVER SDL_VIDEO_EGL_DRIVER
      LD="$GAMEDIR/armhf/lib/ld-linux-armhf.so.3"
      export LD_LIBRARY_PATH="$GAMEDIR/armhf/lib/arm-linux-gnueabihf:$GAMEDIR/armhf/lib:$WESTON_DIR/lib_armhf/graphics/gl4es_glxpass:$WESTON_DIR/lib_armhf/graphics/crusty_glx:$GAMEDIR/gl4es:$WESTON_DIR/lib_armhf:/usr/lib/arm-linux-gnueabihf:/usr/lib32:$GAMEDIR/box86/native${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
      export BOX86_LD_LIBRARY_PATH="$GAMEDIR/box86/x86:$GAMEDIR/box86/native:$GAMEDIR/gamedata/System"
      cd "$GAMEDIR/gamedata/System" || exit 4
      $GPTOKEYB2 "postal2-bin" -c "$GAMEDIR/postal2.ini" >>"$LOG" 2>&1 &
      INPUT_PID=$!
      pm_platform_helper "$GAMEDIR/box86/box86" >/dev/null &
      "$WESTON_DIR/westonwrap32.sh" headless noop kiosk crusty_glx_gl4es \
        HOME="$HOME" \
        XDG_DATA_HOME="$XDG_DATA_HOME" \
        XDG_CONFIG_HOME="$XDG_CONFIG_HOME" \
        "$LD" --library-path "$LD_LIBRARY_PATH" "$GAMEDIR/box86/box86" \
        ./postal2-bin -windowed
      result=$?
      "$WESTON_DIR/westonwrap.sh" cleanup >/dev/null 2>&1 || true
    fi
    if [ "${PM_CAN_MOUNT:-Y}" != "N" ]; then
      ${ESUDO:-} umount "$WESTON_DIR" >/dev/null 2>&1 || true
    fi
  fi
else
  echo "backend=tsps-32to64-gles-bridge"
  SYS="$GAMEDIR/armhf"
  LD="$SYS/lib/ld-linux-armhf.so.3"
  GLBRIDGE="$GAMEDIR/glbridge"
  PRESENTER="$GAMEDIR/postal2_present"
  GAMEBIN="$GAMEDIR/gamedata/System/postal2-bin"
  if [ ! -x "$PRESENTER" ] || [ ! -f "$LD" ] || [ ! -f "$GLBRIDGE/libEGL.so.1" ] || [ ! -x "$GAMEDIR/box86/box86" ] || [ ! -f "$GAMEDIR/gl4es/libGL.so.1" ]; then
    echo "TSPS bridge files are incomplete"
    result=2
  else
    export POSTAL2_WIDTH="${POSTAL2_WIDTH:-640}"
    export POSTAL2_HEIGHT="${POSTAL2_HEIGHT:-480}"
    export SDL_OFFSCREEN_WIDTH="${POSTAL2_SDL_OFFSCREEN_WIDTH:-$POSTAL2_WIDTH}"
    export SDL_OFFSCREEN_HEIGHT="${POSTAL2_SDL_OFFSCREEN_HEIGHT:-$POSTAL2_HEIGHT}"
    export TSPGL_WIDTH="$POSTAL2_WIDTH"
    export TSPGL_HEIGHT="$POSTAL2_HEIGHT"
    export TSPGL_PRESENT="${POSTAL2_PRESENT:-letterbox}"
    export TSPGL_ASPECT_VIEWPORT_720="${POSTAL2_ASPECT_VIEWPORT_720:-0}"
    rm -f /tmp/postal2.present.ready /tmp/tsp-glbridge.sock /tmp/tspgl-xport /tmp/postal2.frame

    (
      unset LD_PRELOAD
      unset LIBGL_ALWAYS_SOFTWARE GALLIUM_DRIVER MESA_LOADER_DRIVER_OVERRIDE
      unset LIBGL_DRIVERS_PATH __EGL_VENDOR_LIBRARY_FILENAMES EGL_PLATFORM
      export LD_LIBRARY_PATH="/usr/trimui/lib:/usr/lib:/lib:/mnt/SDCARD/spruce/flip/lib"
      export SDL_VIDEO_GL_DRIVER=libGLESv2.so
      export SDL_OPENGL_ES_DRIVER=1
      export XDG_RUNTIME_DIR=/tmp
      export TMPDIR=/tmp
      exec "$PRESENTER"
    ) &
    PRES=$!
    n=0
    while [ "$n" -lt 15 ]; do
      [ -f /tmp/postal2.present.ready ] && break
      kill -0 "$PRES" 2>/dev/null || break
      n=$((n + 1))
      sleep 1
    done
    echo "present_ready_wait=$n pid=$PRES"
    if [ ! -f /tmp/postal2.present.ready ]; then
      echo "TSPS presenter failed to become ready"
      result=3
    else
      export PORT_32BIT=Y
      export XDG_RUNTIME_DIR=/tmp
      export TMPDIR=/tmp
      export SDL_VIDEODRIVER="${POSTAL2_SDL_VIDEODRIVER:-offscreen}"
      export LD_LIBRARY_PATH="$GLBRIDGE:$GAMEDIR/box86/native:$SYS/lib/arm-linux-gnueabihf:$SYS/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
      export BOX86_LD_LIBRARY_PATH="$GAMEDIR/box86/native:$GAMEDIR/box86/x86:$GAMEDIR/gamedata/System"
      export BOX86_PREFER_WRAPPED="${POSTAL2_BOX86_PREFER_WRAPPED:-1}"
      export BOX86_X11THREADS=1
      export BOX86_DYNAREC="${POSTAL2_BOX86_DYNAREC:-1}"
      export BOX86_DYNAREC_BIGBLOCK="${POSTAL2_BOX86_DYNAREC_BIGBLOCK:-0}"
      export BOX86_DYNAREC_STRONG="${POSTAL2_BOX86_DYNAREC_STRONG:-1}"
      export BOX86_DYNAREC_FASTNAN="${POSTAL2_BOX86_DYNAREC_FASTNAN:-0}"
      export BOX86_DYNAREC_SAFE="${POSTAL2_BOX86_DYNAREC_SAFE:-1}"
      export BOX86_NOPROT="${POSTAL2_BOX86_NOPROT:-1}"
      export BOX86_EXPORT_ALL="${POSTAL2_BOX86_EXPORT_ALL:-1}"
      export BOX86_ALLOWMISSINGLIBS="${POSTAL2_BOX86_ALLOWMISSINGLIBS:-1}"
      export BOX86_SHOWSEGV="${POSTAL2_BOX86_SHOWSEGV:-1}"
      export BOX86_SHOWBT="${POSTAL2_BOX86_SHOWBT:-1}"
      export BOX86_LOG="${POSTAL2_BOX86_LOG:-0}"
      export BOX86_DLSYM_ERROR="${POSTAL2_BOX86_DLSYM_ERROR:-1}"
      export BOX86_DYNAREC_LOG="${POSTAL2_BOX86_DYNAREC_LOG:-0}"
      export SDL_VIDEO_GL_DRIVER="$GAMEDIR/gl4es/libGL.so.1"
      export SDL_VIDEO_EGL_DRIVER="$GLBRIDGE/libEGL.so.1"
      export LIBGL_GLES="$GLBRIDGE/libGLESv2.so.2"
      export LIBGL_EGL="$GLBRIDGE/libEGL.so.1"
      export LIBGL_ES="${POSTAL2_LIBGL_ES:-2}"
      export LIBGL_GL="${POSTAL2_LIBGL_GL:-15}"
      export LIBGL_NOTEST="${POSTAL2_LIBGL_NOTEST:-1}"
      export LIBGL_SHRINK="${POSTAL2_LIBGL_SHRINK:-4}"
      export LIBGL_FBOFORCETEX="${POSTAL2_LIBGL_FBOFORCETEX:-1}"
      export LIBGL_FB="${POSTAL2_LIBGL_FB:-0}"
      export LIBGL_GETPROCADDRESS=1
      export LIBGL_DEPTH="${POSTAL2_LIBGL_DEPTH:-24}"
      export LIBGL_NOEXTENSION="${POSTAL2_LIBGL_NOEXTENSION:-GL_ATI_vertex_array_object,GL_NV_vertex_array_range,GL_APPLE_vertex_array_range,GL_EXT_vertex_array_range,GL_EXT_compiled_vertex_array,GL_NV_fence,GL_ARB_occlusion_query}"
      export CRUSTY_BLOCK_INPUT=1
      export SDL_AUDIODRIVER="${POSTAL2_SDL_AUDIODRIVER:-alsa}"
      export TEXTINPUTINTERACTIVE=Y

      echo "bridge_presenter=$PRESENTER"
      echo "bridge_loader=$LD"
      echo "bridge_dimensions=${TSPGL_WIDTH}x${TSPGL_HEIGHT} present=${TSPGL_PRESENT}"
      echo "box86_dynarec=$BOX86_DYNAREC bigblock=$BOX86_DYNAREC_BIGBLOCK log=$BOX86_LOG"
      echo "gl4es_es=$LIBGL_ES gl=$LIBGL_GL notest=$LIBGL_NOTEST shrink=$LIBGL_SHRINK fbotex=$LIBGL_FBOFORCETEX fb=$LIBGL_FB"

      if [ ! -x "$controlfolder/gptokeyb2" ]; then
        echo "gptokeyb2 is not available"
        result=4
      else
        cd "$GAMEDIR/gamedata/System" || exit 4
        $GPTOKEYB2 "postal2-bin" -c "$GAMEDIR/postal2.ini" >>"$LOG" 2>&1 &
        INPUT_PID=$!
        sleep 1
        pm_platform_helper "$GAMEDIR/box86/box86"
        env -u LD_PRELOAD "$LD" --library-path "$LD_LIBRARY_PATH" "$GAMEDIR/box86/box86" ./postal2-bin -windowed
        result=$?
      fi
    fi
  fi
fi

echo "exit_code=$result"
echo "===== postal2 end ====="
sync
cleanup
trap - EXIT INT TERM
pm_finish
exit "$result"
