/*
 * Copyright (c) 1996-1998 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * tunableConstant - a constant that might be changed during execution.
 *
 * $Log: tunableConst.h,v $
 * Revision 1.12  2002/12/20 07:50:04  jaw
 * This commit fully changes the class name of "vector" to "pdvector".
 *
 * A nice upshot is the removal of a bunch of code previously under the flag
 * USE_STL_VECTOR, which is no longer necessary in many cases where a
 * functional difference between common/h/Vector.h and stl::vector was
 * causing a crash.
 *
 * Generally speaking, Dyninst and Paradyn now use pdvector exclusively.
 * This commit DOES NOT cover the USE_STL_VECTOR flag, which will now
 * substitute stl::vector for BPatch_Vector only.  This is currently, to
 * the best of my knowledge, only used by DPCL.  This will be updated and
 * tested in a future commit.
 *
 * The purpose of this, again, is to create a further semantic difference
 * between two functionally different classes (which both have the same
 * [nearly] interface).
 *
 * Revision 1.11  2000/07/28 17:21:44  pcroth
 * Updated #includes to reflect util library split
 *
 * Revision 1.10  1999/08/09 05:41:12  csserra
 * - added support for (mips-sgi-irix6.4) native compiler build
 * - eliminated misc. compiler warnings
 *
 * Revision 1.9  1999/06/08 05:53:00  csserra
 * ctor missing member initialization
 *
 * Revision 1.8  1999/03/03 18:15:26  pcroth
 * Updated to support Windows NT as a front-end platform
 * Changes made to X code, to use Tcl analogues when appropriate
 * Also changed in response to modifications in thread library and igen output.
 *
 * Revision 1.7  1997/10/28 20:35:08  tamches
 * dictionary_lite --> dictionary_hash
 *
 * Revision 1.6  1996/08/16 21:04:37  tamches
 * updated copyright for release 1.1
 *
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
 */

#ifndef TUNABLE_CONST_H
#define TUNABLE_CONST_H

#include <assert.h>
#include <iostream.h>
#include <string.h>
#include "common/h/String.h"
#include "common/h/Dictionary.h"

typedef enum { developerConstant, userConstant } tunableUse;
typedef enum { tunableBoolean, tunableFloat } tunableType;

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
   tunableBooleanConstant(const tunableBooleanConstant &src) :
                                          tunableConstantBase(src) {
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

 private:
   float value;
   float minval, maxval;
   isValidFunc isValidValue;
   floatChangeValCallBackFunc newValueCallBack;

   bool simpleRangeCheck(float val); // a default range checker

   tunableFloatConstant(const string &iname,
			const string &idesc,
			float initialValue, 
			float minval, float maxval,
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

   float getMin() const {return minval;}
   float getMax() const {return maxval;}
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
   typedef dictionary_hash<string, tunableBooleanConstant> tunBoolAssocArrayType;
   typedef dictionary_hash<string, tunableFloatConstant>   tunFloatAssocArrayType;

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
   static pdvector<tunableBooleanConstant> getAllBoolTunableConstants();

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
					  float minval, float maxval);
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
   static pdvector<tunableFloatConstant> getAllFloatTunableConstants();

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
				  float minval, float maxval,
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
