#include "registers/loongarch64_regs.h"
/*
 * See the dyninst/COPYRIGHT file for copyright information.
 */

#include "linux-loongarch64.h"
#include "dyntypes.h"
using namespace Dyninst;
#include <sys/user.h>
#include <assert.h>
#include <unistd.h>

// 32-bit versions (not used on loongarch64 but needed for compilation)
Address region_lo(const Address x) {
    return region_lo_64(x);
}

Address region_hi(const Address x) {
    return region_hi_64(x);
}

// floor of inferior malloc address range within a single branch of x
Address region_lo_64(const Address x) {
    const Address floor = getpagesize();
    assert(x >= floor);
    // LoongArch64 has 26-bit branch range (±128MB)
    if ((x > floor) && (x - floor > 0x7FFFFFF))
        return x - 0x7FFFFFF;
    return floor;
}

// ceiling of inferior malloc address range within a single branch of x
Address region_hi_64(const Address x) {
    const Address ceiling = ~(Address)0;
    // LoongArch64 has 26-bit branch range (±128MB)
    if ((x < ceiling) && (ceiling - x > 0x7FFFFFF))
        return x + 0x7FFFFFF;
    return ceiling;
}