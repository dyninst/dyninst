/*
 * tunableConstant - a constant that might be changed during execution.
 *
 * $Log: tunableConst.h,v $
 * Revision 1.5  1995/12/20 02:26:42  tamches
 * general cleanup
 *
 * Revision 1.4  1995/11/06 19:25:54  tamches
 * dictionary_hash --> dictionary_lite
 *
 * Revision 1.3  1995/10/12 18:35:26  tamches
 * Changed a lot of prototypes from "string" to "const string &", which is
 * better because it doesn't make an unneeded and expensive string copy.
 *
 * Revision 1.2  1995/06/24  20:49:48  tamches
 * Removed setValue() and print() for individual tc's.
 *
 * Revision 1.1  1995/02/27  18:50:05  tamches
 * First version of TCthread; files tunableConst.h and .C have
 * simply moved from the util lib (and have been changed); file
 * TCmain.C is completely new.
 *
 * Revision 1.10  1994/12/21  07:10:06  tamches
 * Made the "allConstants" variable protected and added a few member
 * functions to let outside code access it (safely) in a manner useful
 * for doing iterations through all tunable-constants.
 *
 * Revision 1.9  1994/12/21  00:31:44  tamches
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
#include <iostream.h>
#include <string.h>
#include "util/h/String.h"
#include "util/h/DictionaryLite.h"

typedef enum tunableUse { developerConstant, userConstant };
typedef enum tunableType { tunableBoolean, tunableFloat };

typedef bool (*isValidFunc)(float newVal);
typedef void (*booleanChangeValCallBackFunc)(bool value);
typedef void (*floatChangeValCallBackFunc)(float value);

/* ****************************************************************
 * ******************** tunableConstantBase ***********************
 * ****************************************************************
 * 
 * Every tunable constant stored in the central registry is derived from
 * this base class, which provides name, description, type (bool/float),
 * and use (user/developer) fields.
 *
 * Outside code should probably never need to know about this class.
 *
 * To obtain a read-only copy of a tunable constant, see routines like:
 * 
 *   tunableBooleanConstant myConst = tunableConstantRegistry::findBoolTunableConstant(string);
 *   tunableFloatConstant myConst = tunableConstantRegistry::findFloatTunableConstant(string);
 *
 * To make changes take effect in the central registry, try:
 * 
 *   tunableConstantRegistry::setBoolTunableConstant(const string &, bool newValue);
 *   tunableConstantRegistry::setFloatTunableConstant(const string &, float newValue);
 *
 * Note the purposeful returning of **copies**.  There are no pointers held across
 * threads.  This should lead to a pretty clean design.
 *
 * ****************************************************************
*/

class tunableConstantBase {
 protected:
   string name;
   string desc;
   string tag;
   tunableType typeName;
   tunableUse use;

   tunableConstantBase() { } // needed for Pair class

   tunableConstantBase(const string &iname, const string &idesc,
		       const tunableType theType,
		       const tunableUse  theUse);

 public:

   tunableConstantBase(const tunableConstantBase &src) :
      name(src.name), desc(src.desc), tag(src.tag),
      typeName(src.typeName), use(src.use) {
   }

   virtual ~tunableConstantBase();

   bool operator==(const tunableConstantBase &other) const {
      return (this->name == other.name);
   }

   const string &getDesc() const { return desc; }
   const string &getName() const { return name; }
   const string &getTag() const { return tag; }
   tunableUse  getUse()  const { return use; }
   tunableType getType() const { return typeName; }
};

/* ****************************************************************
 * ******************** tunableBooleanConstant ********************
 * ****************************************************************
 * 
 * Outside code should not declare variables of this class, except
 * as the return value of tunableConstantRegistry::findBoolTunableConstant()
 * That's why the meaty constructor was made private.
 *
 * ****************************************************************
 */

class tunableBooleanConstant : public tunableConstantBase {
 friend class tunableConstantRegistry;
 friend class pair<string, tunableBooleanConstant>;

 private:
   bool value;
   booleanChangeValCallBackFunc newValueCallBack;

   tunableBooleanConstant(bool initialValue, 
			  booleanChangeValCallBackFunc cb,
			  tunableUse use,
			  const string &iname,
			  const string &idesc);

 public:
   tunableBooleanConstant() : tunableConstantBase() {} // needed by Pair.h ONLY
   tunableBooleanConstant(unsigned) : tunableConstantBase() {} // needed by Pair.h ONLY
   tunableBooleanConstant(const tunableBooleanConstant &src) {
      this->value = src.value;
      this->newValueCallBack = src.newValueCallBack;
   }

   bool getValue() const {return value;}
};

/* ****************************************************************
 * ******************** tunableFloatConstant **********************
 * ****************************************************************
 * 
 * Outside code should not declare variables of this class, except
 * as the return value of tunableConstantRegistry::findFloatTunableConstant()
 * That's why the meaty constructor was made private.
 *
 * ****************************************************************
 */

class tunableFloatConstant : public tunableConstantBase {
 friend class tunableConstantRegistry;
 friend class pair<string, tunableFloatConstant>;

 private:
   float value;
   float min, max;
   isValidFunc isValidValue;
   floatChangeValCallBackFunc newValueCallBack;

   bool simpleRangeCheck(float val); // a default range checker

   tunableFloatConstant(const string &iname,
			const string &idesc,
			float initialValue, 
			float min, float max,
			floatChangeValCallBackFunc cb,
		        tunableUse use);
   tunableFloatConstant(const string &iname,
			const string &idesc,
			float initialValue, 
			isValidFunc ivf, 
			floatChangeValCallBackFunc cb,
		        tunableUse use);

 public:

   tunableFloatConstant() : tunableConstantBase() {} // needed by Pair.h ONLY
   tunableFloatConstant(unsigned) : tunableConstantBase() {} // needed by Pair.h ONLY
   tunableFloatConstant(const tunableFloatConstant &src);

   float getValue() const {return value;}

   float getMin() const {return min;}
   float getMax() const {return max;}
};

/* *****************************************************************
 * ******************** tunableConstantRegistry ********************
 * *****************************************************************
 *
 * Contains a lot of static member functions meant for use by outside code.
 *
 * Contains two very important local (static) variables: associative arrays
 * for boolean and float constants.
 *
 * *****************************************************************
*/

class tunableConstantRegistry {
 private:
   typedef dictionary_lite<string, tunableBooleanConstant> tunBoolAssocArrayType;
   typedef dictionary_lite<string, tunableFloatConstant>   tunFloatAssocArrayType;

   // the central registry is comprised of these two arrays:
   static tunBoolAssocArrayType  allBoolTunables;
   static tunFloatAssocArrayType allFloatTunables;

 public:
   // Methods not specific to bool v. float:

   static bool existsTunableConstant(const string &);
      // true iff the tunable constant exists, regardless of type.

   static tunableType getTunableConstantType(const string &);
      // Perhaps a prelude to deciding whether to call "findBoolTunableConstant"
      // or "findFloatTunableConstant".  Will (eventually be implemented to) raise
      // an exception if not found.

   static unsigned numTunables();

   static tunableConstantBase getGenericTunableConstantByName(const string &);
      // will (eventually be implemented to) throw an exception if name is not found.

   // Methods specific to boolean tunable constants:
   static unsigned numBoolTunables();

   static bool createBoolTunableConstant(const string &iname,
                                         const string &idesc,
                                         booleanChangeValCallBackFunc cb,
                                         tunableUse type,
                                         const bool initialVal);
      // returns true iff successfully created in the central repository.
      // outside code can use class tunableBooleanConstantDeclarator to avoid the
      // need to bother with this routine and the next one...
   static bool destroyBoolTunableConstant(const string &);
      // returns true iff successfully destroyed.
      // Beware of race conditions...best to only call this routine when
      // completely shutting down paradyn!
     
   static bool existsBoolTunableConstant(const string &);

   static tunableBooleanConstant findBoolTunableConstant(const string &);
   static vector<tunableBooleanConstant> getAllBoolTunableConstants();

   static void setBoolTunableConstant(const string &, bool newValue);
      // makes change take effect in the central repository.  Will eventually be implemented
      // to throw and exception if not found.

   // Methods specific to float tunable constants:
   static unsigned numFloatTunables();

   static bool createFloatTunableConstant(const string &iname,
					  const string &idesc,
                                          floatChangeValCallBackFunc cb,
                                          tunableUse type,
                                          const float initialVal,
					  float min, float max);
      // returns true iff successfully created
      // outside code can use class tunableFloatConstantDeclarator to avoid the
      // need to bother with this routine and the next two...
   static bool createFloatTunableConstant(const string &iname,
					  const string &idesc,
                                          floatChangeValCallBackFunc cb,
                                          tunableUse use,
                                          const float initialVal,
					  isValidFunc ivf);
      // returns true iff successfully created
   static bool destroyFloatTunableConstant(const string &);
      // returns true iff successfully destroyed.
      // Beware of race conditions...best to only call this routine when
      // completely shutting down paradyn!
     
   static bool existsFloatTunableConstant(const string &);

   static tunableFloatConstant findFloatTunableConstant(const string &);
   static vector<tunableFloatConstant> getAllFloatTunableConstants();

   static void setFloatTunableConstant(const string &, float newValue);
};

/* **************************************************************
 * ************************ Declarators *************************
 * **************************************************************
 *
 * Outside code can (and should) use these classes to declare
 * tunable constants.  This class automatically makes the appropriate
 * igen calls to create & destroy a tunable constant in the central
 * registry.
 *
 * **************************************************************
*/

class tunableBooleanConstantDeclarator {
 private:
   string the_name; // needed in the destructor

 public:
   tunableBooleanConstantDeclarator(const string &iname,
				    const string &idesc,
				    bool initialValue, 
				    booleanChangeValCallBackFunc cb,
				    tunableUse type);
   ~tunableBooleanConstantDeclarator();
};

class tunableFloatConstantDeclarator {
 private:
   string the_name;

 public:
   tunableFloatConstantDeclarator(const string &iname,
				  const string &idesc,
				  float initialValue,
				  float min, float max,
				  floatChangeValCallBackFunc cb,
				  tunableUse type);
   tunableFloatConstantDeclarator(const string &iname,
				  const string &idesc,
				  float initialValue, 
				  isValidFunc ivf,
				  floatChangeValCallBackFunc cb,
				  tunableUse type);
   ~tunableFloatConstantDeclarator();
};

#endif
