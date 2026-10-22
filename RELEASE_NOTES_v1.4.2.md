# Yokoi RGDS / RGDS+ — v1.4.2

## English

- Tropical Fish: corrected the reversed GAME A/B mapping. START and Pause GAME A now select GAME A; SELECT and Pause GAME B select GAME B.
- Mario The Juggler: START and SELECT are blocked during an active run. After the three native MISS indicators, a fresh START selects GAME A and a fresh SELECT selects GAME B. Holding a button through Game Over does not restart automatically. Explicit Pause GAME actions remain available.
- Retains v1.4.1 protection on Balloon Fight, Climber and Super Mario Bros., with START only for their single GAME button.

Validated with the supplied ROMs: both modes, active-reset reproduction and prevention, held/late presses, replay at multiple points in the end animation, load/resume and Pause actions. ARM64 emulation and virtual-controller UI checks passed. The other 48 ROMs retained identical CPU states in the native-controls comparison.

Install: close Yokoi, extract the Update ZIP and copy its Roms folder to the SD-card root, replacing application files. Keep lib, the ROM pack, settings and saves. No ROMs or runtime libraries are included.

Known limitation: the previously reported RGDS startup Bus error (exit 135) is still unconfirmed. This release does not claim to fix it.

## Português

- Tropical Fish: corrigida a inversão de GAME A/B. START e GAME A na Pausa selecionam GAME A; SELECT e GAME B na Pausa selecionam GAME B.
- Mario The Juggler: START e SELECT ficam bloqueados durante a partida. Depois das três falhas indicadas pelo jogo, uma nova pressão de START inicia GAME A e SELECT inicia GAME B. Manter o botão premido até ao fim não inicia outra partida automaticamente. As ações explícitas GAME da Pausa continuam disponíveis.
- Mantida a proteção da v1.4.1 em Balloon Fight, Climber e Super Mario Bros., usando apenas START para o único botão GAME.

Validação com as ROMs fornecidas: ambos os modos, reprodução e bloqueio do reset, pressões mantidas e perto do fim, nova partida em vários momentos da animação final, carregar/continuar e ações da Pausa. Passaram os testes em ARM64 emulado e na interface com comando virtual. As outras 48 ROMs mantiveram estados do CPU idênticos na comparação dos comandos nativos.

Instalação: fechar Yokoi, extrair o ZIP Update e copiar a pasta Roms para a raiz do cartão, substituindo os ficheiros da aplicação. Manter lib, pack, configurações e saves. Não inclui ROMs nem bibliotecas.

Limitação conhecida: o Bus error (código 135) anteriormente reportado no arranque da RGDS continua por confirmar. Esta release não é apresentada como correção desse erro.
