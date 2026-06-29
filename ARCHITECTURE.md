# Architecture

The internal design of `endian`: a single header that normalizes the platform
byte-order macros onto one uniform surface. For usage see `README.md`; for the
repo map and invariants see `AGENTS.md`.

## Shape

One header, header-only, nothing behind it but the system endian/byteswap
headers:

```
  endian.hh   platform detection + a uniform set of conversion macros
```

## The three layers

The header builds its surface in three stages, each guarded by `#ifndef` so a
name the platform already provides is left untouched.

First, **platform detection.** A block of `#if defined(__linux__) / __APPLE__ /
__FreeBSD__ / _WIN*` pulls in the right system header (`<endian.h>`,
`<machine/endian.h>`, `<sys/endian.h>`, ...) and normalizes the byte-order
constants to the `__BYTE_ORDER` / `__BIG_ENDIAN` / `__LITTLE_ENDIAN` spellings,
because not every platform names them the same way.

Second, **the raw swaps.** `bswap16/32/64` resolve to `__builtin_bswap*` on
Clang and GCC, to `bswap_*` from `<byteswap.h>` on Linux, to the
`_byteswap_*` intrinsics on MSVC, and to a hand-written shift-and-mask fallback
when nothing else is available. The `__has_builtin` probe (with a fallback
definition for compilers that lack it) decides which path applies.

Third, **the directional conversions.** With `__BYTE_ORDER` known, the header
defines `htobe*` / `be*toh` and `htole*` / `le*toh` so that one direction is a
swap and the other is the identity cast. On a little-endian host `htobe32` is
`bswap32` and `htole32` is a plain `(uint32_t)` cast; on a big-endian host the
roles flip. If `__BYTE_ORDER` matches neither known value, the header is a hard
`#error` rather than guessing.

## The 56-bit variant

Past the standard widths, the header adds `htobe56` / `htole56` / `be56toh` /
`le56toh`. These exist because Xapiand stores HTM trixel ids in 7 bytes, and the
system headers stop at 64 bits. The big-endian form shifts the value up by a byte
and swaps (`bswap64(x << 8)`) so the meaningful 7 bytes land big-end-first; the
little-endian form masks to `0xffffffffffffff` so the 8th byte drops. The net
contract is that only the low 7 bytes survive a round-trip. The test pins this
down by feeding in a value with a noisy top byte and checking it comes back zero.

## Why macros

These are macros, not inline functions, on purpose. Each one resolves to a single
compiler intrinsic (or to the identity), so the conversion folds into one
instruction with no call and no type-juggling at the boundary. The trade-off is
the usual macro hazards: the arguments are pasted in textually, so callers pass
already-typed integer values and don't lean on the macro to do width promotion
for them. That is the original Xapiand contract, preserved here.
