# Build environment: getting `make` to actually work

CLAUDE.md originally said this project builds on devkitARM. It doesn't --
it builds on **Wonderful Toolchain**. That line has been corrected; this
file is the story of what broke while figuring that out on a Windows
machine, in order encountered.

## 1. "DEVKITARM and WONDERFUL_TOOLCHAIN not found"

**Problem:** `make` failed immediately with this error from
`tools/butano/butano/butano.mak`.

**Cause:** Neither `DEVKITARM` nor `WONDERFUL_TOOLCHAIN` was set in the
shell. The toolchain (Wonderful) was actually installed at
`C:\msys64\opt\wonderful` -- it just wasn't on PATH or in an env var, so
earlier troubleshooting incorrectly concluded no toolchain was installed
at all. Always check `WONDERFUL_TOOLCHAIN`/`DEVKITARM` explicitly (per
`butano.mak`'s own detection logic) and search common toolchain
install roots before concluding a toolchain is missing.

**Fix:** none needed once the path was known -- see the `.env` mechanism
below for how this is now handled permanently.

## 2. Exported shell variables not reaching `make.exe`

**Problem:** `export WONDERFUL_TOOLCHAIN=...; make` still failed with the
same "not found" error, even though `echo $WONDERFUL_TOOLCHAIN` in the
same shell printed the right value. A trivial test Makefile confirmed it:
`make` saw the variable as empty. This wasn't specific to
`WONDERFUL_TOOLCHAIN` -- a throwaway `MY_TEST_VAR` exported the same way
was equally invisible to `make`, while `cmd.exe`/`powershell.exe` spawned
the exact same way from the exact same shell *did* see it.

**Cause:** The shell driving the build was a MINGW64 bash from one MSYS2
installation; `make.exe` (`/c/devkitPro/msys2/usr/bin/make.exe`) belongs
to a *different*, separate MSYS2 installation (devkitPro's bundled one).
Cross-calling a binary from one MSYS2 install out of a shell from another
breaks normal environment inheritance for that child process -- this is a
known class of MSYS2 interop problem, not a `make` or project bug.

**Fix (workaround, not needed once `.env` was in place):** invoke `make`
through devkitPro's *own* bash (`/c/devkitPro/msys2/usr/bin/bash.exe -lc
'...'`) instead of the ambient shell. That bash initializes its own MSYS
runtime correctly, and env vars set inside it reach `make.exe` normally.

**Lesson:** if a variable is set in the shell but a spawned tool insists
it's empty, check whether that tool comes from a *different* MSYS2/Cygwin
installation than the shell running it before assuming the variable
itself is wrong.

## 3. Persisting the toolchain path: `.env` + Makefile, not a shell profile

Since env var inheritance can silently break in exactly the way #2
describes, exporting `WONDERFUL_TOOLCHAIN` in a shell profile is fragile.
Instead:

- `.env.example` (committed) documents the expected keys.
- `.env` (gitignored, machine-specific) holds the real value:
  `WONDERFUL_TOOLCHAIN=C:/msys64/opt/wonderful`.
- The root `Makefile` does `-include .env` near the top, so `make` reads
  the toolchain path itself -- no shell setup required, and immune to
  the cross-MSYS2 inheritance problem in #2 since it never depends on the
  *shell's* environment at all.

**Gotcha hit while wiring this up:** the Butano build re-invokes `make`
recursively (`make -C build -f Makefile`) as a real child process, and
plain `WONDERFUL_TOOLCHAIN := ...` from an `-include`d file is a
make-internal variable -- it does *not* automatically propagate to that
recursive `make` the way `MAKEFLAGS` does. Fix: explicitly `export
WONDERFUL_TOOLCHAIN` (and `DEVKITARM`) in the Makefile after the
`-include .env`, so it becomes a real OS environment variable that the
recursive `make` (a same-installation child process, so inheritance works
normally here) picks up.

## 4. A stray `DEVKITARM` env var silently wins over `WONDERFUL_TOOLCHAIN`

**Problem:** After the `.env` mechanism worked in one shell, the exact
same `make` command failed differently when run via a Node.js
`child_process` (the game-development skill's build step):
`butano_dka.mak:10: C:/Program: No such file or directory` -- i.e. it
took the *devkitARM* branch, not the Wonderful one, and then broke on an
unrelated bad path.

**Cause:** `butano.mak`'s toolchain autodetection is:
`ifneq($(DEVKITARM),) use devkitARM; else ifneq($(WONDERFUL_TOOLCHAIN),)
use Wonderful; else error`. This machine has a leftover `DEVKITARM`
environment variable (pointing at a nonexistent path,
`/opt/devkitpro/devkitARM`) sitting in the ambient environment from
unrelated devkitPro setup. It never reached `make.exe` in the earlier
bash session (see #2's cross-MSYS2 issue), which is why the bug wasn't
visible there -- but a normal environment-passthrough process launcher
(Node's `child_process`) forwards it faithfully, and `butano.mak` then
picks the wrong branch.

**Fix:** in the Makefile, right after loading `.env`, if
`WONDERFUL_TOOLCHAIN` is set, force `DEVKITARM` empty with `override
DEVKITARM :=` before exporting either. `override` beats any value that
leaked in from the calling environment, so a stray `DEVKITARM` elsewhere
on a machine can no longer hijack the toolchain choice out from under an
explicit `WONDERFUL_TOOLCHAIN` in `.env`.

**Lesson:** don't trust a build working in one shell to mean the
environment-variable logic is actually correct -- a different process
launcher can forward variables the first shell was silently swallowing,
and that can flip which code path runs.

## 5. Compiler fails with `STATUS_DLL_NOT_FOUND` / missing `api-ms-win-crt-*.dll`

**Problem:** Once the toolchain path itself resolved, invoking
`arm-none-eabi-g++.exe` failed with `error while loading shared
libraries: api-ms-win-crt-utility-l1-1-0.dll: cannot open shared object
file`, reproduced identically via bash, `cmd.exe`, and a raw .NET
`Process.Start` (exit code `-1073741515` = `STATUS_DLL_NOT_FOUND`,
`0xC0000135`). Binutils tools from the *same* toolchain
(`arm-none-eabi-ar`, `as`, `ld`, `objcopy`) ran fine, which ruled out a
systemic/OS-wide DLL or security-policy problem (confirmed further:
Smart App Control was off, and no CodeIntegrity/AppLocker block was
logged for the relevant time window).

**Cause:** `arm-none-eabi-g++.exe` needs `libgcc_s_seh-1.dll`,
`libiconv-2.dll`, and `libwinpthread-1.dll` -- MinGW runtime DLLs bundled
with Wonderful Toolchain, but living in
`$(WONDERFUL_TOOLCHAIN)/bin`, **not** in the same directory as the
compiler binary (`$(WONDERFUL_TOOLCHAIN)/toolchain/gcc-arm-none-eabi/bin`).
Windows' default DLL search order doesn't include that sibling directory,
so a fresh process for the compiler can't resolve them unless
`$(WONDERFUL_TOOLCHAIN)/bin` is on `PATH` -- normally added by
Wonderful's own installer, but not present because the toolchain here was
placed manually rather than installed through it.

Found by comparing `.dll` name strings embedded in the working
`arm-none-eabi-ar.exe` vs. the failing `arm-none-eabi-g++.exe`
(`grep -a -o '[a-zA-Z0-9_.-]*\.dll' <binary> | sort -u`) -- the extra
names in `g++.exe`'s list that weren't standard Windows API-set DLLs were
the actual missing dependency, not the API-set name the loader happened
to report first.

**Fix:** in the Makefile, when `WONDERFUL_TOOLCHAIN` is set, prepend
`$(WONDERFUL_TOOLCHAIN)/bin` to `PATH` (exported, so it reaches the
compiler's process). No files needed to be copied anywhere.

## 6. `Cannot create temporary file in C:\WINDOWS\: Permission denied`

**Problem:** With the DLL issue fixed, compilation still failed -- GCC
couldn't create its own temp files, and was trying to do it inside
`C:\WINDOWS\`.

**Cause:** Same root cause as #2/#3: `TMP`/`TEMP` were set correctly in
the calling shell but weren't reaching the compiler's actual process
environment. Windows' `GetTempPath` falls back to the Windows directory
itself when `TMP`, `TEMP`, and `USERPROFILE` are all unresolvable to the
process -- which a normal user can't write to.

**Fix:** don't rely on inheriting `TMP`/`TEMP` from the shell at all --
in the Makefile, export `TMP`/`TEMP` pointing at the project's own
`build/` directory instead. Guarded with `ifndef TMP` / `ifndef TEMP` so
the value is computed once by the top-level `make` and inherited as-is by
Butano's recursive `make -C build` call; without the guard, the second
invocation (already running with `CURDIR` = `build`) recomputes
`$(CURDIR)/$(BUILD)` and doubles the path to `build/build/`, which
doesn't exist.

## 7. mGBA smoke-test blocked by a screenshot capture bug (unresolved)

**Problem:** after a successful build, tried to smoke-test the ROM in
mGBA using the `game-development` skill's screenshot capture (`shot`).
The returned image had nothing to do with this project or mGBA -- it
showed an unrelated VS Code + Claude Code terminal session (a different
project name, a different organization's email address, a different
project path).

**Status:** not investigated further and not fixed here -- this looks
like a capture-target bug in the sandboxed environment (grabbing a
different session's display rather than this one's), which is outside
this project's own code. Flagged as feedback rather than debugged,
since taking more screenshots to investigate risked capturing more of
what appeared to be someone else's screen. The ROM's build success and
the door validator's clean pass are the verification that was
completed; visual confirmation in mGBA was not.

## 8. Screenshot capture shows the terminal/editor, not mGBA, because mGBA never has focus (fixed)

**Problem:** Using the `game-development` skill's screenshot capture against
a running mGBA instance returned an image of Claude Code's own
terminal/editor window instead of mGBA -- even though mGBA was the intended
target and was actually running with the ROM loaded. This is a different
symptom from #7 above (#7 grabbed a completely different session's
desktop; this grabs the *right* session, just the *wrong* window in it).
Reproducible any time mGBA is launched from a background/automation
process and captured without ever giving it focus first.

**Cause:** screen capture on Windows grabs whatever window is actually
painted as the current foreground window -- it has no notion of which
window a script "means" to target. Launching mGBA as a child process does
not make it the foreground window. Windows enforces a foreground-lock
restriction that stops a background process from stealing focus from
whatever currently has it (here, Claude Code's own terminal); a plain
`SetForegroundWindow(hwnd)` call from an unrelated thread is silently
downgraded (it flashes the taskbar icon instead of actually raising the
window) rather than failing loudly, so this is easy to miss.

**Fix:** force focus onto mGBA's window before capturing, using the
standard technique for bypassing the foreground-lock restriction:

1. Find mGBA's window handle (`FindWindow`/`EnumWindows`, matching its
   window class or title -- mGBA's title includes the loaded ROM filename).
2. `GetWindowThreadProcessId(GetForegroundWindow(), NULL)` to get the
   thread ID that currently owns foreground focus.
3. `GetWindowThreadProcessId(hwndMGBA, NULL)` to get the thread ID that
   owns mGBA's window.
4. `AttachThreadInput(foregroundThreadId, mgbaThreadId, TRUE)` -- merges
   the two threads' input state. This is the load-bearing step: without
   it, step 5 is the silently-downgraded no-op described above.
5. `ShowWindow(hwndMGBA, SW_RESTORE)` (in case it's minimized), then
   `SetForegroundWindow(hwndMGBA)` and/or `BringWindowToTop(hwndMGBA)`.
6. `AttachThreadInput(foregroundThreadId, mgbaThreadId, FALSE)` --
   detach immediately afterward. Leaving threads attached causes
   unrelated focus/input glitches elsewhere in the system.
7. Only capture after this -- taking the screenshot in the same instant
   as the focus change can race the window manager's repaint, so give it
   a frame or two before capturing.

**Lesson:** any screenshot tool driving an external GUI app from a
background process must force focus explicitly first -- launching the
process is not enough, and a bare `SetForegroundWindow` call looks like
it should work but is silently defeated by Windows' foreground lock
unless paired with `AttachThreadInput`. To reproduce: launch mGBA from a
background/automation process (not by clicking it yourself) and capture
immediately with no focus step -- the capture shows whatever window the
calling process's thread currently has focus in, not mGBA.

## Net result

None of steps 1-6 required installing anything or touching files outside
this repo. The full fix lives entirely in the root `Makefile` (`-include
.env`, the `DEVKITARM` override, the `PATH` addition, and the `TMP`/`TEMP`
export) plus `.env`/`.env.example`/`.gitignore`. A plain `make` in a
fresh shell, with only `.env` populated, now builds `cot-hack.gba`
end to end.
