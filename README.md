# Exult TTS: Ultima VII conversations, books and scrolls read aloud

📄 Project page: **[Exult TTS: Ultima VII conversations, books and scrolls read aloud](https://zirize.github.io/exult-tts/)** · More projects: **[zirize.github.io](https://zirize.github.io/)**

A patch for [Exult](https://exult.info/), the Ultima VII engine, that speaks conversation lines,
books and scrolls with a text-to-speech command — by default [Piper](https://github.com/rhasspy/piper)
through `aplay`. Conversation lines start with the speaker's name (`Iolo: ...`), and every line is
also printed to the terminal as `TTS: <line>`, which gives a readable history of the conversation.

## What it does

- **Conversations**: each page of an NPC's speech is spoken as it appears; clicking on to the next
  page cuts the voice off and starts the next one. The first page is spoken with the NPC's name prefix
  (`Iolo: ...`), and terminal output (`TTS: ...`) includes the speaker name across all pages.
- **Books and scrolls**: the visible pages are read when opened or turned; closing the book stops speech.
- **Signs and tombstones**: rune text on signs, plaques and gravestones is translated with ligature
  expansion and read aloud when examined.
- **Text cleanup**: the game's markup (`*` page breaks, `~`, `^`, `@`, font-mapped bytes) is
  turned into plain sentences.
- **Audio options**: a "Read aloud" toggle with three states (No / Yes / Yes + signs) in the Audio
  options menu, saved to `config/audio/tts/enabled` and `config/audio/tts/signs`. It shows
  "Read aloud (no command)" when the speech command is missing.
- The command runs in its own process group with `posix_spawn` (no shell, so quotes in game text
  are safe) and is stopped with a signal to that group; nothing else on the system is killed.

How it hooks into Exult: [docs/tts_patch.md](docs/tts_patch.md).

## Build

The patch is against upstream Exult `97984e9e0cbb4ccacaab0249da9c51b533818f74` (2026-10-29).
It uses POSIX process calls; on other platforms speech is off.

```sh
git clone https://github.com/exult/exult.git
cd exult
git checkout 97984e9e0cbb4ccacaab0249da9c51b533818f74
git apply /path/to/exult-tts/exult_tts.patch
autoreconf -i && ./configure && make
sudo make install
```

## Speech command

Exult runs `config/audio/tts/command "<line>"` from `exult.cfg`; by default that is
`<DATA>/tts_wrapper` in Exult's data directory (`/usr/local/share/exult` after the install above).
Any executable that speaks its first argument and exits will do.

The included `tts_wrapper` uses Piper:

1. Get the Piper binary from the [Piper releases](https://github.com/rhasspy/piper/releases) and
   unpack it to `~/.local/share/piper/` (so `~/.local/share/piper/piper` exists).
2. Download a voice — both the `.onnx` and the `.onnx.json` file — from
   [piper-voices](https://huggingface.co/rhasspy/piper-voices) into `~/.local/share/piper/models/`.
   The default is `en_GB-jenny_dioco-medium`.
3. Install the wrapper: `sudo install -m 755 tts_wrapper /usr/local/share/exult/tts_wrapper`.

Exult logs `TTS: using <path>` at startup, or `TTS: <path> not executable, voice off`.

The wrapper's environment overrides:

| Variable | Default |
|---|---|
| `EXULT_TTS_PIPER_DIR` | `~/.local/share/piper` |
| `EXULT_TTS_PIPER` | `$EXULT_TTS_PIPER_DIR/piper` |
| `EXULT_TTS_MODEL` | `en_GB-jenny_dioco-medium` (voice name under `models/`) |
| `EXULT_TTS_VOICE` | `$EXULT_TTS_PIPER_DIR/models/$EXULT_TTS_MODEL.onnx` |
| `EXULT_TTS_HISTORY` | unset; a file to append every spoken line to |

## Test

`make check EXULT=/path/to/patched/exult` builds the patch's `tts.cc` alone with stub headers and a
fake speech command, and compares what it was asked to say with `test/expected.log`: name prefix,
markup cleanup, cut-off on the next line, and no repeats when a book repaints.

## Tested on

- Ubuntu 26.04.1 LTS, Exult `97984e9` with this patch and Ultima VII game data.
- In game: conversations read aloud with the speaker's name; the Audio options "Read aloud" row.
- Books and scrolls were not tried in game; they are covered only by `make check`.
- The `sudo make install` layout above was not itself run; the game was played from a local build
  with the wrapper in its data directory.
- The patch applies cleanly to a fresh checkout of that commit, builds, and `make check` passes.

## License

GPL-2.0, the same as Exult (see [LICENSE](LICENSE)). Not affiliated with the Exult team; not submitted upstream.
