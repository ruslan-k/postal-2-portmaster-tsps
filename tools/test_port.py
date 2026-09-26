#!/usr/bin/env python3
import json
import os
import pathlib
import subprocess
import sys
import tempfile
import zipfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
PORT = ROOT / "portmaster"
GAME = PORT / "postal2"


def run(*args):
    return subprocess.run(args, check=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT).stdout.strip()


def main():
    meta = json.loads((GAME / "port.json").read_text())
    assert meta["version"] == 4
    assert meta["name"] == "postal2.zip"
    assert meta["items"] == ["Postal 2.sh", "postal2"]
    assert meta["attr"]["availability"] == "paid"
    assert meta["attr"]["arch"] == ["aarch64", "armhf"]
    assert (PORT / "Postal 2.sh").is_file()
    assert (GAME / "setup.sh").is_file()
    assert (PORT / "postal2" / "gameinfo.xml").is_file()
    if not (os.environ.get("ALLOW_LOCAL_PAYLOAD") == "1"):
        assert not (GAME / "gamedata" / "System" / "postal2-bin").exists(), "commercial payload must not be committed"

    subprocess.run(["bash", "-n", str(PORT / "Postal 2.sh")], check=True)
    subprocess.run(["bash", "-n", str(GAME / "setup.sh")], check=True)
    subprocess.run(["bash", "-n", str(ROOT / "tools" / "build_tsps_bridge.sh")], check=True)
    subprocess.run(["bash", "-n", str(ROOT / "tools" / "build_port.sh")], check=True)
    with tempfile.TemporaryDirectory(prefix="postal2-trace-test-") as tmp:
        test_binary = pathlib.Path(tmp) / "test_input_trace"
        subprocess.run(
            ["gcc", "-rdynamic", "-O2", "-Wall", "-Wextra", "-Werror",
             str(ROOT / "tools" / "test_input_trace.c"), "-ldl", "-o", str(test_binary)],
            check=True,
        )
        subprocess.run([str(test_binary)], check=True)

    presenter = GAME / "postal2_present"
    egl = GAME / "glbridge" / "libEGL.so.1"
    box86 = GAME / "box86" / "box86"
    gl4es = GAME / "gl4es" / "libGL.so.1"
    xorg = GAME / "xvfb" / "usr" / "lib" / "xorg" / "Xorg"
    xorg_conf = GAME / "xvfb" / "xorg-dummy.conf"
    xorg_config = xorg_conf.read_text()
    assert 'Modes "640x480"' in xorg_config
    assert 'InputDevice "Postal2KeyboardMouse"' in xorg_config
    assert '"__POSTAL2_INPUT_EVENT__"' in xorg_config
    trace = GAME / "postal2_sdl_input_trace.so"
    assert "ELF 32-bit" in run("file", str(trace))
    assert "Intel 80386" in run("readelf", "-h", str(trace))
    trace_source = (ROOT / "src" / "postal2_sdl_input_trace.c").read_text()
    assert '#include "postal2_frame.h"' in trace_source
    assert 'open(path, O_RDWR)' in trace_source
    assert 'MAP_SHARED' in trace_source
    assert 'hdr[POSTAL2_HDR_CURSOR_X]' in trace_source
    assert 'hdr[POSTAL2_HDR_CURSOR_Y]' in trace_source
    assert 'hdr[POSTAL2_HDR_CURSOR_ON]' in trace_source
    assert 'dlsym(RTLD_DEFAULT, "postal2_fb_set_cursor")' not in trace_source
    assert 'dlopen("libEGL.so.1", RTLD_LAZY | RTLD_NOLOAD)' not in trace_source
    assert "P2-MAP source=relative-accum" in trace_source
    assert "raw=%u,%u rel=%d,%d marker=%d,%d" in trace_source
    assert "sample_trace_record(&records, &record_logs, 128, 32, 512)" in trace_source
    assert "sample_trace_record(&diag_cursor_records, &diag_cursor_logs, 128, 32, 512)" in trace_source
    assert "update_cursor_marker" in trace_source
    assert "cursor_event_position" in trace_source
    presenter_source = (ROOT / "src" / "glbridge" / "server.c").read_text()
    assert "P2-CURSOR guest=" in presenter_source
    assert "int win_w = 640;" in presenter_source
    assert "int win_h = 480;" in presenter_source
    launcher = (PORT / "Postal 2.sh").read_text()
    assert "BOX86_LD_PRELOAD" in launcher
    viewport_rules = (
        r's/^([[:space:]]*[[:alnum:]_]*ViewportX[[:space:]]*=[[:space:]]*)[^[:space:]]*/\1640/',
        r's/^([[:space:]]*[[:alnum:]_]*ViewportY[[:space:]]*=[[:space:]]*)[^[:space:]]*/\1480/',
    )
    assert all(rule in launcher for rule in viewport_rules)
    with tempfile.TemporaryDirectory(prefix="postal2-resolution-test-") as tmp:
        ini = pathlib.Path(tmp) / "Postal2.ini"
        ini.write_text("WindowedViewportX=1280\nWindowedViewportY=720\nFullscreenViewportX=1920\nFullscreenViewportY=1080\nMenuViewportX=1280\nMenuViewportY=720\n")
        for rule in viewport_rules:
            subprocess.run(["sed", "-i", "-E", rule, str(ini)], check=True)
        configured = ini.read_text().splitlines()
        assert configured == [
            "WindowedViewportX=640", "WindowedViewportY=480",
            "FullscreenViewportX=640", "FullscreenViewportY=480",
            "MenuViewportX=640", "MenuViewportY=480",
        ]
    assert launcher.count("export POSTAL2_WIDTH=640") == 3
    assert launcher.count("export POSTAL2_HEIGHT=480") == 3
    assert 'export SDL_OFFSCREEN_WIDTH="$POSTAL2_WIDTH"' in launcher
    assert 'export SDL_OFFSCREEN_HEIGHT="$POSTAL2_HEIGHT"' in launcher
    assert 'POSTAL2_DIAG_UWINDOW_CURSOR="${POSTAL2_DIAG_UWINDOW_CURSOR:-0}"' in launcher
    assert 'POSTAL2_FORCE_CURSOR="${POSTAL2_FORCE_CURSOR:-1}"' in launcher
    assert "POSTAL2_XVFB_WIDTH:-1280" in launcher
    assert "POSTAL2_XVFB_HEIGHT:-720" in launcher
    assert "uwindow_cursor_diag=$POSTAL2_DIAG_UWINDOW_CURSOR" in launcher
    assert 'POSTAL2_FORCE_UNGRAB="${POSTAL2_FORCE_UNGRAB:-0}"' in launcher
    assert "force_ungrab=$POSTAL2_FORCE_UNGRAB" in launcher
    for path in (presenter, egl, box86, gl4es, xorg, xorg_conf):
        assert path.is_file(), f"missing runtime artifact: {path}"
    assert "ARM aarch64" in run("file", str(presenter))
    assert "ARM" in run("file", str(egl))
    assert "ARM" in run("file", str(box86))
    assert "ARM" in run("file", str(gl4es))
    assert "ARM aarch64" in run("file", str(xorg))
    assert not any(path.is_symlink() for path in (GAME / "xvfb").rglob("*")), "xvfb runtime must be symlink-free"
    assert not (GAME / "box86" / "native" / "libSDL2-2.0.so.0").exists(), "core SDL2 must come from the CFW"

    dist = PORT / "dist" / "postal2.zip"
    if dist.exists():
        with zipfile.ZipFile(dist) as zf:
            names = zf.namelist()
            assert "Postal 2.sh" in names
            assert "postal2/setup.sh" in names
            assert not any("postal2/gamedata/System/" in n for n in names)

    print("PASS: metadata, shell syntax, runtime ELF types, and payload exclusion")


if __name__ == "__main__":
    main()
