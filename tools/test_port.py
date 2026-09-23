#!/usr/bin/env python3
import json
import os
import pathlib
import subprocess
import sys
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

    presenter = GAME / "postal2_present"
    egl = GAME / "glbridge" / "libEGL.so.1"
    box86 = GAME / "box86" / "box86"
    gl4es = GAME / "gl4es" / "libGL.so.1"
    for path in (presenter, egl, box86, gl4es):
        assert path.is_file(), f"missing runtime artifact: {path}"
    assert "ARM aarch64" in run("file", str(presenter))
    assert "ARM" in run("file", str(egl))
    assert "ARM" in run("file", str(box86))
    assert "ARM" in run("file", str(gl4es))
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
