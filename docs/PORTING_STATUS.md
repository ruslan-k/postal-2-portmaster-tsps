# POSTAL 2 TSPS port status

## Verified locally

- Source archive: `~/Downloads/Postal 2.zip`.
- The archive contains the Linux x86 `postal2-bin`, SDL 1.2, OpenAL and the game data under `postal2/gamedata`.
- The TSPS bridge source is adapted from the Guacamelee reference and rebuilt with Zig for ARMHF guest and aarch64 presenter targets.
- The package excludes the commercial game payload; local payload staging is git-ignored.

## Device verification

- Target: TSPS Longan at `192.168.50.135`.
- Device architecture: aarch64.
- The earlier menu-launch candidate selected `backend=tsps-bridge-weston-x11`, but Weston failed before game startup with `EGL does not support surfaceless platform` and `hybrid_game_exit=143`; this was the cause of the black screen.
- An explicit X11/GLX validation run reached the real Postal 2 main menu: `POSTAL 2 Share The Pain`, `New Game`, `Load Game`, `Multiplayer`, `Options`, and `Exit`.
- A symlink-free Xorg runtime from that working run is now packaged under `postal2/xvfb`; the launcher default is `POSTAL2_BACKEND=xorg`, while hybrid remains an explicit override.
- The new launcher and all 104 Xorg runtime files were deployed with SHA-256 verification and a launcher backup. Physical menu re-test is pending.
- The user previously confirmed that picture and sound are present on the working X11/GLX variant.

## Facts versus hypotheses

- Fact: the archive has the expected Linux x86 game binary and bundled 32-bit libraries.
- Fact: the generated bridge binaries have ARMHF and aarch64 ELF types respectively.
- Fact: the X11/GLX-capable GL4ES path reaches the Postal 2 menu; the earlier GLX stub no longer blocks startup.
- Fact: the previous menu black screen was caused by the default Weston headless EGL initialization failure, not by a proven black game framebuffer.
- Fact: the runtime log still reports `libopenal.so.1: wrong ELF class: ELFCLASS64`; despite that warning, the user reports audible sound.
- Pending: physical menu relaunch, controls, save path, and a complete in-game session on the packaged-default Xorg path.
