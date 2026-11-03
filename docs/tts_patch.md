# TTS patch (`exult_tts.patch`)

Written against upstream Exult `97984e9e0cbb4ccacaab0249da9c51b533818f74` (2026-10-29).

## Hooks

| Where | What |
|---|---|
| `exult.cc` after `cheat.init()` | `TTS_init()` |
| `usecode/ucinternal.cc` `show_npc_face` | passes `npc->get_npc_name_string()` to `Conversation::show_face` |
| `usecode/conversation.{h,cc}` | `Npc_face_info::speaker`; `show_face(..., speaker)` stores it (only when non-empty) |
| `Conversation::show_npc_message` | `TTS_say` once per displayed page: before each "click for more" wait, and for the last page; the speaker's name only on page 1 |
| `Conversation::init_faces` | `TTS_stop()` (conversation start/end) |
| `gumps/Book_gump.cc`, `Scroll_gump.cc` `paint()` | `TTS_say(visible pages, "", only_if_new=true)`; paint runs on every repaint, so repeats are dropped |
| `Text_gump::~Text_gump` | `TTS_stop()` (book/scroll closed) |
| `gumps/AudioOptions_gump.{h,cc}` | row 13 toggle `id_tts` (outside the `id_first_setting` range, so shown and kept even with audio off; Apply/Help/Cancel moved to row 14); label "Read aloud:" ("Read aloud (no command):" when the command is missing); `do_arrange` right-aligns `id_tts` separately (without that it sat over the label, seen on screen); Apply calls `TTS_set_enabled` and saves `config/audio/tts/enabled` |
| `Makefile.am` | `tts.cc`, `tts.h` in `EXULTSOURCES` (no CMake/MSVC project changes) |

Speaking per page in `show_npc_message` replaced the old hook in `paint_faces`, which ran on
every conversation repaint and had to dedupe; a click mid-sentence now cuts the voice off
because the next `TTS_say` kills the previous one.

## Speaker name

The old patch had no name because `Conversation` only sees a face shape number. The NPC is
known one level up, in `Usecode_internal::show_npc_face` (`get_face_shape` returns the
`Actor*`). `get_npc_name_string()` is the raw NPC name, empty for nameless NPCs (then no
prefix); `get_npc_name()`/`get_name()` would fall back to the shape name ("man") or hide
unmet NPCs. The avatar's lines carry the player's chosen name. Spoken form: `Iolo: <text>`.

## tts.cc

- Config: `config/audio/tts/enabled` (`yes` default), `config/audio/tts/command` (default
  `<DATA>/tts_wrapper`). Not executable → one log line `TTS: ... voice off`; otherwise
  `TTS: using <path>` at startup.
- Every line passed to `TTS_say` (after cleanup, dedupe and name prefix)
  is printed to stdout as `TTS: <line>`, voice on or off: run Exult from a terminal to get a
  readable conversation history. For a file,
  `exult | tee` or `EXULT_TTS_HISTORY` in the wrapper (voice on only).
- Text cleanup: `*` (page break) → sentence end; `~` and whitespace runs → one space; `^`
  (capital marker), `@` (quote glyph), bytes ≥0x80 (font-mapped) and control chars dropped.
- Runs `command "<line>"` with `posix_spawn` in a new process group (no shell, so quotes in game
  text are safe); stop = `kill(-pgid, SIGTERM)` + `waitpid`. Non-POSIX builds: speech off.

## tts_wrapper

Speaks `$1` through Piper → `aplay`, rate read from the model's `.onnx.json`. It no longer
`pkill`s every `piper`/`aplay` on the host (that also killed other Piper users); Exult stops
its own process group. `EXULT_TTS_MODEL` overrides the voice, `EXULT_TTS_HISTORY=<file>` logs
lines; `EXULT_TTS_PIPER_DIR`, `EXULT_TTS_PIPER` and `EXULT_TTS_VOICE` move the Piper directory,
binary and voice file (defaults `~/.local/share/piper/{piper,models/<voice>.onnx}`). Installed as `<DATA>/tts_wrapper` (Exult's data dir).

## Check

`make check EXULT=<patched Exult checkout>` builds `tts.cc` from that tree with stub headers (`test/inc`) and the logging
`test/fakecmd`, then diffs `test/said.log` against `test/expected.log`.
The patch applies cleanly (`git apply --check`) on a fresh checkout of the commit above.
