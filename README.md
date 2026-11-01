# endian

Part of [Kronuz C++ Libraries](https://github.com/Kronuz/cpp-libraries), a collection of small, reusable C++ libraries.

[![CI](https://github.com/Kronuz/endian/actions/workflows/ci.yml/badge.svg)](https://github.com/Kronuz/endian/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![Header-only](https://img.shields.io/badge/header--only-yes-green.svg)

Header-only byte-order conversion for C++20: one set of htobe32/le64toh macros on Linux, macOS, and FreeBSD.
Each name resolves to a compiler byte-swap builtin or to a plain cast, and the
header adds 56-bit variants the system headers lack. Reach for it when integers
go into a wire format or an on-disk layout in a fixed byte order.

## ✨ Highlights

- **One spelling on every platform.** `<endian.h>` (Linux), `<machine/endian.h>`
  (macOS), and `<sys/endian.h>` (FreeBSD) map onto the same names and the same
  `__BYTE_ORDER` constants.
- **The full set.** `bswap16/32/64`, `htobe*`/`be*toh`, and `htole*`/`le*toh`
  for 16, 32, and 64 bits.
- **56-bit variants.** `htobe56`, `htole56`, `be56toh`, and `le56toh` keep the
  low 7 bytes of a `uint64_t` and drop the top one.
- **One instruction or none.** On GCC and Clang a conversion is one
  `__builtin_bswap*` when the host order differs, and a cast when it matches.
- **The platform wins.** Every 16, 32, and 64-bit name is `#ifndef`-guarded, so
  a definition from the system header stays in place.

## 🚀 Quick start

```cpp
#include "endian.hh"

#include <cstdint>
#include <cstdio>
#include <cstring>

int main() {
	uint32_t wire = htobe32(0x11223344);  // big-endian in memory
	unsigned char bytes[4];
	std::memcpy(bytes, &wire, sizeof bytes);
	std::printf("%02x %02x %02x %02x\n", bytes[0], bytes[1], bytes[2], bytes[3]);  // 11 22 33 44
	std::printf("%x\n", be32toh(wire));  // 11223344

	uint64_t id = 0xFF123456789ABCDEULL;  // only the low 7 bytes count
	std::printf("%llx\n", (unsigned long long)be56toh(htobe56(id)));  // 123456789abcde
}
```

## 📦 Installation

Requires a C++20 compiler and CMake 3.14 or newer; no dependencies. CI builds it
on Linux (x86_64, arm64), macOS (arm64, x86_64), and FreeBSD, plus ASan, UBSan,
and TSan builds.

```cmake
include(FetchContent)
FetchContent_Declare(endian
  GIT_REPOSITORY https://github.com/Kronuz/endian.git
  GIT_TAG        a625b942083418cd77dc7a2eea6993137662dbdd)  # or main for the tip
FetchContent_MakeAvailable(endian)

target_link_libraries(my_app PRIVATE endian::endian)
```

```cpp
#include "endian.hh"
```

The header has no dependencies, so copying `endian.hh` onto the include path
works too.

## 🎯 When to use it

Use it to write and read integers in a fixed byte order: network protocols, file
formats, and big-endian keys that sort correctly as bytes.

Not for:

- **C++23 code that only swaps.** `std::byteswap` covers 16, 32, and 64 bits,
  and `std::endian` reports the host order.
- **Windows.** The Windows branch includes an `<include/endian.h>` that this
  repository does not ship, and CI does not build it.
- **Arguments with side effects or narrow types.** The names are macros: pass an
  already-typed unsigned integer, evaluated once, before the call.
- **Full 64-bit values in the 56-bit forms.** `htobe56(x)` shifts `x` left by 8
  first, so the top byte is lost.

## 📚 Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md): platform detection, the swap layers, and
  the 56-bit variant.
- [AGENTS.md](AGENTS.md): repository map, invariants, and traps.
- [Catalog entry](https://github.com/Kronuz/cpp-libraries/blob/main/AGENTS.md#endian)
  in Kronuz C++ Libraries.

## 🛠️ Building and testing

```sh
cmake -B build
cmake --build build
ctest --test-dir build
```

## 🤝 Contributing

Issues and pull requests are welcome. Read [AGENTS.md](AGENTS.md) and the
catalog's [rules of engagement](https://github.com/Kronuz/cpp-libraries/blob/main/AGENTS.md#rules-of-engagement)
first; CI must pass on every platform.

## 📄 License

MIT, Copyright (c) 2026 Germán Méndez Bravo (Kronuz) and Copyright (c) 2015-2020
Dubalu LLC. See [LICENSE](LICENSE). Extracted from
[Xapiand](https://github.com/Kronuz/Xapiand).
