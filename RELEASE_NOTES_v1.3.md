# Yokoi RGDS / RGDS+ — v1.3

## English

Native Linux ARM64 update for existing working Stock OS installations.

### Changes

- Advanced options now starts with **Segments** and **Background color**, followed by Language, Configure buttons and Back. All labels and values are available in PT/EN.
- Segments: None, Light, Dark, with the upstream unlit-segment opacity values 0, 5 and 10.
- Background color: Light yellow, Light blue, Greyish, White, Light green, Cream. Choices are saved in `display.cfg`. Mask games preserve their original dark background; opaque artwork is not recolored.
- L2/R2 selects the previous/next nonempty manufacturer in the library: Nintendo, Tronica or Elektronika. D-pad, touch and L1/R1 stay within the selected manufacturer. L2/R2 continues to swap game screens during gameplay.
- Choose another game restores the active game's manufacturer and selected row, keeps that game visible, and waits for held controls to be released before library navigation resumes.
- New profiles start in English. Existing PT/EN language preferences are retained.
- SDL/ALSA audio negotiation handles channels, frequency and sample format, with a stereo retry and matching conversion/prefill. Audio on RGDS has been confirmed by the user.
- Physical volume keys retain the curve `0.35 × (level/10)²`. The system volume recorded on entry is restored on exit, with cleanup in both the application and launcher.
- Game margins are painted black while preserving aspect ratio. A report of fullscreen/no visible bars on RGDS remains deferred for investigation; this release does not claim to resolve that model-specific report.

### Installation

Extract `Yokoi_RGDS_RGDSPlus_v1.3_Update.zip`. Copy its `Roms` folder to the SD-card root and replace the application files. Keep the existing `Yokoi_RGDSPlus/lib` folder, your `yokoi_pack_rgds.ykp`, configuration files and `saves`.

Use the same `Yokoi.sh` launcher and `Yokoi_RGDSPlus` directory on both models. No ROM pack or runtime libraries are included. Fresh installations require a compatible ARM64 runtime, as documented in `linux/README.md`.

### Validation

Local tests passed for display settings/PT-EN persistence, manufacturer navigation, game-area black margins and segment opacity, audio conversion after forced mono rejection, physical input/volume, system-volume restoration, and library return through physical buttons and touch with a long multi-manufacturer list. The wider UI regression covered calibration, language changes, touch, screen swaps, save/load, deletion and exit. The library-return change has local test coverage; separate hardware confirmation of that final change has not been recorded.

Built with GCC 13.3, glibc 2.39 cross headers and SDL2 2.30 headers/library; SDL2 is loaded from Stock OS at runtime. Highest required symbol versions: GLIBC_2.38 and GLIBCXX_3.4.31.

## Português

Atualização Linux ARM64 para instalações Stock OS já funcionais.

### Alterações

- Opções avançadas: **Segmentos**, **Cor de fundo**, Idioma, Definir botões e Voltar, com tradução PT/EN.
- Segmentos: Nenhum, Claros e Escuros, com as opacidades do original. Seis cores de fundo, guardadas em `display.cfg`.
- L2/R2 alterna entre fabricantes na biblioteca; mantém a troca de ecrãs durante o jogo. Direcional, toque e L1/R1 navegam dentro do fabricante.
- Escolher outro jogo regressa ao mesmo jogo e fabricante, mantendo a seleção visível e aguardando a libertação dos botões.
- Primeira utilização em inglês; preferências PT/EN existentes são respeitadas.
- Compatibilidade áudio SDL/ALSA corrigida; som na RGDS confirmado pelo utilizador.
- Botões físicos de volume com curva `0,35 × (nível/10)²`; reposição do volume inicial do sistema à saída.
- Margens dos jogos pintadas de preto, mantendo as proporções. O relato adicional de fullscreen/ausência de barras na RGDS fica para investigação posterior; esta versão não apresenta esse caso específico como resolvido.

### Instalação

Extrair `Yokoi_RGDS_RGDSPlus_v1.3_Update.zip`. Copiar a pasta `Roms` para a raiz do cartão e substituir os ficheiros. Manter `lib`, o pack, as configurações e os saves. Usar o mesmo launcher `Yokoi.sh` e a mesma pasta `Yokoi_RGDSPlus` em ambos os modelos.

Não inclui pack nem bibliotecas de runtime. Passou nos testes locais; o regresso à lista ainda não tem confirmação física registada.
