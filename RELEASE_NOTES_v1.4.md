# Yokoi RGDS / RGDS+ — v1.4

## English

- Restored native GAME A/B handling. START sends GAME A and SELECT sends GAME B only where the original machine provides it; the ROM handles starting and replaying. The additional frontend Game Over filter is removed.
- Machines with one GAME button use START only. The temporary Climber SELECT-to-START alias is removed. Climber and Balloon Fight therefore follow the original single-button layout.
- Opening a game leaves its native Watch/clock mode running. The button used to open it must be released before a fresh setup press is accepted. START+SELECT remains reserved for Pause; calibrated R also opens Pause.
- Segments and Background colour moved to Pause, immediately after GAME B. A or Left/Right changes the value during the current session, with a lower-screen preview and persistent settings.
- Linux local clock synchronization uses upstream per-title clock mappings during initialization and Watch. Active games and loaded saves are not continuously overwritten with host time.
- The last opened game and manufacturer persist across application restarts and are restored when returning to the library.
- PT/EN interface with English on first use; existing language settings are kept. Manufacturer selection with L2/R2, save slots, calibrated controls and screen swaps are retained.
- RGDS audio compatibility, physical volume keys with `0.35 × (level/10)²`, and restoration of the console's entry volume are retained.

### Installation

Extract `Yokoi_RGDS_RGDSPlus_v1.4_Update.zip`. Close Yokoi, copy `Roms` to the SD-card root and replace application files. Keep `Yokoi_RGDSPlus/lib`, your `yokoi_pack_rgds.ykp`, settings and `saves`. Use the same `Yokoi.sh` launcher.

This is an update for an existing working Linux Stock OS installation. ROMs, packs, runtime libraries, personal saves and test artwork are not included. Fresh installations need a compatible ARM64 runtime. Required symbols reach GLIBC_2.38 and GLIBCXX_3.4.31.

### Validation

The user confirmed successful console tests of all 14 titles/variants selected for additional checking after a controlled 56-ROM comparison. Final local checks cover native input routing, the START-only mapping on both Climber and Balloon Fight variants, opening-button release, Pause, virtual-controller UI and save compatibility. See [validation details](VALIDATION_v1.4.md).

GAME buttons follow the original ROM behaviour; no universal in-game reset lock is added. The previously reported RGDS fullscreen/black-bars behaviour remains deferred. This release does not claim to resolve it.

## Português

- Reposto o tratamento nativo de GAME A/B. START envia GAME A e SELECT envia GAME B apenas quando existe na máquina original. A ROM gere o início e a nova partida; foi retirado o filtro adicional de Game Over.
- Máquinas com um único GAME usam apenas START. Retirada a alternativa temporária SELECT→START do Climber; Climber e Balloon Fight seguem a disposição original de um botão.
- Os jogos abrem em relógio. Soltar o botão usado para abrir o jogo antes de uma nova pressão para iniciar. START+SELECT continua reservado à Pausa; R calibrado também abre a Pausa.
- Segmentos e Cor de fundo na Pausa, imediatamente depois de GAME B. Alteração durante o jogo com A ou Esquerda/Direita, pré-visualização inferior e valores guardados.
- Relógio com hora local do Linux segundo os mapas por título do original, sem reescrever continuamente partidas em curso ou saves carregados.
- Último jogo e fabricante memorizados entre arranques e no regresso à biblioteca.
- Mantêm-se PT/EN, inglês na primeira utilização, L2/R2 para fabricantes, slots, calibrações e troca de ecrãs.
- Mantêm-se som RGDS, botões físicos de volume com curva `0,35 × (nível/10)²` e reposição do volume inicial da consola à saída.

### Instalação

Extrair `Yokoi_RGDS_RGDSPlus_v1.4_Update.zip`. Com o Yokoi fechado, copiar `Roms` para a raiz do cartão e substituir os ficheiros da aplicação. Manter `Yokoi_RGDSPlus/lib`, pack, configurações e `saves`. Usar o mesmo `Yokoi.sh`.

É uma atualização para uma instalação Linux Stock OS já funcional. Não inclui ROMs, pack, bibliotecas, saves pessoais ou imagens de teste. Uma instalação nova necessita de runtime ARM64 compatível.

### Validação

O utilizador confirmou testes bem-sucedidos na consola dos 14 títulos/variantes selecionados para verificação adicional após a comparação de 56 ROMs. Os testes locais finais verificam os comandos nativos, o GAME apenas por START nas variantes de Climber e Balloon Fight, libertação do botão de abertura, Pausa, interface com comando virtual e compatibilidade dos saves. Ver [registo de validação](VALIDATION_v1.4.md).

Os botões seguem o comportamento da ROM original; não é acrescentado um bloqueio universal de reinício durante a partida. A questão anteriormente adiada de fullscreen/barras na RGDS mantém-se pendente e não é apresentada como resolvida.
