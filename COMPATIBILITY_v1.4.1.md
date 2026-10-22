# Compatibility comparison — v1.4.1

## Português

A base desta correção é a v1.4 publicada. A v1.3 foi usada como referência de compatibilidade, sem retirar as funcionalidades da v1.4.

Os ZIPs locais da v1.3 correspondem aos SHA256 publicados em https://github.com/PolluxPT/Yokoi-RGDSPlus/releases/tag/v1.3 . A comparação confirmou:

- Mesmo compilador GCC 13.3 e arquitetura Linux ARM64.
- Mesmo interpretador `/lib/ld-linux-aarch64.so.1` e alinhamento dos segmentos ELF de 65536 bytes.
- Mesmas bibliotecas requeridas e mesmos requisitos de versões de símbolos: até GLIBC_2.38 e GLIBCXX_3.4.31.
- O launcher da v1.3 e o da v1.4 diferem apenas na identificação da versão. A v1.4.1 conserva essa configuração.
- Mantidos o arranque Wayland/ALSA, negociação do formato de áudio, curva de volume e reposição do volume inicial.
- Os executáveis completos v1.3, v1.4 e v1.4.1 carregaram, abriram SDL, carregaram um pack sintético, executaram a emulação e terminaram com código 0 no mesmo ambiente ARM64 emulado (QEMU Cortex-A53, SDL dummy).

Limite: não foi reproduzido o Bus error/código 135 reportado na RGDS. Estes testes não validam os drivers físicos Wayland/áudio dessa consola e não demonstram que o erro foi corrigido. A confirmação numa RGDS permanece pendente. A versão inclui uma mensagem antes da primeira chamada SDL para ajudar a localizar uma eventual falha no arranque.

## English

The code base is published v1.4; v1.3 is the compatibility reference. GCC 13.3, AArch64, dynamic loader, 64 KiB ELF load alignment, library dependencies and required symbol versions match. The launcher retains the v1.3 environment and volume restoration. Full v1.3, v1.4 and v1.4.1 binaries each loaded SDL, ran a synthetic pack and exited with status 0 under the same QEMU Cortex-A53 ARM64 runtime with dummy audio/video.

The reported physical RGDS SIGBUS/exit 135 has not been reproduced or demonstrated fixed. Emulation does not validate the console's Wayland/audio drivers. An early startup log is included to locate a failure more precisely. Physical RGDS confirmation is still outstanding.
