#ifndef TEST_DATA_H
#define TEST_DATA_H

#include "test_lib_dll.h"
#include <vector>

enum start_state_t {
   STOPPED,
   RUNNING,
   NOMUTATEE,
};

enum create_mode_t {
   CREATE = 0,
   USEATTACH = 1,
   BOTH,
};

enum enabled_t {
   DISABLED = 0,
   ENABLED = 1,
};

typedef std::vector<char*> mutatee_list_t;

typedef struct {
   bool alpha_dec_osf5_1;
   bool i386_unknown_linux2_4; 
   bool _i386_unknown_nt4_0; 
   bool _ia64_unknown_linux2_4;
   bool _x86_64_unknown_linux2_4;
   bool mips_sgi_irix6_5;
   bool _rs6000_ibm_aix5_1;
   bool sparc_sun_solaris2_8;
} platforms_t;

struct TESTLIB_DLL_EXPORT TestData {
   char *name;
   char *soname;
   mutatee_list_t &mutatee;
   platforms_t &platforms;
   start_state_t state;
   int oldtest;
   int subtest;
   create_mode_t useAttach;
   enabled_t enabled;

   TestData(char *name, 
         char *soname, 
         mutatee_list_t &mutatee, 
         platforms_t &platforms,
         start_state_t state,
         int oldtest,
         int subtest,
         create_mode_t useAttach,
         enabled_t enabled
         );
};

typedef TestData test_data_t;

#endif /* TEST_DATA_H */
