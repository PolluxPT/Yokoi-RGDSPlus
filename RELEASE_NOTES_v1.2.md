# Yokoi RGDS+ Edition v1.2 — L2/R2 fix

## Português

**L2 e R2 corrigidos.** Troca de ecrãs confirmada pelo responsável do projeto
na sua RGDS+ com Stock OS, após testar a 1.2 RC1. A versão final mantém a mesma
lógica; apenas a identificação e a documentação foram atualizadas.

- Calibração própria de L2/R2, independente do mapeamento automático do SDL.
- Suporte a botões digitais, eixos analógicos e sinais de teclado/evdev.
- Uma troca por pressão, sem repetição enquanto o botão está premido.
- Restantes controlos, saves, interface PT/EN e curva suave do volume conservados.
- Logótipo incluído em `Roms/APPS/Imgs/Yokoi.png`.

### Instalar / atualizar

1. Descarregar **Yokoi_RGDSPlus_v1.2_Update.zip** nos Assets desta release.
2. Extrair e copiar `Roms` para a raiz do cartão, substituindo os ficheiros.
3. Conservar a pasta `lib`, pack de jogos, configurações e `saves` existentes.
4. Ao atualizar da v1.1, seguir os dois passos de calibração de L2 e R2.
   A calibração já guardada na 1.2 RC1 é reutilizada.
5. Durante um jogo, fora da pausa, usar L2 ou R2 para trocar os ecrãs.

Para repetir apenas L2/R2, apagar `swap-rgds-v1.cfg` em
`Roms/APPS/Yokoi_RGDSPlus/`. Se um botão não responder na calibração, manter
START+SELECT permite ignorá-lo; fica desativado até ser configurado.

**Atualização para instalações Linux ARM64 já funcionais em RGDS+ Stock OS.**
Não inclui bibliotecas de execução. Não é uma aplicação Android/GammaOS.
**Sem ROMs, packs de jogos ou saves pessoais.** Criar o próprio pack seguindo o
[guia original de RetroValou — Android (RGDS)](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds/blob/main/CONVERT_ROM/README.md#android-rgds).
Usar `yokoi_pack_rgds.ykp`; o pack 3DS não é compatível.

## English

**L2 and R2 fixed.** Screen swapping confirmed by the project maintainer on
their RGDS+ running Stock OS after testing 1.2 RC1. The final release retains
the same logic; only version identification and documentation have changed.

- Dedicated L2/R2 calibration independent of SDL's automatic controller mapping.
- Digital buttons, analog axes, keyboard and evdev input supported.
- One swap per press, without repeated swaps while holding the trigger.
- Existing controls, saves, PT/EN interface and smooth volume curve retained.
- Logo included at `Roms/APPS/Imgs/Yokoi.png`.

### Install / update

1. Download **Yokoi_RGDSPlus_v1.2_Update.zip** from this release's Assets.
2. Extract and copy `Roms` to the SD-card root, replacing the application files.
3. Keep existing runtime `lib`, game pack, settings and `saves` directory.
4. When upgrading from v1.1, follow the two L2/R2 calibration steps.
   Saved 1.2 RC1 calibration is reused.
5. During active gameplay, outside pause, press L2 or R2 to swap screens.

To repeat only L2/R2 calibration, delete `swap-rgds-v1.cfg` from
`Roms/APPS/Yokoi_RGDSPlus/`. If a trigger does not respond during calibration,
hold START+SELECT to skip it; it stays disabled until configured.

**Update for existing working Linux ARM64 installations on RGDS+ Stock OS.**
Runtime libraries are not included. This is not an Android/GammaOS application.
**No ROMs, game packs or personal saves included.** Create your own pack using
[RetroValou's original guide — Android (RGDS)](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds/blob/main/CONVERT_ROM/README.md#android-rgds).
Use `yokoi_pack_rgds.ykp`; the 3DS pack is not compatible.

Original emulator / Emulador original:
[RetroValou/Yokoi_Game_-_Watch_emulator_3ds](https://github.com/RetroValou/Yokoi_Game_-_Watch_emulator_3ds).
