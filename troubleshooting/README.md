# Troubleshooting log

A running record of non-obvious problems hit while building and developing
this project, and how each was actually resolved. This is not a tutorial or
a design doc -- it's a debugging history, kept so the next person (or the
next Claude Code session) doesn't have to re-derive the same root causes.

Add a new entry whenever a problem takes real investigation to solve --
not for routine bugs whose fix is obvious from the diff, but for anything
where the *cause* wasn't where the symptom pointed.

## Files

- [build-environment.md](build-environment.md) -- getting `make` to
  actually find and run the toolchain on this machine: wrong toolchain
  documented in CLAUDE.md, a stray `DEVKITARM` env var silently winning
  over `WONDERFUL_TOOLCHAIN`, env vars not reaching `make.exe` at all,
  missing runtime DLLs, and GCC's temp-file directory.
- [room-transitions-and-doors.md](room-transitions-and-doors.md) -- the
  room-transition infinite-loop bug (arriving through a door landing
  inside the reciprocal door's own trigger box), the engine-level fix,
  the door validator built to catch this class of bug in room data, and
  the specific rooms it caught.
