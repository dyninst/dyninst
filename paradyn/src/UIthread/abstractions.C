// abstractions.C
// Ariel Tamches

/* $Log: abstractions.C,v $
/* Revision 1.6  1995/10/17 20:50:19  tamches
/* class abstractions is no longer templated.
/* Added change().
/*
 * Revision 1.5  1995/09/20 01:15:48  tamches
 * minor change; some usages of int --> unsigned
 *
 * Revision 1.4  1995/08/07  00:00:34  tamches
 * Added name2index
 *
 * Revision 1.3  1995/07/24  21:32:48  tamches
 * Added getTkWindow(), get*SBName(), and change(string) member
 * functions.
 *
 * Revision 1.2  1995/07/18  03:41:17  tamches
 * Added ctrl-double-click feature for selecting/unselecting an entire
 * subtree (nonrecursive).  Added a "clear all selections" option.
 * Selecting the root node now selects the entire program.
 *
 * Revision 1.1  1995/07/17  04:58:54  tamches
 * First version of the new where axis
 *
 */

#include "abstractions.h"

void abstractions::add(whereAxis *theNewAbstraction,
		       const string &whereAxisName) {
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

   string commandStr = absMenuName + " add radiobutton -label " +
                       string::quote + whereAxisName + string::quote +
                       " -command " + string::quote + "whereAxisChangeAbstraction " +
		       string(theAbstractions.size()) + string::quote +
                       " -variable currMenuAbstraction -value " +
                       string(theAbstractions.size());
   myTclEval(interp, commandStr);
 
   // Now let us set the tcl variable "currMenuAbstraction", which
   // should generate a call to "change", below
   if (firstAbstraction) {
      currAbstractionIndex = 0;
      string commandStr = string("set currMenuAbstraction ") + string(1);
      myTclEval(interp, commandStr);
   }   
}

whereAxis &abstractions::operator[](const string &absName) {
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

int abstractions::name2index(const string &name) const {
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

   string commandStr = horizSBName + " get";
   myTclEval(interp, commandStr);
   assert(2==sscanf(interp->result, "%f %f", &was.horizSBfirst, &was.horizSBlast));

   commandStr = vertSBName + " get";
   myTclEval(interp, commandStr);
   assert(2==sscanf(interp->result, "%f %f", &was.vertSBfirst, &was.vertSBlast));

   // Save current find string
   commandStr = findName + " get";
   myTclEval(interp, commandStr);
   was.findString = interp->result;

   // Set new scrollbar values
   whereAxisStruct &newWas = theAbstractions[currAbstractionIndex = newindex];
   commandStr = horizSBName + " set " + string(newWas.horizSBfirst) + " "
                                      + string(newWas.horizSBlast);
   myTclEval(interp, commandStr);

   commandStr = vertSBName + " set " + string(newWas.vertSBfirst) + " "
                                     + string(newWas.vertSBlast);
   myTclEval(interp, commandStr);

   // Set the new navigate menu:
   newWas.theWhereAxis->rethinkNavigateMenu();

   // Set the new find string
   commandStr = findName + " delete 0 end";
   myTclEval(interp, commandStr);

   commandStr = findName + " insert 0 " + string::quote + newWas.findString + string::quote;
   myTclEval(interp, commandStr);

   // Finally, we must be safe and assume that the toplevel window
   // has been resized...in short, we need to simulate a resize right now.
   // (code in test.C does this for us...)

   return true;
}

bool abstractions::change(const string &newName) {
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
