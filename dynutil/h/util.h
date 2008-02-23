

#ifndef __UTIL_H__
#define __UTIL_H__

#include "dyntypes.h"
namespace Dyninst {

#if !defined(DLLEXPORT)
#if defined (_MSC_VER)
/* If we're on Windows, we need to explicetely export these functions: */
   #if defined(DLL_BUILD)
      #define DLLEXPORT __declspec(dllexport)
   #else
      #define DLLEXPORT __declspec(dllimport)   
   #endif
#else
   #define DLLEXPORT
#endif
#endif


DLLEXPORT unsigned addrHashCommon(const Address &addr);
DLLEXPORT unsigned ptrHash(const void * addr);

DLLEXPORT unsigned addrHash(const Address &addr);
DLLEXPORT unsigned addrHash4(const Address &addr);
DLLEXPORT unsigned addrHash16(const Address &addr);

DLLEXPORT unsigned hash(const std::string &s);
DLLEXPORT std::string itos(int);
DLLEXPORT std::string utos(unsigned);

#define WILDCARD_CHAR '?'
#define MULTIPLE_WILDCARD_CHAR '*'

DLLEXPORT bool wildcardEquiv(const std::string &us, const std::string &them, bool checkCase = false );

}
#endif
