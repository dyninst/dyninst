/*
 * tunableConstant - a constant that might be changed during execution.
 *
 * $Log: tunableConst.h,v $
 * Revision 1.3  1994/08/03 18:37:30  hollings
 * split tunable constant into Boolean and Float sub-classes.
 *
 * Revision 1.2  1994/02/28  23:58:28  hollings
 * Changed global list to be a pointer to a list because I couldn't rely on
 * the order of global constructors.
 *
 * Revision 1.1  1994/02/25  00:25:58  hollings
 * added tunable constants.
 *
 *
 */
#ifndef TUNABLE_CONST_H
#define TUNABLE_CONST_H

#include "util/h/stringPool.h"
#include "util/h/list.h"

typedef enum tunableUse { developerConstant, userConstant };
typedef enum tunableType { tunableBoolean, tunableFloat };

//
// Note: this is an abstract class, and can NOT be directly created.
//
class tunableConstant {
    public:
	char *getDesc() { return(desc); }
	char *getName() { return(name); }
	static List<tunableConstant*> *allConstants;
	static stringPool *pool;
	virtual void print() = NULL;
	tunableType getType() { return(typeName); }
    protected:
	char *desc;
	char *name;
	tunableType typeName;
	tunableUse use;
};

typedef Boolean (*isValidFunc)(float newVale);
typedef void (*booleanChangeValCallBackFunc)(Boolean value);
typedef void (*floatChangeValCallBackFunc)(float value);

class tunableBooleanConstant: public tunableConstant {
    public:
	tunableBooleanConstant(Boolean initialValue, 
			       booleanChangeValCallBackFunc cb,
			       tunableUse type,
			       char *name,
			       char *desc);
	Boolean getValue() { return value; }
	Boolean setValue(Boolean newVal) {
	    value = newVal;
	}
	virtual void print();
    private:
	Boolean value;
	booleanChangeValCallBackFunc newValueCallBack;
};

class tunableFloatConstant: public tunableConstant {
    public:
	tunableFloatConstant(float initialValue, 
			float min, 
			float max, 
			floatChangeValCallBackFunc cb,
		        tunableUse type,
			char *name,
			char *desc);
	tunableFloatConstant(float initialValue, 
			isValidFunc, 
			floatChangeValCallBackFunc cb,
		        tunableUse type,
			char *name,
			char *desc);
	float getValue() { return value; }
	Boolean setValue(float newVal) {
	    if ((isValidValue) && isValidValue(newVal)) {
		value = newVal;
		if (newValueCallBack) newValueCallBack(newVal);
		return(TRUE);
	    } else if (simpleRangeCheck(newVal)) {
		value = newVal;
		if (newValueCallBack) newValueCallBack(newVal);
		return(TRUE);
	    } else {
		return(FALSE);
	    }
	}
	virtual void print();
    private:
	float value;
	float min, max;
	isValidFunc isValidValue;
	Boolean simpleRangeCheck(float val);
	floatChangeValCallBackFunc newValueCallBack;
};

#endif
