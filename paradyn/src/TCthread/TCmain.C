/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/* TCmain.C
 * tunable constants entry point
 *
 * $Log: TCmain.C,v $
 * Revision 1.10  2004/06/21 19:37:34  pcroth
 * Disabled unused function.
 *
 * Revision 1.9  2004/03/23 01:12:28  eli
 * Updated copyright string
 *
 * Revision 1.8  2003/07/18 15:44:32  schendel
 * fix obsolete header file warnings by updating to new C++ header files;
 *
 * Revision 1.7  2003/07/15 22:45:58  schendel
 * rename string to pdstring
 *
 * Revision 1.6  2002/07/25 19:22:32  willb
 * Change default thread lib to libpdthread; threadsafety fixes for FE; gcc-3.1 compliance for AIX
 *   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * thread-safety fixes
 *
 * Revision 1.5  2001/06/20 20:37:34  schendel
 * Change order of including header files to fix dependency error.
 *
 * Revision 1.4  1997/10/28 20:34:49  tamches
 * dictionary_lite --> dictionary_hash
 *
 * Revision 1.3  1996/08/16 21:04:34  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.2  1995/11/06 19:25:36  tamches
 * dictionary_hash --> dictionary_lite
 *
 * Revision 1.1  1995/02/27  18:50:01  tamches
 * First version of TCthread; files tunableConst.h and .C have
 * simply moved from the util lib (and have been changed); file
 * TCmain.C is completely new.
 *
 */

#include <iostream>

#include "../pdMain/paradyn.h"
#include "pdthread/h/thread.h"

#include "tunableConst.h"

// Time to define some important static member variables
typedef dictionary_hash<pdstring, tunableBooleanConstant> tunBoolAssocArrayType;
typedef dictionary_hash<pdstring, tunableFloatConstant>   tunFloatAssocArrayType;

unsigned boolHashFunc(const pdstring &name)  { return pdstring::hash(name); }
unsigned floatHashFunc(const pdstring &name) { return pdstring::hash(name); }

tunBoolAssocArrayType  tunableConstantRegistry::allBoolTunables(boolHashFunc);
tunFloatAssocArrayType tunableConstantRegistry::allFloatTunables(floatHashFunc);

// TC thread entry point:
#if READY
void *TCmain(void *) {
   (void)msg_send(MAINtid, MSG_TAG_TC_READY, NULL, 0);
      // tell pdMain that it's okay now to start launching other
      // threads.  (We insist on going first to properly set up
      // the tunable constant registry; other threads that use
      // tunable constants will agree that TCthread should be up and
      // running before tunables are accessed)

   tunableBooleanConstantDeclarator tcInDeveloperMode("developerMode",
	 "Allow access to all tunable constants, including those limited to developer mode.  (Use with caution)",
	 false, // initial value
	 NULL, // no callback routine (yet)
         userConstant);

   // Wait for UI to quit [i.e. wait for paradyn to shut down].
   // BUT NOTE -- a thr_join(UIMtid, ...) must not take place before UIthread
   // has started.  Why not?  Because until it's started, UIMtid has the value
   // of 0; a thr_join(0, ...) will return after ANY thread quits, which is not
   // what we want at all!  We don't want to shut down TCthread until we're sure
   // that paradyn is quitting.  So, before doing the thr_join(), we wait for an
   // appropriate signal from UIthread...
//   tag_t UItag; unsigned count=0;
//   int UIid = msg_recv(&UItag, NULL, &count);
//   if (UIid == THR_ERR) {
//      cerr << "TC thread -- initial msg_recv() from UIthread failed...quitting" << endl;
//      thr_exit(NULL);
//   }

   // now UIid (result of msg_recv) should equal UIMtid!
//   assert(UIid == UIMtid);

   // so finally, we can thr_join() with confidence.
//   int result=thr_join(UIMtid, NULL, NULL);
   int result=thr_join(MAINtid, NULL, NULL);
   if (result==THR_ERR) {
      cerr << "TC thread -- thr_join with UIM failed...continuing" << endl;
   }

   cout << "TCmain: exiting" << endl;

   thr_exit(NULL);
   return NULL;
}
#endif // READY
