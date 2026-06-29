# endian

A small, header-only, dependency-free **byte-order / endianness** library for
C++20, extracted from [Xapiand](https://github.com/Kronuz/Xapiand).

## What it is

One header, `endian.hh`: a uniform set of byte-order conversion macros that work
the same way on every platform. It maps the per-OS spellings (`<endian.h>` on
Linux, `<machine/endian.h>` on macOS, `<sys/endian.h>` on FreeBSD, the MSVC
intrinsics on Windows) onto one common surface, fills in anything the platform is
missing with `__builtin_bswap*` (or a portable fallback), and adds a 56-bit
(7-byte) variant that the standard headers don't provide. Nothing behind it but
the system endian/byteswap headers.

The full set:

- **Raw swaps:** `bswap16`, `bswap32`, `bswap64`.
- **Host to/from big-endian:** `htobe16/32/64`, `be16/32/64toh`.
- **Host to/from little-endian:** `htole16/32/64`, `le16/32/64toh`.
- **56-bit (7-byte):** `htobe56`, `htole56`, `be56toh`, `le56toh`, for ids that
  live in 7 bytes (Xapiand uses these for HTM trixel ids).

These are macros, not functions, so they fold away to a single intrinsic on the
host. On a little-endian machine `htole32` is the identity and `htobe32` is a
swap; on a big-endian machine it is the other way around. You write the same code
either way.

## Install

CMake with `FetchContent`:

```cmake
include(FetchContent)
FetchContent_Declare(
  endian
  GIT_REPOSITORY https://github.com/Kronuz/endian.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(endian)

target_link_libraries(your_target PRIVATE endian::endian)
```

The `endian` target is a pure `INTERFACE` library: it compiles nothing, requests
`cxx_std_20`, and puts the header directory on your include path. Then:

```cpp
#include "endian.hh"
```

Requires C++20. On macOS it builds with AppleClang/libc++, the same toolchain
Xapiand uses. The header keeps its original filename, so a codebase that already
`#include "endian.hh"` just needs this repo on its include path.

## Usage

```cpp
#include "endian.hh"
#include <cstdint>

// Serialize a 32-bit value big-endian on the wire, decode it back:
uint32_t v   = 0x11223344;
uint32_t net = htobe32(v);     // bytes laid out 11 22 33 44 in memory
uint32_t got = be32toh(net);   // back to host order; got == v

// Little-endian round-trip:
uint64_t id  = 0x0123456789ABCDEFULL;
uint64_t le  = htole64(id);
uint64_t back = le64toh(le);   // back == id

// 7-byte ids: the top byte always drops.
uint64_t trixel = 0xFF123456789ABCDEULL;   // top 0xFF is noise
uint64_t packed = htobe56(trixel);
uint64_t orig   = be56toh(packed);          // 0x00123456789ABCDE
```

`bswap16/32/64` give you the raw swap when you need it directly. The `hto*` /
`*toh` macros are the ones to reach for: they do the right thing per host, so the
call site stays platform-independent.

## Build & test

```sh
cmake -B build && cmake --build build && ctest --test-dir build
```

The test checks the raw swaps against known byte patterns, round-trips every
width through big- and little-endian, verifies the in-memory byte layout the
conversions produce (independent of host endianness), and exercises the 56-bit
helpers (including that the 8th byte is always dropped). It prints
`all endian tests passed` and exits 0.

## Provenance

Extracted from [Xapiand](https://github.com/Kronuz/Xapiand). `endian.hh` had zero
dependencies already (only the platform endian/byteswap headers), so it was
copied verbatim, no decoupling delta. See [ARCHITECTURE.md](ARCHITECTURE.md) for
the design and [AGENTS.md](AGENTS.md) for the repo map and invariants.

## License

MIT, Copyright (c) 2015-2018 Dubalu LLC. See [LICENSE](LICENSE).
