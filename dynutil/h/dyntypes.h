
#if !defined(DYNTYPES_H)
#define DYNTYPES_H

#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifndef FILE__
#define FILE__ strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#endif

#if defined (_MSC_VER)
#include <hash_map>
using stdext::hash_map;
#else
#include <ext/hash_map>
#include <ext/hash_set>
using namespace __gnu_cxx;
namespace __gnu_cxx {
   template<> struct hash<std::string> {
      hash<char*> h;
      unsigned operator()(const std::string &s) const {
         return h(s.c_str());
      };
   };
};

#endif

namespace Dyninst
{
   typedef unsigned long Address;   
   typedef unsigned long Offset;

#if defined(_MSC_VER)
   typedef HANDLE PID;
   typedef HANDLE LWP;
   typedef HANDLE TID;

#define NULL_PID     INVALID_HANDLE_VALUE
#define NULL_LWP     INVALID_HANDLE_VALUE
#define NULL_TID     INVALID_HANDLE_VALUE

#else
   typedef int PID;
   typedef int LWP;
   typedef int TRID;

#define NULL_PID     -1
#define NULL_LWP     -1
#define NULL_TRID     -1
#endif
}

#endif
