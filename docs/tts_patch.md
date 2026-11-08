# TTS patch (`exult_tts.patch`)

Written against upstream Exult `97984e9e0cbb4ccacaab0249da9c51b533818f74` (2026-10-29).

## Hooks

| Where | What |
|---|---|
| `exult.cc` after `cheat.init()` | `TTS_init()` |
| `usecode/ucinternal.cc` `show_npc_face` | resolves `npc` (with fallback for positive NPC numbers, shape `356` Avatar, and `frame->caller_item`) and passes `npc->get_npc_name()` (or `caller_item->get_name()`) to `Conversation::show_face` |
| `usecode/conversation.{h,cc}` | `Npc_face_info::speaker`; `show_face(..., speaker)` stores it (only when non-empty) |
| `Conversation::show_npc_message` | `TTS_say` once per displayed page: before each "click for more" wait, and for the last page; the speaker's name is spoken only on page 1, and passed as `":<speaker>"` on later pages so stdout still prints `<speaker>: <text>` |
| `Conversation::init_faces` | `TTS_stop()` (conversation start/end) |
| `gumps/Book_gump.cc`, `Scroll_gump.cc` `paint()` | `TTS_say(visible pages, "[Book]" / "[Scroll]", only_if_new=true)`; paint runs on every repaint, so repeats are dropped |
| `usecode/intrinsics.cc` `display_runes` | signs, plaques, tombstones: each line through `TTS_runes_to_text`, then one `TTS_say(all lines, "[Sign]")` before the modal `Get_click`, if `TTS_signs_enabled()` |
| `Text_gump::~Text_gump` | `TTS_stop()` (book/scroll/sign closed; `Sign_gump` is a `Text_gump`) |
| `gumps/AudioOptions_gump.{h,cc}` | row 13 three-state toggle `id_tts` (No / Yes / Yes + signs; a separate signs row does not fit: `yForRow(16)` = 205 > 200; (outside the `id_first_setting` range, so shown and kept even with audio off; Apply/Help/Cancel moved to row 14); label "Read aloud:" ("Read aloud (no command):" when the command is missing); `do_arrange` right-aligns `id_tts` separately (without that it sat over the label, seen on screen); Apply calls `TTS_set_enabled` and saves `config/audio/tts/enabled`; for Yes / Yes + signs also `TTS_set_signs` and `config/audio/tts/signs` ("No" keeps the signs choice) |
| `Makefile.am` | `tts.cc`, `tts.h` in `EXULTSOURCES` (no CMake/MSVC project changes) |

Speaking per page in `show_npc_message` replaced the old hook in `paint_faces`, which ran on
every conversation repaint and had to dedupe; a click mid-sentence now cuts the voice off
because the next `TTS_say` kills the previous one.

## Speaker name and stdout tags

The old patch had no name because `Conversation` only sees a face shape number. The NPC is
known one level up, in `Usecode_internal::show_npc_face` (`get_face_shape` returns the
`Actor*` when Usecode passes a negative NPC ID or object pointer). When Usecode passes a
positive integer shape/NPC number or `356` (Avatar), `show_npc_face` falls back to
`gwin->get_npc(shape)` / `gwin->get_main_actor()` or `frame->caller_item`, and reads
`npc->get_npc_name()` (which falls back to the shape/object name for generic NPCs such as
guards or gargoyles). The avatar's lines carry the player's chosen name.

- Regular speaker (`"Iolo"`): stdout prints `TTS: Iolo: <text>` and speech says `Iolo: <text>`.
- Quiet speaker (`":Iolo"`, used on page 2+ of multi-page NPC dialogue): stdout prints
  `TTS: Iolo: <text>`, while speech says only `<text>` without repeating the name aloud.
- Bracketed tag (`"[Book]"`, `"[Scroll]"`, `"[Sign]"`): stdout prints `TTS: [Book] <text>`,
  while speech says only `<text>` without reading the tag aloud.

## tts.cc

- Config: `config/audio/tts/enabled` (`yes` default), `config/audio/tts/signs` (`yes` default),
  `config/audio/tts/command` (default
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

- Rune text (signs): usecode stores it as ASCII capitals drawn with a rune font; ligature
  characters `(` TH, `)` EE, `*` NG, `+` EA, `,` ST, `|` space (the same table as
  `Sign_gump::add_text` for an avatar with the `read` flag). `TTS_runes_to_text` expands them
  and lowercases, so Piper does not spell capitals out. Must run before `clean_text`, which
  would read `*` as a page break.

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
