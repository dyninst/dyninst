/*
 * tunableConst - a system parameter that we might one to change during 
 *    execution of the system.
 *
 * $Log: tunableConst.C,v $
 * Revision 1.2  1994/02/28 23:58:38  hollings
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

Boolean tunableConstant::simpleRangeCheck(float val)
{
    return((val >= min) && (val <= max));
}

tunableConstant::tunableConstant(float initialValue, 
				 float low, 
				 float high, 
				 changeValCallBackFunc cb,
				 char *n,
				 char *d)
{
    value = initialValue;
    min = low;
    max = high;
    desc = d;
    if (!pool) pool = new(stringPool);
    name = pool->findAndAdd(n);
    newValueCallBack = cb;
    if (!allConstants) allConstants = new(List<tunableConstant*>);
    allConstants->add(this, name);
}

tunableConstant::tunableConstant(float initialValue, 
				 isValidFunc func, 
				 changeValCallBackFunc cb,
				 char *n,
				 char *d)
{
    desc = d;
    if (!pool) pool = new(stringPool);
    name = pool->findAndAdd(n);
    isValidValue = func;
    value = initialValue;
    newValueCallBack = cb;
    if (!allConstants) allConstants = new(List<tunableConstant*>);
    allConstants->add(this, name);
}
