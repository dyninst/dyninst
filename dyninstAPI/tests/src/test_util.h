
#ifndef _test_h_
#define _test_h_

#include "BPatch_thread.h"
#include "BPatch_image.h"


#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 10 apr 2001 
#define P_sleep(sec) Sleep(1000*(sec))
#else
#define P_sleep(sec) sleep(sec)
#endif

void waitUntilStopped(BPatch *, BPatch_thread *appThread, 
                      int testnum, const char *testname);
void signalAttached(BPatch_thread *appThread, BPatch_image *appImage);
int startNewProcessForAttach(const char *pathname, char *argv[]);

#endif
