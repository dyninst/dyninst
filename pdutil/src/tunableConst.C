/*
 * tunableConst - a system parameter that we might one to change during 
 *    execution of the system.
 *
 * $Log: tunableConst.C,v $
 * Revision 1.8  1994/12/21 00:34:07  tamches
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

#include "util/h/stringPool.h"
#include "util/h/tunableConst.h"
#include <string.h>

List<tunableConstant*> *tunableConstant::allConstants=NULL;
stringPool *tunableConstant::pool=NULL;

tunableConstant *tunableConstant::findTunableConstant(const char *name) {
   // returns NULL if not found.  a static member function.
   assert(allConstants && pool);

   char *key = (char *)(pool->find(name));
   return allConstants->find(key);
}

tunableBooleanConstant tcInDeveloperMode(false,
         NULL, // presently, no callback function			 
	 userConstant,
	 "developerMode",
	 "Allow access to all tunable constants, including those limited to developer mode.  (Use with caution)");

tunableBooleanConstant::tunableBooleanConstant(bool initialValue,
					       booleanChangeValCallBackFunc cb,
					       tunableUse u,
					       const char *n,
					       const char *d)
{
    value = initialValue;
    if (d)
      desc = strdup(d);
    else
      desc = 0;
    use = u;
    typeName = tunableBoolean;

    if (!pool)
       pool = new stringPool;
    name = (char *)pool->findAndAdd(n);
    newValueCallBack = cb;
    if (!allConstants) allConstants = new(List<tunableConstant*>);
    allConstants->add(this, name);
}

void tunableBooleanConstant::print()
{
   cout << (char*) name << " = ";
   if (value) 
       cout << "True\n";
   else
       cout << "False\n";
}

bool tunableFloatConstant::simpleRangeCheck(float val)
{
    return((val >= min) && (val <= max));
}

tunableFloatConstant::tunableFloatConstant(float initialValue, 
					 float low, 
					 float high, 
					 floatChangeValCallBackFunc cb,
					 tunableUse u,
					 const char *n,
					 const char *d)

{
    value = initialValue;
    min = low;
    max = high;
    if (d)
      desc = strdup(d);
    else
      desc = 0;
    use = u;
    typeName = tunableFloat;
    newValueCallBack = cb;

    if (!pool) pool = new(stringPool);
    name = (char *)pool->findAndAdd(n);
    if (!allConstants) allConstants = new(List<tunableConstant*>);
    allConstants->add(this, name);
}

tunableFloatConstant::tunableFloatConstant(float initialValue, 
					   isValidFunc func, 
					   floatChangeValCallBackFunc cb,
					   tunableUse u,
					   const char *n,
					   const char *d)
{
    if (d)
      desc = strdup(d);
    else
      desc = 0;
    use = u;
    typeName = tunableFloat;
    isValidValue = func;
    value = initialValue;
    newValueCallBack = cb;

    // this kludge added 10/24/94 AT
    min=max=0.0; // so we can detect those tunable float constants with no min/max set

    if (!pool) pool = new(stringPool);
    name = (char *)pool->findAndAdd(n);
    if (!allConstants) allConstants = new(List<tunableConstant*>);
    allConstants->add(this, name);
}

void tunableFloatConstant::print() {
    cout << (char*) name << " = " << value << "\n";
}
