/*
 * tunableConstant - a constant that might be changed during execution.
 *
 * $Log: tunableConst.h,v $
 * Revision 1.9  1994/12/21 00:31:44  tamches
 * Greatly cleaned up the interface; no data members are public any more.
 * Also some minor changes, such as using g++'s built-in "bool" instead
 * of "Boolean".
 *
 * Revision 1.8  1994/11/04  15:52:51  tamches
 * setValue() for boolean tc's now correctly invokes its callback function, if any.
 *
 * Revision 1.7  1994/11/01  16:07:35  markc
 * Added Object classes that provide os independent symbol tables.
 * Added stl-like container classes with iterators.
 *
 * Revision 1.6  1994/10/26  22:32:50  tamches
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

#include <assert.h>
#include "util/h/stringPool.h"
#include "util/h/list.h"

typedef enum tunableUse { developerConstant, userConstant };
typedef enum tunableType { tunableBoolean, tunableFloat };

//
// Note: this is an abstract class, and can NOT be directly created.
//
class tunableConstant {
 protected:
   char *desc;
   char *name;
   tunableType typeName;
   tunableUse use;

   static stringPool *pool; // made protected

 public:
   tunableConstant() {}
   virtual ~tunableConstant() {}

   const char *getDesc() const { // added const
      return desc;
   }
   const char *getName() const { // result type used to be stringhandle
      return name;
   }
   tunableUse getUse() const {
      return use;
   }
   tunableType getType() const {
      return typeName;
   }

   static List<tunableConstant*> *allConstants; // NEEDS TO BE MADE PROTECTED

   static tunableConstant *findTunableConstant(const char *name);
      // returns NULL if not found

   virtual void print() = NULL;
};


// Shouldn't the string pools be made part of the base class?

// typedef Boolean (*isValidFunc)(float newVale);
typedef bool (*isValidFunc)(float newVal);
typedef void (*booleanChangeValCallBackFunc)(bool value);
typedef void (*floatChangeValCallBackFunc)(float value);

class tunableBooleanConstant: public tunableConstant {
    private:
	bool value;
	booleanChangeValCallBackFunc newValueCallBack;

    public:
	tunableBooleanConstant(bool initialValue, 
			       booleanChangeValCallBackFunc cb,
			       tunableUse type,
			       const char *name,
			       const char *desc);
	bool getValue() { return value; }
	bool setValue(bool newVal) {
           value = newVal;
           if (newValueCallBack)
              newValueCallBack(newVal);
           return true;
        }
	virtual void print();
};

class tunableFloatConstant: public tunableConstant {
    private:
	float value;
	float min, max;
	isValidFunc isValidValue;
	bool simpleRangeCheck(float val);
	floatChangeValCallBackFunc newValueCallBack;

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
	bool setValue(float newVal) {
	    if ((isValidValue) && isValidValue(newVal)) {
                // If isValidValue is NULL, we'll always return false!
		value = newVal;
		if (newValueCallBack) newValueCallBack(newVal);
		return true;
	    } else if (simpleRangeCheck(newVal)) {
		value = newVal;
		if (newValueCallBack) newValueCallBack(newVal);
		return true;
	    } else {
		return false;
	    }
	}
	virtual void print();
        float getMin() {return min;} // added 10/21/94 AT
        float getMax() {return max;} // added 10/21/94 AT
};

#endif
