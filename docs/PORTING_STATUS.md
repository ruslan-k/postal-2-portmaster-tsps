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
- Sep 26 relative-mode A/B with the seeded first delta did not fix the user's upper-left cursor limit. At that time, the SDL getter logs had not yet been added; the earlier interpretation of `xrel/yrel` as pointer-position samples was wrong. Game remains 640×480 (`SetRes: 640x480`) in the 1280×720 Xorg/KMS path; the prior absolute x/y rewrite also failed and made the cursor disappear.
- Sep 26 physical run with SDL getter probe: the game queried `SDL_GetRelativeMouseState(NULL,NULL)` twice only; no `SDL_GetMouseState` calls were logged. Fresh raw SDL events show the actual contract: `xy=(256,192), rel=(256,192)` then `xy=(258,192), rel=(2,0)`, with later positions reaching `(639,479)`. The relative-mode shim had incorrectly treated genuine xrel/yrel deltas as absolute coordinates, turning `(2,0)` into `(-254,-192)` and subsequent small motion into zero. This explains the upper-left restriction. Stopped only verified Postal 2 PID 13701 after capturing logs; gptokeyb2/Xorg/presenter cleaned up and MainUI returned.
- Sep 26 cursor-visibility probe: guest called `SDL_ShowCursor(toggle=0, result=1)`; the explicit `toggle=0` is the hide request, while the API return interpretation is not used here. It then called `SDL_WM_GrabInput` modes `0` and later `1`. The KMS frame had no visible cursor while SDL motion events were reaching the guest. This is a strong hide/grab lead; it is not yet proved that SDL's host cursor is the Unreal menu cursor.
- Sep 26 with force-visible cursor, before the geometry A/B: KMS screenshot showed the cursor on the options screen; SDL logs showed `ShowCursor requested=0 effective=1`. At that point X11 root was 1280×720, while the grabbed SDL child was 640×480 at +0,+0. This was a plausible but unproven geometry explanation, not a confirmed root cause.
- Sep 26 geometry A/B result: deployed Xorg dummy mode `640x480` (backup preserved); fresh Xorg log confirms `Virtual size is 640x480`, while the presenter/KMS capture remains 1280×720. User reports cursor is still confined to the upper-left area, so matching Xorg root to the unchanged 640×480 game window did not fix the visible limitation.
- In that run, unmodified SDL motion events reached `(639,479)` and left-button down/up were logged at `(639,479)`, the full 640×480 guest boundary. This proves delivery to the SDL boundary, not that Unreal's UWindow hover/click state or the visible cursor maps to the same coordinates. Next isolate the Unreal mouse-axis accumulator/viewport and pointer-vs-game coordinate transform; do not repeat root-size or guessed scaling A/Bs. KMS output stayed 1280×720. Exact Postal 2 process was stopped after log/screenshot capture; gptokeyb2, Xorg, and presenter exited, MainUI returned.
- 720p: the game config has independent Windowed/Fullscreen/MenuViewport dimensions, but the live game reports `SetRes: 640x480`, and the current presenter source is 640x480 on a 1280x720 KMS target. A previous 1280x720 window-only experiment cropped the UI. A coherent 1280x720 guest+presenter profile is possible to stage, but native TSPS fullscreen correctness/performance are unverified; do not combine that graphics A/B with cursor diagnostics.
- The user previously confirmed that picture and sound are present on the working X11/GLX variant.

## Facts versus hypotheses

- Fact: the archive has the expected Linux x86 game binary and bundled 32-bit libraries.
- Fact: the generated bridge binaries have ARMHF and aarch64 ELF types respectively.
- Fact: the X11/GLX-capable GL4ES path reaches the Postal 2 menu; the earlier GLX stub no longer blocks startup.
- Fact: the previous menu black screen was caused by the default Weston headless EGL initialization failure, not by a proven black game framebuffer.
- Fact: the runtime log still reports `libopenal.so.1: wrong ELF class: ELFCLASS64`; despite that warning, the user reports audible sound.
- Pending: physical menu relaunch, controls, save path, and a complete in-game session on the packaged-default Xorg path.
