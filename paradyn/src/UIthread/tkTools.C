// tkTools.C
// Ariel Tamches

/* $Log: tkTools.C,v $
/* Revision 1.2  1995/11/06 02:29:18  tamches
/* added tclpanic and resizeScrollbar
/*
 */

#include <assert.h>
#include <stdlib.h> // exit()
#include "minmax.h"

#include "tkTools.h"

tkInstallIdle::tkInstallIdle(void (*iUsersRoutine)(ClientData)) {
   currentlyInstalled = false;
   usersRoutine = iUsersRoutine;
}

tkInstallIdle::~tkInstallIdle() {
   if (currentlyInstalled)
      ;
   currentlyInstalled = false;
}

void tkInstallIdle::install(ClientData cd) {
   // Installs installMe(), below.

   if (!currentlyInstalled) {
      Tk_DoWhenIdle(installMe, (ClientData)this);
      usersClientData = cd;

      currentlyInstalled = true;
   }
}

void tkInstallIdle::installMe(ClientData cd) {
   // A static member function that gets installed
   // Please don't confuse this with the routine that does the installation.
   tkInstallIdle *pthis = (tkInstallIdle *)cd;

   pthis->currentlyInstalled = false;
   pthis->usersRoutine(pthis->usersClientData);
}

/* ******************************************************** */

void myTclEval(Tcl_Interp *interp, const string &str) {
   myTclEval(interp, str.string_of());
}

void myTclEval(Tcl_Interp *interp, const char *buffer) {
   if (TCL_OK != Tcl_Eval(interp, buffer)) {
      cerr << interp->result << endl;
      exit(5);
   }
}

void tclpanic(Tcl_Interp *interp, const string &str) {
   cerr << str << ": " << interp->result << endl;
   exit(5);
}

/* ******************************************************** */

void getScrollBarValues(Tcl_Interp *interp, const string &scrollBarName,
			float &first, float &last) {
   string commandStr = scrollBarName + " get";
   myTclEval(interp, commandStr.string_of());
   assert(2==sscanf(interp->result, "%f %f", &first, &last));
}

float moveScrollBar(Tcl_Interp *interp, const string &scrollBarName,
		    float newFirst) {
   // This routine adjusts a scrollbar to a new position.
   // The width of the scrollbar thumb stays unchanged.
   // If pinning is necessary, we make an effort to keep newFirst close to oldFirst

   float oldFirst, oldLast;
   getScrollBarValues(interp, scrollBarName, oldFirst, oldLast);
   const float oldDifference = oldLast-oldFirst;
   if (oldDifference > 1) {
      cerr << "moveScrollBar: I was handed a scrollbar with bad values [first=" << oldFirst << "; last=" << oldLast << "]...ignoring" << endl;
      return newFirst;
   }
   if (oldLast > 1.0) {
      cerr << "moveScrollBar: I was handed a scrollbar with bad last [first=" << oldFirst << "; last=" << oldLast << "]...ignoring" << endl;
      return newFirst;
   }

   newFirst = max(newFirst, 0.0f);

   assert(newFirst >= 0.0);

   float newLast = newFirst + oldDifference;
   if (newLast > 1.0) {
      newLast = 1.0;
      newFirst = 1.0 - oldDifference;
   }

   assert(newLast <= 1.0);
   assert(newFirst >= 0.0);

   char buffer[100];
   sprintf(buffer, "%f %f", newFirst, newLast);

   string commandStr = scrollBarName + " set " + buffer;
   myTclEval(interp, commandStr.string_of());

   return newFirst;
}

int set_scrollbar(Tcl_Interp *interp, const string &sbname,
                  int total_width, 
                  int global_coord, int &screen_coord) {
   int new_first_pix = global_coord - screen_coord;
   float new_first_frac = moveScrollBar(interp, sbname,
                                        1.0*new_first_pix / total_width);
   new_first_pix = (int)(new_first_frac * total_width);
      // will differ from earlier value if pinning occurred.
   
   // now let's modify screen_coord to reflect actual placement.
   // That is, we tried to put global_coord at pixel screen_coord.
   // We may not have succeeded, if pinning occurred.
   // By definition, first_pix = global_coord - screen_coord, so
   // screen_coord = global_coord - first_pix
   screen_coord = global_coord - new_first_pix;

   return new_first_pix;
}

bool processScrollCallback(Tcl_Interp *interp,
                           int argc, char **argv,
                           const string &sbName,
			   int oldOffsetUnits, int totalWidthUnits,
			   int visibleWidthUnits,
                           float &newFirst) {
   // tk4.0 has a complex method of reporting scroll events.
   // It will invoke a callback with a variable number of arguments,
   // depending on whether the sb was unit-moved, page-moved, or dragged.
   // This routine parses those arguments, modifies the scrollbar,
   // and returns true iff anything changed.  If so, the parameter
   // "newFirst" is modified (so you can read it and invoke application-specific
   // stuff.  after all, this routine merely updates the scrollbar)
   
   assert(argc >= 2);

   float oldFirst, oldLast;
   getScrollBarValues(interp, sbName, oldFirst, oldLast);

   float tentativeNewFirst;   
   if (0==strcmp(argv[1], "moveto"))
      tentativeNewFirst = atof(argv[2]);
   else if (0==strcmp(argv[1], "scroll")) {
      int num = atoi(argv[2]);
      if (0==strcmp(argv[3], "units"))
         tentativeNewFirst = (float)(-oldOffsetUnits + num) / totalWidthUnits;
      else if (0==strcmp(argv[3], "pages"))
         tentativeNewFirst = (float)(-oldOffsetUnits + visibleWidthUnits*num) / totalWidthUnits;
      else
         assert(false);
   }
   else {
      cerr << "processScrollCallback: unexpected argv[1] (expected 'moveto' or 'units'): " << argv[1] << endl;
      assert(false);
   }

   float actualNewFirst = moveScrollBar(interp, sbName, tentativeNewFirst);
   if (actualNewFirst != oldFirst) {
      newFirst = actualNewFirst;
      return true;
   }
   return false; // no changes
}

void resizeScrollbar(Tcl_Interp *interp, const string &sbName,
                     int total_width, int visible_width) {
   // A C++ version of resize1Scrollbar (the tcl routine)
   float oldFirst, oldLast;
   getScrollBarValues(interp, sbName, oldFirst, oldLast);

   float newFirst, newLast;
   if (visible_width < total_width) {
      // the usual case: not everything fits
      float fracVisible = 1.0 * visible_width / total_width;

      newFirst = oldFirst;
      newLast = newFirst + fracVisible;

      if (newLast > 1.0) {
         float theOverflow = newLast - 1.0;
         newFirst = oldFirst - theOverflow;
         newLast = newFirst + fracVisible;
      }
   }
   else {
      // the unusual case: everything fits on screen
      newFirst = 0.0;
      newLast = 1.0;
   }

   // some assertion checking
   if (newFirst < 0)
      cerr << "resizeScrollbar warning: newFirst is " << newFirst << endl;
   if (newLast > 1)
      cerr << "resizeScrollbar warning: newLast is " << newLast << endl;

   string commandStr = sbName + " set " + string(newFirst) + " " + string(newLast);
   myTclEval(interp, commandStr);
}
