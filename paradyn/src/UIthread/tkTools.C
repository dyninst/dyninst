/*
 * Copyright (c) 1996-1998 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// tkTools.C
// Ariel Tamches

/* $Id: tkTools.C,v 1.17 2002/07/31 18:29:15 willb Exp $ */

#include <assert.h>
#include <stdlib.h> // exit()
#include "minmax.h"
#include "common/h/headers.h"
#include "tkTools.h"

#if !defined(i386_unknown_nt4_0)
#include <strstream.h>
#endif

tkInstallIdle::tkInstallIdle(void (*iUsersRoutine)(ClientData)) {
   currentlyInstalled = false;
   usersRoutine = iUsersRoutine;
}

tkInstallIdle::~tkInstallIdle() {
   //if (currentlyInstalled)
   //    ;
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
   myTclEval(interp, str.c_str());
}

void myTclEval(Tcl_Interp *interp, const char *buffer) {
   if (TCL_OK != Tcl_Eval(interp, const_cast<char*>(buffer))) {
      tclpanic(interp, "Interpretation of Tcl source failed");
   }
}

void tclpanic(Tcl_Interp *interp, const string &str) {
	ostrstream ostr;

	ostr << str << ": " << Tcl_GetStringResult( interp ) << ends;
#if !defined(i386_unknown_nt4_0)
	cerr << ostr.str() << endl;
#else
	MessageBox( NULL, ostr.str(), "Fatal", MB_ICONSTOP | MB_OK );
#endif // !defined(i386_unknown_nt4_0)
	exit(5);
}

/* ******************************************************** */

void getScrollBarValues(Tcl_Interp *interp, const string &scrollBarName,
			float &first, float &last) {
   string commandStr = scrollBarName + " get";
   myTclEval(interp, commandStr.c_str());
   bool aflag;
   aflag = (2==sscanf( Tcl_GetStringResult( interp ), "%f %f", &first, &last));
   assert(aflag);
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
      newFirst = 1.0f - oldDifference;
   }

   assert(newLast <= 1.0);
   assert(newFirst >= 0.0);

   string buffer;
   buffer = string(newFirst) + string(" ");
   buffer += string(newLast);

   string commandStr = scrollBarName + " set " + buffer;
   myTclEval(interp, commandStr.c_str());

   return newFirst;
}

int set_scrollbar(Tcl_Interp *interp, const string &sbname,
                  int total_width, 
                  int global_coord, int &screen_coord) {
   int new_first_pix = global_coord - screen_coord;
   float new_first_frac = moveScrollBar(interp, sbname,
                                        1.0f * new_first_pix / total_width);
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
      tentativeNewFirst = static_cast<float>(atof(argv[2]));
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
      float fracVisible = 1.0f * visible_width / total_width;

      newFirst = oldFirst;
      newLast = newFirst + fracVisible;

      if (newLast > 1.0) {
         float theOverflow = newLast - 1.0f;
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

void setResultBool(Tcl_Interp *interp, bool val) {
   // Apparantly, tcl scripts cannot use the ! operator on "true"
   // or "false; only on "1" or "0".  Hence, the latter style is preferred.
   if (val)
      //strcpy( Tcl_GetStringResult( interp ), "true");
      strcpy( Tcl_GetStringResult( interp ), "1");
   else
      //strcpy( Tcl_GetStringResult( interp ), "false");
      strcpy( Tcl_GetStringResult( interp ), "0");
}
