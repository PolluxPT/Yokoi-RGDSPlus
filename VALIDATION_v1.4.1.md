# v1.4.1 local validation

Scope: only the Balloon Fight, Climber and Super Mario Bros. single-GAME virtual input classes are marked for the new gate. Other mappings default to native passthrough. No ROM hashes, per-title RAM guesses or inactivity timeout are used to rearm START: the common native CEND sequence is observed. The frontend does not serialize this observation, so existing saves retain their format.

The previous Native Test console feedback established replay availability; it did not establish protection against resets during active play. The user subsequently reproduced the v1.4 regression on the single-GAME family and confirmed Donkey Kong was unaffected. v1.4.1 physical console testing is pending.

## Real ROM regression tests (external user pack, not distributed)

All six variants pass with polling strides of 32 and 256 CPU cycles:

- Watch entry, SELECT unmapped, fresh START, bounded native GAME pulse.
- Full serialized CPU state equals an unpressed reference after repeated in-game START attempts. The raw v1.4 path reproduces state divergence in Balloon Fight and Climber.
- Native end followed by replay after loading a terminal snapshot.
- Loading an active snapshot preserves protection without relying on frontend save data.
- Holding START through native end does not replay; a press made just before end is not queued by the Pause grace.
- Explicit Pause GAME remains an intentional new game; native GAME A/B passthrough is unchanged.

Tests: `linux/tests/single_game_gate.cpp`. No commercial pack or derived artwork is included. The existing input transport, virtual controller UI and save/load checks are also run for this build.

## Final ARM64 execution and native-controls comparison

The ARM64 build of `single_game_gate.cpp` passed under QEMU 8.2.2 with a Cortex-A53 CPU model. This checks the target instruction set and alignment as well as the game logic. All 50 other ROMs were run from identical captured states with the native v1.4 input path versus the new frontend path; complete saved CPU states matched. The six protected variants passed at both polling intervals.

The real Balloon Fight SDL virtual-controller UI test also passed: opening hold, fresh START, SELECT unmapped, repeated START blocked during play, START+SELECT Pause, and protection after resume without recreating the CPU. The generic GAME A/B UI test and six-title save/load/corruption checks passed. These are local tests, not console tests.

## Upstream reference and Native Test

Reference: https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds/tree/f273025cb372c2924e176c04ef821abafd080a17

The original `source/virtual_i_o/3ds_input.cpp` forwards held START/SELECT to GAME A/B. The Android `yokoi_input_mapping.cpp` does likewise. The original CEND instruction in `source/SM5XX/SM5XX_instruction.cpp` enters low-power sleep, with native wake on input or the one-second clock. The existing v1.4 observation counter tracks those entries without changing the ROM, CPU execution, or saved-state format.

The RC4 Native Test found 42 unchanged RAM/LCD comparisons and 14 differences. Its report explicitly distinguished a difference from proof of a reset and asked for console classification. The user's successful replay tests are preserved. The subsequent report of active-play resets on the single-GAME family is the reason for this targeted fix. No single-GAME SELECT alias is restored.

## RGDS startup limitation

See `COMPATIBILITY_v1.4.1.md`. v1.3, v1.4 and v1.4.1 start successfully under the same ARM64 emulation runtime. Dependency versions, loader and ELF load alignment match. **The reported RGDS SIGBUS/exit 135 has not been reproduced or demonstrated fixed.** QEMU with dummy SDL audio/video does not emulate the console's Wayland/graphics/audio drivers. Physical RGDS confirmation remains outstanding. The new early startup log helps distinguish an executable/loader failure from an SDL initialization failure. No automatic fallback or unrelated audio/display changes were introduced.
