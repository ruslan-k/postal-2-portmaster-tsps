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
    assert 'dlsym(RTLD_DEFAULT, "postal2_fb_set_cursor")' in trace_source
    assert "P2-MAP axis_projection=" in trace_source
    assert "static void accumulate_mouse_delta" in trace_source
    launcher = (PORT / "Postal 2.sh").read_text()
    assert "BOX86_LD_PRELOAD" in launcher
    assert 'POSTAL2_DIAG_UWINDOW_CURSOR="${POSTAL2_DIAG_UWINDOW_CURSOR:-1}"' in launcher
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
