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

/*
 * tunableConst - a system parameter that we might one to change during 
 *    execution of the system.
 *
 * $Log: tunableConst.C,v $
 * Revision 1.8  2002/12/20 07:50:04  jaw
 * This commit fully changes the class name of "vector" to "pdvector".
 *
 * A nice upshot is the removal of a bunch of code previously under the flag
 * USE_STL_VECTOR, which is no longer necessary in many cases where a
 * functional difference between common/h/Vector.h and stl::vector was
 * causing a crash.
 *
 * Generally speaking, Dyninst and Paradyn now use pdvector exclusively.
 * This commit DOES NOT cover the USE_STL_VECTOR flag, which will now
 * substitute stl::vector for BPatch_Vector only.  This is currently, to
 * the best of my knowledge, only used by DPCL.  This will be updated and
 * tested in a future commit.
 *
 * The purpose of this, again, is to create a further semantic difference
 * between two functionally different classes (which both have the same
 * [nearly] interface).
 *
 * Revision 1.7  1999/03/03 18:15:26  pcroth
 * Updated to support Windows NT as a front-end platform
 * Changes made to X code, to use Tcl analogues when appropriate
 * Also changed in response to modifications in thread library and igen output.
 *
 * Revision 1.6  1996/08/16 21:04:36  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1995/12/20 02:27:01  tamches
 * general cleanup
 *
 * Revision 1.4  1995/10/12 18:35:59  tamches
 * Changed a lot of prototypes from "string" to "const string &", thus avoiding
 * an unnecessary string copy.
 *
 * Revision 1.3  1995/08/05  17:09:36  krisna
 * no `virtual' outside a class declaration
 *
 * Revision 1.2  1995/06/24 20:49:47  tamches
 * Removed setValue() and print() for individual tc's.
 *
 * Revision 1.1  1995/02/27  18:50:03  tamches
 * First version of TCthread; files tunableConst.h and .C have
 * simply moved from the util lib (and have been changed); file
 * TCmain.C is completely new.
 *
 */

#include <iostream.h>
#include <string.h>
#include "tunableConst.h"


/* *******************************************************************
 * ***************** Tunable Constant Registry Class *****************
 * *******************************************************************
 *
 * Contains a lot of static member functions meant for use by outside code.
 * 
 * *******************************************************************
 */

bool tunableConstantRegistry::existsTunableConstant(const string &theName) {
   return allBoolTunables.defines(theName) || allFloatTunables.defines(theName);
}

unsigned tunableConstantRegistry::numTunables() {
   return allBoolTunables.size() + allFloatTunables.size();
}

tunableType tunableConstantRegistry::getTunableConstantType(const string &theName) {
   // the following strange order avoids compiler warnings...
   if (allBoolTunables.defines(theName))
      return tunableBoolean;
   else if (! allFloatTunables.defines(theName)) {
      cout << "tunable constant registry [getTunableConstantType]: tunable " << theName << " does not exist" << endl;
      assert(false); // not in either list!
   }

   assert(allFloatTunables.defines(theName));
   return tunableFloat;
}

tunableConstantBase tunableConstantRegistry::getGenericTunableConstantByName(const string &theName) {
   // the following strange order avoids compiler warnings...
   if (allBoolTunables.defines(theName))
      return allBoolTunables[theName]; // makes a copy and then returns it
   else if (! allFloatTunables.defines(theName)) {
      cerr << "tunable constant registry error -- could not find generic tc with name of " << theName << endl;
      assert(false); // not in either list!
   }

   assert(allFloatTunables.defines(theName)); // should throw an exception instead
   return allFloatTunables[theName]; // makes a copy and then returns it
}

/* Tunable Constant Registry Routines specific to boolean tunables: */

bool tunableConstantRegistry::existsBoolTunableConstant(const string &theName) {
   return allBoolTunables.defines(theName);
}

unsigned tunableConstantRegistry::numBoolTunables() {
   return allBoolTunables.size();
}

bool tunableConstantRegistry::createBoolTunableConstant(const string &theName,
							const string &theDesc,
							booleanChangeValCallBackFunc cb,
							tunableUse theUse,
							const bool initialVal) {
   // returns true iff successfully created

   if (allBoolTunables.defines(theName)) {
      cerr << "tunable constant: attempt to create boolean tc with name=\"" << theName << "\" ignored (already exists)" << endl;
      return false;
   }

   tunableBooleanConstant theConst(initialVal,
				   cb,
				   theUse,
				   theName, theDesc);
      // protected constructor; available only to us.

   allBoolTunables[theName] = theConst;
   assert(theName == allBoolTunables[theName].getName()); // verify name

//   cout << "tunable const registry: now there are " << allBoolTunables.size() << " bool tcs" << endl;

   return true; // success
}

bool tunableConstantRegistry::destroyBoolTunableConstant(const string &theName) {
   if (!allBoolTunables.defines(theName)) {
      cout << "tunable constant registry note: cannot destroy non-existing bool tunable " << theName << "; ignoring" << endl;
      return false;
   }

   allBoolTunables.undef(theName);
   return true;
}

tunableBooleanConstant tunableConstantRegistry::findBoolTunableConstant(const string &theName) {
   if (!allBoolTunables.defines(theName)) {
      cerr << "tunable boolean constant with name=" << theName << " not found." << endl;
      assert(false);
   }

   return allBoolTunables[theName]; // makes a copy in the process of returning...
}

pdvector<tunableBooleanConstant> tunableConstantRegistry::getAllBoolTunableConstants() {
   return allBoolTunables.values();
}

void tunableConstantRegistry::setBoolTunableConstant(const string &theName,
						     const bool newValue) {
   if (!allBoolTunables.defines(theName)) {
      cerr << "tunable constant registry [setBoolTunableConstant] -- boolean tc with name of " << theName << " does not exist." << endl;
      assert(false);
   }

   tunableBooleanConstant &tbc = allBoolTunables[theName];
   tbc.value = newValue; // used to be tbc.setValue(newValue)

   if (tbc.newValueCallBack)
      tbc.newValueCallBack(newValue);
}

/* Tunable Constant Registry Routines specific to float tunables: */

unsigned tunableConstantRegistry::numFloatTunables() {
   return allFloatTunables.size();
}

bool tunableConstantRegistry::existsFloatTunableConstant(const string &theName) {
   return allFloatTunables.defines(theName);
}

bool tunableConstantRegistry::createFloatTunableConstant(const string &theName,
							 const string &theDesc,
							 floatChangeValCallBackFunc cb,
							 tunableUse theUse,
							 const float initialVal,
							 float minval, float maxval) {
   // returns true iff successfully created
   if (allFloatTunables.defines(theName)) {
      cerr << "tunable constant: attempt to create float tc with name=\"" << theName << "\" ignored (already exists)" << endl;
      return false;
   }

   tunableFloatConstant theConst(theName, theDesc,
				 initialVal,
				 minval, maxval,
				 cb,
				 theUse);
      // protected constructor; available only to us.

   allFloatTunables[theName] = theConst;
   assert(allFloatTunables[theName].getName()==theName); // verify name

   return true; // success
}

bool tunableConstantRegistry::createFloatTunableConstant(const string &theName,
							 const string &theDesc,
							 floatChangeValCallBackFunc cb,
							 tunableUse theUse,
							 const float initialVal,
							 isValidFunc ivf) {
   // returns true iff successfully created
   if (allFloatTunables.defines(theName)) {
      cerr << "tunable constant: attempt to create float tc with name=\"" << theName << "\" ignored (already exists)" << endl;
      return false;
   }

   tunableFloatConstant theConst(theName, theDesc,
				 initialVal,
				 ivf,
				 cb,
				 theUse);
      // protected constructor; available only to us.

   allFloatTunables[theName] = theConst;

   assert(allFloatTunables[theName].getName()==theName);

   return true; // success
}

bool tunableConstantRegistry::destroyFloatTunableConstant(const string &theName) {
   if (!allFloatTunables.defines(theName)) {
      cout << "tunable constant registry note: cannot destroy non-existing float tunable " << theName << "; ignoring" << endl;
      return false;
   }

   allFloatTunables.undef(theName);
   return true;
}

tunableFloatConstant tunableConstantRegistry::findFloatTunableConstant(const string &theName) {
   if (!allFloatTunables.defines(theName)) {
      cerr << "tunable constant registry -- cannot find float tunable constant with name of " << theName << endl;
      assert(false);
   }

   return allFloatTunables[theName]; // makes a copy in the process of returning...
}

pdvector<tunableFloatConstant> tunableConstantRegistry::getAllFloatTunableConstants() {
   return allFloatTunables.values();
}

void tunableConstantRegistry::setFloatTunableConstant(const string &theName,
						      const float newValue) {
   if (!allFloatTunables.defines(theName)) {
      cerr << "tunable constant registry -- cannot set float tunable constant with name of " << theName << endl;
      assert(false);
   }

   tunableFloatConstant &tfc = allFloatTunables[theName];

   bool okay = false; // set to true if change should be made & callback should be invoked
   if (tfc.isValidValue) {
      // an isValidValue checking routine exists; use it now
      if (tfc.isValidValue(newValue))
         // check succeeded; the change to the registry may now go forward
         // and the callback may be invoked.
         okay = true;
   }
   else {
      // perform simpleRangeCheck
      if (tfc.simpleRangeCheck(newValue))
         // check succeeded; change to registry may now go forward
         // and callback may be invoked.
         okay = true;
   }

   if (okay) {
      tfc.value = newValue;
      if (tfc.newValueCallBack)
         tfc.newValueCallBack(newValue);
   }
}

/* ******************************************************************* */
/* **************** Internal routines -- generic TCs ***************** */
/* ******************************************************************* */

tunableConstantBase::tunableConstantBase(const string &theName,
					 const string &theDesc,
					 const tunableType theType,
					 const tunableUse  theUse) :
                       name(theName), desc(theDesc) {
   // a private constructor; outside code must not call this

   this->typeName = theType;
   this->use = theUse;
}

tunableConstantBase::~tunableConstantBase() {
   // name and desc are deallocated automatically
}

/* ******************************************************************* */
/* ************* Internal routines specific to boolean TCs *********** */
/* ******************************************************************* */

tunableBooleanConstant::tunableBooleanConstant(bool initialValue,
					       booleanChangeValCallBackFunc cb,
					       tunableUse u,
					       const string &theName,
					       const string &theDesc) :
                            tunableConstantBase(theName, theDesc, tunableBoolean, u) {
   // a private constructor; outside code cannot call this.

   this->value = initialValue;
   this->newValueCallBack = cb;
}

/* ******************************************************************* */
/* ************* Internal routines specific to float TCs ************* */
/* ******************************************************************* */

tunableFloatConstant::tunableFloatConstant(const tunableFloatConstant &src) :
      tunableConstantBase(src),
      value(src.value), minval(src.minval), maxval(src.maxval), isValidValue(src.isValidValue),
      newValueCallBack(src.newValueCallBack) {
}

bool tunableFloatConstant::simpleRangeCheck(float val) {
    return (val >= minval && val <= maxval);
}

tunableFloatConstant::tunableFloatConstant(const string &theName,
					   const string &theDesc,
					   float initialValue, 
					   float low, float high,
					   floatChangeValCallBackFunc cb,
					   tunableUse u) :
                   tunableConstantBase(theName, theDesc, tunableFloat, u)
{
   // a private constructor; outside code must not call this
   this->value = initialValue;
   this->minval = low;
   this->maxval = high;
   this->isValidValue = NULL;
   this->newValueCallBack = cb;
}

tunableFloatConstant::tunableFloatConstant(const string &theName,
					   const string &theDesc,
					   float initialValue, 
					   isValidFunc func, 
					   floatChangeValCallBackFunc cb,
					   tunableUse u) :
                   tunableConstantBase(theName, theDesc, tunableFloat, u)
{
   // a private constructor; outside code must not call this
   this->value = initialValue;
   this->minval = this->maxval = 0;
   this->isValidValue = func;
   this->newValueCallBack = cb;
}

/* **************************************************************************** */
/* **************** Declarators -- meant for use by outside code ************** */
/* **************************************************************************** */

tunableBooleanConstantDeclarator::tunableBooleanConstantDeclarator
            (const string &theName,
	     const string &theDesc,
	     bool initialValue, 
	     booleanChangeValCallBackFunc cb,
	     tunableUse type) : the_name(theName) {
   const bool result = tunableConstantRegistry::createBoolTunableConstant
                          (theName, theDesc,
			   cb,
			   type,
			   initialValue);
   if (!result) {
      cerr << "tunable constant warning: could not create bool tc \"" << theName << "\"" << endl;
      return;
   }
}

tunableBooleanConstantDeclarator::~tunableBooleanConstantDeclarator() {
   (void)tunableConstantRegistry::destroyBoolTunableConstant(this->the_name);
}

tunableFloatConstantDeclarator::tunableFloatConstantDeclarator
                 (const string &theName,
		  const string &theDesc,
		  float initialValue,
		  float minval, float maxval,
		  floatChangeValCallBackFunc cb,
		  tunableUse type) : the_name(theName) {
   const bool result = tunableConstantRegistry::createFloatTunableConstant(theName,
									   theDesc,
									   cb,
									   type,
									   initialValue,
									   minval, maxval);
   if (!result) {
      cerr << "tunable constant warning: could not create float tc \"" << theName << "\"" << endl;
      return;
   }
}

tunableFloatConstantDeclarator::tunableFloatConstantDeclarator
		 (const string &theName,
		  const string &theDesc,
                  float initialValue, 
		  isValidFunc ivf,
		  floatChangeValCallBackFunc cb,
		  tunableUse type) : the_name(theName) {
   const bool result = tunableConstantRegistry::createFloatTunableConstant
             (theName, theDesc, cb, type, initialValue, ivf);
   if (!result) {
      cerr << "tunable constant warning: could not create float tc \"" << theName << "\"" << endl;
      return;
   }
}

tunableFloatConstantDeclarator::~tunableFloatConstantDeclarator() {
   (void)tunableConstantRegistry::destroyFloatTunableConstant(this->the_name);
}
