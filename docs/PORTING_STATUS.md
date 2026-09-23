# POSTAL 2 TSPS port status

## Verified locally

- Source archive: `~/Downloads/Postal 2.zip`.
- The archive contains the Linux x86 `postal2-bin`, SDL 1.2, OpenAL and the game data under `postal2/gamedata`.
- The TSPS bridge source is adapted from the Guacamelee reference and rebuilt with Zig for ARMHF guest and aarch64 presenter targets.
- The package excludes the commercial game payload; local payload staging is git-ignored.

## Device verification

- Target: TSPS Longan at `192.168.50.135`.
- Device architecture: aarch64.
- Physical launch and gameplay verification: pending.
- Real screen capture: pending.

## Facts versus hypotheses

- Fact: the archive has the expected Linux game binary and bundled 32-bit libraries.
- Fact: the generated bridge binaries have ARMHF and aarch64 ELF types respectively.
- Hypothesis until device launch: the Postal 2 SDL 1.2/OpenGL path will work with the same gl4es and bridge profile used by the reference port.
