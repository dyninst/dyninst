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

#include "util.h"
#include "BPatch_Vector.h"
#include "BPatch_type.h"

/*
 * BPatch_type::BPatch_type
 *
 * EMPTY Constructor for BPatch_type.  
 * 
 */
BPatch_type::BPatch_type(){
  name = NULL;
  ID = 0;
  size = 0;
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
    nullType(_nullType)
{
    /* XXX More later. */
  if( _name)
    name = strdup(_name);
  else
    name = NULL;

}
/*
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type ENUM.  Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char *_name, int _ID, BPatch_dataClass _type)
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
			 int _size)
{

  ID = _ID;
  type_ = _type;

  if( _name)
    name = strdup(_name);
  else
    name = NULL;
  
  if( _type == BPatch_scalar) {
    // Check that void type == void type
    if( _ID == _size ){
      size = 0;
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
			 BPatch_type * _ptr)
{
    
  ID = _ID;
  if(_type == BPatch_scalar){  // could be a typedef for something
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
    }
    else{
      ptr = _ptr;
      size = sizeof(int);
      type_ = _type;
    }
	
    low = NULL;
    hi = NULL;
  }
  else{
    type_=_type;
    ptr = _ptr;
    if( _name)
      name = strdup(_name);
    else
      name = NULL;

    size = sizeof(int);
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
			 const char * _low, const char * _hi)
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
			 BPatch_type * _ptr, int _low, int _hi)
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
  size = ((ptr->getSize())*(_hi+1));
}
/*
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type typedef--pre-existing type.
 * Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char * _name, int _ID, BPatch_type *  _ptr)
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
    size = 0;
    if( _name)
      name = strdup(_name);
    else
      name = NULL;
    hi = NULL;
    low = NULL;
    type_ = BPatch_scalar;
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
			 int _size, BPatch_type * _ptr)
{
  int b_size;

  b_size = _ptr->getSize();
  
  ID = _ID;

  if( b_size != _size)
    printf(" Built-in Type size and stab record size are different, Overriding built-in type size!!\n");
  
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

/*
 * BPatch_type::isCompatible
 *
 * Returns true of the type is compatible with the other specified type, false
 * if it is not.
 *
 * oType	The other type to check for compatibility.
 */
bool BPatch_type::isCompatible(const BPatch_type &otype)
{
    if (nullType || otype.nullType)
	return true;

    // XXX Just compare names for now, we'll have to fix this later.
    if (name == otype.name) return true;
    else return false;
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
			   BPatch_type *_type, int _offset, int _size)
{
  BPatch_field * newField;

  // Create Field for struct or union
  newField = new BPatch_field(_fieldname, _typeDes, _type, _offset, _size);

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

  type = NULL;
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

  type = NULL;
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
  type = suType;
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
  type = suType;
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
				 int _lineNum,int _frameOffset)
{
  if( _name)
    name = strdup(_name);
  else
    name = NULL;
  type = _type;
  lineNum = _lineNum;
  frameOffset = _frameOffset;

}


/*
 * BPatch_localVar destructor
 *
 */
BPatch_localVar::~BPatch_localVar()
{

  //More to do later

}
