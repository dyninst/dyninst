#ifndef DYNINST_EXTENTMAP_H
#define DYNINST_EXTENTMAP_H

#include "rangemap.h"
#include "typedefs.h"

using Extent = Range<rose_addr_t>;
using ExtentMap = RangeMap<Extent>;

#endif
