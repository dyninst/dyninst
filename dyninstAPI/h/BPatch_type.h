/*
 * Copyright (c) 1996 Barton P. Miller
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

#ifndef _BPatch_type_h_
#define _BPatch_type_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"

typedef enum {BPatchSymLocalVar,  BPatchSymGlobalVar, BPatchSymRegisterVar,
	      BPatchSymStaticLocalVar, BPatchSymStaticGlobal,
	      BPatchSymLocalFunc, BPatchSymGlobalFunc, BPatchSymFuncParam,
	      BPatchSymTypeName, BPatchSymAggType, BPatchSymTypeTag}symDescr_t;

/*
 * Symbol Descriptors:
 * BPatchSymLocalVar       - local variable- gnu sun-(empty)
 * BPatchSymGlobalVar      - global variable- gnu sun-'G'
 * BPatchSymRegisterVar    - register variable- gnu sun-'r'
 * BPatchSymStaticLocalVar - static local variable- gnu sun-'V'
 * BPatchSymStaticGlobal   - static global variable- gnu- sun'S'
 * BPatchSymLocalFunc      - local function- gnu sun-'f'
 * BPatchSymGlobalFunc     - global function- gnu- sun'F'
 * BPatchSymFuncParam      - function paramater - gnu- sun'p'
 * BPatchSymTypeName       - type name- gnu sun-'t'
 * BPatchSymAggType        - aggregate type-struct,union, enum- gnu sun-'T'
 * BPatchSymTypeTag        - C++ type name and tag combination
 */

typedef enum {BPatch_scalar, 
	      BPatch_enumerated,
	      BPatch_class,
	      BPatch_structure, 
	      BPatch_union, 
	      BPatch_array, 
	      BPatch_pointer, 
	      BPatch_referance, 
	      BPatch_func,
	      BPatch_typeAttrib,
	      BPatch_built_inType,
	      BPatch_reference,
	      BPatch_unknownType,
	      BPatchSymTypeRange,
	      BPatch_method,
	      BPatch_common} BPatch_dataClass;

/*
 * Type Descriptors:
 * BPatchSymTypeReference - type reference- gnu sun-(empty)
 * BPatch_array - array type- gnu sun-'a'
 * BPatch_enumerated - enumerated type- gnu sun-'e'
 * BPatch_class - Class type
 * BPatch_func - function type- gnu sun-'f'
 * BPatchSymTypeRange - range type- gnu sun-'r'
 * BPatch_structure - structure type- gnu sun-'s'
 * BPatch_union - union specification- gnu sun-'u'
 * BPatch_pointer - pointer type- gnu sun-'*'
 * BPatch_referance - referance type- gnu sun-'*'
 * BPatch_typeAttrib - type attribute (C++)- gnu sun- '@'
 * BPatch_built_inType - built-in type -- gdb doc ->negative type number
 * BPatch_reference - C++ reference to another type- gnu sun- '&'
 * BPatch_method - C++ class method
 */

typedef enum {BPatch_private, BPatch_protected, BPatch_public,
	      BPatch_optimized=9,BPatch_visUnknown}BPatch_visibility;
/*
 * BPatch_visibility: Accessibility of member data and functions
 * These values follow the 'field_name:' after the '/' identifier.
 * BPatch_private   == 0 gnu Sun -- private
 * BPatch_protected == 1 gnu Sun -- protected
 * BPatch_public    == 2 gnu Sun -- public
 * BPatch_optimized == 9 gnu Sun -- field optimized out and is public
 * BPatch_visUnknown visibility not known or doesn't apply(ANSIC), the default
 *
 */

class BPatch_type;
class BPatch_function;

/*
 * A BPatch_field is equivalent to a field in a enum, struct, or union.
 * A field can be an atomic type, i.e, int, char, or more complex like a
 * union or struct.  
 */
class BPATCH_DLL_EXPORT BPatch_field {
  friend class BPatch_variableExpr;

  char *fieldname;
  BPatch_dataClass   typeDes;
  BPatch_visibility  vis;
  /* For Enums */
  int          value;

  /* For structs and unions */
  BPatch_type *type;
  int         offset;
  int         size;

  /* Method vars */
  
public:
  
  // Enum constructor
  BPatch_field(const char * fName,  BPatch_dataClass _typeDes, int eValue);
  // C++ version for Enum constructor
  BPatch_field(const char * fName,  BPatch_dataClass _typeDes, int eValue,
	       BPatch_visibility _vis);	    
  // Struct or Union construct
  BPatch_field(const char * fName,  BPatch_dataClass _typeDes, 
	       BPatch_type *suType, int suOffset, int suSize);
  // C++ version for Struct or Union construct
  BPatch_field(const char * fName,  BPatch_dataClass _typeDes, 
	       BPatch_type *suType, int suOffset, int suSize,
	       BPatch_visibility _vis);
  
  
			      
  const char *getName() { return fieldname; } 
  BPatch_type *getType() { return type; }
  int getValue() { return value;}
  BPatch_visibility getVisibility() { return vis; }
  BPatch_dataClass getTypeDesc() { return typeDes; }
  int getSize() { return size; }
  int getOffset() { return offset; }
}; 

//
// Define an instance of a Common block.  Each subroutine can have its own
//   version of the common block.
//
class BPATCH_DLL_EXPORT BPatch_cblock {
   friend BPatch_type;

private:
  // the list of fields
  BPatch_Vector<BPatch_field *> fieldList;

  // which functions use this list
  BPatch_Vector<BPatch_function *> functions;

public:
  BPatch_Vector<BPatch_field *> *getComponents() { return &fieldList; }
  BPatch_Vector<BPatch_function *> *getFunctions() { return &functions; }
};

class BPATCH_DLL_EXPORT BPatch_type {
private:
  bool	        nullType;
  char		*name;
  int           ID;                /* unique ID of type */
  int           size;              /* size of type */
  char		*low;               /* lower bound */
  char          *hi;                /* upper bound */
  
  BPatch_dataClass   type_;
  BPatch_type 	*ptr;               /* pointer to other type (for ptrs, arrays) */

  /* For enums, structs and union components */
  BPatch_Vector<BPatch_field *> fieldList;

  /* For common blocks */
  BPatch_Vector<BPatch_cblock *> *cblocks;
  
public:
// Start Internal Functions
  BPatch_type();
  
  BPatch_type(const char *_name, bool _nullType = false);
  /* Enum constructor */
  BPatch_type(const char *_name, int _ID,  BPatch_dataClass _type);
  /* Struct, union, range(size), reference(void) constructor and
     Built-in type constructor-negative type numbers defined by gdb doc */
  BPatch_type(const char *_name, int _ID, BPatch_dataClass _type, int _size);
  /* Pointer (internal) constructor */
  BPatch_type(const char *_name, int _ID, BPatch_dataClass _type,
	      BPatch_type * _ptr);
  /* Range (lower and uper bound) constructor */
  BPatch_type(const char *_name, int _ID, BPatch_dataClass _type,
	      const char * _low, const char * _hi);
  /* Array Constructor */
  BPatch_type(const char *_name, int _ID, BPatch_dataClass _type,
	      BPatch_type * _ptr, int _low, int _hi);
  /* Pre-existing type--typedef */
  BPatch_type(const char *_name, int _ID, BPatch_type * _ptr);
  /* defining a type in terms of a builtin type (Type Attribute) */
  BPatch_type(const char *_name, int _ID, BPatch_dataClass _type,
	      int _size, BPatch_type * _ptr);

   /* Add field for Enum */
  void addField(const char * _fieldname,  BPatch_dataClass _typeDes, int value);
  /* Add field for C++(has visibility) Enum */
  void addField(const char * _fieldname,  BPatch_dataClass _typeDes, int value,
		BPatch_visibility _vis);
  /* Add field for struct or union */
  void addField(const char * _fieldname,  BPatch_dataClass _typeDes, 
      BPatch_type *_type, int _offset, int _size);
  /* Add field for C++(has visibility) struct or union */
  void addField(const char * _fieldname,  BPatch_dataClass _typeDes, 
      BPatch_type *_type, int _offset, int _size, BPatch_visibility _vis);
  
 void beginCommonBlock();
 void endCommonBlock(BPatch_function *, void *baseAddr);

 // END Internal Functions

      
 // Constructors for USER DEFINED BPatch_types
  /* Enum constructor */
  BPatch_type(const char *_name, BPatch_dataClass _type);
  /* Struct, union, range(size), reference(void) constructor and
     Built-in type constructor-negative type numbers defined by gdb doc */
  BPatch_type(const char *_name, BPatch_dataClass _type, int _size);
  /* Pointer (internal) constructor */
  BPatch_type(const char *_name, BPatch_type * _ptr, int size_);
  /* Range (lower and uper bound) constructor */
  BPatch_type(const char *_name, BPatch_dataClass _type,
	      const char * _low, const char * _hi);
  /* Array Constructor */
  BPatch_type(const char *_name, BPatch_dataClass _type,
	      BPatch_type * _ptr, int _low, int _hi);
  /* Pre-existing type--typedef */
  BPatch_type(const char *_name, BPatch_type * _ptr);
  /* defining a type in terms of a builtin type (Type Attribute) */
  BPatch_type(const char *_name, BPatch_dataClass _type, int _size,
	      BPatch_type * _ptr);
			    
  int  getID(){ return ID;}
  void setID(int typeId) { ID = typeId; }
  void setDataClass(BPatch_dataClass p1) { type_ = p1; }

  int getSize() const { return size; };
  const char *getName() { return name; }
  const char *getLow() { return low; }
  const char *getHigh() { return hi; }
  BPatch_type *getConstituentType() { return ptr; }
  bool isCompatible(BPatch_type *otype);
  BPatch_dataClass getDataClass() { return type_; }
  BPatch_Vector<BPatch_field *> *getComponents() { 
      return &fieldList; }
  BPatch_Vector<BPatch_cblock *> *getCblocks() { return cblocks; }
};


//
// This class stores information about local variables.
// It is desgined store information about a variable in a function.
// Scope needs to be addressed in this class.

class BPATCH_DLL_EXPORT BPatch_localVar{
  char * name;
  BPatch_type * type;
  int lineNum;
  int frameOffset;

  //USed only in XCOFF/COFF format, can be 
  //   scAbs=5, 
  //   scRegister, scVarRegister, 
  //   scVar=0

  int storageClass; 

  bool frameRelative;
  // scope_t scope;
  
public:
  
  BPatch_localVar(char * _name,  BPatch_type * _type, int _lineNum,
		  int _frameOffset, int _sc = 5 /* scAbs */, bool fr=true);
  ~BPatch_localVar();

  const char *  getName()        { return name; }
  BPatch_type * getType()        { return type; }
  int           getLineNum()     { return lineNum; }
  int           getFrameOffset() { return frameOffset; }
  int 		getFrameRelative() { return frameRelative; }
  int 		getSc()		 { return storageClass; }
  
};

#endif /* _BPatch_type_h_ */
