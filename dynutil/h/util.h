

#ifndef __UTIL_H__
#define __UTIL_H__

#include "dyntypes.h"
namespace Dyninst {

#if !defined(SYMTAB_EXPORT)
  #if defined(_MSC_VER)
    #if defined SYMTAB_LIB
      #define SYMTAB_EXPORT __declspec(dllexport)
    #else
      #define SYMTAB_EXPORT __declspec(dllimport)
    #endif
  #else
    #define SYMTAB_EXPORT
  #endif
#endif

#if !defined(COMMON_EXPORT)
  #if defined (_MSC_VER)
    #if defined(COMMON_LIB)
       #define COMMON_EXPORT __declspec(dllexport)
    #else
       #define COMMON_EXPORT __declspec(dllimport)   
    #endif
  #else
    #define COMMON_EXPORT
  #endif
#endif

#if !defined(INSTRUCTION_EXPORT)
  #if defined(_MSC_VER)
    #if defined(INSTRUCTION_LIB)
      #define INSTRUCTION_EXPORT __declspec(dllexport)
    #else
      #define INSTRUCTION_EXPORT __declspec(dllimport)
    #endif
  #else
    #define INSTRUCTION_EXPORT
#endif
#endif


COMMON_EXPORT unsigned addrHashCommon(const Address &addr);
COMMON_EXPORT unsigned ptrHash(const void * addr);
COMMON_EXPORT unsigned ptrHash(void * addr);

COMMON_EXPORT unsigned addrHash(const Address &addr);
COMMON_EXPORT unsigned addrHash4(const Address &addr);
COMMON_EXPORT unsigned addrHash16(const Address &addr);

COMMON_EXPORT unsigned stringhash(const std::string &s);
COMMON_EXPORT std::string itos(int);
COMMON_EXPORT std::string utos(unsigned);

#define WILDCARD_CHAR '?'
#define MULTIPLE_WILDCARD_CHAR '*'

COMMON_EXPORT bool wildcardEquiv(const std::string &us, const std::string &them, bool checkCase = false );

}
#endif
