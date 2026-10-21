# Yokoi Linux / RGDS+ — 1.2 Final

Native SDL2 frontend, no JNI/Android or RetroArch. Upstream base: f273025.

## Build

On Debian/Ubuntu Linux install `g++`, `libsdl2-dev`, `make`.
From the repository root run `linux/build.sh`. The output is `linux/build/yokoi`.
PNG decoding is bundled LodePNG; text uses a rasterized DejaVu Sans font.
No SDL_image, SDL_ttf, OpenGL, Android SDK or embedded ROMs are required.

For cross-compiling, set CXX, SDL_CFLAGS, SDL_LIBS, CPPFLAGS (sysroot),
LDFLAGS and OUTPUT before invoking the script. Use ARM64 SDL headers/libraries.
The distributed ARM64 binary was built using GCC 13.2 and glibc 2.39 cross
headers; the highest required libc symbol version is GLIBC_2.38. SDL is loaded
from the Stock OS. The supplied C++ runtime comes from the user's working
ScummVM package and provides the required GLIBCXX/CXXABI versions.

## Run

Place an RGDS/Android pack `yokoi_pack_rgds.ykp` next to the executable, or
pass its path as the first argument. Format v2/v3, content version >=3,
platform 2. This port explicitly rejects 3DS packs.

Keyboard: arrows, Z/X/A/S for ABXY, Enter for Start, Backspace for Select,
Q for Time, F2 for swapping screens, P/Escape for Pause.
SDL game controllers use the corresponding controls. An optional
`gamecontrollerdb.txt` can supply additional mappings.

The launcher `Yokoi.sh` targets `Roms/APPS/Yokoi_RGDSPlus/yokoi`, uses the
firmware SDL2 Wayland/ALSA backends, and records `Yokoi_Arranque.txt`.
It doesn't use the ScummVM preload screen shims; SDL creates two independent
fullscreen windows. Display ordering and controller mapping require testing
on the actual device. L2 and R2 swap game panel roles; the library always keeps the list above the cover.

## Tests

`--version`: identify build without initializing SDL.
`--smoke-test PACK`: three menu frames then exit.
`--test-game PACK INDEX`: start game, pulse Game A, run briefly then exit.
`YOKOI_TEST_DUAL=1` creates two test windows even on one display.
Tests use `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy` when headless.
Test screenshots are written to `/tmp/yokoi-test-0.bmp` and `...-1.bmp`.
`tests/synthetic_pack.py` generates original diagnostics with zero-filled
ROMs and colored textures, using Pillow and the upstream pack writer.

## Changes

- Added native menu, PNG texture loader, SDL2 two-panel drawing and timing.
- Reused upstream CPU selection, virtual inputs and controller mapping.
- Added SDL queued audio and resampling, Stock volume gain curve.
- Converted pack bottom-left coordinates and mirrored images to SDL.
- Fixed case-sensitive include paths and a Windows backslash include.
- Added a virtual SM5XX destructor for safe polymorphic ownership.
- Linux accepts RGDS packs only. Melody data is validated before SM511/512 use.

Initial scope excludes touch, save states and Android settings UI.
The standard game emulation quirks of upstream remain.

## 0.2: hardware input, volume and response

START/SELECT are read from BTN_START/BTN_SELECT on the ANBERNIC evdev
node. If that node is unavailable, the known Stock rk3568 joystick button
order (Select 8, Start 9) replaces the generic SDL Back/Start mapping.
SDL setup button events are suppressed in this profile, so L2/R2 cannot
trigger setup operations through the old mapping. Other SDL controls stay
as before. The selected devices and SDL mapping are logged at startup.

Volume keys are discovered by EVIOCGBIT capabilities and read nonblockingly.
SDL volume key events are a fallback when no evdev volume node is available.
Press changes one level; hold repeats after 450 ms, then every 120 ms.
Stock volume and local fallback volume-level.txt are saved. The software gain
is exactly 0.35*(level/10)^2 with levels 0..10, and a brief on-screen bar
shows the selected level. External Stock changes are checked at 4 Hz.

Input and CPU execution run at 240 Hz with 60 Hz drawing. Button-down events
are latched for at least 16 ms so a quick press/release during rendering is
not lost. Audio uses 512-sample blocks instead of 2048, with queue backlog
limited to 50 ms instead of 200 ms. Per-frame sysfs file reads were removed.

Regression test: compile linux/tests/input_volume.cpp with the linux/ and
Android input include directories and SDL2. It checks kernel key identities,
short taps, volume hold/repeat, gain/persistence and a virtual SDL controller
that reproduces the reported incorrect L2/R2 setup mapping.

## 0.3: calibrated controls and independent timing

The 0.2 assumption about BTN_START/BTN_SELECT and fallback button positions was
not valid on the user's unit. It is superseded by a four-step first-run wizard:
press the physical START, SELECT, L1, R1 buttons, releasing between each step.
The raw SDL joystick button indices and GUID are saved in buttons-rgds-v1.cfg.
These calibrated buttons are removed from their generic SDL bindings to prevent
Select from swapping screens or L2/R2 from acting as setup buttons. Remove only
that configuration file to repeat the wizard. No preset mapping is shipped.

CPU execution, LCD state publication and audio generation now run on a separate
worker thread. Cycle targets follow monotonic elapsed time at the CPU's native
32768Hz. SDL drawing and Wayland presentation cannot slow the CPU by stalling
the main loop. Snapshots are copied under a short mutex; controls and volume
use atomics. The worker stops and joins before CPU or audio resources are freed.
Static backgrounds and LCD ghost segments are cached in SDL render targets
when supported, reducing repeated per-frame draw calls.

Audio uses 1024-sample device buffers, 40ms initial prefill, native-rate DC
blocking and a 6500Hz low-pass filter before SDL resampling. Volume transitions
are ramped, with the same target gain 0.35*(level/10)^2. Routine audio queue
clears were removed. This addresses frontend clicks and starvation; it is not
a claim that every upstream SM510 sound-emulation limitation is corrected.

Diagnostics print emulated versus real elapsed time and audio queue size.
For stress tests only, YOKOI_TEST_RENDER_DELAY_MS inserts a delay per panel,
and YOKOI_TEST_FRAMES bounds the rendering test duration. With two panels and
35ms artificial delay per panel, local tests of Donkey Kong/Donkey Kong Jr.
maintained approximately 99.9% emulation speed. These were not tests of the
actual player reaching the Donkey Kong defeat/bonus-scoring sequence.

Tests:
- tests/input_volume.cpp: arbitrary calibrated indices, persistence, no Select
  screen-swap collision, volume keys/repeat/gain.
- tests/calibration_flow.cpp: runs the real calibration UI with a virtual controller
  whose generic SDL bindings conflict with the chosen physical button roles.
  Build with MAIN_SOURCE=linux/tests/calibration_flow.cpp; run with dummy SDL
  drivers. It saves a temporary profile and checks all eight raw indices.
- tests/audio_filter.cpp: stable 4096Hz tone, DC rejection, silence and mute.


## Version 0.4

The dark two-panel library uses embedded antialiased DejaVu Sans fonts. On the
lower touchscreen, swipe left/right to change game and double tap to start
Game A. A/START starts Game A; SELECT starts Game B; Y loads the selected slot.
New games receive a 250ms setup pulse after two seconds. Live START/SELECT are
masked during gameplay, preventing accidental resets. START+SELECT opens Pause.

The physical special/Guide button opens Pause; B resumes and A confirms.
The pause menu offers Continue, Save, Load, New game A/B, Library and Exit.
Left/right chooses one of three slots. Saves are atomic checksummed files in
saves/<game.ref>.slot1.yks (through slot3), matched to the ROM and CPU model.
All mutable CPU state is included. Save states are version-specific; keep the
saves folder when updating. No automatic saving is performed.

buttons-rgds-v2.cfg stores START, SELECT, L1, R1, A, B, X and Y raw indices.
An existing v1 profile retains the first four; the wizard asks only A/B/X/Y.
Without a profile it asks all eight. Generic SDL face mappings are bypassed.
For UP/DOWN plus LEFT/RIGHT games, the unused dpad horizontal directions also
control the right player, alongside Y/A. This covers Donald and Squish.

Panorama/static mask backgrounds now use near-black as on upstream Android.
The launcher enables native lower-panel touch and restores the previous touch
mode on exit. Volume and worker timing retain the 0.3 implementation.

Tests added: state_roundtrip.cpp checks six games across CPU models and corrupt
save rejection; ui_flow.cpp exercises swipe/double tap, Guide Pause and two
save slots through the actual UI. Both normal and ASan/UBSan UI runs passed
locally with dummy SDL. Physical RGDS+ verification remains necessary.


## Version 0.5

Approved palette: charcoal #191B22, mint selection #83D6C0 with dark text,
soft-white main text #EEF2F5, information #B8C2CC and orange edition #FFA45B.
Library roles are fixed: top list and header, lower cover only. The strip and
BIBLIOTECA label were removed. The top footer shows the release date and all
three occupied/empty slots, with occupied states in mint.

PT/EN is selected with the Language button and saved in language.cfg. Press X
for that button; left/right selects Language or Configure buttons; A confirms;
down returns to the games. Up from the first game also reaches these buttons.
GAME A and GAME B retain their original hardware labels in both languages.
L1/R1 jump to the first game of the previous/next distinct initial, wrapping.
L2/R2 trigger rising edges swap game panels. R1 no longer swaps library panels.

Configure buttons repeats all eight physical-button steps and the Anbernic R
step. On upgrade, existing v2 calibration is retained and only R is requested.
R may produce SDL joystick, keyboard or evdev events; the wizard records the
observed type/code in special-rgds.cfg. Selected kernel key devices are read
without grabbing input; key and button presses are logged for diagnosis.
If firmware does not expose R, hold START+SELECT during its calibration to
skip it. START+SELECT remains the Pause fallback. No physical-R success is
claimed until verified on the console.

Pause order: Resume, Choose another game, Save state, Load state, Manage slots,
GAME A - New, GAME B - New, Exit (localized). Slot management selects/deletes
one of three saves; delete and exit dialogs default to Cancel. Save format is
unchanged from 0.4. Y in the library finds an occupied slot when the selected
slot is empty. Manual saves and the volume curve are unchanged.

ui_flow.cpp exercises PT/EN persistence, initials, recalibration, touch, both
triggers, an R button whose SDL Guide mapping is deliberately wrong, save/load,
slot deletion with cancel/confirm, discovery of slot3 and confirmed exit.
calibration_flow.cpp covers v1 migration and physical face-button calibration.


## Version 0.6

The title uses the original 32px bold face; RGDS+ Edition stays right-aligned
in orange. A subtle one-pixel separator sits below the title and another
above Release date. Selection restores the 0.4 style: dark mint-tinted row,
bright mint left marker and light bold selected name, over the charcoal base.
The cover now fills exactly 1024x768, without application margins or a frame.

Language and Configure buttons moved to X / Advanced options. This separate
menu contains Language, Configure buttons and Back. Up/down selects, A confirms
and B returns to the game list. R also backs out of advanced options; in the
game list it opens the existing Exit confirmation. The main footer now reads
Release date, L1/R1: jump, X: advanced options and R: Exit in the chosen language.
The package includes the user-supplied icon at Roms/APPS/Imgs/Yokoi.png.

Local ui_flow regression passed through advanced options, language persistence,
recalibration, returning to the game list, game launch, pause, saves, deletion
and exit confirmation. The ARM64 build and both menu panels were inspected.


## Version 0.7

All frontend menus now share selectionRow and menuHeader rendering helpers:
charcoal base, subdued blue #233B58 selected row, #79B9FF left marker, light
bold selected text, muted secondary text and subtle dividers. This includes
the pause menu, slot manager, advanced options, calibration and exit/delete
confirmation buttons. Notifications and volume indicators use the same palette.
LCD emulation colors are unchanged. RGDS+ Edition uses the smaller 20px face,
right aligned in orange, beside the 32px bold application title.
The original user icon remains bundled at Roms/APPS/Imgs/Yokoi.png.


## Final 1.0

Removed the unsolicited game name from the Pause and Manage slots headings.
Capitalized Load state, Jump and Advanced options in PT/EN main-menu hints.
Game textures are loaded lazily on entry; the menu only decodes the lower
panel cover. The worker timing, sound, volume curve and save format are intact.
The launcher no longer runs a redundant --version process or loader --list on
every launch; YOKOI_DIAGNOSTICS=1 retains the dependency diagnostic. The normal
loader still reports startup failures into Yokoi_Arranque.txt.
Startup timings log SDL/input, pack loading and first presented frame.

Local paired benchmark, 8 launches each, two dummy SDL panels and the same
pack/game: median complete --smoke-test process time was 103.8ms (0.7) versus
73.2ms (1.0). This includes 3 frames, diagnostic BMP output and teardown;
it is not an ARM64 cold-SD or Wayland benchmark. Hardware improvement remains
to be measured. The real UI regression passed through language, recalibration,
launching, triggers, pause, save/load, deletion and exit confirmation. Menus
were visually checked in PT/EN, and Panorama rendering after lazy load checked.


## Final 1.1

Pause title uses the 40px face. The first row now starts at y=112 instead of
160; the footer and panel shrink by the same 48px. Touch row hitboxes follow
the new origin. Capitalized PT/EN action labels in Pause, slot management
and Advanced options (Slot, Confirm, Back, Select, Delete).

## 1.2: physical L2/R2 calibration

Stock Linux ARM64 update; L2/R2 operation confirmed by the maintainer on their RGDS+. L2/R2 no longer
rely on SDL's generic controller trigger mapping. On upgrade, only these two
controls are requested. The existing eight-button and special R profiles stay
unchanged. Configure buttons in Advanced options repeats all controls.

Bindings are stored separately in `swap-rgds-v1.cfg`, matched to the controller
GUID. Sources: raw SDL joystick button, raw axis (rest/direction and hysteresis),
keyboard scancode, or evdev key. Generic SDL controller axis events are ignored
to prevent duplicate swaps. Hold START+SELECT to skip an unavailable trigger;
a skipped trigger is disabled until recalibration. Delete only this file to
repeat L2/R2 setup. Menu display roles remain fixed; swap only during gameplay.

Validation: `tests/swap_profile.cpp` covers edges, neutral values, both axis
directions, hysteresis, source/device isolation, persistence and keyboard/evdev.
`tests/ui_flow.cpp` verifies each swap independently through an atomic counter,
including a held press; run again with `YOKOI_TEST_DIGITAL=1` for digital L2/R2
and `YOKOI_TEST_SPECIAL_KEY=1` for keyboard special R.

PT: Na primeira abertura, calibrar L2/R2. Os restantes botões mantêm a sua
configuração. Para repetir apenas estes dois passos, apagar `swap-rgds-v1.cfg`.
Manter START+SELECT permite ignorar um botão sem resposta. Só há troca de
ecrãs durante o jogo, fora da pausa. Funcionamento L2/R2 confirmado pelo responsável do projeto na sua RGDS+.
