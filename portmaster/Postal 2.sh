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
case "${POSTAL2_TSPS_BRIDGE:-auto}" in
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
cleanup() {
  if [ "$INPUT_PID" -ne 0 ]; then
    kill "$INPUT_PID" 2>/dev/null || true
    wait "$INPUT_PID" 2>/dev/null || true
    INPUT_PID=0
  fi
  if [ "$PRES" -ne 0 ]; then
    kill "$PRES" 2>/dev/null || true
    wait "$PRES" 2>/dev/null || true
    PRES=0
  fi
  rm -f /tmp/postal2.present.ready /tmp/tsp-glbridge.sock /tmp/tspgl-xport /tmp/postal2.frame
}
trap cleanup EXIT INT TERM

if [ "$BRIDGE" -ne 1 ]; then
  echo "This build targets TSPS and requires the TSPS bridge"
  result=2
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
      export LIBGL_DEPTH=24
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
