# AGENTS.md

Working notes for agents modifying this repository. For the design read
`ARCHITECTURE.md`; for usage read `README.md`. This file covers the repo layout,
how to build and test, the invariants you must not break, and the traps that are
easy to fall into.

## Repo map

```
endian.hh                    Uniform byte-order macros (bswap16/32/64, htobe*/be*toh, htole*/le*toh, 56-bit variant). Header. Verbatim from Xapiand.
test/test.cc                 Runnable smoke test: raw swaps, round-trips, byte layout, 56-bit helpers.
CMakeLists.txt               INTERFACE library `endian` (+ alias endian::endian); CTest test `endian`.
LICENSE                      MIT, Copyright (c) 2015-2018 Dubalu LLC.
README.md                    What it is, install, usage.
ARCHITECTURE.md              Internal design: platform detection, the three layers, the 56-bit variant.
```

Everything is header-only. There is no `.cc` to compile except the test. The
CMake target is a pure `INTERFACE` library that only adds the source dir to the
include path and requests `cxx_std_20`.

## Build and run the test

```sh
cmake -B build && cmake --build build && ctest --test-dir build
```

Expected output ends with `all endian tests passed`, exit 0. The test target is
`endian_test`; the registered CTest name is `endian`.

## Conventions

- **C++20.** The target requests `cxx_std_20` to stay uniform with the sibling
  libraries; the header itself is older-standard-friendly, but don't drop the
  target below it.
- **Zero external dependencies.** The only includes are the platform
  endian/byteswap headers (`<endian.h>`, `<machine/endian.h>`, `<byteswap.h>`,
  the MSVC intrinsics, ...), pulled in conditionally by the platform-detection
  block. Don't add anything else.
- **Filename is stable.** The header keeps its original Xapiand name
  (`endian.hh`) so a consumer that already `#include`s it just needs this repo on
  the include path. Don't rename it.
- Tabs for indentation, double quotes in code, no em dashes in prose.

## Load-bearing invariants

- **The surface is macros, not functions.** `bswap*`, `htobe*`/`be*toh`,
  `htole*`/`le*toh`, and the 56-bit forms are all `#define`s, each resolving to a
  single intrinsic or an identity cast. The arguments are pasted textually, so
  callers pass already-typed integers. Don't "modernize" these into inline
  functions or templates: it would change the contract and the one-instruction
  fold the call sites rely on.
- **Every define is `#ifndef`-guarded.** A name the platform header already
  provides is left as the system's. Keep new definitions guarded the same way so
  the header never fights the platform.
- **One direction is the identity, the other is a swap.** Which is which depends
  on `__BYTE_ORDER`. Don't hard-code a swap on both paths; the no-op direction
  must stay a plain cast so it costs nothing on the matching host.
- **`__BYTE_ORDER` matching neither known value is a hard `#error`.** The header
  refuses to guess endianness. Keep that `#error` rather than picking a default.

## The 56-bit helpers

- **`htobe56`/`htole56`/`be56toh`/`le56toh` are a 7-byte variant** the standard
  headers don't provide. Xapiand uses them for HTM trixel ids. The contract is
  that only the low 7 bytes survive: the big-endian form is `bswap64(x << 8)`,
  the little-endian form masks to `0xffffffffffffff`. Anything in the 8th byte is
  dropped. The test checks this with a noisy top byte. Keep all four.

## How to extend

- **Add a width or variant.** Mirror the existing pattern: detect the platform's
  own define first, fall back to `bswap*` for the swap direction and an identity
  cast for the no-op direction, guard every define with `#ifndef`.
- **Always extend the smoke test.** `test/test.cc` is the only executable check.
  It decides host endianness at runtime, independent of the macros under test, so
  it can verify both the value round-trips and the actual byte layout. Keep that
  independence when you add cases.

## Traps

- **These are macros, so beware side effects and types in arguments.** Don't pass
  an expression with side effects (it may be evaluated more than once in the
  fallback paths) and pass a properly-typed unsigned integer; the macro won't
  promote a narrow or signed value for you.
- **`htobe56(x)` shifts `x` left by 8 first.** If you feed it a value that
  already uses the full 64 bits, the top byte is shifted out and lost. That is
  intended for 7-byte ids; don't reuse the 56-bit forms for general 64-bit data.

## Standalone vs. Xapiand

This is a standalone extraction from
[Xapiand](https://github.com/Kronuz/Xapiand). `endian.hh` had zero dependencies
already (only the platform endian/byteswap headers), so it was copied verbatim,
there is no decoupling delta. Keep it that way; any change here should be
reconcilable with upstream as a plain edit.
