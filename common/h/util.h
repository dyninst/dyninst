

#ifndef __UTIL_H__
#define __UTIL_H__

#include "headers.h"

DLLEXPORT unsigned addrHashCommon(const Address &addr);
DLLEXPORT unsigned ptrHash(const void * addr);
#if 0
DLLEXPORT unsigned addrHash(Address addr);
DLLEXPORT unsigned addrHash4(Address addr);
DLLEXPORT unsigned addrHash16(Address addr);
#endif

DLLEXPORT unsigned addrHash(const Address &addr);
DLLEXPORT unsigned addrHash4(const Address &addr);
DLLEXPORT unsigned addrHash16(const Address &addr);
#endif
