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
- The launcher and all 104 Xorg runtime files were deployed with SHA-256 verification and a launcher backup. Physical menu re-test is pending.
- 2026-09-26: the installed launcher had uncommitted Xorg evdev integration: start gptokeyb2 first, attach its `Fake Keyboard Mouse` event node, and use a mouse-driven root map (D-pad motion, A left-click). These exact installed files are now the repository starting point; the former tracked map's `[controls:menu]` was never selected by the launcher.
- Latest two device launches (Sep 24 and Sep 26) both terminated with Box86 `Unimplemented Opcode (EA) F0 40 2D E9 02 40 A0 E1`, `SIGILL`, `xvfb_game_exit=1`. Xorg attached event5 as a keyboard/mouse; that does not prove guest input or UI action. The Sept 26 Unreal log reached `Startup` and initialized an SDL viewport, then stopped. Cause of SIGILL remains unknown.
- Sep 26 physical run: B skipped the intro, but the menu did not navigate. The live i386 probe was present in `/proc/6992/maps`; SDL received mouse motion and left-button press/release. Repeated motion had `x/y=639,479` while `xrel/yrel` varied with the physical stick; left clicks were also at `639,479`. Thus input reaches the guest, but hover/click coordinates are suspect. The user reports that the visible cursor vanishes when using sticks. This run remained live during observation; do not infer the earlier SIGILL recurred.
- Sep 26 physical A/B with `coord_mode=absolute`: the guest loaded the new ELF from `/proc/8566/maps`, logged raw mouse events at `639,479` and rewritten absolute x/y (e.g. `383,168`, later `610,478`). User reports that stick movement still makes the cursor disappear; no menu navigation. This disproves absolute x/y correction alone. The run was live when inspected and did not show a new SIGILL.
- Sep 26 relative-mode A/B with the seeded first delta did not fix the user's upper-left cursor limit. Fresh launcher logs show `x/y=639,479` while the physical-position-like `xrel/yrel` changes: first `(256,192)`, next `(383,168)`, later `(639,361..479)`. The shim's normalized deltas accumulate across the full 639×479 range in its host test, but this was not sufficient for the visible Unreal cursor. Game remains 640×480 (`SetRes: 640x480`) in the 1280×720 Xorg/KMS path; prior absolute x/y rewrite also failed and made the cursor disappear. The first-offset diagnosis was incomplete.
- Next diagnostic candidate (no coordinate mutation beyond the existing relative mode): the i386 SDL interposer now logs up to 80 calls/results from `SDL_GetMouseState` and `SDL_GetRelativeMouseState`, in addition to Poll/Peep events. This distinguishes state-polling from event-driven cursor input; it is diagnostic, not a claimed fix. Deployed with backup after user confirmed Postal 2 was closed; target hash must match local before asking for the next physical menu run.
- 720p: the game config has independent Windowed/Fullscreen/MenuViewport dimensions, but the live game reports `SetRes: 640x480`, and the current presenter source is 640x480 on a 1280x720 KMS target. A previous 1280x720 window-only experiment cropped the UI. A coherent 1280x720 guest+presenter profile is possible to stage, but native TSPS fullscreen correctness/performance are unverified; do not combine that graphics A/B with cursor diagnostics.
- The user previously confirmed that picture and sound are present on the working X11/GLX variant.

## Facts versus hypotheses

- Fact: the archive has the expected Linux x86 game binary and bundled 32-bit libraries.
- Fact: the generated bridge binaries have ARMHF and aarch64 ELF types respectively.
- Fact: the X11/GLX-capable GL4ES path reaches the Postal 2 menu; the earlier GLX stub no longer blocks startup.
- Fact: the previous menu black screen was caused by the default Weston headless EGL initialization failure, not by a proven black game framebuffer.
- Fact: the runtime log still reports `libopenal.so.1: wrong ELF class: ELFCLASS64`; despite that warning, the user reports audible sound.
- Pending: physical menu relaunch, controls, save path, and a complete in-game session on the packaged-default Xorg path.
