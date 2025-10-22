#ifndef DYNINST_TESTS_EMITTER
#define DYNINST_TESTS_EMITTER

#include <array>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>

#include "dyninstAPI/src/codegen.h"

namespace Dyninst {

  template<size_t N>
  using emitter_buffer_t = std::array<uint8_t, N>;

  template <size_t N>
  inline bool verify_emitter(codeGen &gen, std::array<uint8_t, N> const& expected) {
    auto *buf = reinterpret_cast<uint8_t*>(gen.start_ptr());

    bool const equal = [&]() {
      if(gen.used() != N) {
        return false;
      }
      return std::equal(expected.begin(), expected.end(), buf);
    }();

    std::cerr << "Encoded " << std::dec << gen.used() << " bytes 0x";
    for(auto i=0U; i<gen.used(); i++) {
      std::cerr << std::setw(2) << std::setfill('0') << std::hex << static_cast<uint32_t>(buf[i]);
    }

    if(!equal) {
      std::cerr << ", FAILED: expected " << N << " bytes 0x";
      for(auto b : expected) {
        std::cerr << std::setw(2) << std::setfill('0') << std::hex << static_cast<uint32_t>(b);
      }
      std::cerr << "\n";
    } else {
      std::cerr << ", OK\n";
    }

    // Reset the codegen's internal buffer so it can be reused
    gen.setIndex(0);
    std::memset(buf, 0, N);

    // Deallocate all registers
    gen.rs()->cleanSpace();

    return equal;
  }

}
#endif
