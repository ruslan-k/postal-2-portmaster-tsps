## Notes

This TSPS port follows the bridge layout from [guacamelee-portmaster-tsps](https://github.com/ruslan-k/guacamelee-portmaster-tsps). Thanks to the original bridge author and maintainer, star0081, for the reference implementation:

- Box86 runs the Linux x86 game binary.
- gl4es translates the game's fixed-function OpenGL calls to GLES.
- `glbridge` proxies the 32-bit GLES calls to a native aarch64 presenter.
- `gptokeyb2` exposes the handheld controls as the keyboard and mouse controls expected by Postal 2.

## Install

1. Obtain a legally owned Linux Postal 2 1409 installation.
2. Copy its archive, named exactly `Postal 2.zip`, into the installed `postal2` directory. The archive must contain `postal2/gamedata/System/postal2-bin`.
3. Install the generated `postal2.zip` with PortMaster and launch `Postal 2` once. The first run extracts the game data and creates the isolated configuration directory.

The commercial game payload is not committed to this repository and is excluded from the generated PortMaster zip.

## Controls

- Left stick: move with WASD.
- Right stick: mouse look.
- R2: primary mouse button / fire.
- L2: secondary mouse button.
- A: interact or confirm.
- B: jump or cancel in menus.
- X: reload.
- Y: crouch.
- L1/R1: kick and use the alternate action mapped by the game.
- Start: pause or escape.
- D-pad up/down: previous/next weapon in-game; menu directions in menus.

The mapping is in `portmaster/postal2/postal2.ini` and can be changed without modifying the game payload.

## Build

The bridge build uses Zig 0.13.0 or a compatible Zig release:

```sh
ZIG=/path/to/zig tools/build_tsps_bridge.sh
```

The gl4es artifact in the tree is the TSPS-tested build inherited from the reference port. To rebuild it from the pinned gl4es source, use:

```sh
tools/build_gl4es_tsps.sh
```

To stage local data for a device test without adding it to git:

```sh
tools/stage_local_payload.sh "$HOME/Downloads/Postal 2.zip"
```

Then build the distributable package and run the checks:

```sh
tools/build_port.sh
python3 tools/test_port.py
```

## Repository policy

Changes are tracked directly on `main` as requested. The repository contains the launcher, bridge source, runtime components and build/test tooling, but not the paid game files.
