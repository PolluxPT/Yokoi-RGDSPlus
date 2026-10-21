# Yokoi: Game & Watch Emulator — RGDS+ Edition

  <img src="assets/Yokoi.png"  alt="Yokoi — Game & Watch Emulator for RGDS+" width="350">

[![Descarregar](https://img.shields.io/badge/Descarregar-v1.2-blue?style=for-the-badge)](https://github.com/PolluxPT/Yokoi-RGDSPlus/releases/latest)

**ROMs não incluídas.** Cria o teu pack seguindo as instruções do projeto original.

**[English](README.md) | [Português](README.pt-PT.md)**

Adaptação nativa e standalone do Yokoi para **Anbernic RGDS+ com Stock OS
(Linux ARM64)**. Versão **1.2**. Esta é uma versão Linux; não é o APK Android
do projeto original.

Emulador original de **RetroValou**: [projeto Yokoi](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds).
Projeto de adaptação à RGDS+: **Filipe Baeta**, desenvolvido com assistência
do ChatGPT/Codex. Revisão original de base: `f273025`.

> **Versão 1.2 — L2/R2 corrigidos:** troca de ecrãs confirmada pelo responsável do projeto na sua RGDS+ com Stock OS. Na primeira abertura após atualizar da v1.1, calibrar L2/R2; os restantes botões mantêm a configuração. A calibração da 1.2 RC1 também é conservada. Ver [notas PT/EN](RELEASE_NOTES_v1.2.md).

## ROMs não incluídas

O repositório e as releases **não incluem ROMs, packs de jogos, artwork original
dos jogos ou saves pessoais**. Cada utilizador deve fornecer o seu próprio pack
compatível. Não são disponibilizadas ligações para descarregar jogos.

## Criar o seu pack de jogos — projeto original

As ferramentas e instruções para preparar o pack estão no projeto de RetroValou:

- [GitHub original do Yokoi](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds)
- [Guia CONVERT_ROM — secção Android (RGDS)](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds/blob/main/CONVERT_ROM/README.md#android-rgds)

Foi através desse projeto que foi criado o pack utilizado nos testes desta
adaptação. Para esta versão Linux, seguir a secção **Android (RGDS)** e gerar
**`yokoi_pack_rgds.ykp`**. Depois de cumprir os pré-requisitos do guia original,
os comandos documentados, dentro da pasta `CONVERT_ROM`, são:

```sh
python convert_original.py --target rgds
python convert_3ds.py --target rgds
```

Copiar o resultado para `Roms/APPS/Yokoi_RGDSPlus/yokoi_pack_rgds.ykp` no cartão.
**O pack 3DS não é compatível. Não enviar o pack gerado para este repositório.**

## Funcionalidades

- Dois ecrãs SDL2 e fundos dos Panorama Screen corrigidos.
- Interface PT/EN, menus azuis, opções avançadas e calibração dos botões.
- Touch: deslizar para escolher e dois toques para GAME A.
- L1/R1 para saltos por letra e L2/R2 para trocar os ecrãs durante o jogo.
- Botão R calibrado para Pausa, com alternativa START+SELECT.
- Três slots manuais por jogo e gestão para apagar estados.
- Confirmação para apagar/sair e novos GAME A/B através da pausa.
- Botões de volume e curva suave nos níveis iniciais.
- Carregamento das imagens de emulação apenas ao entrar no jogo.

## Instalação — pacote de atualização

A release v1.2 é uma **atualização para uma instalação Linux RGDS+ já funcional**.
Não inclui as bibliotecas de execução nem o pack de jogos.

1. Extrair o ZIP da release e copiar `Roms` para a raiz do cartão, substituindo os ficheiros da aplicação.
2. Conservar `Yokoi_RGDSPlus/lib`, pack, configurações e pasta `saves` existentes.
3. Se necessário, colocar o seu `yokoi_pack_rgds.ykp` junto ao executável `yokoi`.
4. Abrir Yokoi em APPS e seguir a calibração, caso seja solicitada.

Uma instalação nova necessita também das bibliotecas ARM64 compatíveis. O binário
usa SDL2 e exige versões de símbolos até GLIBC_2.38 e GLIBCXX_3.4.31. Essas
bibliotecas **não são distribuídas nesta release**. Ver `linux/README.md`.

## Controlos

| Ecrã | Controlo | Ação |
|---|---|---|
| Lista de jogos | Direcional / deslizar no touch | Escolher jogo |
| Lista de jogos | A / START / dois toques | GAME A |
| Lista de jogos | SELECT | GAME B |
| Lista de jogos | Y | Carregar um slot ocupado |
| Lista de jogos | L1 / R1 | Letra anterior / seguinte |
| Lista de jogos | X | Opções avançadas: idioma, botões e voltar |
| Lista de jogos | R | Sair, com confirmação |
| Durante o jogo | R ou START+SELECT | Pausa |
| Durante o jogo | L2 / R2 | Trocar ecrãs |
| Pausa | Cima / Baixo; A; B | Escolher; confirmar; continuar |
| Pausa | Esquerda / Direita | Escolher slot |
| Gerir slots | Direcional; A; B | Escolher slot; pedir para apagar; voltar |

GAME A e GAME B mantêm os nomes do hardware original nos dois idiomas.
Se o firmware não expuser R, manter START+SELECT nessa etapa permite ignorar a
sua calibração.

## Compilar o executável Linux

Em Debian/Ubuntu, instalar compilador C++20 e desenvolvimento SDL2:

```sh
sudo apt install g++ libsdl2-dev
bash linux/build.sh
```

O resultado é `linux/build/yokoi`, para a arquitetura do computador de compilação.
Para a RGDS+, compilar num sistema ARM64 ou usar as variáveis de compilação
cruzada documentadas em [`linux/README.md`](linux/README.md).
O código em `android/app/src/main/cpp` contém funções CPU/input partilhadas com
esta versão Linux; o repositório não contém a aplicação Android completa.

## Créditos e notas

- Emulador original: [RetroValou](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds).
- Créditos originais: MAME, projetos FPGA Game & Watch de Adam Gastineau,
  documentação Sharp e ABerguerand. Ver [NOTICES.pt-PT.md](NOTICES.pt-PT.md).
- PNG: LodePNG, copyright Lode Vandevenne, com aviso original conservado.
- Fonte: DejaVu Sans, licença em `linux/FONT-LICENSE.txt`.

O README original descreve o código como **Public Domain / Free to use**.
Os componentes de terceiros conservam as suas próprias licenças e avisos.
Não é aplicada uma nova licença global a esses componentes. Projeto não oficial,
sem afiliação à Nintendo ou Anbernic. Os nomes e marcas identificam o hardware
e os sistemas emulados, não constituindo uma alegação de apoio dessas entidades.
