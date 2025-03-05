#include "basereg_check.h"
#include "registers/ppc64_regs.h"

int main() {
  BASEREG_CHECK(ppc64::fsr0, ppc64::vsr32);
  BASEREG_CHECK(ppc64::fsr31, ppc64::vsr63);

  BASEREG_CHECK(ppc64::fpr0, ppc64::vsr32);
  BASEREG_CHECK(ppc64::fpr31, ppc64::vsr63);

  BASEREG_CHECK(ppc64::vsr0, ppc64::vsr0);
  BASEREG_CHECK(ppc64::vsr31, ppc64::vsr31);
  BASEREG_CHECK(ppc64::vsr63, ppc64::vsr63);

  BASEREG_CHECK(ppc64::cr, ppc64::cr);
  BASEREG_CHECK(ppc64::cr0, ppc64::cr);
  BASEREG_CHECK(ppc64::cr0l, ppc64::cr);
  BASEREG_CHECK(ppc64::cr0g, ppc64::cr);
  BASEREG_CHECK(ppc64::cr0e, ppc64::cr);
  BASEREG_CHECK(ppc64::cr0s, ppc64::cr);
  BASEREG_CHECK(ppc64::cr7, ppc64::cr);
  BASEREG_CHECK(ppc64::cr7l, ppc64::cr);
  BASEREG_CHECK(ppc64::cr7g, ppc64::cr);
  BASEREG_CHECK(ppc64::cr7e, ppc64::cr);
  BASEREG_CHECK(ppc64::cr7s, ppc64::cr);

  BASEREG_CHECK(ppc64::fpscw, ppc64::fpscw);
  BASEREG_CHECK(ppc64::fpscw0, ppc64::fpscw);
  BASEREG_CHECK(ppc64::fpscw1, ppc64::fpscw);
  BASEREG_CHECK(ppc64::fpscw2, ppc64::fpscw);
  BASEREG_CHECK(ppc64::fpscw3, ppc64::fpscw);
  BASEREG_CHECK(ppc64::fpscw4, ppc64::fpscw);
  BASEREG_CHECK(ppc64::fpscw5, ppc64::fpscw);
  BASEREG_CHECK(ppc64::fpscw6, ppc64::fpscw);
  BASEREG_CHECK(ppc64::fpscw7, ppc64::fpscw);

  return EXIT_SUCCESS;
}
