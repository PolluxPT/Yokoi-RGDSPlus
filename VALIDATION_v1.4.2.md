# v1.4.2 validation

## Scope and evidence

Tropical Fish TF_104 had GAME A and B reversed in the inherited upstream input mapping. MAME's original-machine wiring agrees with the user's report: IN.1 bit 0x02 is GAME A, bit 0x04 is GAME B. The change is confined to TF_104 in virtual_input.h. Tests inspect the actual ROM-driven LCD labels: GAME A at (1,5,0), GAME B at (0,5,0), for physical commands and explicit Pause actions.

Reference: https://github.com/mamedev/mame/blob/master/src/mame/handheld/hh_sm510.cpp (gnw_tfish input ports).
Upstream Yokoi reference: f273025cb372c2924e176c04ef821abafd080a17.

Mario The Juggler MB_108 accepts GAME A/B during play. Unlike the single-GAME family, it continues animating after Game Over and does not execute CEND in the observed end sequence. Its native MISS icons are (1,5,0), (0,6,0), (1,6,0); the Watch AM/PM indicators are (0,11,0), (0,12,3). The isolated MB_108 guard uses those displayed states. After a start request, the old terminal display must clear before a new game end is recognized. A 250 ms native pulse reliably reaches both GAME modes; press eligibility is sampled before the chord grace so a press made during play cannot be queued across Game Over. No ROM bytes or save format are modified.

## Completed checks

- juggler_tropical.cpp: both modes at polling strides of 32 and 256 cycles. Raw START and SELECT reproduce in-game state divergence; the protected run's complete saved CPU state matches a no-button reference.
- Both physical and explicit Pause GAME commands light the correct Tropical Fish GAME label.
- Mario: held presses across the final miss; late presses before it; replay into A and B from a loaded terminal state, tested at 16 offsets separated by 125 ms; explicit Pause GAME actions.
- The same regression executable passed as ARM64 under QEMU Cortex-A53.
- physical_juggler_ui.cpp: real ROM plus SDL virtual controller; initial Watch, fresh START, blocked START/SELECT while playing, Pause and resume without recreating the CPU.
- Existing single-GAME tests passed for all six variants. The other 48 ROMs match the native v1.4 CPU state comparison (excluding the two titles corrected here).
- The final ARM64 application loaded the actual Mario The Juggler pack, SDL dummy video/audio, rendered, emulated and exited 0 under QEMU.
- Archive CRCs, source/binary contents and SHA256 assets verified at packaging.

Physical console confirmation of these fixes remains pending. The existing RGDS startup SIGBUS is not claimed fixed. See COMPATIBILITY_v1.4.2.md.
