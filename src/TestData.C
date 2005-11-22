#include "TestData.h"

TestData::TestData(
      char *n, 
      char *so, 
      mutatee_list_t &m, 
      platforms_t &p,
      start_state_t st,
      int old,
      int sub,
      create_mode_t use,
      enabled_t en
      ) : mutatee(m), platforms(p)

{
   name = n;
   soname = so;
   state = st;
   oldtest = old;
   subtest = sub;
   useAttach = use;
   enabled = en;
}
