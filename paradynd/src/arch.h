/*
 * $Log: arch.h,v $
 * Revision 1.3  1995/08/24 15:03:40  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 *
 */

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4) || defined(sparc_tmc_cmost7_3)
#include "arch-sparc.h"
#endif

#if defined(hppa1_1_hp_hpux)
#include "arch-hppa.h"
#endif

#if defined(rs6000_ibm_aix3_2)
#include "arch-power.h"
#endif
