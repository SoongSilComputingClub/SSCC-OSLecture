# SSCC OS Study Track

This directory stores the student-facing overlays for each study session.

The important change is that these files are now meant to be applied
incrementally, not read side-by-side with the full reference tree. Each
`sessionXX_*` folder only contains the new source files opened in that session.

To build a cumulative student packet:

1. Pick the target session.
2. Run `scripts/materialize_student_stage.sh <session> <output-dir>` for the starter packet, or
   `scripts/materialize_student_stage.sh --solution <session> <output-dir>` for the filled-in one.
3. The script copies only the files allowed for that stage and overlays the
   session files up to that point.
4. `session02` through `session11` now use the strict minimal packet mode.
5. When a session has `START_MANIFEST.tsv`, the selected stage gets that
   starter overlay, while earlier sessions still apply their solved
   `MANIFEST.tsv` overlays.

Example:

```sh
scripts/materialize_student_stage.sh session04 ../pack-session04
scripts/materialize_student_stage.sh --solution session04 ../pack-session04-solved
```

Recommended branch flow:

- `codex/session02-start`
- `codex/session02-solution`
- `codex/session03-start`
- `codex/session03-solution`
- ...

Session mapping:

- `session01_ot`
  No code overlay. Environment and architecture walkthrough only.
- `session02_stage0`
  Stage0 boot path and a temporary stage1 assembly hello-world.
- `session03_protected_mode`
  Real-mode startup, GDT setup, and the C-based stage1 FAT loader.
- `session04_memory`
  Physical memory allocator and early paging.
- `session05_drivers`
  Device registry and minimal ISA UART driver.
- `session06_bios_video`
  BIOS video wrappers and mode selection command.
- `session07_libc`
  Minimal freestanding libc functions plus `vcprintf`.
- `session08_shell`
  Line input, variable expansion, parsing, and dispatch.
- `session09_graphics`
  Framebuffer console and a tiny ANSI subset.
- `session10_fat`
  Read-only FAT subset.
- `session11_elf`
  ELF loading subset.
- `session12_showcase`
  Demo and compare against the reference implementation.

Notes:

- The reference implementation under `vellum/` stays untouched for instructor use.
- The materialized student packet is the one students should see.
- Each later session now replaces the boot-stage `init.c` and BIOS-side
  `CMakeLists.txt` so the emitted artifact matches that week's topic.
- `session06` keeps BIOS video calls in `stage1`, because the 16-bit BIOS thunk
  is only safe there once the later kernels grow larger.
- Each session can also widen the copied file set through `FILES.lst`, so the
  student checkout gradually grows toward the real repository layout.
- This makes `session09` look like “session08 solved + session09 starter”, which
  is the intended cumulative teaching flow.
- The same starter/solution split now exists from `session04` onward, so the
  selected stage exposes allocator/UART/BIOS-video/libc TODOs while later
  sessions automatically carry their solved versions forward.
