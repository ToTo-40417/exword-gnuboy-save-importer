# Gnuboy Save Importer for EX-word

日本語 | [English](#english)

PC上のGame Boyセーブデータを、CASIO EX-word版Gnuboyの保存場所へ安全に戻すためのhomebrewツールです。セーブデータを個人用D01へ埋め込み、本体上でバックアップ、書込み、全バイト再読込検証を行います。

- APPID: `SAVWR`
- 書込み前に本体キーで確認
- 既存saveを`.bak`へ退避
- 対応サイズを検査
- 書込み後に全内容を照合
- ROMやsaveをリポジトリへ保存しない

```bash
export DEVKITPRO="$HOME/toolchains/devkitPro"
export DEVKITSH4="$DEVKITPRO/devkitSH4"
export PATH="$DEVKITPRO/tools/bin:$DEVKITSH4/bin:$PATH"
make SAVE_FILE=/path/to/game.sav SAVE_TARGET=game.sav SAVE_DRIVE=drv0
```

自分で合法的に取得したセーブデータだけを使用してください。生成される個人用payloadとD01はGitへ追加しないでください。書込み中は電源やUSBを切断しないでください。

GPL-2.0。ROM、save、CASIO firmware、端末認証情報は配布物に含みません。

## English

Gnuboy Save Importer restores a PC-side Game Boy save to Gnuboy on a CASIO EX-word. It embeds the save into a personal D01, then backs up the existing file, writes the payload, reads it back, and verifies every byte on-device.

- APPID: `SAVWR`
- Requires an explicit key confirmation before writing
- Backs up the existing save as `.bak`
- Validates supported SRAM sizes
- Performs full post-write verification
- Never stores ROMs or saves in the repository

Use only save data you legally obtained yourself. Never commit the generated personal payload or D01. Do not remove power or disconnect USB during a write.

GPL-2.0. ROMs, save files, CASIO firmware, and device authentication data are not distributed.
