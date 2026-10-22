# Yokoi Linux / RGDS / RGDS+ — v1.4.2

Native C++20/SDL2 frontend based on Yokoi revision `f273025`. No Android APK, JNI or RetroArch is required. The shared code under `android/app/src/main/cpp` supplies CPU/input utilities only.

## Build

On Debian/Ubuntu install `g++` and `libsdl2-dev`, then run from the repository root:

```sh
bash linux/build.sh
```

Output: `linux/build/yokoi`, for the host architecture. For RGDS/RGDS+, build on ARM64 or supply `CXX`, `CPPFLAGS` (sysroot), `SDL_CFLAGS`, `SDL_LIBS`, `LDFLAGS` and `OUTPUT` for an ARM64 toolchain and ARM64 SDL2 development files. `MAIN_SOURCE` selects a test source instead of `linux/main.cpp`.

The release is built with GCC 13.3 and SDL2 2.30 headers. Highest required symbols: GLIBC_2.38 and GLIBCXX_3.4.31. SDL is loaded from the console runtime. No SDL_image or SDL_ttf is needed: PNG decoding is bundled LodePNG and the UI font is embedded DejaVu Sans with its notice.

## Runtime and installation

The update archive supplies:

- `Roms/APPS/Yokoi.sh`
- `Roms/APPS/Yokoi_RGDSPlus/yokoi`
- `Roms/APPS/Imgs/Yokoi.png`

Keep the existing working `lib` directory, pack, configuration and `saves`. These are not distributed in the update. The same directory and launcher serve both console models. Fresh installations need compatible ARM64 runtime libraries as well as a user-created pack; this is not a self-contained first-install package.

The pack path defaults to `yokoi_pack_rgds.ykp` beside the executable; a path can also be passed as the first argument. The loader accepts RGDS/Android packs (platform 2, pack format v2/v3, content version >=3), not 3DS packs. See the root README for original conversion instructions.

The launcher selects the Stock Wayland/ALSA environment and writes `Yokoi_Arranque.txt`. The app creates two independent SDL windows and uses calibrated Anbernic input profiles. L2/R2 changes manufacturer in the library and swaps game screens during play. Touch swipe/double tap is supported.

## Setup input and game state

`native_setup.h` forwards GAME A/B to the existing virtual-input mapping. It does not inspect lives, Game Over state, ROM hashes or inactivity. GAME A/B machines retain that native behaviour. `single_game_gate.h` additionally guards only mappings marked `single_game_button`: Balloon Fight, Climber and Super Mario Bros., both variants of each. START produces a bounded GAME pulse; SELECT is unmapped. Fresh CEND entries rearm GAME after native game end. Press eligibility is recorded before the Pause chord grace so an in-game press cannot be queued across Game Over. Holding through Game Over requires release and a new press. The guard is recreated from the existing CPU on resume/load without changing save formats; explicit Pause GAME actions remain intentional new games.

The library-open button must be released first. A short grace reserves START+SELECT for Pause; short taps receive a minimum native input duration. Explicit Pause GAME A/B actions use a native setup pulse after initialization. A physical setup request stops Watch-only host-clock refresh so active game state is not overwritten.

START maps to GAME A or the sole GAME button. SELECT maps to GAME B only where implemented by the original input configuration. There is no Climber SELECT-to-START alias. Both variants of Balloon Fight, Climber and Super Mario Bros. use START only.

Watch initialization and refresh use upstream per-title local-time addresses. Loaded saves and active games are not continuously modified to match host time. `last-game.cfg` atomically stores the last opened ROM reference and manufacturer. Saves retain their existing format.

## Display, language, audio and volume

Pause contains Resume, Choose another game, Save, Load, Manage slots, GAME A, GAME B, Segments, Background colour and Exit (translated in PT). Display changes apply without a CPU reset and are saved in `display.cfg`; the lower panel previews the chosen game. Advanced options contains Language, Configure buttons and Back. English is the default for new settings; existing PT/EN preferences remain valid.

SDL audio negotiates sample format, frequency and channels, with a stereo fallback and conversion for the obtained device format. Physical volume uses `0.35 × (level/10)²`. The app and launcher restore entry volume on exit; existing crash/signal cleanup is retained. Runtime libraries and firmware are not modified.

Black game margins preserve aspect ratio in the renderer. The separate RGDS fullscreen/black-bars report remains deferred.

## Checks

`--version` prints version without starting SDL. `--smoke-test PACK` renders a few library frames. `--test-game PACK INDEX` opens a selected game in Watch. Use `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy` for headless UI tests; `YOKOI_TEST_DUAL=1` enables two test windows on one display.

- `tests/single_game_gate.cpp`: full CPU snapshot comparisons on all six external ROMs, active reset reproduction, native end/replay, held/late presses and saved-state resume.
- `tests/native_setup.cpp`: opening hold, fresh/repeated setup presses, short taps, Pause chord and absence of SELECT-to-START alias.
- `tests/single_game_input.cpp`: verifies actual Balloon Fight/Climber/Super Mario Bros. input mappings from an external pack: SELECT does not press GAME; START does.
- `tests/physical_start_ui.cpp`: virtual-controller library entry, native START/SELECT forwarding, Pause and resume without recreating the CPU.
- `tests/state_roundtrip.cpp`: save/load/replay and rollback after corrupted files across six real games.
- `tests/synthetic_pack.py`: generates original synthetic test fixtures, with no commercial ROM data.
- `tests/native_rom_compare.cpp`: reproducible RAM/LCD comparison with and without native setup presses, using an external pack.

Other tests cover display settings, clock, language, calibration, library return, screen swaps, audio and volume. See `VALIDATION_v1.4.2.md` for recorded results and console feedback. Do not distribute test ROM packs, game artwork, personal saves or screenshots.

## v1.4.2 input fixes

TF_104 GAME A/B wiring is corrected in virtual_input.h. juggler_gate.h applies only to MB_108, observing its native MISS/Watch display and protecting both setup buttons. Tests: juggler_tropical.cpp and physical_juggler_ui.cpp. SingleGameGate remains unchanged.
