// tkTools.h
// Ariel Tamches

// Some C++ stuff that I have found to be both useful and generic
// across all the tk4.0 programs I've written...

#ifndef _TK_TOOLS_H_
#define _TK_TOOLS_H_

#include "tclclean.h"
#include "tkclean.h"
#ifdef PARADYN
#include "util/h/String.h"
#else
#include "String.h"
#endif

#include <iostream.h>

class tkInstallIdle {
 private:
   bool currentlyInstalled;

   void (*usersRoutine)(ClientData cd);
   ClientData usersClientData;
      // needed since we overwrite clientdata with "this"

 private:
   static void installMe(ClientData cd);
      // "cd" will be "this"

 public:
   tkInstallIdle(void (*iUsersRoutine)(ClientData));
  ~tkInstallIdle();

   void install(ClientData cd);
};

void myTclEval(Tcl_Interp *interp, const string &);
void myTclEval(Tcl_Interp *interp, const char *);

void getScrollBarValues(Tcl_Interp *, const string &sbName,
                        float &theFirst, float &theLast);

float moveScrollBar(Tcl_Interp *, const string &sbName,
                    float tentativeNewFirst);
   // returns actual new-first, after (possible) pinning

int set_scrollbar(Tcl_Interp *interp, const string &sbname,
                  int total_width, 
                  int global_coord, int &screen_coord);

bool processScrollCallback(Tcl_Interp *interp,
                           int argc, char **argv,
                           const string &sbName,
			   int oldoffsetUnits, int totalWidthUnits,
			   int visibleWidthUnits,
                           float &newFirst);
   // tk4.0 has a complex method of reporting scroll events.
   // It will invoke a callback with a variable number of arguments,
   // depending on whether the sb was unit-moved, page-moved, or dragged.
   // This routine parses those arguments, modifies the scrollbar,
   // and returns true iff anything changed.  If so, the parameter
   // "newFirst" is modified (so you can read it and invoke application-specific
   // stuff.  After all, this routine merely updates the scrollbar)
   
#endif
