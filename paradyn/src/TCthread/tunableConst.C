/*
 * tunableConst - a system parameter that we might one to change during 
 *    execution of the system.
 *
 * $Log: tunableConst.C,v $
 * Revision 1.3  1995/08/05 17:09:36  krisna
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
 * Revision 1.9  1994/12/21  07:10:40  tamches
 * Made the "allConstants" variable protected and added a few member
 * functions to let outside code access it (safely) in a manner useful
 * for doing iterations through all tunable-constants.
 *
 * Revision 1.8  1994/12/21  00:34:07  tamches
 * Added "findTunableConstant" static member function to the base class.
 * Outside code had been doing similar things by peeking at class
 * variables, which is now disallowed in tunableConst.h.  No more compile
 * warnings.
 *
 * Revision 1.7  1994/11/04  15:54:02  tamches
 * Added "developerMode" tunable constant, which is a boolean tc in user mode.
 * The user interface (tcl) will now look at this tc to determine whether it
 * is in developer mode or not.
 *
 * Revision 1.6  1994/10/26  22:34:03  tamches
 * Set min/max to 0 for float constructor that did not define them.
 *
 * Revision 1.5  1994/09/22  03:18:47  markc
 * Added error checking code in constructor
 *
 * Revision 1.4  1994/08/20  23:17:30  markc
 * Added new machine type.
 * Cast stringHandle to (char*) to print in tunableConst.C
 * Added cast to char* for printing names which are stringHandles.
 *
 * Revision 1.3  1994/08/03  18:37:39  hollings
 * split tunable constant into Boolean and Float sub-classes.
 *
 * Revision 1.2  1994/02/28  23:58:38  hollings
 * Changed global list to be a pointer to a list because I couldn't rely on
 * the order of global constructors.
 *
 * Revision 1.1  1994/02/25  00:26:17  hollings
 * added tuntable constants
 *
 *
 */

#include <iostream.h>
#include <string.h>
#include "tunableConst.h"

/* ******************************************************************* */
/* ***************** IGEN calls not specific to tc type ************** */
/* ******************************************************************* */

bool tunableConstantRegistry::existsTunableConstant(const string theName) {
   return allBoolTunables.defines(theName) || allFloatTunables.defines(theName);
}

bool tunableConstantRegistry::existsBoolTunableConstant(const string theName) {
   return allBoolTunables.defines(theName);
}

bool tunableConstantRegistry::existsFloatTunableConstant(const string theName) {
   return allFloatTunables.defines(theName);
}

int tunableConstantRegistry::numTunables() {
   return allBoolTunables.size() + allFloatTunables.size();
}

tunableType tunableConstantRegistry::getTunableConstantType(const string theName) {
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

tunableConstantBase tunableConstantRegistry::getGenericTunableConstantByName(const string theName) {
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

/* ******************************************************************* */
/* ***************** IGEN calls specific to boolean tc's ************* */
/* ******************************************************************* */

int tunableConstantRegistry::numBoolTunables() {
   return allBoolTunables.size();
}

bool tunableConstantRegistry::createBoolTunableConstant(const string theName,
							const string theDesc,
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

bool tunableConstantRegistry::destroyBoolTunableConstant(const string theName) {
   if (!allBoolTunables.defines(theName)) {
      cout << "tunable constant registry note: cannot destroy non-existing bool tunable " << theName << "; ignoring" << endl;
      return false;
   }

   allBoolTunables.undef(theName);
   return true;
}

tunableBooleanConstant tunableConstantRegistry::findBoolTunableConstant(const string theName) {
   if (!allBoolTunables.defines(theName)) {
      cerr << "tunable boolean constant with name=" << theName << " not found." << endl;
      assert(false);
   }

   return allBoolTunables[theName]; // makes a copy in the process of returning...
}

vector<tunableBooleanConstant> tunableConstantRegistry::getAllBoolTunableConstants() {
   return allBoolTunables.values();
}

void tunableConstantRegistry::setBoolTunableConstant(const string theName,
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

/* ******************************************************************* */
/* ***************** IGEN calls specific to float tc's *************** */
/* ******************************************************************* */

int tunableConstantRegistry::numFloatTunables() {
   return allFloatTunables.size();
}

bool tunableConstantRegistry::createFloatTunableConstant(const string theName,
							 const string theDesc,
							 floatChangeValCallBackFunc cb,
							 tunableUse theUse,
							 const float initialVal,
							 float min, float max) {
   // returns true iff successfully created
   if (allFloatTunables.defines(theName)) {
      cerr << "tunable constant: attempt to create float tc with name=\"" << theName << "\" ignored (already exists)" << endl;
      return false;
   }

   tunableFloatConstant theConst(theName, theDesc,
				 initialVal,
				 min, max,
				 cb,
				 theUse);
      // protected constructor; available only to us.

   allFloatTunables[theName] = theConst;
   assert(allFloatTunables[theName].getName()==theName); // verify name

//   cout << "tunable const registry: now there are " << allFloatTunables.size() << " float tcs." << endl;

//   vector<string> floatTCNames = allFloatTunables.keys();
//   for (int i=0; i<floatTCNames.size(); i++)
//      cout << "   " << floatTCNames[i] << endl;

   return true; // success
}

bool tunableConstantRegistry::createFloatTunableConstant(const string theName,
							 const string theDesc,
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

//   cout << "tunable const registry: just created float tc " << theName << "; verifying name." << endl;
   assert(allFloatTunables[theName].getName()==theName);

//   cout << "tunable const registry: name verified; now there are " << allFloatTunables.size() << " float tcs; they are:" << endl;

   return true; // success
}

bool tunableConstantRegistry::destroyFloatTunableConstant(const string theName) {
   if (!allFloatTunables.defines(theName)) {
      cout << "tunable constant registry note: cannot destroy non-existing float tunable " << theName << "; ignoring" << endl;
      return false;
   }

   allFloatTunables.undef(theName);
   return true;
}

tunableFloatConstant tunableConstantRegistry::findFloatTunableConstant(const string theName) {
   if (!allFloatTunables.defines(theName)) {
      cerr << "tunable constant registry -- cannot find float tunable constant with name of " << theName << endl;
      assert(false);
   }

   return allFloatTunables[theName]; // makes a copy in the process of returning...
}

vector<tunableFloatConstant> tunableConstantRegistry::getAllFloatTunableConstants() {
   return allFloatTunables.values();
}

void tunableConstantRegistry::setFloatTunableConstant(const string theName,
						      const float newValue) {
   if (!allFloatTunables.defines(theName)) {
      cerr << "tunable constant registry -- cannot set float tunable constant with name of " << theName << endl;
      assert(false);
   }

   tunableFloatConstant &tfc = allFloatTunables[theName];

   if (tfc.isValidValue && tfc.isValidValue(newValue)) {
      tfc.value = newValue; // used to be allFloatTunables[theName].setValue(newValue);
      if (tfc.newValueCallBack)
         tfc.newValueCallBack(newValue);
   }
   else if (tfc.simpleRangeCheck(newValue)) {
      tfc.value = newValue;
      if (tfc.newValueCallBack) 
         tfc.newValueCallBack(newValue);
   }
}

/* ******************************************************************* */
/* **************** Internal routines -- generic TCs ***************** */
/* ******************************************************************* */

tunableConstantBase::tunableConstantBase(const string theName,
					 const string theDesc,
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
					       const string theName,
					       const string theDesc) :
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
      value(src.value), min(src.min), max(src.max), isValidValue(src.isValidValue),
      newValueCallBack(src.newValueCallBack) {
//   cout << "welcome to tunableFloatConstant [sort of]" << endl;
//   cout.flush();
}

bool tunableFloatConstant::simpleRangeCheck(float val) {
    return (val >= min && val <= max);
}

tunableFloatConstant::tunableFloatConstant(const string theName,
					   const string theDesc,
					   float initialValue, 
					   float low, float high,
					   floatChangeValCallBackFunc cb,
					   tunableUse u) :
                   tunableConstantBase(theName, theDesc, tunableFloat, u)
{
   // a private constructor; outside code must not call this
   this->value = initialValue;
   this->min = low;
   this->max = high;
   this->isValidValue = NULL;
   this->newValueCallBack = cb;
}

tunableFloatConstant::tunableFloatConstant(const string theName,
					   const string theDesc,
					   float initialValue, 
					   isValidFunc func, 
					   floatChangeValCallBackFunc cb,
					   tunableUse u) :
                   tunableConstantBase(theName, theDesc, tunableFloat, u)
{
   // a private constructor; outside code must not call this
   this->value = initialValue;
   this->min = this->max = 0;
   this->isValidValue = func;
   this->newValueCallBack = cb;
}

/* ************************************************************** */
/* ************************ Declarators ************************* */
/* ************************************************************** */

tunableBooleanConstantDeclarator::tunableBooleanConstantDeclarator
            (const string theName,
	     const string theDesc,
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
//   cout << "Welcome to ~tunableBooleanConstantDeclarator for " << this->the_name << endl;

   // igen call:
   (void)tunableConstantRegistry::destroyBoolTunableConstant(this->the_name);
}

tunableFloatConstantDeclarator::tunableFloatConstantDeclarator
                 (const string theName,
		  const string theDesc,
		  float initialValue,
		  float min, float max,
		  floatChangeValCallBackFunc cb,
		  tunableUse type) : the_name(theName) {
   // igen call:
   const bool result = tunableConstantRegistry::createFloatTunableConstant(theName,
									   theDesc,
									   cb,
									   type,
									   initialValue,
									   min, max);
   if (!result) {
      cerr << "tunable constant warning: could not create float tc \"" << theName << "\"" << endl;
      return;
   }
}

tunableFloatConstantDeclarator::tunableFloatConstantDeclarator
		 (const string theName,
		  const string theDesc,
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
//   cout << "Welcome to ~tunableFloatConstantDeclarator for " << this->the_name << endl;
   // igen call:
   (void)tunableConstantRegistry::destroyFloatTunableConstant(this->the_name);
}
