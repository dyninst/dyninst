/*
 * tunableConstant - a constant that might be changed during execution.
 *
 * $Log: tunableConst.h,v $
 * Revision 1.2  1995/06/24 20:49:48  tamches
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
#include "util/h/Dictionary.h"

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
 * But outside code shouldn't declare variables of this type (or of type
 * tunableBoolConstant or tunableFloatConstant---this is something new to
 * be aware of!).  To create a tunable constant, see class
 * tunableBooleanConstantDeclarator or class tunableFloatConstantDeclarator.
 * These are stub classes that just call the appropriate new static member
 * function of tunableConstantRegistry to create and destroy tunable
 * constants in the central registry.  You could call them manually if you really
 * want, for example:
 *
 *    tunableConstantRegistry::createFloatTunableConstant(name, desc,
 *                   callback, type, initialValue, min, max);
 *
 *    tunableConstantRegistry::destroyBoolTunableConstant(name);
 *
 * To obtain a read-only copy of a tunable constant, see routines like:
 * 
 *   tunableBooleanConstant myConst = tunableConstantRegistry::findBoolTunableConstant(string);
 *   tunableFloatConstant myConst = tunableConstantRegistry::findFloatTunableConstant(string);
 *
 * Any changes you make to the **copy** that gets returned by these two functions
 * are *** NOT *** really propagated to the central registry.  That's why we say
 * the copy that gets returned is "read-only" (in effect).  To make changes take
 * effect in the central registry, try:
 * 
 *   tunableConstantRegistry::setFloatTunableConstant(const string, const float newValue);
 *   tunableConstantRegistry::setBoolTunableConstant(const string, const bool newValue);
 *
 * Note the static member functions of class "tunableConstantRegistry".  Eventually,
 * these will evolve to become igen calls.  These are the routines that outside code
 * should feel free to call.
 *
 * Note the purposeful returning of **copies**.  There are no pointers held across
 * threads.  This should lead to a pretty clean design.
 *
 * TO DO LIST:
 * 1) turn the static functions of class tunableConstantRegistry into real igen calls
 * 2) there is a big problem with the "callback" feature: shouldn't callback functions
 *    be arbitrary igen calls? (after all, they can certainly be in different threads)
 *    How to do this? [try replacing passing ptr-to-function when creating a TC
 *    with: a threadid (who to callback to) and other stuff]
 * ****************************************************************
*/


/* **************************************************************** */
/* **************************************************************** */


class tunableConstantBase {
 protected:
   string name;
   string desc;
   string tag;
   tunableType typeName;
   tunableUse use;

   tunableConstantBase() { } // needed for Pair class

   tunableConstantBase(const string iname, const string idesc,
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

/* **************************************************************** */
/* ******************** tunableBooleanConstant ******************** */
/* **************************************************************** */

class tunableBooleanConstant : public tunableConstantBase {
 friend class tunableConstantRegistry;
 friend class pair<string, tunableBooleanConstant>;

 private:
   bool value;
   booleanChangeValCallBackFunc newValueCallBack;

   tunableBooleanConstant(bool initialValue, 
			  booleanChangeValCallBackFunc cb,
			  tunableUse use,
			  string iname,
			  string idesc);

 public:
   tunableBooleanConstant() : tunableConstantBase() {} // needed by Pair.h ONLY
   tunableBooleanConstant(unsigned) : tunableConstantBase() {} // needed by Pair.h ONLY
   tunableBooleanConstant(const tunableBooleanConstant &src) {
      this->value = src.value;
      this->newValueCallBack = src.newValueCallBack;
   }

   bool getValue() const {return value;}
};

/* **************************************************************** */
/* ********************** tunableFloatConstant ******************** */
/* **************************************************************** */

class tunableFloatConstant : public tunableConstantBase {
 friend class tunableConstantRegistry;
 friend class pair<string, tunableFloatConstant>;

 private:
   float value;
   float min, max;
   isValidFunc isValidValue;
   floatChangeValCallBackFunc newValueCallBack;

   bool simpleRangeCheck(float val); // a default range checker

   tunableFloatConstant(const string iname,
			const string idesc,
			float initialValue, 
			float min, float max,
			floatChangeValCallBackFunc cb,
		        tunableUse use);
   tunableFloatConstant(const string iname,
			const string idesc,
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
 * The tunable constant central registry.
 * 
 * Contains a lot of static member functions (that will evolve into igen calls),
 * which outside code is encouraged to call at will.
 *
 * Contains two very important local (static) variables: associative arrays
 * for boolean and float constants.
 *
 * *****************************************************************
*/

class tunableConstantRegistry {
 private:
   typedef dictionary_hash<string, tunableBooleanConstant> tunBoolAssocArrayType;
   typedef dictionary_hash<string, tunableFloatConstant>   tunFloatAssocArrayType;

   // the central registry is comprised of these two arrays:
   static tunBoolAssocArrayType  allBoolTunables;
   static tunFloatAssocArrayType allFloatTunables;

 public:
   // (FUTURE) IGEN CALLS:

   static bool existsTunableConstant(const string);
      // true iff the tunable constant exists.  Does not return the type.

   static bool existsBoolTunableConstant(const string);
      // true iff the tunable constant exists, and is boolean.

   static bool existsFloatTunableConstant(const string);
      // true iff the tunable constant exists, and is float.

   static tunableType getTunableConstantType(const string);
      // Perhaps a prelude to deciding whether to call "findBoolTunableConstant"
      // or "findFloatTunableConstant".  Will (eventually be implemented to) raise
      // an exception if not found.

   static int numTunables();

   static tunableConstantBase getGenericTunableConstantByName(const string);
      // will (eventually be implemented to) throw an exception if name is not found.

   // (FUTURE) IGEN CALLS SPECIFIC TO BOOLEAN TC'S:
   static int numBoolTunables();
   static bool createBoolTunableConstant(const string iname,
                                         const string idesc,
                                         booleanChangeValCallBackFunc cb,
                                         tunableUse type,
                                         const bool initialVal);
      // returns true iff successfully created
      // outside code can use class tunableBooleanConstantDeclarator to avoid the
      // need to bother with this routine and the next one...

   static bool destroyBoolTunableConstant(const string);
      // returns true iff successfully destroyed.
      // Beware of race conditions...best to only call this routine when
      // completely shutting down paradyn!
     
   static tunableBooleanConstant findBoolTunableConstant(const string);
   static vector<tunableBooleanConstant> getAllBoolTunableConstants();

   static void setBoolTunableConstant(const string, const bool newValue);

   // (FUTURE) IGEN CALLS SPECIFIC TO FLOAT TC'S:
   static int numFloatTunables();
   static bool createFloatTunableConstant(const string iname,
					  const string idesc,
                                          floatChangeValCallBackFunc cb,
                                          tunableUse type,
                                          const float initialVal,
					  float min, float max);
      // returns true iff successfully created
      // outside code can use class tunableFloatConstantDeclarator to avoid the
      // need to bother with this routine and the next one...

   static bool createFloatTunableConstant(const string iname,
					  const string idesc,
                                          floatChangeValCallBackFunc cb,
                                          tunableUse use,
                                          const float initialVal,
					  isValidFunc ivf);
      // returns true iff successfully created

   static bool destroyFloatTunableConstant(const string);
      // returns true iff successfully destroyed.
      // Beware of race conditions...best to only call this routine when
      // completely shutting down paradyn!
     
   static tunableFloatConstant findFloatTunableConstant(const string);
   static vector<tunableFloatConstant> getAllFloatTunableConstants();

   static void setFloatTunableConstant(const string, const float newValue);
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
   tunableBooleanConstantDeclarator(const string iname,
				    const string idesc,
				    bool initialValue, 
				    booleanChangeValCallBackFunc cb,
				    tunableUse type);
   ~tunableBooleanConstantDeclarator();
};

class tunableFloatConstantDeclarator {
 private:
   string the_name;

 public:
   tunableFloatConstantDeclarator(const string iname,
				  const string idesc,
				  float initialValue,
				  float min, float max,
				  floatChangeValCallBackFunc cb,
				  tunableUse type);
   tunableFloatConstantDeclarator(const string iname,
				  const string idesc,
				  float initialValue, 
				  isValidFunc ivf,
				  floatChangeValCallBackFunc cb,
				  tunableUse type);
   ~tunableFloatConstantDeclarator();
};

#endif
