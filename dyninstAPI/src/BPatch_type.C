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

#include <stdio.h>

#define BPATCH_FILE


#include "util.h"
#include "BPatch_Vector.h"
#include "BPatch_type.h"
#include "BPatch_collections.h"
#include "showerror.h"
#include "BPatch.h"

// This is the ID that is decremented for each type a user defines. It is
// Global so that every type that the user defines has a unique ID.
// jdd 7/29/99
static int USER_BPATCH_TYPE_ID = -1000;

/*
 * BPatch_type::BPatch_type
 *
 * EMPTY Constructor for BPatch_type.  
 * 
 */
BPatch_type::BPatch_type() :
    nullType(true), cblocks(NULL)
{
  name = NULL;
  ID = 0;
  size = sizeof(int);
  low = NULL;
  hi = NULL;
  ptr = NULL;
}

/*
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type.  Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char *_name, bool _nullType) :
  nullType(_nullType), cblocks(NULL)
{
    /* XXX More later. */
  if( _name)
    name = strdup(_name);
  else
    name = NULL;

  size = sizeof(int);
  low = NULL;
  hi = NULL;
  ptr = NULL;

}

/*
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type ENUM.  Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char *_name, int _ID, BPatch_dataClass _type):
nullType(false), cblocks(NULL)
{
  ID = _ID;
  type_ = _type;
  // This is just for now XXX 
  size = sizeof(int);
  low = NULL;
  hi= NULL;
  ptr = NULL;
  if( _name)
    name = strdup(_name);
  else
    name = NULL;
}

/*
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type Structs, Unions, range(size),
 * and reference(void) and built-in type. These are the negative type
 * numbers defined in the gdb doc.  The type ID neg. type num.
 * Creates a type object representing a type
 * with the features given in the parameters.
 * A collection of the built-in types is created in for the image and
 * is gloabally accessible to all modules. This way it is only created once..
 */
BPatch_type::BPatch_type(const char *_name, int _ID, BPatch_dataClass _type,
			 int _size):
nullType(false), cblocks(NULL)
{
  ID = _ID;
  type_ = _type;

  if( _name)
    name = strdup(_name);
  else
    name = NULL;
  
  low = NULL;
  hi = NULL;
  ptr = NULL;
  type_ = _type;
  //typeCol = NULL;
  size = _size;

  if( _type == BPatch_dataScalar) {
    // Check that void type == void type
    if( _ID == _size ){

      low = NULL;
      hi = NULL;
      type_ = _type;
      ptr = NULL;
      //typeCol = NULL;
      size = 0;
    }
    else{
    size = _size;
    low = NULL;
    hi = NULL;
    type_ = _type;
    ptr = NULL;
    //typeCol = NULL;
    }
  }
  else{
    size = _size;
    low = NULL;
    hi = NULL;
    type_ = _type;
    ptr = NULL;
    //typeCol = NULL;
  }
}

/*
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type Pointers (and Internal).
 * Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char *_name, int _ID, BPatch_dataClass _type,
			 BPatch_type * _ptr):
nullType(false), cblocks(NULL)
{
    
  ID = _ID;
  if(_type == BPatch_dataScalar){  // could be a typedef for something
    if( _name)
      name = strdup(_name);
    else
      name = NULL;
    if(_ptr ){
      if(_ptr->ptr) // point to whatever it points to
	ptr = _ptr->ptr;
      else  // or just point to the oldType
	ptr = _ptr;
      type_ = _ptr->type_;
      size = _ptr->size;
      fieldList = _ptr->fieldList;
    } else{
      ptr = _ptr;
      size = sizeof(int);
      type_ = _type;
    }
	
    low = NULL;
    hi = NULL;
  } else{
    type_=_type;
    ptr = _ptr;
    if( _name)
      name = strdup(_name);
    else
      name = NULL;

    size = sizeof(void *);
    low = NULL;
    hi = NULL;
  }
}

/*
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type Range with lower and upper bound.
 * Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char *_name, int _ID, BPatch_dataClass _type,
			 const char * _low, const char * _hi):
nullType(false), cblocks(NULL)
{
  ID = _ID;
  type_ = _type;

  if( _name)
    name = strdup(_name);
  else
    name = NULL;
  
  name = strdup(_name);
  low = strdup(_low);
  hi = strdup(_hi);
  /* need to change for smaller types, maybe case statement
     needs to be arch. independent and there may be a lot of cases*/
  size = sizeof(int);
  ptr = NULL;
}

/*
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type Array.
 * Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char *_name, int _ID, BPatch_dataClass _type,
			 BPatch_type * _ptr, int _low, int _hi):
  nullType(false), cblocks(NULL)
{
  
  char temp[255];

  ID = _ID;
  type_ = _type;
  ptr = _ptr;

  if( _name)
    name = strdup(_name);
  else
    name = NULL;


  sprintf(temp, "%d", _low);
  low = strdup(temp);

  sprintf(temp, "%d", _hi);
  hi = strdup(temp);

  /* size = sizeof(artype)*(_hi+1)
     need to find out how big that size is first */
  size = ((ptr->getSize())*(_hi-_low+1));
}

/*
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type typedef--pre-existing type.
 * Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char * _name, int _ID, BPatch_type *  _ptr):
nullType(false), cblocks(NULL)
{
  if(_ptr){
    ID = _ID;
    size = _ptr->size;

    if( _name)
      name = strdup(_name);
    else
      name = NULL;

    if(_ptr->low)
      low = strdup(_ptr->low);
    else
      low = NULL;
    if(_ptr->hi)
      hi = strdup(_ptr->hi);
    else
      hi = NULL;
    type_ = _ptr->type_;
    if(_ptr->ptr)
      ptr = _ptr->ptr;
    else
      ptr = _ptr;
  }
  else{
    ID = _ID;
    size = sizeof(int);
    if( _name)
      name = strdup(_name);
    else
      name = NULL;
    hi = NULL;
    low = NULL;
    type_ = BPatch_dataScalar;
    ptr = _ptr;
  }
}

/*
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type Type Attributes.  These are types defined
 * by the built-in types and use the '@' as the type identifier.
 * Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char * _name, int _ID, BPatch_dataClass _type,
			 int _size, BPatch_type * _ptr):
nullType(false), cblocks(NULL)
{
  int b_size;

  b_size = _ptr->getSize();
  
  ID = _ID;

  if( b_size != _size) {
    sprintf(errorLine, "Built-in Type size %d and Stabs record size %d differ"
        " for [%d]%s: Overriding built-in type size!!",
        b_size, _size, _ID, _name);
    BPatch_reportError(BPatchWarning, 112, errorLine);
  }
  
  size = _size;

  if( _name)
    name = strdup(_name);
  else
    name = NULL;

  low = NULL;
  hi = NULL;
  type_ = _type;
  ptr = _ptr;
}

//---------------------------------------------------------------------
//  User defined BPatch_Type Constructors
//
//  These functions allow the user to create their own types and manipulate
//  these types.  The user defined types are stored with the other system
//  types.  They can be differentiated from system types by their type ID.  It
//  is less than -1000.  jdd 7/29/99


/* USER_DEFINED BPATCH_TYPE
 *  
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type ENUM.  Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char *_name, BPatch_dataClass _type):
nullType(false), cblocks(NULL)
{
  ID = USER_BPATCH_TYPE_ID;
  USER_BPATCH_TYPE_ID--;
  
  type_ = _type;
  // This is just for now XXX jdd 8/5/99, may need to be changed later
  size = sizeof(int);
  low = NULL;
  hi= NULL;
  ptr = NULL;
  if( _name)
    name = strdup(_name);
  else
    name = NULL;
}

/* USER_DEFINED BPATCH_TYPE
 * 
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type Structs, Unions, range(size),
 * and reference(void) and built-in type. These are the negative type
 * numbers defined in the gdb doc.  The type ID neg. type num.
 * Creates a type object representing a type
 * with the features given in the parameters.
 * A collection of the built-in types is created in for the image and
 * is gloabally accessible to all modules. This way it is only created once..
 */
BPatch_type::BPatch_type(const char *_name, BPatch_dataClass _type,
			 int _size):
nullType(false), cblocks(NULL)
{
  ID =USER_BPATCH_TYPE_ID;
  USER_BPATCH_TYPE_ID--;
  
  type_ = _type;

  if( _name)
    name = strdup(_name);
  else
    name = NULL;
  
  size = _size;
  low = NULL;
  hi = NULL;
  type_ = _type;
  if(_size == -1){
     ptr = (BPatch_type *)_size; // check this field before adding new struct, union component
                  // to see if the size needs to be computed.
  }
  else
    ptr = NULL;
}

/* USER_DEFINED BPATCH_TYPE
 * 
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type Pointers (and Internal).
 * Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char *_name, BPatch_type * _ptr, int size_):
nullType(false), cblocks(NULL)
{
  ID = USER_BPATCH_TYPE_ID;
  USER_BPATCH_TYPE_ID--;

  type_ = BPatch_dataPointer;
  size = size_;
  
  if( _name)
    name = strdup(_name);
  else
    name = NULL;

  if(_ptr ){
    if(_ptr->ptr) // point to whatever it points to
      ptr = _ptr->ptr;
    else  // or just point to the oldType
      ptr = _ptr;
    fieldList = _ptr->fieldList;
  }
  else{
    ptr = _ptr;
  }
	
  low = NULL;
  hi = NULL;
}

/* USER_DEFINED BPATCH_TYPE
 * 
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type Range with lower and upper bound.
 * Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char *_name, BPatch_dataClass _type,
			 const char * _low, const char * _hi):
nullType(false), cblocks(NULL)
{
  ID = USER_BPATCH_TYPE_ID;
  USER_BPATCH_TYPE_ID--;

  type_ = _type;

  if( _name)
    name = strdup(_name);
  else
    name = NULL;
  
  name = strdup(_name);
  low = strdup(_low);
  hi = strdup(_hi);
  /* need to change for smaller types, maybe case statement
     needs to be arch. independent and there may be a lot of cases*/
  size = sizeof(int);
  ptr = NULL;
}

/* USER_DEFINED BPATCH_TYPE
 * 
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type Array.
 * Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char *_name, BPatch_dataClass _type,
			 BPatch_type * _ptr, int _low, int _hi):
  nullType(false),
  cblocks(NULL)
{
  ID = USER_BPATCH_TYPE_ID;
  USER_BPATCH_TYPE_ID--;
  
  char temp[255];

  type_ = _type;
  ptr = _ptr;

  if( _name)
    name = strdup(_name);
  else
    name = NULL;


  sprintf(temp, "%d", _low);
  low = strdup(temp);

  sprintf(temp, "%d", _hi);
  hi = strdup(temp);

  /* size = sizeof(artype)*(_hi+1)
     need to find out how big that size is first */
  size = ((ptr->getSize())*(_hi+1));
}

/* USER_DEFINED BPATCH_TYPE
 * 
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type typedef--pre-existing type.
 * Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char * _name, BPatch_type *  _ptr):
nullType(false)
{
  if(_ptr){
    ID = USER_BPATCH_TYPE_ID;
    USER_BPATCH_TYPE_ID--;
 
    size = _ptr->size;

    if( _name)
      name = strdup(_name);
    else
      name = NULL;

    if(_ptr->low)
      low = strdup(_ptr->low);
    else
      low = NULL;
    if(_ptr->hi)
      hi = strdup(_ptr->hi);
    else
      hi = NULL;
    type_ = _ptr->type_;
    if(_ptr->ptr)
      ptr = _ptr->ptr;
    else
      ptr = _ptr;
  }
  else{
    ID = USER_BPATCH_TYPE_ID;
    USER_BPATCH_TYPE_ID--;
 
    size = sizeof(int);
    if( _name)
      name = strdup(_name);
    else
      name = NULL;
    hi = NULL;
    low = NULL;
    type_ = BPatch_dataScalar;
    ptr = _ptr;
  }
}

/* USER_DEFINED BPATCH_TYPE
 * 
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type Type Attributes.  These are types defined
 * by the built-in types and use the '@' as the type identifier.
 * Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char * _name, BPatch_dataClass _type,
			 int _size, BPatch_type * _ptr):
nullType(false)  
{
  int b_size;

  b_size = _ptr->getSize();
  ID = USER_BPATCH_TYPE_ID;
  USER_BPATCH_TYPE_ID--;

  if( b_size != _size) {
    sprintf(errorLine, "Built-in Type size %d and Stabs record size %d differ"
        " for [%d]%s: Overriding built-in type size!!",
        b_size, _size, ID, _name);
    BPatch_reportError(BPatchWarning, 112, errorLine);
  }
  
  size = _size;

  if( _name)
    name = strdup(_name);
  else
    name = NULL;

  low = NULL;
  hi = NULL;
  type_ = _type;
  ptr = _ptr;
}

void BPatch_type::beginCommonBlock()
{
    BPatch_Vector<BPatch_field*> emptyList;

    if (!cblocks) cblocks = new BPatch_Vector<BPatch_cblock *>;

    // null out field list
    fieldList = emptyList;
}

void BPatch_type::endCommonBlock(BPatch_function *func, void *baseAddr)
{
    int i, j;

    // create local variables in func's scope for each field of common block
    for (j=0; j < fieldList.size(); j++) {
	BPatch_localVar *locVar;
	locVar = new BPatch_localVar((char *) fieldList[j]->getName(), 
	    fieldList[j]->getType(), 0, 
	    fieldList[j]->getOffset()+(Address) baseAddr, 5, 
		false);
	func->localVariables->addLocalVar( locVar);
    }

    // look to see if the field list matches an existing block
    for (i=0; i < cblocks->size(); i++) {
	BPatch_cblock *curr = (*cblocks)[i];
	for (j=0; j < fieldList.size(); j++) {
	    if (strcmp(fieldList[j]->getName(),curr->fieldList[j]->getName()) ||
		(fieldList[j]->getOffset() !=curr->fieldList[j]->getOffset()) ||
		(fieldList[j]->getSize() != curr->fieldList[j]->getSize())) {
		break; // no match
	    }
	}
	if (j == fieldList.size() && (j == curr->fieldList.size())) {
	    // match
	    curr->functions.push_back(func);
	    return;
	}
    }

    // this one is unique
    BPatch_cblock *newBlock = new BPatch_cblock();
    newBlock->fieldList = fieldList;
    newBlock->functions.push_back(func);
    cblocks->push_back(newBlock);

    // create local variables in func's scope for each field of common block
    for (j=0; j < fieldList.size(); j++) {
	BPatch_localVar *locVar;
	locVar = new BPatch_localVar((char *) fieldList[j]->getName(), 
	    fieldList[j]->getType(), 0, 
	    fieldList[j]->getOffset()+(Address) baseAddr, 5, 
		false);
	func->localVariables->addLocalVar( locVar);
    }

    return;
}


/*
 * BPatch_type::isCompatible
 *
 * Returns true of the type is compatible with the other specified type, false
 * if it is not (if it breaks out of the case).
 *
 * oType	The other type to check for compatibility.
 */
bool BPatch_type::isCompatible(BPatch_type *otype)
{
  // simple compare.
  if (nullType || otype->nullType)
	return true;
  
  // name, ID and type are the same them it has to be the same BPatch_type
  if ((ID == otype->ID) && (type_ == otype->type_)) {
      if (name && otype->name && !strcmp(name,otype->name)) {
	  return true;
      }
  }

  if ((type_ == BPatch_dataUnknownType) || (otype->type_ == BPatch_dataUnknownType)) {
      BPatch_reportError(BPatchWarning, 112,
		       "One or more unknown BPatch_types");
      return true;
  }

  switch(type_){
  case BPatch_dataScalar:
      if (!strcmp(name,otype->name) && (size == otype->size)) {
	  return true;
      } else {
	  BPatch_reportError(BPatchWarning, 112, "scalar's not compatible");
	  return false;
      }
      break;

  case BPatchSymTypeRange:
      if (!(strcmp(name,otype->name))&& (ID == otype->ID)&&(size == otype->size))
          return true;
      if (!(strcmp(name,otype->name))&&(size == otype->size))
          return true;
      break;

  case BPatch_dataReference:
      if (!(strcmp(name,otype->name)) && (size == otype->size))
          return true;
      break;

  case BPatch_dataBuilt_inType:
      if (!(strcmp(name,otype->name))&&(size == otype->size))
          return true;
      break;

  case BPatch_dataUnknownType:
      // should be caught above
      assert(0);
      break;

  case BPatch_dataTypeAttrib:
    if (!(strcmp(name,otype->name))&&(size == otype->size))
        return true;
    break;

  case BPatch_dataPointer:
    {
      BPatch_type *pType1, *pType2;

      if (otype->type_ != BPatch_dataPointer) {
	  BPatch_reportError(BPatchWarning, 112, 
	      "Pointer and non-Pointer are not type compatible");
	  return false;
      } else {
	  // verify type that each one points to is compatible
	  pType1 = this->ptr;
	  pType2 = otype->ptr;
	  if (!pType1 || !pType2 || !pType1->isCompatible(pType2)) {
	      return false;
	  } else {
	      return true;
	  }
      }
    }
    break;

  case BPatch_dataFunction:
    if (!(strcmp(name,otype->name))&&(ID == otype->ID)) {
	  return true;
    } else {
	  BPatch_reportError(BPatchWarning, 112, 
	      "function call not compatible");
	  return false;
    }
    break;

  case BPatch_dataArray:
    {
      BPatch_type * arType1, * arType2;
      
      if (otype->type_ != BPatch_dataArray) {
	  BPatch_reportError(BPatchWarning, 112, 
	      "Array and non-array are not type compatible");
	  return false;
      }

      // verify that the number of elements is the same
      if ((atoi(this->hi) - atoi(this->low)) != (atoi(otype->hi) - atoi(otype->low))) {
	  char message[80];
	  sprintf(message, "Incompatible number of elements [%s..%s] vs. [%s..%s]",
	      this->low, this->hi, otype->low, otype->hi);
	  BPatch_reportError(BPatchWarning, 112, message);
	  return false;
      }

      // verify that elements of the array are compatible
      arType1 = this->ptr;
      arType2 = otype->ptr;
      if (!arType1 || !arType2 || !arType1->isCompatible(arType2)) {
	  // no need to report error, recursive call will
	  return false;
      } else {
	  return true;
      }
    }
    break;

  case BPatch_dataEnumerated:
    {
      if( !strcmp( name, otype->name) && (ID == otype->ID))
	return true;
      BPatch_Vector<BPatch_field *> * fields1 = this->getComponents();
      // BPatch_Vector<BPatch_field *> * fields2 = ((BPatch_type)otype).getComponents();
      BPatch_Vector<BPatch_field *> * fields2 = (BPatch_Vector<BPatch_field *> *) &(otype->fieldList);
      
      if( fields1->size() != fields2->size()) {
	BPatch_reportError(BPatchWarning, 112, "enumerated type mismatch ");
	return false;
      }
      
      //need to compare componment by component to verify compatibility
      for(int i=0;i<fields1->size();i++){
	BPatch_field * field1 = (*fields1)[i];
	BPatch_field * field2 = (*fields2)[i];
	if( (field1->getValue() != field2->getValue()) ||
	    (strcmp(field1->getName(), field2->getName())))
	  BPatch_reportError(BPatchWarning, 112, "enum element mismatch ");
	  return false;
      }
      // Everything matched so they are the same
      return true;
    }
    break;

  case BPatch_dataStructure:
  case BPatch_dataUnion:
    {
      if (!strcmp( name, otype->name) && (ID == otype->ID))
	  return true;
      BPatch_Vector<BPatch_field *> * fields1 = this->getComponents();
      // The line below does not work in linux.
      // BPatch_Vector<BPatch_field *> * fields2 = ((BPatch_type)otype).getComponents();
      BPatch_Vector<BPatch_field *> * fields2 = (BPatch_Vector<BPatch_field *> *) &(otype->fieldList);

      if (fields1->size() != fields2->size()) {
	  BPatch_reportError(BPatchWarning, 112, 
	      "struct/union numer of elements mismatch ");
	  return false;
      }
    
      //need to compare componment by component to verify compatibility
      for (int i=0;i<fields1->size();i++) {
	BPatch_field * field1 = (*fields1)[i];
	BPatch_field * field2 = (*fields2)[i];
	
	BPatch_type * ftype1 = (BPatch_type *)field1->getType();
	BPatch_type * ftype2 = (BPatch_type *)field2->getType();
	
	if(!(ftype1->isCompatible(ftype2))) {
	     BPatch_reportError(BPatchWarning, 112, 
	          "struct/union field type mismatch ");
	     return false;
	}
      }
      return true;
    }
    break;

  default:
    cerr<<"UNKNOWN TYPE, UNABLE TO COMPARE!!"<<endl;
  }

  char message[256];
  if (name && otype->name) {
      sprintf(message, "%s is not compatible with %s ", name, otype->name);
  } else {
      sprintf(message, "unknown type mismatch ");
  }
  BPatch_reportError(BPatchWarning, 112, message);
  return( false );
}


/*
 * void BPatch_type::addField
 * Creates field object and adds it to the list of fields for this
 * BPatch_type object.
 *     ENUMS
 */
void BPatch_type::addField(const char * _fieldname, BPatch_dataClass _typeDes,
			   int value)
{
  BPatch_field * newField;

  newField = new BPatch_field(_fieldname, _typeDes, value);

  // Add field to list of enum fields
  fieldList.push_back(newField);
}

/*
 * void BPatch_type::addField
 * Creates field object and adds it to the list of fields for this
 * BPatch_type object.
 *     ENUMS C++ - have visibility
 */
void BPatch_type::addField(const char * _fieldname, BPatch_dataClass _typeDes,
			   int value, BPatch_visibility _vis)
{
  BPatch_field * newField;

  newField = new BPatch_field(_fieldname, _typeDes, value, _vis);

  // Add field to list of enum fields
  fieldList.push_back(newField);
}

/*
 * void BPatch_type::addField
 * Creates field object and adds it to the list of fields for this
 * BPatch_type object.
 *     STRUCTS OR UNIONS
 */
void BPatch_type::addField(const char * _fieldname, BPatch_dataClass _typeDes,
			   BPatch_type *_type, int _offset, int _nsize)
{
  BPatch_field * newField;

  // API defined structs/union's size are defined on the fly.
  if (this->type_ == BPatch_dataStructure)
      this->size += _nsize;
  else if ( this->type_ == BPatch_dataUnion) {
      if( _nsize > size) size = _nsize;
  } else if (type_ == BPatch_dataCommon) {
      if (size < _offset + _nsize) size = _offset + _nsize; 
  } assert ( this->size > 0 );

  // Create Field for struct or union
  newField = new BPatch_field(_fieldname, _typeDes, _type, _offset, _nsize);

  // Add field to list of struct/union fields
  fieldList.push_back(newField);

}

/*
 * void BPatch_type::addField
 * Creates field object and adds it to the list of fields for this
 * BPatch_type object.
 *     STRUCTS OR UNIONS C++ have visibility
 */
void BPatch_type::addField(const char * _fieldname, BPatch_dataClass _typeDes,
			   BPatch_type *_type, int _offset, int _size,
			   BPatch_visibility _vis)
{
  BPatch_field * newField;

  // Create Field for struct or union
  newField = new BPatch_field(_fieldname, _typeDes, _type, _offset, _size,
			      _vis);

  // Add field to list of struct/union fields
  fieldList.push_back(newField);

}

/*
 * BPatch_field::BPatch_field
 *
 * Constructor for BPatch_field.  Creates a field object for 
 * an enumerated type.
 * type = offset = size = 0;
 */
BPatch_field::BPatch_field(const char * fName, BPatch_dataClass _typeDes,
			   int evalue)
{
  typeDes = _typeDes;
  value = evalue;
  fieldname = strdup(fName);

  _type = NULL;
  offset = size = 0;
  vis = BPatch_visUnknown;
  // printf("adding field %s\n", fName);

}

/*
 * BPatch_field::BPatch_field
 *
 * Constructor for BPatch_field.  Creates a field object for 
 * an enumerated type with a C++ visibility value
 * type = offset = size = 0;
 */
BPatch_field::BPatch_field(const char * fName, BPatch_dataClass _typeDes,
			   int evalue, BPatch_visibility _vis)
{
  typeDes = _typeDes;
  value = evalue;
  fieldname = strdup(fName);

  _type = NULL;
  offset = size = 0;
  vis = _vis;
  // printf("adding field %s\n", fName);

}

/*
 * BPatch_field::BPatch_field
 *
 * Constructor for BPatch_field.  Creates a field object for 
 * an struct and union types.
 * value= 0;
 */
BPatch_field::BPatch_field(const char * fName, BPatch_dataClass _typeDes,
			   BPatch_type *suType, int suOffset, int suSize)
{
  
  typeDes = _typeDes;
  _type = suType;
  offset = suOffset;
  fieldname = strdup(fName);
  size = suSize;
  
  value = 0;
  vis = BPatch_visUnknown;
}

/*
 * BPatch_field::BPatch_field
 *
 * Constructor for BPatch_field.  Creates a field object for 
 * a struct and union types for C++ fields that have visibility.
 * value= 0;
 */
BPatch_field::BPatch_field(const char * fName, BPatch_dataClass _typeDes,
			   BPatch_type *suType, int suOffset, int suSize,
			   BPatch_visibility _vis)
{
  
  typeDes = _typeDes;
  _type = suType;
  offset = suOffset;
  fieldname = strdup(fName);
  size = suSize;
  
  value = 0;
  vis = _vis;
}


/**************************************************************************
 * BPatch_localVar
 *************************************************************************/
/*
 * BPatch_localVar Constructor
 *
 */
BPatch_localVar::BPatch_localVar(char * _name,  BPatch_type * _type,
			     int _lineNum,int _frameOffset, int _sc, bool fr)
{
  if( _name)
    name = strdup(_name);
  else
    name = NULL;
  type = _type;
  lineNum = _lineNum;
  frameOffset = _frameOffset;
  frameRelative = fr;
  storageClass = _sc; //Only for COFF format. Default value is scAbs
}


/*
 * BPatch_localVar destructor
 *
 */
BPatch_localVar::~BPatch_localVar()
{

  //XXX jdd 5/25/99 More to do later

}

