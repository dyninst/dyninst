/*
 * tunableConstant - a constant that might be changed during execution.
 *
 * $Log: tunableConst.h,v $
 * Revision 1.1  1994/02/25 00:25:58  hollings
 * added tunable constants.
 *
 *
 */
#ifndef TUNABLE_CONST_H
#define TUNABLE_CONST_H

#include "util/h/list.h"

typedef Boolean (*isValidFunc)(float newVale);
typedef void (*changeValCallBackFunc)(float value);

class tunableConstant {
    public:
	tunableConstant(float initialValue, 
			float min, 
			float max, 
			changeValCallBackFunc cb,
			char *name,
			char *desc);
	tunableConstant(float initialValue, 
			isValidFunc, 
			changeValCallBackFunc cb,
			char *name,
			char *desc);
	float getValue() { return value; }
	char *getDesc() { return(desc); }
	char *getName() { return(name); }
	Boolean setValue(float newVal) {
	    if ((isValidValue) && isValidValue(newVal)) {
		if (newValueCallBack) newValueCallBack(newVal);
		return(TRUE);
	    } else if (simpleRangeCheck(newVal)) {
		if (newValueCallBack) newValueCallBack(newVal);
		return(TRUE);
	    } else {
		return(FALSE);
	    }
	}
    private:
	char *desc;
	char *name;
	float value;
	float min, max;
	isValidFunc isValidValue;
	Boolean simpleRangeCheck(float val);
	changeValCallBackFunc newValueCallBack;
	static List<tunableConstant*> allConstants;
};

#endif
