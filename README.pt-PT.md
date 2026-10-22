# Yokoi: Game & Watch Emulator — Edição RGDS / RGDS+

<img src="assets/Yokoi.png" alt="Yokoi — Game & Watch Emulator" width="350">

[![Descarregar](https://img.shields.io/badge/Descarregar-v1.4-blue?style=for-the-badge)](https://github.com/PolluxPT/Yokoi-RGDSPlus/releases/latest)

**[English](README.md) | [Português](README.pt-PT.md)**

Adaptação nativa e standalone do Yokoi para **Anbernic RGDS / RGDS+ com Stock OS, Linux ARM64**. Versão **1.4**.

Emulador original: [RetroValou / Yokoi](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds), revisão de base `f273025`. Projeto de adaptação: **Filipe Baeta**, desenvolvido com assistência do ChatGPT/Codex. Esta é uma aplicação Linux, não o APK Android original.

## Novidades da 1.4

- **Comandos GAME nativos:** START envia GAME A e SELECT envia GAME B quando existe. A ROM gere o início e a nova partida; foi retirado o filtro adicional de Game Over.
- **Máquinas com um único botão GAME:** usar START. Aplica-se às variantes de Climber e Balloon Fight; SELECT não é uma alternativa ao botão GAME.
- Os jogos abrem em modo relógio. Soltar o botão usado para abrir o jogo antes de premir START ou SELECT para jogar.
- **Segmentos** e **Cor de fundo** na Pausa, imediatamente depois de GAME B. Alteração durante o jogo, pré-visualização no ecrã inferior e valores guardados.
- Sincronização do relógio com a hora local do Linux, segundo os mapas por jogo do original.
- Último jogo aberto e fabricante memorizados entre arranques da aplicação.

Ver [notas completas PT/EN](RELEASE_NOTES_v1.4.md).

## Funcionalidades

- Dois ecrãs SDL2, navegação por toque e calibração dos controlos.
- Menus PT/EN; inglês na primeira utilização, respeitando preferências existentes.
- L1/R1 para navegar por inicial; L2/R2 para fabricantes e troca dos ecrãs durante o jogo.
- Pausa com R calibrado ou START+SELECT.
- Três slots manuais por jogo, gestão de saves e confirmação para sair.
- Compatibilidade áudio RGDS e botões físicos de volume com ganho `0,35 × (nível/10)²`.
- Volume da consola registado à entrada e reposto à saída.

## Instalação

Descarregar **`Yokoi_RGDS_RGDSPlus_v1.4_Update.zip`** em [Releases](https://github.com/PolluxPT/Yokoi-RGDSPlus/releases/latest).

1. Fechar o Yokoi e extrair o ZIP de atualização.
2. Copiar `Roms` para a raiz do cartão, substituindo os ficheiros da aplicação.
3. Manter `Yokoi_RGDSPlus/lib`, pack, configurações e `saves`.
4. Abrir o mesmo `Yokoi.sh` em APPS. Não é necessário criar outro launcher nem recalibrar, salvo indicação da aplicação.

É uma atualização para uma instalação Linux já funcional. Não inclui bibliotecas nem pack. Uma instalação nova necessita de runtime ARM64 compatível. O binário exige SDL2 e símbolos até GLIBC_2.38 / GLIBCXX_3.4.31; ver [notas Linux](linux/README.md).

## Controlos

| Ecrã | Controlo | Ação |
|---|---|---|
| Biblioteca | Direcional / deslizar no touch | Escolher jogo |
| Biblioteca | A / START / dois toques | Abrir em modo relógio |
| Biblioteca | Y | Carregar um slot ocupado |
| Biblioteca | L1 / R1 | Inicial anterior / seguinte |
| Biblioteca | L2 / R2 | Fabricante anterior / seguinte |
| Biblioteca | X | Opções avançadas: Idioma, Definir botões, Voltar |
| Biblioteca | R | Confirmação de saída |
| Relógio / Fim da partida | START | GAME A ou o único botão GAME |
| Relógio / Fim da partida | SELECT | GAME B, apenas quando existe na máquina original |
| Durante o jogo | START / SELECT | Comportamento da ROM original |
| Durante o jogo | R ou START+SELECT | Pausa |
| Durante o jogo | L2 / R2 | Trocar ecrãs |
| Pausa | GAME A / GAME B; A | Nova partida explícita, quando suportada |
| Pausa | Segmentos / Cor de fundo | A ou Esquerda/Direita; pré-visualização no ecrã inferior |
| Pausa | Cima / Baixo; A; B | Escolher; confirmar; continuar |
| Pausa | Esquerda / Direita | Escolher slot |
| Gestão de slots | Direcional; A; B | Escolher slot; pedir para apagar; voltar |

START+SELECT continua reservado à Pausa. GAME A/B mantém os nomes do hardware original nos dois idiomas. Se a máquina tem um único GAME, usa apenas START: não é criado GAME B nem uma alternativa SELECT→START.

## Criar o seu pack de jogos

**Não são distribuídos ROMs, packs, artwork original dos jogos nem saves pessoais.**

Seguir o [guia do Yokoi original — alvo Android (RGDS)](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds/blob/main/CONVERT_ROM/README.md#android-rgds). Depois de cumprir os pré-requisitos, dentro de `CONVERT_ROM`:

```sh
python convert_original.py --target rgds
python convert_3ds.py --target rgds
```

Colocar o seu `yokoi_pack_rgds.ykp` em `Roms/APPS/Yokoi_RGDSPlus/`. O pack 3DS não é suportado. Não enviar conteúdo dos jogos para este repositório.

## Validação e limites conhecidos

Os testes locais abrangem encaminhamento dos comandos, libertação do botão de abertura, Pausa, interface com comando virtual e save/load/reprodução com recuperação após ficheiros inválidos. A comparação controlada abrangeu 56 ROMs. O utilizador confirmou testes bem-sucedidos na consola dos 14 títulos/variantes selecionados para verificação adicional; ver [registo de validação](VALIDATION_v1.4.md). A configuração final do botão único usa apenas START, conforme esse feedback.

A release segue as regras nativas de cada ROM; não acrescenta um bloqueio universal de reinício durante a partida. O relógio depende dos mapas por título do original. O relato anterior de fullscreen/barras na RGDS continua adiado e não é apresentado como resolvido nesta versão.

## Compilar

Em Debian/Ubuntu:

```sh
sudo apt install g++ libsdl2-dev
bash linux/build.sh
```

É criado `linux/build/yokoi` para a arquitetura do computador. Para a consola, compilar em ARM64 ou usar as [variáveis de compilação cruzada](linux/README.md). `android/app/src/main/cpp` contém auxiliares CPU/input partilhados, não a aplicação Android completa.

## Créditos e notas

- Yokoi original: [RetroValou](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds).
- Créditos originais: MAME, projetos FPGA Game & Watch de Adam Gastineau, documentação Sharp e ABerguerand.
- PNG: LodePNG, copyright Lode Vandevenne.
- Fonte: DejaVu Sans, com avisos em `linux/FONT-LICENSE.txt`.

O README original descreve o código como Public Domain / Free to use. São conservados os avisos dos componentes de terceiros; ver [NOTICES.pt-PT.md](NOTICES.pt-PT.md). Projeto não oficial, sem afiliação à Nintendo ou Anbernic.
