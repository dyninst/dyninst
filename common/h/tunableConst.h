/*
 * tunableConstant - a constant that might be changed during execution.
 *
 * $Log: tunableConst.h,v $
 * Revision 1.6  1994/10/26 22:32:50  tamches
 * Defaulted min&max to 0 for floats with no min/max in constructor.
 * Wrote min() and max() functions.
 * Wrote use() function
 * other minor changes to get to work with new tclTunable code
 *
 * Revision 1.5  1994/09/22  03:15:59  markc
 * changed char* to const char *
 *
 * Revision 1.4  1994/08/05  16:01:55  hollings
 * More consistant use of stringHandle vs. char *.
 *
 * Revision 1.3  1994/08/03  18:37:30  hollings
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
	stringHandle getName() { return(name); }
        tunableUse getUse() {return use;} // added 10/21/94 AT
	static List<tunableConstant*> *allConstants;
	static stringPool *pool;
	virtual void print() = NULL;
	tunableType getType() { return(typeName); }
    protected:
	char *desc;
	stringHandle name;
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
			       const char *name,
			       const char *desc);
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
			const char *name,
			const char *desc);
	tunableFloatConstant(float initialValue, 
			isValidFunc, 
			floatChangeValCallBackFunc cb,
		        tunableUse type,
			const char *name,
			const char *desc);
	float getValue() { return value; }
	Boolean setValue(float newVal) {
	    if ((isValidValue) && isValidValue(newVal)) {
                // If isValidValue is NULL, we'll always return false!
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
        float getMin() {return min;} // added 10/21/94 AT
        float getMax() {return max;} // added 10/21/94 AT
    private:
	float value;
	float min, max;
	isValidFunc isValidValue;
	Boolean simpleRangeCheck(float val);
	floatChangeValCallBackFunc newValueCallBack;
};

#endif
