#include "basereg_check.h"
#include "registers/ppc32_regs.h"

int main() {
  BASEREG_CHECK(ppc32::fsr0, ppc32::fpr0);
  BASEREG_CHECK(ppc32::fsr31, ppc32::fpr31);

  BASEREG_CHECK(ppc32::fpr0, ppc32::fpr0);
  BASEREG_CHECK(ppc32::fpr31, ppc32::fpr31);

  BASEREG_CHECK(ppc32::cr, ppc32::cr);
  BASEREG_CHECK(ppc32::cr0, ppc32::cr);
  BASEREG_CHECK(ppc32::cr0l, ppc32::cr);
  BASEREG_CHECK(ppc32::cr0g, ppc32::cr);
  BASEREG_CHECK(ppc32::cr0e, ppc32::cr);
  BASEREG_CHECK(ppc32::cr0s, ppc32::cr);
  BASEREG_CHECK(ppc32::cr7, ppc32::cr);
  BASEREG_CHECK(ppc32::cr7l, ppc32::cr);
  BASEREG_CHECK(ppc32::cr7g, ppc32::cr);
  BASEREG_CHECK(ppc32::cr7e, ppc32::cr);
  BASEREG_CHECK(ppc32::cr7s, ppc32::cr);


  BASEREG_CHECK(ppc32::fpscw, ppc32::fpscw);
  BASEREG_CHECK(ppc32::fpscw0, ppc32::fpscw);
  BASEREG_CHECK(ppc32::fpscw1, ppc32::fpscw);
  BASEREG_CHECK(ppc32::fpscw2, ppc32::fpscw);
  BASEREG_CHECK(ppc32::fpscw3, ppc32::fpscw);
  BASEREG_CHECK(ppc32::fpscw4, ppc32::fpscw);
  BASEREG_CHECK(ppc32::fpscw5, ppc32::fpscw);
  BASEREG_CHECK(ppc32::fpscw6, ppc32::fpscw);
  BASEREG_CHECK(ppc32::fpscw7, ppc32::fpscw);

  return EXIT_SUCCESS;
}
