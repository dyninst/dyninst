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

#include <stdio.h>

#define BPATCH_FILE


#include "util.h"
#include "BPatch_Vector.h"
#include "BPatch_typePrivate.h"
#include "BPatch_collections.h"
#include "showerror.h"
#include "BPatch.h"

#ifdef i386_unknown_nt4_0
#define snprintf _snprintf
#endif

static int findIntrensicType(const char *name);

// This is the ID that is decremented for each type a user defines. It is
// Global so that every type that the user defines has a unique ID.
// jdd 7/29/99
int BPatch_type::USER_BPATCH_TYPE_ID = -1000;


/* These are the wrappers for constructing a type.  Since we can create
   types six ways to Sunday, let's do them all in one centralized place. */


BPatch_type *BPatch_type::createFake(const char *_name) {
   // Creating a fake type without a name is just silly
   assert(_name != NULL);

   BPatch_type *t = new BPatch_type(_name);
   t->type_ = BPatch_dataNullType;

   return t;
}

/*
 * ENUM
 */

BPatch_typeEnum::BPatch_typeEnum(int _ID, const char *_name)
   : BPatch_fieldListType(_name, _ID, BPatch_dataUnion) {
   size = sizeof(int);
}

BPatch_typeEnum::BPatch_typeEnum(const char *_name)
   : BPatch_fieldListType(_name, USER_BPATCH_TYPE_ID--, BPatch_dataUnion) {
   size = sizeof(int);
}

bool BPatch_typeEnum::isCompatible(const BPatch_type *otype) const {
   const BPatch_typeTypedef *otypedef = dynamic_cast<const BPatch_typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType());

   const BPatch_typeEnum *oEnumtype = dynamic_cast<const BPatch_typeEnum *>(otype);

   if (oEnumtype == NULL)
      return false;
      
   if( name && oEnumtype->name && !strcmp( name, oEnumtype->name) && (ID == oEnumtype->ID))
      return true;
   
   const BPatch_Vector<BPatch_field *> * fields1 = this->getComponents();
   const BPatch_Vector<BPatch_field *> * fields2 = oEnumtype->getComponents();
   //BPatch_Vector<BPatch_field *> * fields2 = &oEnumtype->fieldList;
   
   if( fields1->size() != fields2->size()) {
      BPatch_reportError(BPatchWarning, 112, "enumerated type mismatch ");
      return false;
   }
   
   //need to compare componment by component to verify compatibility
   for(unsigned int i=0;i<fields1->size();i++){
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

/* 
 * POINTER
 */

BPatch_typePointer::BPatch_typePointer(int _ID, BPatch_type *_ptr, const char *_name) 
   : BPatch_type(_name, _ID, BPatch_dataPointer) {
   assert(_ptr != NULL);
   size = sizeof(void *);
   ptr = _ptr;
   _ptr->incrRefCount();

   if (_name == NULL && _ptr->getName() != NULL) {
      char buf[1024];
      snprintf(buf,1023,"%s *",_ptr->getName());
      name = strdup(buf);
   }
}

BPatch_typePointer::BPatch_typePointer(BPatch_type *_ptr, const char *_name) 
   : BPatch_type(_name, USER_BPATCH_TYPE_ID--, BPatch_dataPointer) {
   assert(_ptr != NULL);
   size = sizeof(void *);
   ptr = _ptr;

   if (_name == NULL && _ptr->getName() != NULL) {
      char buf[1024];
      snprintf(buf,1023,"%s *",_ptr->getName());
      name = strdup(buf);
   }
}

bool BPatch_typePointer::isCompatible(const BPatch_type *otype) const {
   const BPatch_typeTypedef *otypedef = dynamic_cast<const BPatch_typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType());

   const BPatch_typePointer *oPointertype = dynamic_cast<const BPatch_typePointer *>(otype);

   if (oPointertype == NULL) {
      BPatch_reportError(BPatchWarning, 112, 
                         "Pointer and non-Pointer are not type compatible");
      return false;
   }
   // verify type that each one points to is compatible
   return ptr->isCompatible(oPointertype->ptr);
}

void BPatch_typePointer::fixupUnknowns(BPatch_module *module) {
   if (ptr->getDataClass() == BPatch_dataUnknownType)
      ptr = module->getModuleTypes()->findType(ptr->getID());
}

/*
 * FUNCTION
 */

BPatch_typeFunction::BPatch_typeFunction(int _ID, BPatch_type *_retType, const char *_name)
   : BPatch_fieldListType(_name, _ID, BPatch_dataFunction), retType(_retType) {
   size = sizeof(void *);
   retType->incrRefCount();
}

bool BPatch_typeFunction::isCompatible(const BPatch_type *otype) const {
   const BPatch_typeTypedef *otypedef = dynamic_cast<const BPatch_typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType());

   const BPatch_typeFunction *oFunctiontype = dynamic_cast<const BPatch_typeFunction *>(otype);

   if (oFunctiontype == NULL)
      return false;

   if (retType != oFunctiontype->retType)
      return false;

   const BPatch_Vector<BPatch_field *> * fields1 = this->getComponents();
   const BPatch_Vector<BPatch_field *> * fields2 = oFunctiontype->getComponents();
   //const BPatch_Vector<BPatch_field *> * fields2 = (BPatch_Vector<BPatch_field *> *) &(otype->fieldList);
   
   if (fields1->size() != fields2->size()) {
      BPatch_reportError(BPatchWarning, 112, 
                         "function number of params mismatch ");
      return false;
   }
    
   //need to compare componment by component to verify compatibility
   for (unsigned int i=0;i<fields1->size();i++) {
      BPatch_field * field1 = (*fields1)[i];
      BPatch_field * field2 = (*fields2)[i];
      
      BPatch_type * ftype1 = (BPatch_type *)field1->getType();
      BPatch_type * ftype2 = (BPatch_type *)field2->getType();
      
      if(!(ftype1->isCompatible(ftype2))) {
         BPatch_reportError(BPatchWarning, 112, 
                            "function param type mismatch ");
         return false;
      }
   }
   return true;
}   

void BPatch_typeFunction::fixupUnknowns(BPatch_module *module) {
   if (retType->getDataClass() == BPatch_dataUnknownType)
      retType = module->getModuleTypes()->findType(retType->getID());

   for (unsigned int i = 0; i < fieldList.size(); i++)
      fieldList[i]->fixupUnknown(module);
}

/*
 * RANGE
 */

BPatch_typeRange::BPatch_typeRange(int _ID, int _size, const char *_low, const char *_hi, const char *_name)
   : BPatch_rangedType(_name, _ID, BPatchSymTypeRange, _size, _low, _hi) 
{
}

bool BPatch_typeRange::isCompatible(const BPatch_type *otype) const {
   const BPatch_typeTypedef *otypedef = dynamic_cast<const BPatch_typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType());

   const BPatch_typeRange *oRangetype = dynamic_cast<const BPatch_typeRange *>(otype);

   if (oRangetype == NULL)
      return false;

   return getSize() == oRangetype->getSize();
}

/*
 * ARRAY
 */

BPatch_typeArray::BPatch_typeArray(int _ID,
                                   BPatch_type *_base,
                                   int _low,
                                   int _hi,
                                   const char *_name,
                                   unsigned int _sizeHint)
   : BPatch_rangedType(_name, _ID, BPatch_dataArray, 0, _low, _hi), arrayElem(_base), sizeHint(_sizeHint) {
   assert(_base != NULL);
   arrayElem->incrRefCount();
}

BPatch_typeArray::BPatch_typeArray(BPatch_type *_base,
                                   int _low,
                                   int _hi,
                                   const char *_name)
   : BPatch_rangedType(_name, USER_BPATCH_TYPE_ID--, BPatch_dataArray, 0, _low, _hi), arrayElem(_base), sizeHint(0) {
   assert(_base != NULL);
   arrayElem->incrRefCount();
}

bool BPatch_typeArray::operator==(const BPatch_type &otype) const {
   try {
      const BPatch_typeArray &oArraytype = dynamic_cast<const BPatch_typeArray &>(otype);
      return (BPatch_rangedType::operator==(otype) && 
              (*arrayElem)==*oArraytype.arrayElem);
   } catch (...) {
      return false;
   }
}

void BPatch_typeArray::merge(BPatch_type *other) {
   // There are wierd cases where we may define an array with an element
   // that is a forward reference
   
   BPatch_typeArray *otherarray = dynamic_cast<BPatch_typeArray *>(other);

   if ( otherarray == NULL || this->ID != otherarray->ID || 
        this->arrayElem->getDataClass() != BPatch_dataUnknownType) {
      bperr( "Ignoring attempt to merge dissimilar types.\n" );
      return;
   }

   arrayElem->decrRefCount();
   otherarray->arrayElem->incrRefCount();
   arrayElem = otherarray->arrayElem;
}

void BPatch_typeArray::updateSize() {
   unsigned int elemSize;

   if (sizeHint)
      elemSize = sizeHint;
   else
      elemSize = arrayElem->getSize();
   if (atoi(hi))
      size = (atoi(hi) - atoi(low) + 1) * elemSize;
   else
      size = elemSize;   
}

bool BPatch_typeArray::isCompatible(const BPatch_type *otype) const {
   const BPatch_typeTypedef *otypedef = dynamic_cast<const BPatch_typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType());

   const BPatch_typeArray *oArraytype = dynamic_cast<const BPatch_typeArray *>(otype);

   if (oArraytype == NULL) {
      BPatch_reportError(BPatchWarning, 112, 
                         "Array and non-array are not type compatible");
      return false;      
   }
   unsigned int ec1, ec2;

   ec1 = atoi(hi) - atoi(low) + 1;
   ec2 = atoi(oArraytype->hi) - atoi(oArraytype->low) + 1;
   if (ec1 != ec2) {
      char message[80];
      sprintf(message, "Incompatible number of elements [%s..%s] vs. [%s..%s]",
	      this->low, this->hi, oArraytype->low, oArraytype->hi);
      BPatch_reportError(BPatchWarning, 112, message);
      return false;
   }
   return arrayElem->isCompatible(oArraytype->arrayElem);
}

void BPatch_typeArray::fixupUnknowns(BPatch_module *module) {
   if (arrayElem->getDataClass() == BPatch_dataUnknownType)
      arrayElem = module->getModuleTypes()->findType(arrayElem->getID());
}

/*
 * STRUCT
 */

BPatch_typeStruct::BPatch_typeStruct(int _ID, const char *_name) 
   : BPatch_fieldListType(_name, _ID, BPatch_dataStructure) {}

BPatch_typeStruct::BPatch_typeStruct(const char *_name) 
   : BPatch_fieldListType(_name, USER_BPATCH_TYPE_ID--, BPatch_dataStructure) {}

void BPatch_typeStruct::merge(BPatch_type *other) {
   // Merging is only for forward references
   assert(!fieldList.size());

   BPatch_typeStruct *otherstruct = dynamic_cast<BPatch_typeStruct *>(other);

   if( otherstruct == NULL || this->ID != otherstruct->ID) {
      bperr( "Ignoring attempt to merge dissimilar types.\n" );
      return;
   }

   if (otherstruct->name != NULL)
      name = strdup(otherstruct->name);
   size = otherstruct->size;

   fieldList = otherstruct->fieldList;

   if (otherstruct->derivedFieldList) {
      derivedFieldList = new BPatch_Vector<BPatch_field *>;
      *derivedFieldList = *otherstruct->derivedFieldList;
   }
}

void BPatch_typeStruct::updateSize() {
   size = 0;
   for (unsigned int i = 0; i < fieldList.size(); i++) 
      size += fieldList[i]->getType()->getSize();
}

bool BPatch_typeStruct::isCompatible(const BPatch_type *otype) const {
   const BPatch_typeTypedef *otypedef = dynamic_cast<const BPatch_typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType());

   const BPatch_typeStruct *oStructtype = dynamic_cast<const BPatch_typeStruct *>(otype);

   if (oStructtype == NULL)
      return false;

   const BPatch_Vector<BPatch_field *> * fields1 = this->getComponents();
   const BPatch_Vector<BPatch_field *> * fields2 = oStructtype->getComponents();
   //const BPatch_Vector<BPatch_field *> * fields2 = (BPatch_Vector<BPatch_field *> *) &(otype->fieldList);
   
   if (fields1->size() != fields2->size()) {
      BPatch_reportError(BPatchWarning, 112, 
                         "struct/union numer of elements mismatch ");
      return false;
   }
    
   //need to compare componment by component to verify compatibility
   for (unsigned int i=0;i<fields1->size();i++) {
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

void BPatch_typeStruct::fixupUnknowns(BPatch_module *module) {
   for (unsigned int i = 0; i < fieldList.size(); i++)
      fieldList[i]->fixupUnknown(module);
}

/*
 * UNION
 */

BPatch_typeUnion::BPatch_typeUnion(int _ID, const char *_name) 
   : BPatch_fieldListType(_name, _ID, BPatch_dataUnion) {}

BPatch_typeUnion::BPatch_typeUnion(const char *_name)
   : BPatch_fieldListType(_name, USER_BPATCH_TYPE_ID--, BPatch_dataUnion) {}

void BPatch_typeUnion::merge(BPatch_type *other) {
   // Merging is only for forward references
   assert(!fieldList.size());

   BPatch_typeUnion *otherunion = dynamic_cast<BPatch_typeUnion *>(other);

   if( otherunion == NULL || this->ID != otherunion->ID) {
      bperr( "Ignoring attempt to merge dissimilar types.\n" );
      return;
   }

   if (otherunion->name != NULL)
      name = strdup(otherunion->name);
   size = otherunion->size;

   fieldList = otherunion->fieldList;

   if (otherunion->derivedFieldList) {
      derivedFieldList = new BPatch_Vector<BPatch_field *>;
      *derivedFieldList = *otherunion->derivedFieldList;
   }
}

void BPatch_typeUnion::updateSize() {
   size = 0;
   for (unsigned int i = 0; i < fieldList.size(); i++) {
      if (fieldList[i]->getType()->getSize() > size)
         size = fieldList[i]->getType()->getSize();
   }
}

bool BPatch_typeUnion::isCompatible(const BPatch_type *otype) const {
   const BPatch_typeTypedef *otypedef = dynamic_cast<const BPatch_typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType());

   const BPatch_typeUnion *oUniontype = dynamic_cast<const BPatch_typeUnion *>(otype);

   if (oUniontype == NULL)
      return false;

   const BPatch_Vector<BPatch_field *> * fields1 = this->getComponents();
   const BPatch_Vector<BPatch_field *> * fields2 = oUniontype->getComponents();
   //const BPatch_Vector<BPatch_field *> * fields2 = (BPatch_Vector<BPatch_field *> *) &(otype->fieldList);
   
   if (fields1->size() != fields2->size()) {
      BPatch_reportError(BPatchWarning, 112, 
                         "struct/union numer of elements mismatch ");
      return false;
   }
    
   //need to compare componment by component to verify compatibility
   for (unsigned int i=0;i<fields1->size();i++) {
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

void BPatch_typeUnion::fixupUnknowns(BPatch_module *module) {
   for (unsigned int i = 0; i < fieldList.size(); i++)
      fieldList[i]->fixupUnknown(module);
}

/*
 * SCALAR
 */

BPatch_typeScalar::BPatch_typeScalar(int _ID, unsigned int _size, const char *_name) 
   : BPatch_type(_name, _ID, BPatch_dataScalar) {
   size = _size;
}

BPatch_typeScalar::BPatch_typeScalar(unsigned int _size, const char *_name) 
   : BPatch_type(_name, USER_BPATCH_TYPE_ID--, BPatch_dataScalar) {
   size = _size;
}

bool BPatch_typeScalar::isCompatible(const BPatch_type *otype) const {
   bool ret = false;
   const BPatch_typeTypedef *otypedef = dynamic_cast<const BPatch_typeTypedef *>(otype);
   if (otypedef != NULL)  {
      ret =  isCompatible(otypedef->getConstituentType());
      return ret;
   }

   const BPatch_typeScalar *oScalartype = dynamic_cast<const BPatch_typeScalar *>(otype);
   if (oScalartype == NULL) {
      //  Check to see if we have a range type, which can be compatible.
      const BPatch_typeRange *oRangeType = dynamic_cast<const BPatch_typeRange *>(otype);
      if (oRangeType != NULL) {
        if ( name == NULL || oRangeType->getName() == NULL)
           return size == oRangeType->getSize();
        else if (!strcmp(name, oRangeType->getName()))
           return size == oRangeType->getSize();
        else if (size == oRangeType->getSize()) {
          int t1 = findIntrensicType(name);
          int t2 = findIntrensicType(oRangeType->getName());
          if (t1 & t2 & (t1 == t2)) {
            return true;
          }
        }

      }
      return false;
   }

   if ( name == NULL || oScalartype->name == NULL)
      return size == oScalartype->size;
   else if (!strcmp(name, oScalartype->name))
      return size == oScalartype->size;
   else if (size == oScalartype->size) {
      int t1 = findIntrensicType(name);
      int t2 = findIntrensicType(oScalartype->name);
      if (t1 & t2 & (t1 == t2)) {
         return true;
      }
   }
   return false;
}

/* 
 * COMMON BLOCK
 */

BPatch_typeCommon::BPatch_typeCommon(int _ID, const char *_name) 
   : BPatch_fieldListType(_name, _ID, BPatch_dataCommon) {}

BPatch_typeCommon::BPatch_typeCommon(const char *_name) 
   : BPatch_fieldListType(_name, USER_BPATCH_TYPE_ID--, BPatch_dataCommon) {}

void BPatch_typeCommon::beginCommonBlock() {
    BPatch_Vector<BPatch_field*> emptyList;

    // null out field list
    fieldList = emptyList;
}

void BPatch_typeCommon::endCommonBlock(BPatch_function *func, void *baseAddr) {
    unsigned int i, j;

    // create local variables in func's scope for each field of common block
    for (j=0; j < fieldList.size(); j++) {
	BPatch_localVar *locVar;
	locVar = new BPatch_localVar((char *) fieldList[j]->getName(), 
				     fieldList[j]->getType(), 0, 
				     fieldList[j]->getOffset()+(Address) baseAddr,
				     -1, BPatch_storageAddr);
	func->localVariables->addLocalVar( locVar);
    }

    // look to see if the field list matches an existing block
    for (i=0; i < cblocks.size(); i++) {
	BPatch_cblock *curr = cblocks[i];
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
    cblocks.push_back(newBlock);
}

void BPatch_typeCommon::fixupUnknowns(BPatch_module *module) {
   for (unsigned int i = 0; i < cblocks.size(); i++)
      cblocks[i]->fixupUnknowns(module);   
}

/*
 * TYPEDEF
 */

BPatch_typeTypedef::BPatch_typeTypedef(int _ID, BPatch_type *_base, const char *_name, unsigned int _sizeHint) 
   : BPatch_type(_name, _ID, BPatch_dataTypeDefine) {
   assert(_base != NULL);
   base = _base;
   sizeHint = _sizeHint / 8;
   size = 0;
   base->incrRefCount();
}

BPatch_typeTypedef::BPatch_typeTypedef(BPatch_type *_base, const char *_name) 
   : BPatch_type(_name, USER_BPATCH_TYPE_ID--, BPatch_dataTypeDefine) {
   // The whole point of a typedef is to rename a type
   assert(_base != NULL && _name != NULL);
   base = _base;
   sizeHint = 0;
   size = 0;
   base->incrRefCount();
}

bool BPatch_typeTypedef::operator==(const BPatch_type &otype) const {
   try {
      const BPatch_typeTypedef &oDeftype = dynamic_cast<const BPatch_typeTypedef &>(otype);
      return base==oDeftype.base;
   } catch (...) {
      return false;
   }
}

BPatch_Vector<BPatch_field *> *BPatch_typeTypedef::getComponents() const {
   BPatch_fieldListType *otype = dynamic_cast<BPatch_fieldListType *>(base);
   if (otype == NULL)
      return NULL;
   else
      return otype->getComponents();
}

const char *BPatch_typeTypedef::getLow() const {
   BPatch_rangedType *otype = dynamic_cast<BPatch_rangedType *>(base);
   if (otype == NULL)
      return NULL;
   else
      return otype->getLow();
}

const char *BPatch_typeTypedef::getHigh() const {
   BPatch_rangedType *otype = dynamic_cast<BPatch_rangedType *>(base);
   if (otype == NULL)
      return NULL;
   else
      return otype->getHigh();
}

void BPatch_typeTypedef::fixupUnknowns(BPatch_module *module) {
   if (base->getDataClass() == BPatch_dataUnknownType)
      base = module->getModuleTypes()->findType(base->getID());   
}

/*
 * REFERENCE
 */

BPatch_typeRef::BPatch_typeRef(int _ID, BPatch_type *_refType, const char *_name)
   : BPatch_type(_name, _ID, BPatch_dataReference), BPatch_fieldListInterface(), BPatch_rangedInterface(), refType(_refType) {
   refType->incrRefCount();
}

bool BPatch_typeRef::operator==(const BPatch_type &otype) const {
   try {
      const BPatch_typeRef &oReftype = dynamic_cast<const BPatch_typeRef &>(otype);
      return refType==oReftype.refType;
   } catch (...) {
      return false;
   }
}

bool BPatch_typeRef::isCompatible(const BPatch_type *otype) const {
   const BPatch_typeTypedef *otypedef = dynamic_cast<const BPatch_typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType());

   const BPatch_typeRef *oReftype = dynamic_cast<const BPatch_typeRef *>(otype);
   if (oReftype == NULL) {
      return false;
   }
   return refType->isCompatible(const_cast<BPatch_type *>(oReftype->refType));
}   

BPatch_Vector<BPatch_field *> *BPatch_typeRef::getComponents() const {
   BPatch_fieldListType *otype = dynamic_cast<BPatch_fieldListType *>(refType);
   if (otype == NULL)
      return NULL;
   else
      return otype->getComponents();
}

const char *BPatch_typeRef::getLow() const {
   BPatch_rangedType *otype = dynamic_cast<BPatch_rangedType *>(refType);
   if (otype == NULL)
      return NULL;
   else
      return otype->getLow();
}

const char *BPatch_typeRef::getHigh() const {
   BPatch_rangedType *otype = dynamic_cast<BPatch_rangedType *>(refType);
   if (otype == NULL)
      return NULL;
   else
      return otype->getHigh();
}

void BPatch_typeRef::fixupUnknowns(BPatch_module *module) {
   if (refType->getDataClass() == BPatch_dataUnknownType)
      refType = module->getModuleTypes()->findType(refType->getID());
}

/*
 * BPatch_type::BPatch_type
 *
 * EMPTY Constructor for BPatch_type.  
 * 
 */
BPatch_type::BPatch_type(const char *_name, int _ID, BPatch_dataClass _type) :
   ID(_ID), size(sizeof(/*long*/ int)), type_(_type), refCount(1)
{
  if (_name != NULL)
     name = strdup(_name);
  else
     name = NULL;
}

/* BPatch_type destructor
 * Basic destructor for proper memory management.
 */
BPatch_type::~BPatch_type()
{
    if (name) free(name);
}

bool BPatch_type::operator==(const BPatch_type &otype) const {
   return (ID==otype.ID && size==otype.size && type_ == otype.type_ &&
           (name == otype.name || 
            (name != NULL && otype.name != NULL && !strcmp(name, otype.name))));
}

/* 
 * Subclasses of BPatch_type, with interfaces
 */

/*
 * FIELD LIST
 */

BPatch_fieldListType::BPatch_fieldListType(const char *_name, int _ID, BPatch_dataClass _class) 
   : BPatch_type(_name, _ID, _class), derivedFieldList(NULL) {}

BPatch_fieldListType::~BPatch_fieldListType() {
   if (derivedFieldList != NULL)
      delete derivedFieldList;
}

bool BPatch_fieldListType::operator==(const BPatch_type &otype) const {
   try {
      const BPatch_fieldListType &oFieldtype = dynamic_cast<const BPatch_fieldListType &>(otype);
      if (fieldList.size() != oFieldtype.fieldList.size())
         return false;
      for (unsigned int i = 0; i < fieldList.size(); i++) {
         if (fieldList[i] != oFieldtype.fieldList[i])
            return false;
      }
      return BPatch_type::operator==(otype);
   } catch (...) {
      return false;
   }
}

BPatch_Vector<BPatch_field *> * BPatch_fieldListType::getComponents() const {
   if (derivedFieldList == NULL)
      const_cast<BPatch_fieldListType *>(this)->fixupComponents();

   return derivedFieldList;
}

void BPatch_fieldListType::fixupComponents() {

   // bperr "Getting the %d components of '%s' at 0x%x\n", fieldList.size(), getName(), this );
   /* Iterate over the field list.  Recursively (replace)
      '{superclass}' with the superclass's non-private fields. */
   derivedFieldList = new BPatch_Vector< BPatch_field * >();
   for( unsigned int i = 0; i < fieldList.size(); i++ ) {
      BPatch_field * currentField = fieldList[i];
      // bperr( "Considering field '%s'\n", currentField->getName() );
      if( strcmp( currentField->getName(), "{superclass}" ) == 0 ) {
         /* Note that this is a recursive call.  However, because
            the class-graph is acyclic (Stroustrup SpecialEd pg 308),
            we're OK. */
         // bperr( "Found superclass '%s'...\n", currentField->getType()->getName() );
         BPatch_fieldListInterface *superclass = dynamic_cast<BPatch_fieldListInterface *>(currentField->getType());
         assert (superclass != NULL);
         const BPatch_Vector<BPatch_field *> * superClassFields = superclass->getComponents();
         // bperr( "Superclass has %d components.\n", superClassFields->size() );
         /* FIXME: do we also need to consider the visibility of the superclass itself? */
         /* FIXME: visibility can also be described on a per-name basis in the
            subclass.  We have now way to convey this information currently, but I'm not
            sure that it matters for our purposes... */
         for( unsigned int i = 0; i < superClassFields->size(); i++ ) {
            BPatch_field * currentSuperField = (*superClassFields)[i];
            // bperr( "Considering superfield '%s'\n", currentSuperField->getName() );
            if( currentSuperField->getVisibility() != BPatch_private ) {
               derivedFieldList->push_back( currentSuperField );
            }
         } /* end super-class iteration */
      } /* end if currentField is a superclass */
      else {
         derivedFieldList->push_back( currentField );
      }
   } /* end field iteration */
}

/*
 * RANGED
 */

BPatch_rangedType::BPatch_rangedType(const char *_name, int _ID, BPatch_dataClass _class, int _size, int _low, int _hi) 
   : BPatch_type(_name, _ID, _class) {
   char buf[16];

   sprintf(buf, "%d", _low);
   low = strdup(buf);
   sprintf(buf, "%d", _hi);
   hi = strdup(buf);

   size = _size;
}

BPatch_rangedType::BPatch_rangedType(const char *_name, int _ID, BPatch_dataClass _class, int _size, const char *_low, const char *_hi) 
   : BPatch_type(_name, _ID, _class) {

   low = strdup(_low);
   hi = strdup(_hi);

   size = _size;
}

BPatch_rangedType::~BPatch_rangedType() {
   if (low != NULL) free(low);
   if (hi != NULL) free(hi);
}

bool BPatch_rangedType::operator==(const BPatch_type &otype) const {
   try {
      const BPatch_rangedType &oRangedtype = dynamic_cast<const BPatch_rangedType &>(otype);
      return (!strcmp(low, oRangedtype.low) && !strcmp(hi, oRangedtype.hi) &&
              BPatch_type::operator==(otype));
   } catch (...) {
      return false;
   }
}

#ifdef IBM_BPATCH_COMPAT
char *BPatch_type::getName(char *buffer, int max) const
{
  if (!name) {
     strncpy(buffer, "bad type name", (max > strlen("bad_type_name")) ?
             (strlen("bad_type_name") +1) : max);
     char msg[256];
     sprintf(msg, "%s[%d]: bad type name!", __FILE__, __LINE__);
     BPatch_reportError(BPatchWarning, 112, msg);
     return buffer;
  }

  if (max > strlen(name)) {
    strcpy (buffer, name);
    return buffer;
  } else {
    strncpy (buffer, name, max-1)[max-1] = '\0';
  }
   return NULL;
}
#endif

//
// Define the type compatability among the intrensic types of the various
//     languages.  For example int in c is compatiable to integer*4 in Fortran.
//     Each equivelence class is given a unique number.
//
struct intrensicTypes_ {
    const char *name;
    int tid;
};

struct intrensicTypes_ intrensicTypes[] = {
    { "int",		1 },
    { "integer*4", 	1 },
    { "INTEGER*4", 	1 },
    { NULL,		0 },
};

static int findIntrensicType(const char *name)
{
    struct intrensicTypes_ *curr;

    for (curr = intrensicTypes; curr->name; curr++) {
	if (name && curr->name && !strcmp(name, curr->name)) {
	    return curr->tid;
	}
    }

    return 0;
}

/*
 * void BPatch_type::addField
 * Creates field object and adds it to the list of fields for this
 * BPatch_type object.
 *     ENUMS
 */
void BPatch_fieldListType::addField(const char * _fieldname, int value)
{
  BPatch_field * newField;

  newField = new BPatch_field(_fieldname, BPatch_dataScalar, value);

  // Add field to list of enum fields
  fieldList.push_back(newField);
}

/*
 * void BPatch_type::addField
 * Creates field object and adds it to the list of fields for this
 * BPatch_type object.
 *     ENUMS C++ - have visibility
 */
void BPatch_fieldListType::addField(const char * _fieldname, BPatch_dataClass _typeDes,
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
void BPatch_fieldListType::addField(const char * _fieldname, BPatch_dataClass _typeDes,
			   BPatch_type *_type, int _offset, int _nsize)
{
  BPatch_field * newField;
  
  /*
  if (!( this->size > 0 || ( (_typeDes == BPatch_dataMethod || _typeDes == BPatch_dataUnknownType ) && this->size >= 0) ) ) {
  	bperr( "Invalid size: this->size = %d, _nsize = %d", this->size, _nsize );
  	if( _typeDes == BPatch_dataMethod ) { bperr( " ... the field is a data method" ); }
  	if( _typeDes == BPatch_dataUnknownType ) { bperr( " ... the field is an unknown type" ); }
  	}
  */

  // Create Field for struct or union
  newField = new BPatch_field(_fieldname, _typeDes, _type, _offset, _nsize);

  // Add field to list of struct/union fields
  fieldList.push_back(newField);

  // API defined structs/union's size are defined on the fly.
  postFieldInsert(_offset, _nsize);
}

/*
 * void BPatch_type::addField
 * Creates field object and adds it to the list of fields for this
 * BPatch_type object.
 *     STRUCTS OR UNIONS C++ have visibility
 */
void BPatch_fieldListType::addField(const char * _fieldname, BPatch_dataClass _typeDes,
			   BPatch_type *_type, int _offset, int _size,
			   BPatch_visibility _vis)
{
  BPatch_field * newField;

  // Create Field for struct or union
  newField = new BPatch_field(_fieldname, _typeDes, _type, _offset, _size,
			      _vis);

  // Add field to list of struct/union fields
  fieldList.push_back(newField);

  // API defined structs/union's size are defined on the fly.
  postFieldInsert(_offset, _size);
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
  // bperr("adding field %s\n", fName);

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
  // bperr("adding field %s\n", fName);

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

  _type->incrRefCount();
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
  
  _type->incrRefCount();
  value = 0;
  vis = _vis;
}

void BPatch_field::copy(BPatch_field &oField) {
   typeDes = oField.typeDes;
   _type = oField._type;
   offset = oField.offset;
   if (oField.fieldname != NULL)
      fieldname = strdup(oField.fieldname);
   size = oField.size;
   value = oField.value;
   vis = oField.vis;

   if (_type != NULL)
      _type->incrRefCount();
}

BPatch_field::BPatch_field(BPatch_field &oField) {
   copy(oField);
}

BPatch_field &BPatch_field::operator=(BPatch_field &oField) {
   copy(oField);
   return *this;
}

BPatch_field::~BPatch_field() {
   if (_type != NULL) 
      _type->decrRefCount();
   if (fieldname != NULL)
      free(fieldname);
}

void BPatch_field::fixupUnknown(BPatch_module *module) {
   if (_type->getDataClass() == BPatch_dataUnknownType)
      _type = module->getModuleTypes()->findType(_type->getID());
}

/**************************************************************************
 * BPatch_localVar
 *************************************************************************/
/*
 * BPatch_localVar Constructor
 *
 */
BPatch_localVar::BPatch_localVar(const char * _name,  BPatch_type * _type,
				 int _lineNum, long _frameOffset, int _reg,
				 BPatch_storageClass _storageClass)
{
    name = ( _name ? strdup(_name) : NULL );
    type = _type;
    lineNum = _lineNum;
    frameOffset = _frameOffset;
    reg = _reg;
    storageClass = _storageClass;

    type->incrRefCount();
}

/*
 * BPatch_localVar destructor
 *
 */
BPatch_localVar::~BPatch_localVar()
{
    //XXX jdd 5/25/99 More to do later
    if (name) free(name);
    type->decrRefCount();
}

void BPatch_localVar::fixupUnknown(BPatch_module *module) {
   if (type->getDataClass() == BPatch_dataUnknownType)
      type = module->getModuleTypes()->findType(type->getID());
}

/**************************************************************************
 * BPatch_localVar
 *************************************************************************/

void BPatch_cblock::fixupUnknowns(BPatch_module *module) {
   for (unsigned int i = 0; i < fieldList.size(); i++) {
      fieldList[i]->fixupUnknown(module);
   }
}
