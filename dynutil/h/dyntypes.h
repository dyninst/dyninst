
#if !defined(DYNTYPES_H)
#define DYNTYPES_H

#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifndef FILE__
#define FILE__ strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#endif

#include <boost/functional/hash.hpp>
//#define dyn_hash_set boost::unordered_set
//#define dyn_hash_map boost::unordered_map

#if defined (_MSC_VER)
  //**************** Windows ********************
  #include <hash_map>
  #define dyn_hash_map stdext::hash_map
  #define DECLTHROW(x)
#elif defined(__GNUC__)
  #define DECLTHROW(x) throw(x)
  //***************** GCC ***********************
   #if (__GNUC__ > 4) || \
      (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
      //**************** GCC >= 4.3.0 ***********
      #include <tr1/unordered_set>
      #include <tr1/unordered_map>
      #define dyn_hash_set std::tr1::unordered_set
      #define dyn_hash_map std::tr1::unordered_map
   #else
      //**************** GCC < 4.3.0 ************
      #include <ext/hash_map>
      #include <ext/hash_set>
      #define dyn_hash_set __gnu_cxx::hash_set
      #define dyn_hash_map __gnu_cxx::hash_map    
      using namespace __gnu_cxx;
      namespace __gnu_cxx {
 
        template<> struct hash<std::string> {
           hash<char*> h;
           unsigned operator()(const std::string &s) const {
             return h(s.c_str());
           };
        };
      }

   #endif
#else
   #error Unknown compiler
#endif


namespace Dyninst
{
   typedef unsigned long Address;   
   typedef unsigned long Offset;

#if defined(_MSC_VER)
   typedef HANDLE PID;
   typedef HANDLE LWP;
   typedef HANDLE THR_ID;

#define NULL_PID     INVALID_HANDLE_VALUE
#define NULL_LWP     INVALID_HANDLE_VALUE
#define NULL_THR_ID     INVALID_HANDLE_VALUE

#else
   typedef int PID;
   typedef int LWP;
   typedef int THR_ID;

#define NULL_PID     -1
#define NULL_LWP     -1
#define NULL_THR_ID     -1
#endif
}

#endif
