cmake_minimum_required(VERSION 3.14)
project(endian LANGUAGES CXX)

# --- Library target ---
# endian is header-only and dependency-free: one header (endian.hh) that maps
# the platform byte-order macros (htobe16/be64toh/...) onto a uniform set of
# names, plus a 56-bit (7-byte) variant for HTM trixel ids. Nothing but the
# system endian/byteswap headers behind it. So the target is a pure INTERFACE
# library that just puts the source dir on the consumer's include path.
add_library(endian INTERFACE)
target_include_directories(endian INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(endian INTERFACE cxx_std_20)
# Alias so consumers can link `endian::endian` whether they add this via
# add_subdirectory or FetchContent.
add_library(endian::endian ALIAS endian)

# Only build the test when this repo is the top-level project, so consumers
# (e.g. via FetchContent) don't build our tests.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  enable_testing()
  add_executable(endian_test test/test.cc)
  target_link_libraries(endian_test PRIVATE endian)
  add_test(NAME endian COMMAND endian_test)
endif()
