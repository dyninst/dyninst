

#ifndef __UTIL_H__
#define __UTIL_H__

#include "dyntypes.h"
namespace Dyninst {

#if !defined(DLLEXPORT_COMMON)
#if defined (_MSC_VER)
/* If we're on Windows, we need to explicetely export these functions: */
   #if defined(DLL_BUILD)
      #define DLLEXPORT_COMMON __declspec(dllexport)
   #else
      #define DLLEXPORT_COMMON __declspec(dllimport)   
   #endif
#else
   #define DLLEXPORT_COMMON
#endif
#endif


DLLEXPORT_COMMON unsigned addrHashCommon(const Address &addr);
DLLEXPORT_COMMON unsigned ptrHash(const void * addr);
DLLEXPORT_COMMON unsigned ptrHash(void * addr);

DLLEXPORT_COMMON unsigned addrHash(const Address &addr);
DLLEXPORT_COMMON unsigned addrHash4(const Address &addr);
DLLEXPORT_COMMON unsigned addrHash16(const Address &addr);

DLLEXPORT_COMMON unsigned hash(const std::string &s);
DLLEXPORT_COMMON std::string itos(int);
DLLEXPORT_COMMON std::string utos(unsigned);

#define WILDCARD_CHAR '?'
#define MULTIPLE_WILDCARD_CHAR '*'

DLLEXPORT_COMMON bool wildcardEquiv(const std::string &us, const std::string &them, bool checkCase = false );

}
#endif
