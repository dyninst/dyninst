#include "auxvtypes.h"

#include <cstdlib>
using namespace std;

const char *auxv_type_to_string(int type) {
  static const char *unknown = "UNKNOWN";
  static const char *names[] = {
    "AT_NULL",		       // 0
    "AT_IGNORE",	
    "AT_EXECFD",	
    "AT_PHDR",		
    "AT_PHENT",	
    "AT_PHNUM",	         // 5
    "AT_PAGESZ",	
    "AT_BASE",		
    "AT_FLAGS",	
    "AT_ENTRY",	
    "AT_NOTELF",	       // 10
    "AT_UID",		
    "AT_EUID",		
    "AT_GID",		
    "AT_EGID",		
    "AT_PLATFORM",       // 15
    "AT_HWCAP",		
    "AT_CLKTCK",	
    "AT_FPUCW",	
    "AT_DCACHEBSIZE",	
    "AT_ICACHEBSIZE",	   // 20
    "AT_UCACHEBSIZE",	
    "AT_IGNOREPPC",	
    "AT_SECURE",	
    unknown,
    unknown,           // 25
    unknown,
    unknown,
    unknown,
    unknown,
    unknown,           // 30,
    unknown,
    "AT,_SYSINFO",
    "AT_SYSINFO_EHDR",
    "AT_L1I_CACHESHAPE",
    "AT_L1D_CACHESHAPE", // 35
    "AT_L2_CACHESHAPE",	
    "AT_L3_CACHESHAPE"
  };
  
  const size_t size = (sizeof(names) / sizeof(const char*));
  return (type >= 0 && type < size) ? names[type] : unknown;
}
