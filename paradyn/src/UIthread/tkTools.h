// tkTools.h
// Ariel Tamches

// Some C++ stuff that I have found to be both useful and generic
// across all the tk4.0 programs I've written...

/*
 * $Log: tkTools.h,v $
 * Revision 1.4  1996/02/02 18:54:33  tamches
 * added setResultBool
 *
 * Revision 1.3  1995/11/29 00:20:37  tamches
 * added tcl_cmd_installer
 *
 * Revision 1.2  1995/11/06 02:28:02  tamches
 * added tclpanic and resizeScrollbar
 *
 */

#ifndef _TK_TOOLS_H_
#define _TK_TOOLS_H_

#include "tcl.h"
#include "tk.h"

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
void tclpanic(Tcl_Interp *interp, const string &str);

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

void resizeScrollbar(Tcl_Interp *interp, const string &sbName,
                     int total_width, int visible_width);

void setResultBool(Tcl_Interp *interp, bool val);

class tcl_cmd_installer {
 private:
   static void dummy_delete_proc(ClientData) {}

 public:
   tcl_cmd_installer(Tcl_Interp *interp, const string &tclCmdName, Tcl_CmdProc proc) {
      Tcl_CreateCommand(interp, tclCmdName.string_of(), proc, NULL, &dummy_delete_proc);
   }

   tcl_cmd_installer(Tcl_Interp *interp, const string &tclCmdName, Tcl_CmdProc proc, ClientData cd) {
      Tcl_CreateCommand(interp, tclCmdName.string_of(), proc, cd, &dummy_delete_proc);
   }
  ~tcl_cmd_installer() {}
};

#endif
