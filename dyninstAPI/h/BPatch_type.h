/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
#include "BPatch_eventLock.h"
#include <string.h>	

#if defined (USES_DWARF_DEBUG)
#include <dwarf.h>
#include <libdwarf.h>
#endif

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

#if defined (IBM_BPATCH_COMPAT)
#define BPatch_dataBuilt_inType BPatch_dataUnknownType
#endif

typedef enum {BPatch_dataScalar, 
	      BPatch_dataEnumerated,
	      BPatch_dataTypeClass,
	      BPatch_dataStructure, 
	      BPatch_dataUnion, 
	      BPatch_dataArray, 
	      BPatch_dataPointer, 
	      BPatch_dataReferance, 
	      BPatch_dataFunction,
	      BPatch_dataTypeAttrib,
	      BPatch_dataReference,
	      BPatch_dataUnknownType,
	      BPatchSymTypeRange,
	      BPatch_dataMethod,
	      BPatch_dataCommon,
	      BPatch_dataPrimitive,
	      BPatch_dataTypeNumber,
	      BPatch_dataTypeDefine,
              BPatch_dataNullType
} BPatch_dataClass;


#define BPatch_scalar	BPatch_dataScalar
#define BPatch_enumerated	BPatch_dataEnumerated
#define BPatch_typeClass	BPatch_dataTypeClass
#define BPatch_structure	BPatch_dataStructure 
#define BPatch_union	BPatch_dataUnion 
#define BPatch_array	BPatch_dataArray 
#define BPatch_pointer	BPatch_dataPointer 
#define BPatch_reference	BPatch_dataReferance 
#define BPatch_typeAttrib	BPatch_dataTypeAttrib
#define BPatch_unknownType	BPatch_dataUnknownType
#define BPatch_typeDefine	BPatch_dataTypeDefine

/*
 * Type Descriptors:
 * BPatchSymTypeReference - type reference- gnu sun-(empty)
 * BPatch_dataArray - array type- gnu sun-'a'
 * BPatch_dataEnumerated - enumerated type- gnu sun-'e'
 * BPatch_dataTypeClass - Class type
 * BPatch_dataFunction - function type- gnu sun-'f'
 * BPatchSymTypeRange - range type- gnu sun-'r'
 * BPatch_dataStructure - structure type- gnu sun-'s'
 * BPatch_dataUnion - union specification- gnu sun-'u'
 * BPatch_dataPointer - pointer type- gnu sun-'*'
 * BPatch_dataReferance - referance type- gnu sun-'*'
 * BPatch_dataTypeAttrib - type attribute (C++)- gnu sun- '@'
 * BPatch_dataReference - C++ reference to another type- gnu sun- '&'
 * BPatch_dataMethod - C++ class method
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

typedef enum {
    BPatch_storageAddr,
    BPatch_storageAddrRef,
    BPatch_storageReg,
    BPatch_storageRegRef,
    BPatch_storageRegOffset,
    BPatch_storageFrameOffset
} BPatch_storageClass;

/*
 * BPatch_storageClass: Encodes how a variable is stored.
 * 
 * BPatch_storageAddr		- Absolute address of variable.
 * BPatch_storageAddrRef	- Address of pointer to variable.
 * BPatch_storageReg		- Register which holds variable value.
 * BPatch_storageRegRef		- Register which holds pointer to variable.
 * BPatch_storageRegOffset	- Address of variable = $reg + address.
 * BPatch_storageFrameOffset	- Address of variable = $fp  + address.
 */

class BPatch_type;
class BPatch_function;
class BPatch_module;

/*
 * A BPatch_field is equivalent to a field in a enum, struct, or union.
 * A field can be an atomic type, i.e, int, char, or more complex like a
 * union or struct.
 */

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_field

class BPATCH_DLL_EXPORT BPatch_field : public BPatch_eventLock{
  friend class BPatch_variableExpr;
  friend class BPatch_typeFunction;
  friend class BPatch_typeStruct;
  friend class BPatch_typeUnion;
  friend class BPatch_cblock;
  char *fieldname;
  BPatch_dataClass   typeDes;
  BPatch_visibility  vis;
  /* For Enums */
  int          value;

  /* For structs and unions */
  BPatch_type *_type;
  int         offset;
  int         size;

  /* Method vars */
  
  protected:
  void copy(BPatch_field &);
  void fixupUnknown(BPatch_module *);

  public:
  // Enum constructor
  API_EXPORT_CTOR(Enum, (fName, _typeDes, eValue),
  BPatch_field,(const char * fName,  BPatch_dataClass _typeDes, int eValue));
  // C++ version for Enum constructor
  API_EXPORT_CTOR(EnumCpp, (fName, _typeDes, eValue, _vis),
  BPatch_field, (const char * fName,  BPatch_dataClass _typeDes, int eValue,
	       BPatch_visibility _vis));	    
  // Struct or Union construct
  API_EXPORT_CTOR(SU, (fName, _typeDes, suType, suOffset, suSize),
  BPatch_field,(const char * fName,  BPatch_dataClass _typeDes, 
	       BPatch_type *suType, int suOffset, int suSize));
  // C++ version for Struct or Union construct
  API_EXPORT_CTOR(SUCpp, (fName, _typeDes, suType, suOffset, suSize, _vis),
  BPatch_field,(const char * fName,  BPatch_dataClass _typeDes, 
	       BPatch_type *suType, int suOffset, int suSize,
	       BPatch_visibility _vis));

  // Copy constructor
  BPatch_field(BPatch_field &f);

  API_EXPORT_DTOR(_dtor,(),
  ~,BPatch_field,());
  
  API_EXPORT_OPER(_equals, (src),
  BPatch_field &,operator=,(BPatch_field &src));

  API_EXPORT_OPER(_equals_equals, (ofield),
  bool ,operator==,(const BPatch_field &ofield));
			      
  API_EXPORT(Int, (),
  const char *,getName,()); 

  API_EXPORT(Int, (),
  BPatch_type *,getType,());

  API_EXPORT(Int, (),
  int,getValue,());

  API_EXPORT(Int, (),
  BPatch_visibility,getVisibility,());

  API_EXPORT(Int, (),
  BPatch_dataClass,getTypeDesc,());

  API_EXPORT(Int, (),
  int,getSize,());

  API_EXPORT(Int, (),
  int,getOffset,());
}; 

//
// Define an instance of a Common block.  Each subroutine can have its own
//   version of the common block.
//
#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_cblock
class BPATCH_DLL_EXPORT BPatch_cblock : public BPatch_eventLock{
   friend class BPatch_typeCommon;

private:
  // the list of fields
  BPatch_Vector<BPatch_field *> fieldList;

  // which functions use this list
  BPatch_Vector<BPatch_function *> functions;

  void fixupUnknowns(BPatch_module *);
public:
  API_EXPORT(Int, (),
  BPatch_Vector<BPatch_field *> *,getComponents,());
  API_EXPORT(Int, (),
  BPatch_Vector<BPatch_function *> *,getFunctions,());
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_type
class BPATCH_DLL_EXPORT BPatch_type : public BPatch_eventLock{
    friend class BPatch;
    friend class BPatch_typeCollection;
    friend char *parseStabString(BPatch_module *, int, char *,
                                 int, BPatch_typeCommon *);
protected:
  char		*name;
  int           ID;                /* unique ID of type */
  unsigned int  size;              /* size of type */
  
  BPatch_dataClass   type_;

  /* For common blocks */

  static int USER_BPATCH_TYPE_ID;

  // INTERNAL DATA MEMBERS

  unsigned int refCount;

 protected:
  virtual void updateSize() {}
  // Simple Destructor
  virtual ~BPatch_type();
  BPatch_type(const char *name = NULL, int _ID = 0, BPatch_dataClass = BPatch_dataNullType);

  virtual void merge( BPatch_type * /* other */ ) { assert(0); }

  // A few convenience functions

  static BPatch_type *createFake(const char *_name);
  /* Placeholder for real type, to be filled in later */
  static BPatch_type *createPlaceholder(int _ID, const char *_name = NULL) 
         { return new BPatch_type(_name, _ID, BPatch_dataUnknownType); }

public:
  virtual bool operator==(const BPatch_type &) const;

  int  getID() const { return ID;}

  API_EXPORT(Int, (),
  unsigned int,getSize,());

  const char *getName() const { return name; }

#ifdef IBM_BPATCH_COMPAT
  char *getName(char *buffer, int max) const; 
  BPatch_dataClass type() const { return type_; }
#endif
  BPatch_dataClass getDataClass() const { return type_; }
  virtual const char *getLow() const { return NULL; }
  virtual const char *getHigh() const { return NULL; }
  virtual BPatch_Vector<BPatch_field *> * getComponents() const { return NULL; }
  virtual bool isCompatible(BPatch_type * /* otype */) { return true; }
  virtual bool isCompatibleInt(BPatch_type * /* otype */) { return true; }
  virtual BPatch_type *getConstituentType() const { return NULL; }
  virtual BPatch_Vector<BPatch_cblock *> *getCblocks() const { return NULL; }

  // INTERNAL METHODS

  void incrRefCount() { ++refCount; }
  void decrRefCount() { assert(refCount > 0); if (!--refCount) delete this; }
  virtual void fixupUnknowns(BPatch_module *) { }
};

//
// This class stores information about local variables.
// It is desgined store information about a variable in a function.
// Scope needs to be addressed in this class.

#if defined (os_osf)
class eCoffSymbol;
#endif

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_localVar
class process;
class BPatch_fieldListType;
class BPATCH_DLL_EXPORT BPatch_localVar : public BPatch_eventLock{
    friend class BPatch;
    friend class BPatch_function;
    friend class BPatch_typeCommon;
    friend class BPatch_localVarCollection;
    friend char *parseStabString(BPatch_module *, int, char *,
                                 int, BPatch_typeCommon *);
#if defined(USES_DWARF_DEBUG)
    friend bool walkDwarvenTree(Dwarf_Debug &, char *, Dwarf_Die,
                        BPatch_module *, process *, BPatch_function *,
                        BPatch_typeCommon *, BPatch_fieldListType *);
#endif
#if defined (os_osf)
   friend void eCoffParseProc(BPatch_module *, eCoffSymbol &, bool);
#endif

    char *name;
    BPatch_type *type;
    int lineNum;
    long frameOffset;
    int reg;
    BPatch_storageClass storageClass;
    // scope_t scope;

    BPatch_localVar(const char *_name,  BPatch_type *_type,
		    int _lineNum, long _frameOffset, int _reg=-1,
		    BPatch_storageClass _storageClass=BPatch_storageFrameOffset);
    ~BPatch_localVar();

    void fixupUnknown(BPatch_module *);

public:
    const char *	getName() { return name; }
    BPatch_type *	getType() { return type; }
    int			getLineNum() { return lineNum; }
    long		getFrameOffset() { return frameOffset; }
    int			getRegister() { return reg; }
    BPatch_storageClass	getStorageClass() { return storageClass; }

};

#endif /* _BPatch_type_h_ */
