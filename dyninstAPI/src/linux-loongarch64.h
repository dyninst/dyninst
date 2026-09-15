/* See the dyninst/COPYRIGHT file for copyright information. */

#ifndef LINUX_LOONGARCH64_H
#define LINUX_LOONGARCH64_H

#include "dyntypes.h"

using namespace Dyninst;

// Linux-specific LoongArch64 definitions
extern Address region_lo(const Address x);
extern Address region_hi(const Address x);
extern Address region_lo_64(const Address x);
extern Address region_hi_64(const Address x);

#endif
