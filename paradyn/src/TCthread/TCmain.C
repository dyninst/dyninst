/* TCmain.C
 * tunable constants entry point
 *
 * $Log: TCmain.C,v $
 * Revision 1.2  1995/11/06 19:25:36  tamches
 * dictionary_hash --> dictionary_lite
 *
 * Revision 1.1  1995/02/27  18:50:01  tamches
 * First version of TCthread; files tunableConst.h and .C have
 * simply moved from the util lib (and have been changed); file
 * TCmain.C is completely new.
 *
 *
 */


#include <iostream.h>

#include "thread/h/thread.h"
#include "../pdMain/paradyn.h"

#include "tunableConst.h"

// Time to define some important static member variables
typedef dictionary_lite<string, tunableBooleanConstant> tunBoolAssocArrayType;
typedef dictionary_lite<string, tunableFloatConstant>   tunFloatAssocArrayType;

unsigned boolHashFunc(const string &name)  { return string::hash(name); }
unsigned floatHashFunc(const string &name) { return string::hash(name); }

tunBoolAssocArrayType  tunableConstantRegistry::allBoolTunables(boolHashFunc);
tunFloatAssocArrayType tunableConstantRegistry::allFloatTunables(floatHashFunc);

// TC thread entry point:
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
