# POSTAL 2 TSPS port status

## Verified locally

- Source archive: `~/Downloads/Postal 2.zip`.
- The archive contains the Linux x86 `postal2-bin`, SDL 1.2, OpenAL and the game data under `postal2/gamedata`.
- The TSPS bridge source is adapted from the Guacamelee reference and rebuilt with Zig for ARMHF guest and aarch64 presenter targets.
- The package excludes the commercial game payload; local payload staging is git-ignored.

## Device verification

- Target: TSPS Longan at `192.168.50.135`.
- Device architecture: aarch64.
- The standard PortMaster launcher reached the real Postal 2 process through `backend=tsps-bridge-weston-x11`.
- A real KMS capture from the X11/GLX validation run showed the Postal 2 main menu: `POSTAL 2 Share The Pain`, `New Game`, `Load Game`, `Multiplayer`, `Options`, and `Exit`.
- The user confirmed that picture and sound are present.
- The tested process tree was stopped afterwards. No Postal 2 launcher, Box86 game, presenter, gptokeyb2, or Weston wrapper remained; MainUI stayed alive. The post-stop KMS capture showed SpruceOS's lock screen, so the physical button must wake/unlock the UI before the next menu launch.

## Facts versus hypotheses

- Fact: the archive has the expected Linux x86 game binary and bundled 32-bit libraries.
- Fact: the generated bridge binaries have ARMHF and aarch64 ELF types respectively.
- Fact: the working launcher uses the TSPS presenter and the X11/GLX-capable GL4ES path instead of the earlier GLX stub.
- Fact: the runtime log still reports `libopenal.so.1: wrong ELF class: ELFCLASS64`; despite that warning, the user reports audible sound.
- Pending: physical control mapping, save path, and a complete in-game session still need the user's direct hardware test.
