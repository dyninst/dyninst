#include "common/h/timing.h"

#ifdef os_bgp
double calcCyclesPerSecondOS() {
  return 8.5e8;
}
#endif // bgp stuff

#ifdef os_bgl
double calcCyclesPerSecondOS() {
  return 7e8;  
}
#endif // bgl stuff
