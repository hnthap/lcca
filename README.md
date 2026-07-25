# LCCA — Lunar Calendar C API

A small, dependency-free **C99 library** for converting dates between the
Gregorian calendar and the Vietnamese/Chinese **lunisolar calendar**, using the
astronomical algorithms of Jean Meeus (*Astronomical Algorithms*, Willmann-Bell,
1991).

LCCA is a C port of the astronomy engine of the TypeScript
[Lunar Calendar API](https://github.com/hnthap/lunar-calendar-api), repackaged as
a native shared library (`liblcca.dll` / `liblcca.so`) that can be linked
directly into C/C++ programs or called from other runtimes via FFI. A PowerShell
module is included as a reference consumer.

> **Status:** pre-release, under construction. The numerical engine is complete
> and unit-tested; see [TODO.md](./TODO.md) for the remaining work (a more
> accurate Delta T model, documentation cross-checks, packaging).

## Why LCCA?

Lunisolar calendars govern cultural and religious life for over a billion
people — Lunar New Year (*Tết* / Spring Festival), festivals, ancestral rites.
Most calendar libraries either omit lunisolar support or hard-code lookup tables
with a limited date range. LCCA instead computes dates **astronomically** from
first principles (Sun longitude, New Moon epochs, solar terms), so it is not
bound to a precomputed range, and does it in portable C with:

- **No dependencies** beyond the C standard library and `libm`.
- **No heap allocation, no global state.** Every function is a pure computation;
  all structs are passed and returned by value. This makes the whole API
  reentrant and thread-safe.
- **A safety-critical posture.** Fixed-width numeric types, a non-aborting
  assertion facility, and a JPL-style "no variadics in library code" discipline
  (see [Design notes](#design-notes)).

## Features

- **Bidirectional conversion** — Gregorian ↔ lunisolar, with full metadata
  (lunation number, leap-month flag, month length).
- **Leap-month handling** — via the traditional rule (the first month between
  consecutive Month 11 boundaries that contains no Principal Solar Term).
- **Timezone-aware** — offsets are decimal hours from UTC (`+07:00` → `7.0`).
- **Extended date range** — works for historical and future dates (accuracy
  degrades for very ancient dates, as with any Meeus-based method).
- **Strict validation** — rejects impossible Gregorian dates rather than
  producing silent garbage.
- **Cultural rendering** (via the PowerShell module) — Chinese and Vietnamese
  representations, including sexagenary-cycle (stem/branch) year names.

## Build

The build is CMake + CTest, wrapped by a cross-platform `Makefile`. You need a
C99 compiler (GCC, Clang, or MSVC), CMake ≥ 3.15, and `make`.

```sh
make dev        # (default) Debug build: sanitizers on, warnings-as-errors
make release    # Release build: -O3, sanitizers off
make build      # incremental build of the current configuration
make test       # build, then run the full CTest suite
make docs       # generate Doxygen HTML into docs/html/index.html
make clean
```

The library is produced as `build/liblcca.dll` (Windows) or `build/liblcca.so`
(Linux/macOS). To configure manually without the wrapper:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Usage (C)

Link against `liblcca` and include the public headers from `include/`. The
high-level API lives in [`lcca_calendar.h`](include/lcca_calendar.h).

```c
#include "lcca_calendar.h"
#include <stdio.h>

int main(void) {
    /* July 19th, 2024, in time zone +07:00 */
    lcca_gregorian_date g = {.time_zone = 7.0, .year = 2024, .month = 7, .day = 19};

    lcca_lunar_date lunar = lcca_convert_gregorian_to_lunar(g);

    /* -> day 14, month 6 (not leap), lunar year ~2024 */
    printf("%d-%02d-%02d (leap=%d, month_size=%d)\n",
           lunar.year, lunar.month, lunar.day, lunar.leap, lunar.month_size);

    /* Convert back */
    lcca_gregorian_date back = lcca_convert_lunar_to_gregorian(lunar);
    printf("%d-%02d-%02d\n", back.year, back.month, back.day);
    return 0;
}
```

```sh
cc example.c -Iinclude -Lbuild -llcca -lm -o example
```

### Core public API

From [`lcca_calendar.h`](include/lcca_calendar.h):

| Function                                                              | Purpose                      |
| --------------------------------------------------------------------- | ---------------------------- |
| `lcca_convert_gregorian_to_lunar`                                     | Gregorian → lunisolar        |
| `lcca_convert_lunar_to_gregorian`                                     | Lunisolar → Gregorian        |
| `lcca_is_valid_gregorian_date`                                        | Validate a Gregorian date    |
| `lcca_convert_gregorian_to_jd_ut` / `lcca_convert_jd_ut_to_gregorian` | Gregorian ↔ Julian Day (UT)  |
| `lcca_get_delta_t_seconds` / `_td`                                    | Delta T (TD − UT) estimation |

The low-level astronomical engine (Sun longitude, New Moon JD, lunation number
`k`, winter solstice, leap-month detection) is exposed in
[`lcca_mechanics.h`](include/lcca_mechanics.h) for advanced use and testing;
most consumers only need the calendar header.

## Usage (PowerShell)

`Lunar.psm1` calls the DLL through P/Invoke and renders culturally-formatted
dates. Install it with:

```powershell
./Install-LunarModule.ps1     # copies Lunar.psm1 + liblcca.dll into your module path
```

Then:

```powershell
Import-Module Lunar
Get-LunarDateRepresentation -Language Vietnamese   # for today
Get-LunarDateRepresentation -Language Chinese -Lunar (Get-LunarDate -Date '2025-08-11')
```

The module works on both Windows PowerShell 5.1 and PowerShell 7+ (`pwsh`). On
PowerShell 7 (.NET Core), the `LunarAPI` type is compiled into an in-memory
assembly with no on-disk location, so the default P/Invoke probing cannot find
`liblcca.dll` on its own; the module registers a `DllImportResolver` that loads
the DLL from the module folder (or a `build/` subfolder when run from source).
This means `liblcca.dll` must sit next to `Lunar.psm1` — which is exactly what
`Install-LunarModule.ps1` arranges.

> **Note:** the C# struct layout in `Lunar.psm1` must stay byte-compatible with
> the C structs in `lcca_calendar.h`. If you change a struct, update the
> P/Invoke definition to match.

## Domain conventions

A few conventions are load-bearing for correct results:

- **Time scales.** JD (Julian Day) is the continuous time base. **TD** (Dynamic
  Time) is used for celestial-motion math; **UT** (Universal Time) is used when
  mapping events to civil dates. Names carry the scale as a suffix (`_jd_td`,
  `_jd_ut`).
- **Lunation number `k`.** An integer index of successive New Moons, with
  `k = 0` at the New Moon of ~6 January 2000.
- **Astronomical year numbering.** AD 1 → `1`, 1 BC → `0`, 2 BC → `-1`.
- **Timezones.** Decimal hour offsets from UTC, validated to −24..+24.

## Design notes

- **Stateless & thread-safe.** No heap, no globals; structs by value. These are
  documented guarantees, not incidental — preserve them when contributing.
- **Assertions don't abort.** `lcca_c_assert()` reports a failed precondition to
  a fixed-signature debugging hook and evaluates to `false` without terminating,
  suiting embedded/safety-critical hosts that must not crash. The fixed signature
  (no variadics in library code) follows JPL C Coding Standard Rule 20.
- **"The Paranoia Engine."** The build applies an aggressive warnings-as-errors
  regime (`-Wall -Wextra -Werror -pedantic`, `-Wconversion`, `-fanalyzer`, strict
  C90 compatibility, and more), plus ASan/UBSan in debug builds. New code is
  expected to compile clean under all of it.
- **Verification.** Numerical tests compare against hand-computed oracles within
  documented per-quantity tolerances, cross-checked against the TypeScript
  reference implementation.

## Relationship to the TypeScript API

The reference [Lunar Calendar API](https://github.com/hnthap/lunar-calendar-api)
is a TypeScript HTTP service; LCCA ports its astronomy module to a native library
with a different distribution model (link/FFI rather than REST).

## License

MIT © 2026 Huynh Nhan Thap. See [LICENSE](./LICENSE).
