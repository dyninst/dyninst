// shgPhases.C
// Ariel Tamches
// Analagous to "abstractions.h" for the where axis; this class
// basically manages several "shg"'s, as defined in shgPhases.h

/* $Log: shgPhases.C,v $
/* Revision 1.3  1996/01/09 01:06:43  tamches
/* changes to reflect moving phase id to shg class
/*
 * Revision 1.2  1995/11/29 00:20:05  tamches
 * removed some warnings
 *
 * Revision 1.1  1995/10/17 22:08:36  tamches
 * initial version, for the new search history graph
 *
 */

#include <limits.h>
#include "tkTools.h" // myTclEval()
#include "shgPhases.h"

shgPhases::shgPhases(const string &iMenuName,
		     const string &iHorizSBName, const string &iVertSBName,
		     const string &iCurrItemLabelName,
		     Tcl_Interp *iInterp, Tk_Window iTkWindow) :
		        menuName(iMenuName), horizSBName(iHorizSBName),
			vertSBName(iVertSBName),
			currItemLabelName(iCurrItemLabelName)
{
   currShgPhaseIndex = UINT_MAX;

   interp = iInterp;
   theTkWindow = iTkWindow;
}

shgPhases::~shgPhases() {
   for (unsigned i=0; i < theShgPhases.size(); i++)
      delete theShgPhases[i].theShg;

   currShgPhaseIndex = UINT_MAX;
}

void shgPhases::add(shg *theNewShg, const string &theNewShgPhaseName) {
   shgStruct theStruct;
   theStruct.theShg = theNewShg;
   //theStruct.phaseID = phaseID;
   theStruct.phaseName = theNewShgPhaseName;
   theStruct.horizSBfirst = theStruct.horizSBlast = 0.0;
   theStruct.vertSBfirst = theStruct.vertSBlast = 0.0;

   theShgPhases += theStruct;
   const bool firstShg = (theShgPhases.size() == 1);
   
//   string commandStr = menuName + " add radiobutton -label " +
//                       string::quote + theNewShgPhaseName + string::quote +
//                       " -command " + string::quote + "shgChangePhase " +
//                       string(theShgPhases.size()) + string::quote +
//                       " -variable currShgPhase -value " +
//                       string(theShgPhases.size());
   string commandStr = menuName + " add radiobutton -label " +
                       string::quote + theNewShgPhaseName + string::quote +
                       " -command " + string::quote + "shgChangePhase " +
                       string(theNewShg->getPhaseId()) + string::quote +
                       " -variable currShgPhase -value " +
                       string(theNewShg->getPhaseId());
   myTclEval(interp, commandStr);

   if (firstShg) {
      currShgPhaseIndex = 0;
      string commandStr = string("set currShgPhase ") + string(theNewShg->getPhaseId());
      myTclEval(interp, commandStr);

      commandStr = string(".shg.nontop.currphasearea.label2 config -text ") +
	           "\"" + theNewShgPhaseName + "\"";
      myTclEval(interp, commandStr);
   }
}

shg &shgPhases::getByID(int phaseID) {
   for (unsigned i=0; i < theShgPhases.size(); i++) {
      if (theShgPhases[i].getPhaseId() == phaseID)
         return *(theShgPhases[i].theShg);
   }
   assert(false);
   abort();
}

int shgPhases::name2id(const string &phaseName) const {
   // returns -1 if not found
   for (unsigned i=0; i < theShgPhases.size(); i++)
      if (phaseName == theShgPhases[i].phaseName)
         return theShgPhases[i].getPhaseId();

   return -1;
}

const string &shgPhases::id2name(int id) const {
   for (unsigned lcv=0; lcv < theShgPhases.size(); lcv++)
      if (theShgPhases[lcv].getPhaseId() == id)
         return theShgPhases[lcv].phaseName;
   assert(false);
   abort();
}

bool shgPhases::change(unsigned newIndex) {
   // returns true iff any changes
   if (newIndex == currShgPhaseIndex)
      return false;

   // Save curr sb values
   shgStruct &theShgStruct=theShgPhases[currShgPhaseIndex];

   string commandStr = horizSBName + " get";
   myTclEval(interp, commandStr);
   assert(2==sscanf(interp->result, "%f %f", &theShgStruct.horizSBfirst,
		                             &theShgStruct.horizSBlast));

   commandStr = vertSBName + " get";
   myTclEval(interp, commandStr);
   assert(2==sscanf(interp->result, "%f %f", &theShgStruct.vertSBfirst,
		                             &theShgStruct.vertSBlast));

   // Set new sb values
   shgStruct &theNewShgStruct=theShgPhases[currShgPhaseIndex = newIndex];

   commandStr = horizSBName + " set " + string(theNewShgStruct.horizSBfirst) + " "
                                      + string(theNewShgStruct.horizSBlast);
   myTclEval(interp, commandStr);

   commandStr = vertSBName + " set " + string(theNewShgStruct.vertSBfirst) + " "
                                     + string(theNewShgStruct.vertSBlast);
   myTclEval(interp, commandStr);

   return true;
}

bool shgPhases::change(const string &newPhaseName) {
   // returns true iff successful
   for (unsigned i=0; i < theShgPhases.size(); i++) {
      shgStruct &theShgStruct = theShgPhases[i];
      if (theShgStruct.phaseName == newPhaseName) {
         (void)change(i);
         return true;
      }
   }

   return false; // could not find any phase with that name
}

shg &shgPhases::getCurrent() {
   assert(existsCurrent());
   assert(theShgPhases[currShgPhaseIndex].theShg);
   return *(theShgPhases[currShgPhaseIndex].theShg);
}

const shg &shgPhases::getCurrent() const {
   assert(existsCurrent());
   assert(theShgPhases[currShgPhaseIndex].theShg);
   return *(theShgPhases[currShgPhaseIndex].theShg);
}

void shgPhases::resizeEverything() {
   if (!existsCurrent())
      return;

   for (unsigned i=0; i < theShgPhases.size(); i++) {
      theShgPhases[i].theShg->recursiveRethinkLayout();
      theShgPhases[i].theShg->resize(i==currShgPhaseIndex);
   }
}   
