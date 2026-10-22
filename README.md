# Yokoi: Game & Watch Emulator — RGDS / RGDS+ Edition

<img src="assets/Yokoi.png" alt="Yokoi — Game & Watch Emulator" width="350">

[![Download](https://img.shields.io/badge/Download-v1.4.2-blue?style=for-the-badge)](https://github.com/PolluxPT/Yokoi-RGDSPlus/releases/latest)

**[English](README.md) | [Português](README.pt-PT.md)**

Native standalone Linux ARM64 port of Yokoi for **Anbernic RGDS / RGDS+ Stock OS**. Version **1.4.2**.

Original emulator: [RetroValou / Yokoi](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds), base revision `f273025`. RGDS adaptation project: **Filipe Baeta**, developed with assistance from ChatGPT/Codex. This is a Linux application, not the upstream Android APK.

## What's new in 1.4.2

- **Tropical Fish:** corrected GAME A/B selection from physical buttons and Pause.
- **Mario The Juggler:** START/SELECT protected during play; both modes available again after Game Over.

- **Single GAME reset fix:** Balloon Fight, Climber and Super Mario Bros. use START to begin/replay; START is ignored during a run and reenabled after native Game Over. Release and press again to replay. Other GAME A/B machines retain native behaviour, with the Mario The Juggler protection described above.
- **Single GAME button:** use START. This applies to both variants of Balloon Fight, Climber and Super Mario Bros.; SELECT is not an alternative GAME button.
- Games open in their native Watch/clock mode. Release the library-open button before pressing START or SELECT to play.
- **Segments** and **Background colour** are in Pause, immediately after GAME B. Changes apply during the current game, with a lower-screen preview, and are saved.
- Linux local clock synchronization in Watch, using upstream per-game clock mappings.
- The last opened game and manufacturer are remembered across application restarts.

See [full PT/EN release notes](RELEASE_NOTES_v1.4.2.md).

Known issue: the reported RGDS startup Bus error (exit 135) has not been reproduced or demonstrated fixed. See [compatibility comparison](COMPATIBILITY_v1.4.2.md).

## Features

- Two-screen SDL2 display, touch browsing and calibrated controls.
- PT/EN menus; English on first use, retaining existing language preferences.
- L1/R1 initial-letter navigation; L2/R2 manufacturer browsing and in-game screen swaps.
- Pause with calibrated R or START+SELECT.
- Three manual save slots per game, load/delete management and exit confirmation.
- RGDS audio compatibility and physical volume keys with gain `0.35 × (level/10)²`.
- Console volume recorded on entry and restored on exit.

## Installation

Download **`Yokoi_RGDS_RGDSPlus_v1.4.2_Update.zip`** from [Releases](https://github.com/PolluxPT/Yokoi-RGDSPlus/releases/latest).

1. Close Yokoi and extract the update ZIP.
2. Copy its `Roms` folder to the SD-card root, replacing application files.
3. Keep `Yokoi_RGDSPlus/lib`, your pack, settings and `saves`.
4. Launch the same `Yokoi.sh` from APPS. No new launcher or recalibration is required unless requested by the app.

This is an update for an existing working Linux installation. No runtime libraries or ROM pack are included. Fresh installations need a compatible ARM64 runtime. The distributed binary requires SDL2 and symbols up to GLIBC_2.38 / GLIBCXX_3.4.31; see [Linux build/runtime notes](linux/README.md).

## Controls

| Screen | Control | Action |
|---|---|---|
| Library | D-pad / touch swipe | Browse games |
| Library | A / START / double tap | Open in Watch mode |
| Library | Y | Load an occupied save slot |
| Library | L1 / R1 | Previous / next initial |
| Library | L2 / R2 | Previous / next manufacturer |
| Library | X | Advanced options: Language, Configure buttons, Back |
| Library | R | Exit confirmation |
| Watch / Game Over | START | GAME A, or the single GAME button |
| Watch / Game Over | SELECT | GAME B, only where the original machine has it |
| Gameplay | START / SELECT | Single GAME and Mario The Juggler: blocked during play; other machines: native ROM behaviour |
| Gameplay | R or START+SELECT | Pause |
| Gameplay | L2 / R2 | Swap game screens |
| Pause | GAME A / GAME B; A | Explicit new game, where supported |
| Pause | Segments / Background colour | A or Left/Right to adjust; lower-screen preview |
| Pause | Up / Down; A; B | Select; confirm; resume |
| Pause | Left / Right | Save-slot selection |
| Slot manager | D-pad; A; B | Select slot; request deletion; back |

START+SELECT remains a Pause chord. GAME A/B labels retain the original hardware wording in both languages. Machines with one GAME button use START only; no artificial GAME B mode or SELECT-to-START alias is added.

## Supply your own game pack

**No ROMs, ROM packs, original game artwork or personal saves are distributed.**

Follow the [original Yokoi conversion instructions — Android (RGDS) target](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds/blob/main/CONVERT_ROM/README.md#android-rgds). After completing its prerequisites, from `CONVERT_ROM`:

```sh
python convert_original.py --target rgds
python convert_3ds.py --target rgds
```

Put your own `yokoi_pack_rgds.ykp` in `Roms/APPS/Yokoi_RGDSPlus/`. The 3DS pack is not supported. Do not upload game content to this repository.

## Validation and known limits

Local CPU snapshot comparisons cover all six single-GAME variants: initial Watch, START, blocked in-game resets, held and late presses across Game Over, saved-state resume and replay, at two polling intervals. Virtual-controller UI and save compatibility checks are also retained; see [validation](VALIDATION_v1.4.2.md). These local checks require confirmation on the console.

The reset guard applies to the three single-GAME input families and Mario The Juggler. Tropical Fish uses the corrected GAME A/B wiring. Clock synchronization depends on upstream per-title mappings. The previously reported RGDS fullscreen/black-bars behaviour remains deferred; this release does not claim to fix it.

## Build

On Debian/Ubuntu:

```sh
sudo apt install g++ libsdl2-dev
bash linux/build.sh
```

This builds `linux/build/yokoi` for the host architecture. Build on ARM64 or use the [cross-build variables](linux/README.md) for the console. `android/app/src/main/cpp` contains shared CPU/input helpers, not a complete Android frontend.

## Credits and notices

- Original Yokoi: [RetroValou](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds).
- Upstream credits include MAME, Adam Gastineau's Game & Watch FPGA projects, Sharp documentation and ABerguerand.
- PNG decoder: LodePNG, copyright Lode Vandevenne.
- Font: DejaVu Sans, with notices in `linux/FONT-LICENSE.txt`.

The original README describes its code as Public Domain / Free to use. Third-party notices are retained; see [NOTICES.md](NOTICES.md). This is an unofficial project, unaffiliated with Nintendo or Anbernic.
