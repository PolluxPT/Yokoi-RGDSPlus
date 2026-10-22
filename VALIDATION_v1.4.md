# Validation record — v1.4

## Console feedback

The user confirmed successful tests of the native-controls build on RGDS+ for:

1. Balloon Fight — Crystal Screen (single GAME; START)
2. Balloon Fight — New Wide Screen (single GAME; START)
3. Bomb Sweeper (including GAME B)
4. Boxing
5. Climber — Crystal Screen
6. Climber — New Wide Screen
7. Donkey Kong 3
8. Donkey Kong Hockey
9. Fire — Silver
10. Gold Cliff
11. Judge — purple version
12. Mario The Juggler
13. Safe Buster
14. Zelda

The final requested mapping removes the temporary Climber SELECT alternative. Both Climber and Balloon Fight variants use START for the sole GAME button. This final mapping is checked locally against the actual virtual-input configurations; it does not add an artificial GAME B mode.

## Local verification

- Native setup transport: opening-button release; repeated A/B without Game Over profiles; short taps; Pause chord; no SELECT-to-START substitution; explicit Pause start.
- Four single-GAME mappings: `DR_106`, `DR_802`, `BF_107`, `BF_803`; SELECT does not press the hardware GAME bit and START does.
- Virtual-controller UI: holding START to open an entry leaves Watch; a fresh START reaches the ROM; SELECT is forwarded during play; Pause and resume preserve the CPU instance.
- Save/load/replay and corruption rollback previously passed with the native frontend across Ball, Donkey Kong, Mario The Juggler, Mickey & Donald, Squish and Zelda. The final change does not alter save serialization.
- The controlled 56-ROM comparison found unchanged RAM/LCD after native setup presses in 42 scenarios and differences in 14; those 14 were then checked successfully on the console. This comparison is a specific scenario, not proof of every possible game state or ROM revision.
- Final ARM64 package: correct executable architecture/version, matching runtime symbol requirements, shell syntax, source/package equality, ZIP integrity and SHA-256 manifests.

Earlier 1.4 work verified Pause display settings, Watch clock synchronization, fresh-process last-game persistence, PT/EN UI and preserved volume restoration. RGDS sound compatibility had already been confirmed by the user. The deferred RGDS fullscreen/black-bars report is not included as a resolved issue.

## Português

O utilizador confirmou os 14 testes na consola acima. A versão final mantém o comportamento nativo e segue a indicação posterior: se existe apenas um botão GAME, usa START. A alternativa SELECT do Climber foi retirada e a configuração final foi verificada localmente. Não é introduzido um bloqueio universal de reinício durante a partida; as regras são as da ROM original.
