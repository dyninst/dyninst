/*
 * tunableConst - a system parameter that we might one to change during 
 *    execution of the system.
 *
 * $Log: tunableConst.C,v $
 * Revision 1.4  1994/08/20 23:17:30  markc
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

List<tunableConstant*> *tunableConstant::allConstants;
stringPool *tunableConstant::pool;

tunableBooleanConstant::tunableBooleanConstant(Boolean initialValue,
					       booleanChangeValCallBackFunc cb,
					       tunableUse u,
					       char *n,
					       char *d)
{
    value = initialValue;
    desc = d;
    use = u;
    typeName = tunableBoolean;

    if (!pool) pool = new(stringPool);
    name = pool->findAndAdd(n);
    newValueCallBack = cb;
    if (!allConstants) allConstants = new(List<tunableConstant*>);
    allConstants->add(this, name);
}

void tunableBooleanConstant::print()
{
   cout << (char*) name << " = ";
   if (value == TRUE) {
       cout << "True\n";
   } else {
       cout << "False\n";
   }
}

Boolean tunableFloatConstant::simpleRangeCheck(float val)
{
    return((val >= min) && (val <= max));
}

tunableFloatConstant::tunableFloatConstant(float initialValue, 
					 float low, 
					 float high, 
					 floatChangeValCallBackFunc cb,
					 tunableUse u,
					 char *n,
					 char *d)

{
    value = initialValue;
    min = low;
    max = high;
    desc = d;
    use = u;
    typeName = tunableFloat;
    newValueCallBack = cb;

    if (!pool) pool = new(stringPool);
    name = pool->findAndAdd(n);
    if (!allConstants) allConstants = new(List<tunableConstant*>);
    allConstants->add(this, name);
}

tunableFloatConstant::tunableFloatConstant(float initialValue, 
					   isValidFunc func, 
					   floatChangeValCallBackFunc cb,
					   tunableUse u,
					   char *n,
					   char *d)
{
    desc = d;
    use = u;
    typeName = tunableFloat;
    isValidValue = func;
    value = initialValue;
    newValueCallBack = cb;

    if (!pool) pool = new(stringPool);
    name = pool->findAndAdd(n);
    if (!allConstants) allConstants = new(List<tunableConstant*>);
    allConstants->add(this, name);
}

void tunableFloatConstant::print()
{
    cout << (char*) name << " = " << value << "\n";
}
