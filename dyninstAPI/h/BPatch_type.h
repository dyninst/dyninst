/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _BPatch_type_h_
#define _BPatch_type_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include <string.h>	
#include <assert.h>
#include <map>

#include "Type.h"
#include "Variable.h"

class BPatch_type;
namespace Dyninst { 
   namespace SymtabAPI {
      class Type;
      BPATCH_DLL_EXPORT boost::shared_ptr<Type> convert(const BPatch_type *, Type::do_share_t);
      inline Type* convert(const BPatch_type* t) {
        return convert(t, Type::share).get();
      }
   }
}

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
 * A field can be an atomic type, i.e, int  char, or more complex like a
 * union or struct.
 */


class BPATCH_DLL_EXPORT BPatch_field {
  friend class BPatch_variableExpr;
  friend class BPatch_cblock;
  
  BPatch_dataClass   typeDes;
  /* For Enums */
  int          value;

  /* For structs and unions */
  int         size;
  
  //Symtab field
  Dyninst::SymtabAPI::Field *fld;

  protected:
  void copy(BPatch_field &);
  void fixupUnknown(BPatch_module *);

  public:

  // Copy constructor
  BPatch_field(BPatch_field &f);
  BPatch_field(Dyninst::SymtabAPI::Field *fld_ = NULL, 
	       BPatch_dataClass typeDescriptor = BPatch_dataUnknownType, 
	       int value_ = 0, 
	       int size_ = 0);

  ~BPatch_field();
  
  BPatch_field & operator=(BPatch_field &src);

  const char * getName(); 

  BPatch_type * getType();

  int getValue();

  BPatch_visibility getVisibility();

  BPatch_dataClass getTypeDesc();

  int getSize();

  int getOffset();
}; 

//
// Define an instance of a Common block.  Each subroutine can have its own
//   version of the common block.
//
#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_cblock
class BPATCH_DLL_EXPORT BPatch_cblock {
private:
  // the list of fields
  BPatch_Vector<BPatch_field *> fieldList;

  // which functions use this list
  BPatch_Vector<BPatch_function *> functions;

  Dyninst::SymtabAPI::CBlock *cBlk{};

  void fixupUnknowns(BPatch_module *);
public:
  BPatch_cblock(Dyninst::SymtabAPI::CBlock *cBlk_);
  BPatch_cblock() {}
  
  BPatch_Vector<BPatch_field *> * getComponents();
  BPatch_Vector<BPatch_function *> * getFunctions();
};

class BPATCH_DLL_EXPORT BPatch_type{
    friend class BPatch;
    friend class BPatch_module;
    friend class BPatch_function;
    friend class BPatch_typeCollection;
    friend class BPatch_localVar;
    friend class BPatch_field;
    friend class BPatch_addressSpace;
    
protected:
  int           ID;                /* unique ID of type */
  static std::map<Dyninst::SymtabAPI::Type*,  BPatch_type *> type_map;
  BPatch_dataClass   type_;

  //Symtab type
  boost::shared_ptr<Dyninst::SymtabAPI::Type> typ;

  /* For common blocks */

  static int USER_BPATCH_TYPE_ID;

  // INTERNAL DATA MEMBERS

  unsigned int refCount;

 protected:
  // Simple Destructor
  virtual ~BPatch_type();
  static BPatch_type *findOrCreateType(boost::shared_ptr<Dyninst::SymtabAPI::Type> type);
  static BPatch_type *findOrCreateType(Dyninst::SymtabAPI::Type* ty) {
    return findOrCreateType(ty->reshare());
  }
  
  // A few convenience functions
  BPatch_dataClass convertToBPatchdataClass(Dyninst::SymtabAPI::dataClass type);
  Dyninst::SymtabAPI::dataClass convertToSymtabType(BPatch_dataClass type);

  static BPatch_type *createFake(const char *_name);
  /* Placeholder for real type, to be filled in later */
  static BPatch_type *createPlaceholder(int _ID, const char *_name = NULL) 
         { return new BPatch_type(_name, _ID, BPatch_dataUnknownType); }

public:
  BPatch_type(const char *name = NULL, int _ID = 0, BPatch_dataClass = BPatch_dataNullType);
  BPatch_type(boost::shared_ptr<Dyninst::SymtabAPI::Type> typ_);
  BPatch_type(Dyninst::SymtabAPI::Type* t)
    : BPatch_type(t->reshare()) {}
  virtual bool operator==(const BPatch_type &) const;

  int  getID() const { return ID;}

  unsigned int getSize();

  boost::shared_ptr<Dyninst::SymtabAPI::Type> getSymtabType(Dyninst::SymtabAPI::Type::do_share_t) const;
  Dyninst::SymtabAPI::Type* getSymtabType() const {
    return getSymtabType(Dyninst::SymtabAPI::Type::share).get();
  }

//Define all of these in .C 
  const char *getName() const;

  BPatch_dataClass getDataClass() const { return type_; }

  unsigned long getLow() const;
  unsigned long getHigh() const;
  BPatch_Vector<BPatch_field *> * getComponents() const;
  bool isCompatible(BPatch_type * otype);
  BPatch_type *getConstituentType() const;
  BPatch_Vector<BPatch_cblock *> *getCblocks() const;

  // INTERNAL METHODS

  void incrRefCount() { ++refCount; }
  void decrRefCount() { assert(refCount > 0); if (!--refCount) delete this; }
  void fixupUnknowns(BPatch_module *) { }
};

//
// This class stores information about local variables.
// It is desgined store information about a variable in a function.
// Scope needs to be addressed in this class.

class BPATCH_DLL_EXPORT BPatch_localVar{
    friend class BPatch;
    friend class BPatch_function;

    BPatch_type *type{};
    BPatch_storageClass storageClass{};
    // scope_t scope;

    Dyninst::SymtabAPI::localVar *lVar{};

public:
    //  Internal use only
    BPatch_localVar(Dyninst::SymtabAPI::localVar *lVar_);
    ~BPatch_localVar();
    BPatch_localVar() {}

    void fixupUnknown(BPatch_module *);
    Dyninst::SymtabAPI::localVar *getSymtabVar();
    BPatch_storageClass convertToBPatchStorage(Dyninst::VariableLocation *loc);

public:
    //  end of functions for internal use only
    const char *	getName();
    BPatch_type *	getType();
    int			getLineNum();
    long		getFrameOffset();
    int			getRegister();
    BPatch_storageClass	getStorageClass();

};

#endif /* _BPatch_type_h_ */
