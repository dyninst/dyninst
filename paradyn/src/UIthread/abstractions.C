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

// abstractions.C
// Ariel Tamches

/* $Id: abstractions.C,v 1.15 2004/03/23 01:12:29 eli Exp $ */

#include "abstractions.h"

void abstractions::add(whereAxis *theNewAbstraction,
		       const pdstring &whereAxisName) {
   whereAxisStruct theStruct;
   theStruct.abstractionName = whereAxisName;
   theStruct.theWhereAxis = theNewAbstraction;
   theStruct.horizSBfirst = theStruct.horizSBlast = 0.0;
   theStruct.vertSBfirst  = theStruct.vertSBlast = 0.0;

   bool firstAbstraction = false;

   theAbstractions += theStruct;
   if (theAbstractions.size() == 1) {
      firstAbstraction = true;
   }

   // Now let us set the tcl variable "currMenuAbstraction", which
   // should generate a call to "change", below
   if (firstAbstraction) {
      currAbstractionIndex = 0;
      pdstring commandStr = pdstring("set currMenuAbstraction ") + pdstring(1);
      myTclEval(interp, commandStr);
   }   
}

whereAxis &abstractions::operator[](const pdstring &absName) {
   // given an abstraction name, this routine returns the where axis
   // structure.  If no abstraction/where-axis exists with the given
   // name, however, we ADD A NEW WHERE-AXIS and return that one.
   // This routine pretty much ignores the concept of a current where axis.

   for (unsigned i=0; i < theAbstractions.size(); i++)
      if (absName == theAbstractions[i].abstractionName) {
         assert(theAbstractions[i].theWhereAxis);
         return *(theAbstractions[i].theWhereAxis);
      }

   // cout << "abstractions[]: adding a new where axis..." << endl;
   whereAxis *theNewWhereAxis = new whereAxis
                 (interp, theTkWindow, "Whole Program",
                  horizSBName, vertSBName, navigateMenuName);
   assert(theNewWhereAxis);

   add(theNewWhereAxis, absName);
   return *theNewWhereAxis;
}

int abstractions::name2index(const pdstring &name) const {
   // returns -1 if not found
   for (unsigned i=0; i < theAbstractions.size(); i++)
      if (name == theAbstractions[i].abstractionName)
         return i;

   return -1;
}

bool abstractions::change(unsigned newindex) {
   // returns true iff any changes
   if (newindex == currAbstractionIndex)
      // nothing to do...
      return false;

   // Save current scrollbar values
   whereAxisStruct &was = theAbstractions[currAbstractionIndex];

   pdstring commandStr = horizSBName + " get";
   myTclEval(interp, commandStr);
   bool aflag;
   aflag=(2==sscanf(Tcl_GetStringResult(interp),
       "%f %f",&was.horizSBfirst,&was.horizSBlast));
   assert(aflag);

   commandStr = vertSBName + " get";
   myTclEval(interp, commandStr);
   aflag = (2==sscanf(Tcl_GetStringResult(interp),
       "%f %f", &was.vertSBfirst, &was.vertSBlast));
   assert(aflag);

   // Save current find string
   commandStr = findName + " get";
   myTclEval(interp, commandStr);
   was.findString = Tcl_GetStringResult(interp);

   // Set new scrollbar values
   whereAxisStruct &newWas = theAbstractions[currAbstractionIndex = newindex];
   commandStr = horizSBName + " set " + pdstring(newWas.horizSBfirst) + " "
                                      + pdstring(newWas.horizSBlast);
   myTclEval(interp, commandStr);

   commandStr = vertSBName + " set " + pdstring(newWas.vertSBfirst) + " "
                                     + pdstring(newWas.vertSBlast);
   myTclEval(interp, commandStr);

   // Set the new navigate menu:
   newWas.theWhereAxis->rethinkNavigateMenu();

   // Set the new find string
   commandStr = findName + " delete 0 end";
   myTclEval(interp, commandStr);

   commandStr = findName + " insert 0 " + "\"" + newWas.findString + "\"";
   myTclEval(interp, commandStr);

   // Finally, we must be safe and assume that the toplevel window
   // has been resized...in short, we need to simulate a resize right now.
   // (code in test.C does this for us...)

   return true;
}

bool abstractions::change(const pdstring &newName) {
   // unlike change(unsigned), we return true if successful (as opposed
   // to if any changes were made)

   for (unsigned i=0; i < theAbstractions.size(); i++) {
      whereAxisStruct &was = theAbstractions[i];
      if (was.abstractionName == newName) {
         (void)change(i);
         return true; // success         
      }
   }

   return false; // could not find any abstraction with that name
}

void abstractions::drawCurrent(bool doubleBuffer, bool xsynchOn) {
   // checks batch mode; if true, rethinks before the redraw
   if (!existsCurrent()) return;

   if (inBatchMode) {
      //cerr << "rethinking before redraw since batch mode is true!" << endl;
      resizeEverything(true); // resorts, rethinks layout...expensive!
   }

   getCurrent().draw(doubleBuffer, xsynchOn);
}

void abstractions::startBatchMode() {
   inBatchMode = true;
      // when true, draw() can draw garbage, so in such cases, we rethink
      // before drawing.  (expensive, of course)
}

void abstractions::endBatchMode() {
   inBatchMode = false;
   resizeEverything(true); // resorts, rethinks layout.  expensive.
}

bool abstractions::selectUnSelectFromFullPathName(const pdstring &name, bool select) {
   assert(existsCurrent());
   return getCurrent().selectUnSelectFromFullPathName(name, select);
}
