/*
 * tunableConst - a system parameter that we might one to change during 
 *    execution of the system.
 *
 * $Log: tunableConst.C,v $
 * Revision 1.1  1994/02/25 00:26:17  hollings
 * added tuntable constants
 *
 *
 */

#include "util/h/tunableConst.h"

List<tunableConstant*> tunableConstant::allConstants;

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
    name = n;
    newValueCallBack = cb;
    allConstants.add(this);
}

tunableConstant::tunableConstant(float initialValue, 
				 isValidFunc func, 
				 changeValCallBackFunc cb,
				 char *n,
				 char *d)
{
    desc = d;
    name = n;
    isValidValue = func;
    value = initialValue;
    newValueCallBack = cb;
    allConstants.add(this);
}
