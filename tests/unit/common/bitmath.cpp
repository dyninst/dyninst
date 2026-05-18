#include "common/src/bitmath.h"

#include <cstdlib>
#include <iostream>

static inline bool po2(int x, bool expected) {
  using Dyninst::isPowerOf2;
  if(isPowerOf2(x) != expected) {
    std::cerr << "isPowerOf2(" << x << ") failed.\n";
    return false;
  }
  return true;
}

static inline bool l2(int x, int exponent, bool expected) {
  boost::optional<uint8_t> val = Dyninst::ilog2(x);

  if(!val) {
    if(expected) {
      std::cerr << "ilog2(" << x << ") failed: no value found.\n";
      return false;
    }
    return true;
  }
  const bool equal = *val == exponent;
  if(equal != expected) {
    std::cerr << "ilog2(" << x << ") failed: mismatched exponents. "
              << "Expected " << exponent << ", got " << static_cast<int>(*val) << "\n";
    return false;
  }
  return true;
}

int main() {
  bool passed = true;

  passed &= po2(0, false);
  passed &= po2(3, false);
  passed &= po2(2, true);
  passed &= po2(1<<6, true);

  passed &= l2(2, 1, true);
  passed &= l2(4, 2, true);
  passed &= l2(-1, 1, false);
  passed &= l2(0, 1, false);
  passed &= l2(0, 1, false);
  passed &= l2(16, 3, false);
  passed &= l2(16, 4, true);

  return !passed ? EXIT_FAILURE : EXIT_SUCCESS;
}
