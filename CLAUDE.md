# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

LCCA (Lunar Calendar C API) is a C99 shared library that converts between the
Gregorian and the Vietnamese/Chinese lunisolar calendar. It is a C port of the
astronomy module of the TypeScript [Lunar Calendar API](https://github.com/hnthap/lunar-calendar-api),
which remains the reference implementation for verifying numerical output. The
library builds to `liblcca.dll` / `liblcca.so` and is consumed both directly as
a C library and via a PowerShell module (`Lunar.psm1`) through P/Invoke.

The project is under construction; `TODO.md` tracks remaining work (unit-test
coverage, Delta T accuracy, documentation cross-checks). Consult it before
assuming a feature is complete.

## Build, test, and docs

The `Makefile` is a thin cross-platform wrapper around CMake + CTest. Prefer it:

```sh
make dev        # (default) clean + configure + build in Debug: sanitizers ON, warnings-as-errors
make release    # clean + configure + build in Release: -O3, sanitizers OFF
make build      # incremental build of current configuration (no clean)
make test       # build, then run the full CTest suite (--output-on-failure)
make clean      # remove build/ and the copied compile_commands.json
make docs       # generate Doxygen HTML into docs/html/index.html
make clean_docs
```

`make build` copies `build/compile_commands.json` to the repo root so clangd
picks it up.

To run a single test suite, invoke its executable directly (each test file
compiles to its own binary) or filter with CTest:

```sh
ctest --test-dir build -R test_mechanics --output-on-failure   # one suite by regex
./build/test_mechanics.exe                                     # run the binary directly
```

To configure/build manually without the Makefile:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DLCCA_DEVELOPER_MODE=ON
cmake --build build
```

There is no separate lint step: strictness is enforced at compile time (see
below). Formatting is LLVM style with 4-space indent (`.clang-format`).

## Architecture

The library is a **stateless, pure-computation** numerical library: no heap
allocation, no global mutable state, all structs passed and returned by value,
all functions reentrant/thread-safe. Preserve these invariants — they are load-
bearing for the safety-critical posture and are documented as guarantees in the
public headers.

Layering (headers in `include/`, implementations in `src/`; header-only modules
have no `.c`):

- **`lcca_numeric.h`** (header-only) — foundation. Fixed-width type aliases
  (`lcca_i8/i16/i32/i64`, `lcca_f32/f64`, `lcca_bool`) over `<stdint.h>` /
  `<stdbool.h>`, plus compile-time size assertions. Everything depends on this;
  use these aliases, not raw C primitives, in public API.
- **`lcca_math.h`** (header-only, `static inline`) — degree-based angle helpers
  (`lcca_normalize_degrees`, `lcca_sin_degrees`, `LCCA_PI`). Astronomical
  formulae are expressed in degrees, not radians.
- **`lcca_common.h`** (header-only) — the assertion facility. `lcca_c_assert(e)`
  evaluates `e`; on failure it calls a fixed-signature debugging hook
  (`lcca_tst_debugging`, prints to stderr) and evaluates to `false` **without
  aborting**. This non-aborting behavior is deliberate and shapes both error
  handling and the test harness (tests keep running after a failed assertion).
  The fixed-signature hook exists to comply with JPL C Coding Standard Rule 20
  (no variadic functions in library code).
- **`lcca_calendar.[h|c]`** — the high-level public API most consumers use.
  Date/time structs (`lcca_gregorian_date`, `lcca_lunar_date`, `lcca_time`),
  validation, Gregorian↔JD conversion, Delta T, and Gregorian↔lunar conversion.
- **`lcca_mechanics.[h|c]`** — low-level astronomical engine underneath the
  calendar API (Sun true longitude, New Moon JD, lunation number `k`, winter
  solstice, Month 11 identification, leap-month detection). Most applications
  should not call these directly; they exist for the calendar layer, advanced
  users, and test oracles.

### Domain conventions (important for correctness)

- **Time scales**: JD (Julian Day) is the continuous time base. **TD** (Dynamic
  Time) is used for ephemeris/celestial-motion math; **UT** (Universal Time) is
  used when mapping events to civil dates. Function and variable names carry the
  scale as a suffix (`_jd_td`, `_jd_ut`) — respect it; mixing scales is a bug.
- **Lunation number `k`**: integer index of successive New Moons, with `k = 0`
  ≈ the New Moon of 6 January 2000.
- **Astronomical year numbering**: AD 1 → 1, 1 BC → 0, 2 BC → -1.
- **Time zones**: decimal hour offsets from UTC (+07:00 → `7.0`), validated to
  the range −24..+24 (see the rationale in `lcca_calendar.h`'s file header).
- **Leap month rule**: the first lunar month between consecutive Month 11
  boundaries that contains no Principal Solar Term.
- Algorithms follow Jean Meeus, *Astronomical Algorithms* (Willmann-Bell, 1991);
  function docs cite the relevant chapter. When changing math, verify against
  both Meeus and the TypeScript reference implementation.

### Preconditions and error model

Functions document `@pre`/`@post` in their header comments. Violating a
precondition is undefined behavior from the caller's view; implementations may
call `lcca_c_assert()` and, for functions that document a sentinel (e.g.
`lcca_get_gregorian_month_size` returns `-1`), return that sentinel after the
assertion fires. `lcca_convert_lunar_to_gregorian` deliberately skips deep lunar
validation (round-trip validation would recurse) and applies only basic bounds.

## Testing

Tests live in `tests/test_*.c`; CMake globs each into its own executable and
registers it with CTest. There is no third-party framework — each file defines a
`RUN_TEST` macro and a `tests_passed` flag, and compares floating-point results
against hand-computed **oracles** within per-quantity epsilons (documented
inline with the reasoning behind each tolerance). When adding tests, follow the
existing epsilon-with-justification pattern rather than exact `==` on doubles.
Cross-check expected values against the TypeScript reference implementation.

## Compiler strictness (developer mode)

`CMakeLists.txt` applies an aggressive warnings-as-errors regime (dubbed "The
Paranoia Engine"): `-Wall -Wextra -Werror -pedantic` plus `-Wconversion`,
`-fanalyzer`, strict-C90 compatibility warnings, and many more, each added only
if the compiler supports it. GCC extensions are disabled (`CMAKE_C_EXTENSIONS
OFF`). In Debug / `LCCA_DEVELOPER_MODE=ON`, ASan + UBSan (+ LSan on non-Win/Mac)
are enabled when the linker can actually link them. Expect new code to compile
clean under all of this — a new warning fails the build.

Note: `.clangd` removes `-fanalyzer` from the flags it sees (clangd doesn't
understand that GCC-only flag), so clangd diagnostics may differ slightly from a
real build. Trust `make build` as the source of truth.

## PowerShell module

`Lunar.psm1` P/Invokes `liblcca.dll` (currently only
`lcca_convert_gregorian_to_lunar`) and renders lunar dates in Chinese and
Vietnamese, including sexagenary-cycle stem/branch year names. The struct layout
declared in the `Add-Type` C# block **must stay byte-compatible** with the C
structs in `lcca_calendar.h` — if you change a struct's fields or ordering,
update the P/Invoke definition too.

- `Install-LunarModule.ps1` — copies `Lunar.psm1` + the DLL into the user's
  PowerShell module path.
- `Lunar.Tests.ps1` — Pester tests for the representation/cycle-math functions.