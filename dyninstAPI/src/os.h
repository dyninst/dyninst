
#ifndef _OS_HDR
#define _OS_HDR

/*
 * This is an initial attempt at providing an os abstraction for the paradynd
 * I am doing this so I can compile the paradynd on solaris
 *
 * This should enforce the abstract os operations
 */ 

/*
 * $Log: os.h,v $
 * Revision 1.1  1994/11/01 16:50:03  markc
 * Abstract os support.  No os specific code here.  Includes os specific
 * file.
 *
 */

#if defined(sparc_sun_sunos4_1_3)
#include "sunos.h"
#elif defined(sparc_sun_solaris2_3)
#include "solaris.h"
#elif defined(sparc_tmc_cmost7_3)
#include "cmost.h"
#endif

#include "util/h/String.h"
#include "process.h"
#include "util/h/Types.h"

bool osAttach(int process_id);
bool osStop(int process_id);
bool osDumpCore(int pid, char *dumpTo);
bool osDumpImage(const string &imageFileName, int pid, const Address a);
bool osForwardSignal(int pid, int status);
void osTraceMe();

int PCptrace(int request, process *proc, char *addr, int data, char *addr2);

#endif
