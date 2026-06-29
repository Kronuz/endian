// Smoke test for the standalone endian library.
//
// Exercises endian.hh: the uniform byte-order conversion macros
// (htobe16/htole16/be16toh/le16toh and the 32/64-bit widths), the raw bswap*
// helpers, and the 56-bit (7-byte) variant used for HTM trixel ids. The macros
// resolve to whichever direction is a swap and whichever is a no-op on the host,
// so the test checks both the value round-trips and the actual byte layout the
// conversions produce.
//
// Build via CMake: cmake -B build && cmake --build build && ctest --test-dir build
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "endian.hh"


// ---------------------------------------------------------------------------
// Is this host little-endian? Decided at runtime by inspecting the bytes of a
// known value, independent of the macros under test so it can't mask a bug.
// ---------------------------------------------------------------------------

static bool host_is_little_endian() {
	const uint32_t one = 1;
	unsigned char first;
	std::memcpy(&first, &one, 1);
	return first == 1;
}


// ---------------------------------------------------------------------------
// Raw byte swaps: known-pattern checks across the 16/32/64-bit widths.
// ---------------------------------------------------------------------------

static void test_bswap() {
	assert(bswap16(uint16_t(0x1122)) == uint16_t(0x2211));
	assert(bswap32(uint32_t(0x11223344)) == uint32_t(0x44332211));
	assert(bswap64(uint64_t(0x1122334455667788ULL)) == uint64_t(0x8877665544332211ULL));

	// A swap is its own inverse.
	assert(bswap16(bswap16(uint16_t(0xABCD))) == uint16_t(0xABCD));
	assert(bswap32(bswap32(uint32_t(0xDEADBEEF))) == uint32_t(0xDEADBEEF));
	assert(bswap64(bswap64(uint64_t(0x0123456789ABCDEFULL))) == uint64_t(0x0123456789ABCDEFULL));

	std::printf("endian bswap OK: bswap16/bswap32/bswap64\n");
}


// ---------------------------------------------------------------------------
// Round-trips: host -> big/little -> host is the identity for every width.
// ---------------------------------------------------------------------------

static void test_roundtrip() {
	const uint16_t v16 = 0xABCD;
	const uint32_t v32 = 0x89ABCDEF;
	const uint64_t v64 = 0x0123456789ABCDEFULL;

	assert(be16toh(htobe16(v16)) == v16);
	assert(le16toh(htole16(v16)) == v16);
	assert(be32toh(htobe32(v32)) == v32);
	assert(le32toh(htole32(v32)) == v32);
	assert(be64toh(htobe64(v64)) == v64);
	assert(le64toh(htole64(v64)) == v64);

	std::printf("endian round-trip OK: host -> be/le -> host across 16/32/64\n");
}


// ---------------------------------------------------------------------------
// Byte layout: confirm host-to-big-endian actually lays bytes out big-end-first
// in memory, and host-to-little-endian little-end-first, regardless of host.
// ---------------------------------------------------------------------------

static void test_byte_layout() {
	// 0x11223344 stored big-endian is the bytes 11 22 33 44; little-endian is
	// 44 33 22 11. The macros must produce exactly that, whichever the host is.
	const uint32_t v = 0x11223344;
	const unsigned char be[4] = { 0x11, 0x22, 0x33, 0x44 };
	const unsigned char le[4] = { 0x44, 0x33, 0x22, 0x11 };

	uint32_t be_packed = htobe32(v);
	uint32_t le_packed = htole32(v);
	unsigned char be_bytes[4];
	unsigned char le_bytes[4];
	std::memcpy(be_bytes, &be_packed, 4);
	std::memcpy(le_bytes, &le_packed, 4);
	assert(std::memcmp(be_bytes, be, 4) == 0);
	assert(std::memcmp(le_bytes, le, 4) == 0);

	// One direction is always a no-op on the host; the other is a swap. Pin
	// down which, so a regression that swaps the wrong way is caught.
	if (host_is_little_endian()) {
		assert(htole32(v) == v);
		assert(htobe32(v) == bswap32(v));
	} else {
		assert(htobe32(v) == v);
		assert(htole32(v) == bswap32(v));
	}

	std::printf("endian byte-layout OK: be/le packing matches memory order\n");
}


// ---------------------------------------------------------------------------
// 56-bit (7-byte) helpers: htobe56/htole56/be56toh/le56toh. Xapiand uses these
// for HTM trixel ids, which live in 7 bytes; the top byte must always drop.
// ---------------------------------------------------------------------------

static void test_56bit() {
	// Only the low 7 bytes are meaningful. The stray high byte (0xFF here) must
	// not survive a host -> big-endian -> host round-trip.
	const uint64_t id = 0xFF123456789ABCDEULL;        // top byte 0xFF is noise
	const uint64_t id56 = id & 0x00FFFFFFFFFFFFFFULL;  // 0x00123456789ABCDE

	assert(be56toh(htobe56(id)) == id56);
	assert(le56toh(htole56(id)) == id56);

	// A clean 56-bit value (top byte already zero) survives unchanged.
	const uint64_t clean = 0x00AABBCCDDEEFF00ULL;
	assert(be56toh(htobe56(clean)) == clean);
	assert(le56toh(htole56(clean)) == clean);

	// Whichever direction is the no-op on this host masks to 7 bytes outright.
	if (host_is_little_endian()) {
		assert(htole56(id) == id56);
	} else {
		assert(htobe56(id) == id56);
	}

	std::printf("endian 56-bit OK: htobe56/htole56 drop the 8th byte and round-trip\n");
}


int main() {
	test_bswap();
	test_roundtrip();
	test_byte_layout();
	test_56bit();
	std::printf("all endian tests passed\n");
	return 0;
}
