# Yokoi RGDS / RGDS+ — v1.4.1

## Português

Corrige a regressão da v1.4 nas máquinas com um único botão GAME: Balloon Fight, Climber e Super Mario Bros. (Crystal Screen e New Wide Screen).

- START inicia a partir do relógio e permite nova partida após o fim.
- Durante a partida, START fica bloqueado e não reinicia o jogo.
- Manter START premido até ao fim não inicia automaticamente outra partida. Soltar e voltar a premir.
- SELECT continua sem função GAME nestas máquinas.
- As máquinas GAME A/B, incluindo Donkey Kong, mantêm o tratamento da v1.4.
- START+SELECT para Pausa e as ações explícitas GAME da Pausa mantêm-se disponíveis.

Copiar a pasta `Roms` do Update ZIP para a raiz do cartão, com o Yokoi fechado, substituindo os ficheiros. Manter pack, bibliotecas, saves e configurações. Atualização para instalação Linux Stock OS funcional; sem ROMs ou bibliotecas incluídas.

Os testes locais passaram nas seis variantes, com comparação do estado completo do CPU e verificação de nova partida. O binário foi comparado com a v1.3 publicada e executado em ARM64 emulado. O Bus error reportado na RGDS ainda não foi reproduzido nem demonstrado resolvido; a confirmação física permanece pendente. A questão das barras na RGDS continua adiada.

## English

Fixes the v1.4 reset regression on single-GAME machines: Balloon Fight, Climber and Super Mario Bros. (Crystal Screen and New Wide Screen).

- START begins from Watch and starts another run after native game end.
- START is ignored during a run, preventing accidental resets.
- Holding START through game end does not restart automatically. Release and press again.
- SELECT remains unmapped to GAME on these machines.
- GAME A/B machines, including Donkey Kong, retain v1.4 input handling.
- START+SELECT Pause and explicit Pause GAME actions are retained.

With Yokoi closed, copy `Roms` from the Update ZIP to the SD-card root and replace application files. Keep the existing pack, runtime libraries, saves and settings. This updates a working Linux Stock OS installation; ROMs and runtime libraries are not included. ARM64 binary: SDL2, GLIBC_2.38 / GLIBCXX_3.4.31 maximum required symbols.

Local full-CPU-state comparison and replay tests passed on all six variants. The binary was compared with published v1.3 and executed under ARM64 emulation. The reported RGDS Bus error has not been reproduced or demonstrated fixed; physical confirmation remains pending. The previously deferred RGDS black-bars issue is unchanged.
