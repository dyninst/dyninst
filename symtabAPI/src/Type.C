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

#include "common/src/vgannotations.h"

#include <string.h>
			     
#include <stdio.h>

#include "symutil.h"
#include "Type.h"
#include "Symtab.h"
#include "Module.h"
#include "Collections.h"
#include "Function.h"

#include "Type-mem.h"
#include <iostream>
#include "concurrent.h"

#include <boost/atomic.hpp>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

//#include "collections.h"
//#include "debug.h" TODO: We want such behaviour. LATER!

static int findIntrensicType(std::string &name);

// This is the ID that is decremented for each type a user defines. It is
// Global so that every type that the user defines has a unique ID.
static boost::atomic<typeId_t> user_type_id(-10000);

static typeId_t getUniqueTypeId()
{
  return user_type_id.fetch_add(-1);
}

typeId_t Type::getUniqueTypeId()
{
  return ::getUniqueTypeId();
}

void Type::updateUniqueTypeId(typeId_t ID_)
{
  typeId_t val = user_type_id.load();
  while((ID_ < 0) && (val >= ID_))
  {
    user_type_id.compare_exchange_weak(val, val-1);
  }
}

/* These are the wrappers for constructing a type.  Since we can create
   types six ways to Sunday, let's do them all in one centralized place. */

Type::unique_ptr_Type Type::createFake(std::string name) 
{
   // Creating a fake type without a name is just silly
   assert(name != std::string(""));

   Type *t = new Type(name);
   t->type_ = dataNullType;

   return t;
}

// Memory size of the maximum-sized subclass of Type we can possibly have.
const std::size_t Type::max_size = std::max({
  sizeof(Type),
  sizeof(typeEnum),
  sizeof(typePointer),
  sizeof(typeFunction),
  sizeof(typeSubrange),
  sizeof(typeArray),
  sizeof(typeStruct),
  sizeof(typeUnion),
  sizeof(typeCommon),
  sizeof(typeScalar),
  sizeof(typeTypedef),
  sizeof(typeRef),
  sizeof(fieldListType),
  sizeof(rangedType),
  sizeof(derivedType),
});

Type::unique_ptr_Type Type::createPlaceholder(typeId_t ID, std::string name)
{
  void *mem = malloc(max_size);
  assert(mem);
  Type *placeholder_type = new(mem) Type(name, ID, dataUnknownType);
  return placeholder_type;
}

/*
 * Type::Type
 *
 * EMPTY Constructor for type.  
 * 
 */
Type::Type(std::string name, typeId_t ID, dataClass dataTyp) :
   ID_(ID), 
   name_(name), 
   size_(sizeof(int)), 
   type_(dataTyp), 
   updatingSize(false)
{
	if (!name.length()) 
		name = std::string("unnamed_") + std::string(dataClass2Str(type_));
}

Type::Type(std::string name, dataClass dataTyp) :
   ID_(getUniqueTypeId()),
   name_(name), 
   size_(sizeof(/*long*/ int)), 
   type_(dataTyp), 
   updatingSize(false)
{
	if (!name.length()) 
		name = std::string("unnamed_") + std::string(dataClass2Str(type_));
}

/* type destructor
 * Basic destructor for proper memory management.
 */
Type::~Type()
{
}

const char *Dyninst::SymtabAPI::dataClass2Str(dataClass dc)
{
   switch(dc) {
      CASE_RETURN_STR(dataEnum);
      CASE_RETURN_STR(dataPointer);
      CASE_RETURN_STR(dataFunction);
      CASE_RETURN_STR(dataSubrange);
      CASE_RETURN_STR(dataArray);
      CASE_RETURN_STR(dataStructure);
      CASE_RETURN_STR(dataUnion);
      CASE_RETURN_STR(dataCommon);
      CASE_RETURN_STR(dataScalar);
      CASE_RETURN_STR(dataTypedef);
      CASE_RETURN_STR(dataReference);
      CASE_RETURN_STR(dataUnknownType);
      CASE_RETURN_STR(dataNullType);
      CASE_RETURN_STR(dataTypeClass);
   };
   return "bad_data_class";
}

namespace Dyninst {
namespace SymtabAPI {

const char *visibility2Str(visibility_t v) 
{
   switch(v) {
      CASE_RETURN_STR(visPrivate);
      CASE_RETURN_STR(visProtected);
      CASE_RETURN_STR(visPublic);
      CASE_RETURN_STR(visUnknown);
   };
   return "bad_visibility";
}
}
}

bool Type::operator==(const Type &otype) const 
{
	return (ID_ == otype.ID_ && name_ == otype.name_ && size_== otype.size_ && type_ == otype.type_);
}

unsigned int Type::getSize()
{
	if (!size_) 
		const_cast<Type *>(this)->updateSize(); 
	return size_;
}

bool Type::setSize(unsigned int size)
{
	size_ = size;
	return true;
}

std::string &Type::getName()
{
    return name_;
}

bool Type::setName(std::string name)
{
    if (name.empty()) return false;
    name_ = std::move(name);
    return true;
}

typeId_t Type::getID() const
{
    return ID_;
}

dataClass Type::getDataClass() const
{
    return type_;
}

void Type::fixupUnknowns(Module *){
}

typeEnum *Type::getEnumType(){
    return dynamic_cast<typeEnum *>(this);
}

typePointer *Type::getPointerType(){
    return dynamic_cast<typePointer *>(this);
}
 
typeFunction *Type::getFunctionType(){
    return dynamic_cast<typeFunction *>(this);
}
 
typeSubrange *Type::getSubrangeType(){
    return dynamic_cast<typeSubrange *>(this);
}
	   
typeArray *Type::getArrayType(){
    return dynamic_cast<typeArray *>(this);
}

typeStruct *Type::getStructType(){
    return dynamic_cast<typeStruct *>(this);
}

typeUnion *Type::getUnionType(){
    return dynamic_cast<typeUnion *>(this);
}

typeScalar *Type::getScalarType(){
    return dynamic_cast<typeScalar *>(this);
}

typeCommon *Type::getCommonType(){
    return dynamic_cast<typeCommon *>(this);
}

typeTypedef *Type::getTypedefType(){
    return dynamic_cast<typeTypedef *>(this);
}
	
typeRef *Type::getRefType(){
    return dynamic_cast<typeRef *>(this);
}

std::string Type::specificType()
{
	if (getEnumType()) return std::string("typeEnum");
	if (getPointerType()) return std::string("typePointer");
	if (getFunctionType()) return std::string("typeFunction");
	if (getSubrangeType()) return std::string("typeSubrange");
	if (getArrayType()) return std::string("typeArray");
	if (getStructType()) return std::string("typeStruct");
	if (getUnionType()) return std::string("typeUnion");
	if (getScalarType()) return std::string("typeScalar");
	if (getCommonType()) return std::string("typeCommon");
	if (getTypedefType()) return std::string("typeTypedef");
	if (getRefType()) return std::string("typeRef");
	return std::string("badType");
}
bool Type::isCompatible(Type * /*oType*/)
{
   return true;
}

/*
 * ENUM
 */
typeEnum::typeEnum(int ID, std::string name)
    : Type(name, ID, dataEnum)
{
	size_ = sizeof(int);
}

typeEnum::typeEnum(std::string name)
   : Type(name, ::getUniqueTypeId(), dataEnum)
{
   size_ = sizeof(int);
}

typeEnum *typeEnum::create(std::string &name, dyn_c_vector< std::pair<std::string, int> *> &constants, Symtab *obj)
{
   typeEnum *typ = new typeEnum(name);
   for(unsigned i=0; i<constants.size();i++)
   	typ->addConstant(constants[i]->first, constants[i]->second);
    
    if(obj)
       obj->addType(typ);
    //obj->addType(typ); TODO: declare a static container if obj is NULL and add to it.
    //Symtab::noObjTypes->push_back(typ); ??
    return typ;	
}

typeEnum *typeEnum::create(std::string &name, dyn_c_vector<std::string> &constNames, Symtab *obj)
{
   typeEnum *typ = new typeEnum(name);
   for(unsigned i=0; i<constNames.size();i++)
   	typ->addConstant(constNames[i], i);
    if(obj)
       obj->addType(typ);
    //obj->addType(typ); TODO: declare a static container if obj is NULL and add to it.
    //Symtab::noObjTypes->push_back(typ); ??
    return typ;	
}	

dyn_c_vector<std::pair<std::string, int> > &typeEnum::getConstants()
{
   return consts;
}

bool typeEnum::addConstant(const std::string &constName, int value)
{
   consts.push_back(std::pair<std::string, int>(constName, value));
   return true;
}

bool typeEnum::isCompatible(Type *otype) 
{
   if((otype->getDataClass() == dataUnknownType) || (otype->getDataClass() == dataNullType))
       return true;
   typeTypedef *otypedef = dynamic_cast<typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType(Type::share));

   typeEnum *oEnumtype = dynamic_cast<typeEnum *>(otype);

   if (oEnumtype == NULL)
      return false;
      
   if ( (name_ != "") &&( oEnumtype->name_ != "") && (name_ == oEnumtype->name_) && (ID_ == oEnumtype->ID_))
      return true;
   
   const dyn_c_vector< std::pair<std::string, int> > &fields1 = this->getConstants();
   const dyn_c_vector< std::pair<std::string, int> > &fields2 = oEnumtype->getConstants();
   
   if ( fields1.size() != fields2.size()) 
   {
      //reportError(BPatchWarning, 112, "enumerated type mismatch ");
      return false;
   }
   
   //need to compare componment by component to verify compatibility

   for (unsigned int i=0;i<fields1.size();i++)
   {
      const std::pair<std::string, int> &field1 = fields1[i];
      const std::pair<std::string, int> &field2 = fields2[i];
      if ( (field1.second != field2.second) ||
          (field1.first != field2.first))
      {
         // reportError(BPatchWarning, 112, "enum element mismatch ");
   	 return false;
      } 
   }
   // Everything matched so they are the same
   return true;
}

/* 
 * POINTER
 */

typePointer::typePointer(int ID, boost::shared_ptr<Type> ptr, std::string name) 
   : derivedType(name, ID, 0, dataPointer) {
   size_ = sizeof(void *);
   if (ptr)
     setPtr(ptr);
}

typePointer::typePointer(boost::shared_ptr<Type> ptr, std::string name) 
   : derivedType(name, getUniqueTypeId(), 0, dataPointer) {
   size_ = sizeof(void *);
   if (ptr)
     setPtr(ptr);
}

typePointer *typePointer::create(std::string &name, boost::shared_ptr<Type> ptr, Symtab *obj)
{
   if(!ptr)
   	return NULL;
   typePointer *typ = new typePointer(ptr, name);

   if(obj)
   	obj->addType(typ);
   //obj->addType(typ); TODO: declare a static container if obj is NULL and add to it.
   //Symtab::noObjTypes->push_back(typ); ??
				   
   return typ;	
}

typePointer *typePointer::create(std::string &name, boost::shared_ptr<Type> ptr, int size, Symtab *obj)
{
   if(!ptr)
   	return NULL;
   typePointer *typ = new typePointer(ptr, name);
   typ->setSize(size);

   if(obj)
   	obj->addType(typ);
   //obj->addType(typ); TODO: declare a static container if obj is NULL and add to it.
   //Symtab::noObjTypes->push_back(typ); ??
				   
   return typ;	
}

bool typePointer::setPtr(boost::shared_ptr<Type> ptr) { 
  assert(ptr);
  baseType_ = ptr; 

  if (name_ == "" && ptr->getName() != "") {
     name_ = std::string(ptr->getName())+" *";
  }
  return true;
}

bool typePointer::isCompatible(Type *otype) {
   if((otype->getDataClass() == dataUnknownType) || (otype->getDataClass() == dataNullType))
       return true;
   typeTypedef *otypedef = dynamic_cast<typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType(Type::share));

   typePointer *oPointertype = dynamic_cast<typePointer *>(otype);

   if (oPointertype == NULL) {
      //reportError(BPatchWarning, 112, 
      //                   "Pointer and non-Pointer are not type compatible");
      return false;
   }
   // verify type that each one points to is compatible
   return baseType_->isCompatible(oPointertype->baseType_);
}

void typePointer::fixupUnknowns(Module *module) 
{
   if (baseType_->getDataClass() == dataUnknownType) 
   {
	  typeCollection *tc = typeCollection::getModTypeCollection(module);
	  assert(tc);
      baseType_ = tc->findType(baseType_->getID(), Type::share);
   }
}

/*
 * FUNCTION
 */

typeFunction::typeFunction(typeId_t ID, boost::shared_ptr<Type> retType, std::string name) :
    Type(name, ID, dataFunction), 
	retType_(retType) 
{
   size_ = sizeof(void *);
}

typeFunction::typeFunction(boost::shared_ptr<Type> retType, std::string name) :
    Type(name, ::getUniqueTypeId(), dataFunction),
	retType_(retType) 
{
   size_ = sizeof(void *);
}

typeFunction *typeFunction::create(std::string &name, boost::shared_ptr<Type> retType, dyn_c_vector<boost::shared_ptr<Type>> &paramTypes, Symtab *obj)
{
    typeFunction *type = new typeFunction(retType, name);
    for(unsigned i=0;i<paramTypes.size();i++)
	type->addParam(paramTypes[i]);
    if(obj)
        obj->addType(type);
    //obj->addType(type); TODO: declare a static container if obj is NULL and add to it.
    //Symtab::noObjTypes->push_back(type); ??
    return type;
}

boost::shared_ptr<Type> typeFunction::getReturnType(Type::do_share_t) const{
    return retType_;
}

bool typeFunction::setRetType(boost::shared_ptr<Type> rtype) {
    retType_ = rtype;
    return true;
}

bool typeFunction::addParam(boost::shared_ptr<Type> paramType){
    params_.push_back(paramType);
    return true;
}

dyn_c_vector<boost::shared_ptr<Type>> &typeFunction::getParams(){
    return params_;
}

bool typeFunction::isCompatible(Type *otype) {
   if((otype->getDataClass() == dataUnknownType) || (otype->getDataClass() == dataNullType))
       return true;
   typeTypedef *otypedef = dynamic_cast<typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType(Type::share));

   typeFunction *oFunctiontype = dynamic_cast<typeFunction *>(otype);

   if (oFunctiontype == NULL)
      return false;

   if (retType_ != oFunctiontype->retType_)
      return false;

   dyn_c_vector<boost::shared_ptr<Type>>& fields1 = this->getParams();
   dyn_c_vector<boost::shared_ptr<Type>>& fields2 = oFunctiontype->getParams();
   
   if (fields1.size() != fields2.size()) {
      //reportError(BPatchWarning, 112, 
      //                   "function number of params mismatch ");
      return false;
   }
    
   //need to compare componment by component to verify compatibility
   for (unsigned int i=0;i<fields1.size();i++) {
      if(!(fields1[i]->isCompatible(fields2[i]))) {
         //reportError(BPatchWarning, 112, 
         //                   "function param type mismatch ");
         return false;
      }
   }
   return true;
}   

void typeFunction::fixupUnknowns(Module *module) 
{
	typeCollection *tc = typeCollection::getModTypeCollection(module);
	assert(tc);

	if (retType_->getDataClass() == dataUnknownType) 
   {
      retType_ = tc->findType(retType_->getID(), Type::share);
   }

   for (unsigned int i = 0; i < params_.size(); i++)
   {
      params_[i] = tc->findType(params_[i]->getID(), Type::share);
   }	 
}

typeFunction::~typeFunction()
{ 
}

/*
 * RANGE
 */

//typeSubRange::typeSubRange(int ID, int size, const char *_low, const char *_hi, const char *_name)
//   : rangedType(_name, _ID, BPatchSymTypeRange, _size, _low, _hi) 
//{
//}

typeSubrange::typeSubrange(typeId_t ID, int size, long low, long hi, std::string name)
  : rangedType(name, ID, dataSubrange, size, low, hi)
{
}

typeSubrange::typeSubrange(int size, long low, long hi, std::string name)
  : rangedType(name, getUniqueTypeId(), dataSubrange, size, low, hi)
{
}

typeSubrange *typeSubrange::create(std::string &name, int size, long low, long hi, Symtab *obj)
{
   typeSubrange *typ = new typeSubrange(size, low, hi, name);

   if(obj)
       obj->addType(typ);
   return typ;
}

bool typeSubrange::isCompatible(Type *otype) {
   if((otype->getDataClass() == dataUnknownType) || (otype->getDataClass() == dataNullType))
       return true;
   typeTypedef *otypedef = dynamic_cast<typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType(Type::share));

   typeSubrange *oRangetype = dynamic_cast<typeSubrange *>(otype);

   if (oRangetype == NULL)
      return false;

   return getSize() == oRangetype->getSize();
}

/*
 * ARRAY
 */

typeArray::typeArray(typeId_t ID,
		boost::shared_ptr<Type> base,
		long low,
		long hi,
		std::string name,
		unsigned int sizeHint) :
	rangedType(name, ID, dataArray, 0, low, hi), 
	arrayElem(base), 
	sizeHint_(sizeHint) 
{
	//if (!base) arrayElem = Symtab::type_Error;
}

typeArray::typeArray(boost::shared_ptr<Type> base,
		long low,
		long hi,
		std::string name,
		unsigned int sizeHint) :
	rangedType(name, getUniqueTypeId(), dataArray, 0, low, hi),
	arrayElem(base), 
	sizeHint_(sizeHint) 
{
	assert(base);
}

typeArray *typeArray::create(std::string &name, boost::shared_ptr<Type> type, long low, long hi, Symtab *obj)
{
	typeArray *typ = new typeArray(type, low, hi, name);

	if(obj)
		obj->addType(typ);

	return typ;	
}

bool typeArray::operator==(const Type &otype) const 
{
	try {
		const typeArray &oArraytype = dynamic_cast<const typeArray &>(otype);
		if (sizeHint_ != oArraytype.sizeHint_) return false;
		if (arrayElem && !oArraytype.arrayElem) return false;
		if (!arrayElem && oArraytype.arrayElem) return false;
		if (arrayElem)
		{
			if (arrayElem->getID() != oArraytype.arrayElem->getID()) return false;
		}
		return (rangedType::operator==(otype)); 
	} catch (...) 
	{
		return false;
	}
}

void typeArray::merge(Type *other) 
{
	// There are wierd cases where we may define an array with an element
	// that is a forward reference

	typeArray *otherarray = dynamic_cast<typeArray *>(other);

	if ( otherarray == NULL || this->ID_ != otherarray->ID_ || 
			this->arrayElem->getDataClass() != dataUnknownType) 
	{
		//bperr( "Ignoring attempt to merge dissimilar types.\n" );
		return;
	}

	arrayElem = otherarray->arrayElem;
}

boost::shared_ptr<Type> typeArray::getBaseType(Type::do_share_t) const
{
	return arrayElem;
}

void typeArray::updateSize()
{    
	if (updatingSize) 
	{
		size_ = 0;
		return;
	}

	updatingSize = true;
	// Is our array element's Type still a placeholder?
	if (arrayElem->getDataClass() == dataUnknownType)
		size_ = 0;

	// Otherwise we can now calculate the array type's size
	else 
	{
		// Calculate the size of a single element
		unsigned int elemSize = sizeHint_ ? sizeHint_ : arrayElem->getSize();

		// Calculate the size of the whole array
		size_ = elemSize * (hi_ ? hi_ - low_ + 1 : 1);
	}
	updatingSize = false;
}

bool typeArray::isCompatible(Type *otype) 
{
	if (  (otype->getDataClass() == dataUnknownType) 
			|| (otype->getDataClass() == dataNullType))
		return true;

	typeTypedef *otypedef = dynamic_cast<typeTypedef *>(otype);

	if (otypedef != NULL) 
	{
		return isCompatible(otypedef->getConstituentType(Type::share));
	}

	typeArray *oArraytype = dynamic_cast<typeArray *>(otype);

	if (oArraytype == NULL) 
	{
		//reportError(BPatchWarning, 112, 
		//                   "Array and non-array are not type compatible");
		return false;      
	}
	unsigned int ec1, ec2;

   ec1 = hi_ - low_ + 1;
   ec2 = oArraytype->hi_ - oArraytype->low_ + 1;

   if (ec1 != ec2) 
   {
      char message[80];
      sprintf(message, "Incompatible number of elements [%lu..%lu] vs. [%lu..%lu]",
	      this->low_, this->hi_, oArraytype->low_, oArraytype->hi_);
      //reportError(BPatchWarning, 112, message);
      return false;
   }

   return arrayElem->isCompatible(oArraytype->arrayElem);
}

void typeArray::fixupUnknowns(Module *module) 
{
   if (arrayElem->getDataClass() == dataUnknownType) 
   {
	  typeCollection *tc = typeCollection::getModTypeCollection(module);
	  assert(tc);
	  arrayElem = tc->findType(arrayElem->getID(), Type::share);
   }
}

/*
 * STRUCT
 */

typeStruct::typeStruct(typeId_t ID, std::string name) :
    fieldListType(name, ID, dataStructure) 
{ 
}

typeStruct::typeStruct(std::string name)  :
    fieldListType(name, getUniqueTypeId(), dataStructure)
{
}

typeStruct *typeStruct::create(std::string &name, dyn_c_vector< std::pair<std::string, boost::shared_ptr<Type>> *> &flds,
                                                                Symtab *obj)
{
   int offset = 0;
   typeStruct *typ = new typeStruct(name);
   for(unsigned i=0;i<flds.size();i++)
   {
   	   typ->addField(flds[i]->first, flds[i]->second, offset);
       // Calculate next offset (in bits) into the struct
       offset += (flds[i]->second->getSize() * 8);
   }
   if(obj)
   	obj->addType(typ);
   //obj->addType(typ); TODO: declare a static container if obj is NULL and add to it.
   //Symtab::noObjTypes->push_back(typ); ??
				   
   return typ;	
}

typeStruct *typeStruct::create(std::string &name, dyn_c_vector<Field *> &flds, Symtab *obj)
{
   typeStruct *typ = new typeStruct(name);
   for(unsigned i=0;i<flds.size();i++)
   	typ->addField(flds[i]);
   if(obj)
   	obj->addType(typ);
   //obj->addType(typ); TODO: declare a static container if obj is NULL and add to it.
   //Symtab::noObjTypes->push_back(typ); ??
				   
   return typ;	
}

void typeStruct::merge(Type *other) {
   // Merging is only for forward references
//   assert(!fieldList.size());

   typeStruct *otherstruct = dynamic_cast<typeStruct *>(other);

   if( otherstruct == NULL || this->ID_ != otherstruct->ID_) {
      //bperr( "Ignoring attempt to merge dissimilar types.\n" );
      return;
   }

   if (otherstruct->name_ != "")
      name_ = std::string(otherstruct->name_);
   size_ = otherstruct->size_;

   fieldList = otherstruct->fieldList;

   if (otherstruct->derivedFieldList) {
       derivedFieldList = new dyn_c_vector<Field*>;
      *derivedFieldList = *otherstruct->derivedFieldList;
   }
}

void typeStruct::updateSize()
{
   if (updatingSize) {
      size_ = 0;
      return;
   }
   updatingSize = true;

    // Calculate the size of the entire structure
    size_ = 0;
    for(unsigned int i = 0; i < fieldList.size(); ++i) {
	size_ += fieldList[i]->getSize();

	// Is the type of this field still a placeholder?
	if(fieldList[i]->getType(Type::share)->getDataClass() == dataUnknownType) {
	    size_ = 0;
         break;
	}
    }
   updatingSize = false;
}

void typeStruct::postFieldInsert(int) 
{
}

bool typeStruct::isCompatible(Type *otype) 
{
   if((otype->getDataClass() == dataUnknownType) || (otype->getDataClass() == dataNullType))
       return true;
   typeTypedef *otypedef = dynamic_cast<typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType(Type::share));

   typeStruct *oStructtype = dynamic_cast<typeStruct *>(otype);

   if (oStructtype == NULL)
      return false;

   const dyn_c_vector<Field *> * fields1 = this->getComponents();
   const dyn_c_vector<Field *> * fields2 = oStructtype->getComponents();
   
   if (fields1->size() != fields2->size()) {
      //reportError(BPatchWarning, 112, 
      //                   "struct/union numer of elements mismatch ");
      return false;
   }
    
   //need to compare componment by component to verify compatibility
   for (unsigned int i=0;i<fields1->size();i++) {
      Field * field1 = (*fields1)[i];
      Field * field2 = (*fields2)[i];
            
      if(!(field1->getType(Type::share)->isCompatible(field2->getType(Type::share)))) {
         //reportError(BPatchWarning, 112, 
         //                   "struct/union field type mismatch ");
         return false;
      }
   }
   return true;
}

void typeStruct::fixupUnknowns(Module *module) 
{
   for (unsigned int i = 0; i < fieldList.size(); i++)
      fieldList[i]->fixupUnknown(module);
}

/*
 * UNION
 */

typeUnion::typeUnion(typeId_t ID, std::string name) :
    fieldListType(name, ID, dataUnion) 
{ 
}

typeUnion::typeUnion(std::string name)  :
    fieldListType(name, getUniqueTypeId(), dataUnion)
{
}

typeUnion *typeUnion::create(std::string &name, dyn_c_vector< std::pair<std::string, boost::shared_ptr<Type>> *> &flds,
                                                                Symtab *obj)
{
   typeUnion *typ = new typeUnion(name);
   for(unsigned i=0;i<flds.size();i++)
   	typ->addField(flds[i]->first, flds[i]->second, 0);
   if(obj)
   	obj->addType(typ);
   //obj->addType(typ); TODO: declare a static container if obj is NULL and add to it.
   //Symtab::noObjTypes->push_back(typ); ??
				   
   return typ;	
}

typeUnion *typeUnion::create(std::string &name, dyn_c_vector<Field *> &flds, Symtab *obj)
{
   typeUnion *typ = new typeUnion(name);
   for(unsigned i=0;i<flds.size();i++)
   	typ->addField(flds[i]);
   if(obj)
   	obj->addType(typ);
   //obj->addType(typ); TODO: declare a static container if obj is NULL and add to it.
   //Symtab::noObjTypes->push_back(typ); ??
				   
   return typ;	
}

void typeUnion::merge(Type *other) {
   typeUnion *otherunion = dynamic_cast<typeUnion *>(other);

   if( otherunion == NULL || this->ID_ != otherunion->ID_) {
      //bperr( "Ignoring attempt to merge dissimilar types.\n" );
      return;
   }

   if (!fieldList.size())
      return;

   if (otherunion->name_ != "")
      name_ = std::string(otherunion->name_);
   size_ = otherunion->size_;

   fieldList = otherunion->fieldList;

   if (otherunion->derivedFieldList) {
      derivedFieldList = new dyn_c_vector<Field*>;
      *derivedFieldList = *otherunion->derivedFieldList;
   }
}

void typeUnion::updateSize()
{
   if (updatingSize) {
      size_ = 0;
      return;
   }
   updatingSize = true;

    // Calculate the size of the union
    size_ = 0;
    for(unsigned int i = 0; i < fieldList.size(); ++i) {
	if(fieldList[i]->getSize() > size_)
	    size_ = fieldList[i]->getSize();

	// Is the type of this field still a placeholder?
        if(fieldList[i]->getType(Type::share)->getDataClass() == dataUnknownType) {
            size_ = 0;
         break;
        }
    }
   updatingSize = false;
}

void typeUnion::postFieldInsert(int nsize) {
	if ((unsigned int) nsize > size_) size_ = nsize; 
}

bool typeUnion::isCompatible(Type *otype) {
   if((otype->getDataClass() == dataUnknownType) || (otype->getDataClass() == dataNullType))
       return true;
   typeTypedef *otypedef = dynamic_cast<typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType(Type::share));

   typeUnion *oUniontype = dynamic_cast<typeUnion *>(otype);

   if (oUniontype == NULL)
      return false;

   const dyn_c_vector<Field *> * fields1 = this->getComponents();
   const dyn_c_vector<Field *> * fields2 = oUniontype->getComponents();
   
   if (fields1->size() != fields2->size()) {
      //reportError(BPatchWarning, 112, 
      //                   "struct/union numer of elements mismatch ");
      return false;
   }
    
   //need to compare componment by component to verify compatibility
   for (unsigned int i=0;i<fields1->size();i++) {
      Field * field1 = (*fields1)[i];
      Field * field2 = (*fields2)[i];
      
      if(!(field1->getType(Type::share)->isCompatible(field2->getType(Type::share)))) {
         //reportError(BPatchWarning, 112, 
         //                   "struct/union field type mismatch ");
         return false;
      }
   }
   return true;
}

void typeUnion::fixupUnknowns(Module *module) {
   for (unsigned int i = 0; i < fieldList.size(); i++)
      fieldList[i]->fixupUnknown(module);
}

/*
 * SCALAR
 */

   
typeScalar::typeScalar(typeId_t ID, unsigned int size, std::string name, bool isSigned) :
    Type(name, ID, dataScalar), isSigned_(isSigned)
{
   size_ = size;
}

typeScalar::typeScalar(unsigned int size, std::string name, bool isSigned) :
    Type(name, ::getUniqueTypeId(), dataScalar), isSigned_(isSigned)
{
   size_ = size;
}

typeScalar *typeScalar::create(std::string &name, int size, Symtab *obj)
{
   typeScalar *typ = new typeScalar(size, name);
   
   if(obj)
   	obj->addType(typ);
   //obj->addType(typ); TODO: declare a static container if obj is NULL and add to it.
   //Symtab::noObjTypes->push_back(typ); ??
				   
   return typ;	
}

bool typeScalar::isSigned(){
    return isSigned_;
}

bool typeScalar::isCompatible(Type *otype) {
   if((otype->getDataClass() == dataUnknownType) || (otype->getDataClass() == dataNullType))
       return true;
   bool ret = false;
   const typeTypedef *otypedef = dynamic_cast<const typeTypedef *>(otype);
   if (otypedef != NULL)  {
      ret =  isCompatible(otypedef->getConstituentType(Type::share));
      return ret;
   }

   typeScalar *oScalartype = dynamic_cast<typeScalar *>(otype);
   if (oScalartype == NULL) {
      //  Check to see if we have a range type, which can be compatible.
      typeSubrange *oSubrangeType = dynamic_cast<typeSubrange *>(otype);
      if (oSubrangeType != NULL) {
        if ( name_ == "" || oSubrangeType->getName() == "")
           return size_ == oSubrangeType->getSize();
        else if (name_ == oSubrangeType->getName())
           return size_ == oSubrangeType->getSize();
        else if (size_ == oSubrangeType->getSize()) {
          int t1 = findIntrensicType(name_);
          int t2 = findIntrensicType(oSubrangeType->getName());
          if (t1 & t2 & (t1 == t2)) {
            return true;
          }
        }
      }
      return false;
   }

   if ( name_ == "" || oScalartype->name_ == "")
      return size_ == oScalartype->size_;
   else if (name_ == oScalartype->name_)
      return size_ == oScalartype->size_;
   else if (size_ == oScalartype->size_) {
      int t1 = findIntrensicType(name_);
      int t2 = findIntrensicType(oScalartype->name_);
      if (t1 & t2 & (t1 == t2)) {
         return true;
      }
   }
   return false;
}

/* 
 * COMMON BLOCK
 */

typeCommon::typeCommon(int ID, std::string name) :
    fieldListType(name, ID, dataCommon) 
{}

typeCommon::typeCommon(std::string name) :
    fieldListType(name, getUniqueTypeId(), dataCommon)
{}

void typeCommon::beginCommonBlock() 
{
    fieldList.clear();
}

void typeCommon::endCommonBlock(Symbol *func, void *baseAddr) 
{
    unsigned int i, j;

    // create local variables in func's scope for each field of common block
    for (j=0; j < fieldList.size(); j++) {

	    localVar *locVar;
    	locVar = new localVar(fieldList[j]->getName(), 
	        			     fieldList[j]->getType(Type::share), "", 0, (Function *) func);
#if 0
    	VariableLocation *loc = (VariableLocation *)malloc(sizeof(VariableLocation));
#endif
    	VariableLocation loc;
        loc.stClass = storageAddr;
        loc.refClass = storageNoRef;
        loc.frameOffset = fieldList[j]->getOffset()+(Offset) baseAddr;
        locVar->addLocation(loc);

	// localVar->addField() TODO????
	//fieldList[j]->getOffset()+(Offset) baseAddr, -1, storageAddr);
	
        func->getFunction()->addLocalVar(locVar);
    }

    // look to see if the field list matches an existing block
    for (i=0; i < cblocks.size(); i++) {
	CBlock *curr = cblocks[i];
	for (j=0; j < fieldList.size(); j++) {
	    if ((fieldList[j]->getName() == curr->fieldList[j]->getName()) ||
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
    CBlock *newBlock = new CBlock();
    newBlock->fieldList = fieldList;
    newBlock->functions.push_back(func);
    cblocks.push_back(newBlock);
}

void typeCommon::fixupUnknowns(Module *module) {
   for (unsigned int i = 0; i < cblocks.size(); i++)
      cblocks[i]->fixupUnknowns(module);   
}

dyn_c_vector<CBlock *> *typeCommon::getCblocks() const
{
	return const_cast<dyn_c_vector<CBlock*>*>(&cblocks);
}

/*
 * TYPEDEF
 */

typeTypedef::typeTypedef(typeId_t ID, boost::shared_ptr<Type> base, std::string name, unsigned int sizeHint) :
    derivedType(name, ID, 0, dataTypedef) 
{
	baseType_ = base;
#if 0
	if (NULL == base)
		baseType_ = Symtab::type_Error;
	else
		baseType_ = base;
#endif
	sizeHint_ = sizeHint / 8;
}

typeTypedef::typeTypedef(boost::shared_ptr<Type> base, std::string name, unsigned int sizeHint) :
	derivedType(name, getUniqueTypeId(), 0, dataTypedef)
{
   assert(base != NULL);
   baseType_ = base;
   sizeHint_ = sizeHint / 8;
}

typeTypedef *typeTypedef::create(std::string &name, boost::shared_ptr<Type> baseType, Symtab *obj)
{
   if(!baseType)
   	return NULL;
   typeTypedef *typ = new typeTypedef(baseType, name);

   if(obj)
   	obj->addType(typ);
   //obj->addType(typ); TODO: declare a static container if obj is NULL and add to it.
   //Symtab::noObjTypes->push_back(typ); ??
				   
   return typ;	
}

bool typeTypedef::operator==(const Type &otype) const {
   try {
      const typeTypedef &oDeftype = dynamic_cast<const typeTypedef &>(otype);
      return baseType_==oDeftype.baseType_;
   } catch (...) {
      return false;
   }
}

bool typeTypedef::isCompatible(Type *otype){
    return baseType_->isCompatible(otype);
}

void typeTypedef::updateSize()
{
   if (updatingSize) {
      size_ = 0;
      return;
   }
   updatingSize = true;

    // Is our base type still a placeholder?
    if(baseType_->getDataClass() == dataUnknownType)
	size_ = 0;

    // Otherwise we can now calculate the type definition's size
    else {
	// Calculate the size of the type definition
	size_ = sizeHint_ ? sizeHint_ : baseType_->getSize();
    }
   updatingSize = false;
}

void typeTypedef::fixupUnknowns(Module *module) 
{
   if (baseType_->getDataClass() == dataUnknownType) 
   {
	  typeCollection *tc = typeCollection::getModTypeCollection(module);
	  assert(tc);
      baseType_ = tc->findType(baseType_->getID(), Type::share);
   }
}

/*
 * REFERENCE
 */

typeRef::typeRef(int ID, boost::shared_ptr<Type> refType, std::string name) :
    derivedType(name, ID, 0, dataReference) 
{
   baseType_ = refType;
}

typeRef::typeRef(boost::shared_ptr<Type> refType, std::string name) :
    derivedType(name, getUniqueTypeId(), 0, dataReference)
{
   baseType_ = refType;
}

typeRef *typeRef::create(std::string &name, boost::shared_ptr<Type> ref, Symtab *obj)
{
   typeRef *typ = new typeRef(ref, name);

   if(obj)
   	obj->addType(typ);
   //obj->addType(typ); TODO: declare a static container if obj is NULL and add to it.
   //Symtab::noObjTypes->push_back(typ); ??
				   
   return typ;	
}

bool typeRef::operator==(const Type &otype) const {
   try {
      const typeRef &oReftype = dynamic_cast<const typeRef &>(otype);
      return baseType_== oReftype.baseType_;
   } catch (...) {
      return false;
   }
}

bool typeRef::isCompatible(Type *otype) {
   if((otype->getDataClass() == dataUnknownType) || (otype->getDataClass() == dataNullType))
       return true;
   typeTypedef *otypedef = dynamic_cast<typeTypedef *>(otype);
   if (otypedef != NULL) return isCompatible(otypedef->getConstituentType(Type::share));

   typeRef *oReftype = dynamic_cast< typeRef *>(otype);
   if (oReftype == NULL) {
      return false;
   }
   return baseType_->isCompatible(oReftype->getConstituentType(Type::share));
}   

void typeRef::fixupUnknowns(Module *module) 
{
   if (baseType_->getDataClass() == dataUnknownType) 
   {
	  typeCollection *tc = typeCollection::getModTypeCollection(module);
	  assert(tc);
      baseType_ = tc->findType(baseType_->getID(), Type::share);
   }
}
		      
/* 
 * Subclasses of class Type, with interfaces
 */

/*
 * FIELD LIST Type
 */

fieldListType::fieldListType(std::string &name, typeId_t ID, dataClass typeDes) :
    Type(name, ID, typeDes), derivedFieldList(NULL)
{   
   size_ = 0;
}

fieldListType::~fieldListType() 
{
   fieldList.clear();
}

bool fieldListType::operator==(const Type &otype) const 
{
   try 
   {
      const fieldListType &oFieldtype = dynamic_cast<const fieldListType &>(otype);
      if (fieldList.size() != oFieldtype.fieldList.size())
         return false;
      for (unsigned int i = 0; i < fieldList.size(); i++) 
	  {
		  Field *f1 = fieldList[i];
		  Field *f2 = oFieldtype.fieldList[i];
		  if (f1 && !f2) return false;
		  if (!f1 && f2) return false;
		  if (f1)
		  {
			  if ( !((*f1) == (*f2)) )
				  return false;
		  }
      }
      return Type::operator==(otype);
   } 
   catch (...) 
   {
      return false;
   }
}

dyn_c_vector<Field *> * fieldListType::getComponents() const
{
   if (derivedFieldList == NULL)
       const_cast<fieldListType *>(this)->fixupComponents();
   return derivedFieldList;
}

dyn_c_vector<Field *> *fieldListType::getFields() const
{
   return const_cast<dyn_c_vector<Field *> *>(&fieldList);
}

void fieldListType::fixupComponents() 
{
   // bperr "Getting the %d components of '%s' at 0x%x\n", fieldList.size(), getName(), this );
   /* Iterate over the field list.  Recursively (replace)
      '{superclass}' with the superclass's non-private fields. */
   derivedFieldList = new dyn_c_vector< Field * >();
   for( unsigned int i = 0; i < fieldList.size(); i++ ) {
      Field * currentField = fieldList[i];
      // bperr( "Considering field '%s'\n", currentField->getName() );
      if( currentField->getName() ==  "{superclass}" ) {
         /* Note that this is a recursive call.  However, because
            the class-graph is acyclic (Stroustrup SpecialEd pg 308),
            we're OK. */
         // bperr( "Found superclass '%s'...\n", currentField->getType()->getName() );
         auto& superclass = dynamic_cast<fieldListInterface&>(*currentField->getType(Type::share));
         const dyn_c_vector<Field *> * superClassFields = superclass.getComponents();
         // bperr( "Superclass has %d components.\n", superClassFields->size() );
         /* FIXME: do we also need to consider the visibility of the superclass itself? */
         /* FIXME: visibility can also be described on a per-name basis in the
            subclass.  We have now way to convey this information currently, but I'm not
            sure that it matters for our purposes... */
         for( unsigned int i = 0; i < superClassFields->size(); i++ ) {
            Field * currentSuperField = (*superClassFields)[i];
            // bperr( "Considering superfield '%s'\n", currentSuperField->getName() );
            
            if( currentSuperField->getVisibility() != visPrivate ) {
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
 * void fieldListType::addField
 * Creates field object and adds it to the list of fields for this
 * type object.
 *     STRUCTS OR UNIONS
 */
void fieldListType::addField(std::string fieldname, boost::shared_ptr<Type> type, int offsetVal, visibility_t vis)
{
  Field * newField;
  newField = new Field(fieldname, type, offsetVal, vis);

  // Add field to list of struct/union fields
  fieldList.push_back(newField);

  // API defined structs/union's size are defined on the fly.
  postFieldInsert(type->getSize());
}

void fieldListType::addField(Field *fld)
{
  Field *newField = new Field(*fld);
  // Add field to list of struct/union fields
  fieldList.push_back(newField);

  // API defined structs/union's size are defined on the fly.
  postFieldInsert(newField->getSize());
}

void fieldListType::addField(unsigned num, std::string fieldname, boost::shared_ptr<Type> type, int offsetVal, visibility_t vis)
{
  Field * newField;
  newField = new Field(fieldname, type, offsetVal, vis);

  if(num >fieldList.size()+1)
  	num = fieldList.size();
  // Add field to list of struct/union fields
    dyn_c_vector<Field*> newFieldList;
    std::copy(fieldList.begin(), fieldList.begin() + num, back_inserter(newFieldList));
    newFieldList.push_back(newField);
    std::copy(fieldList.begin() + num, fieldList.end(), back_inserter(newFieldList));
    std::swap(fieldList, newFieldList);

  // API defined structs/union's size are defined on the fly.
  postFieldInsert(type->getSize());
}

void fieldListType::addField(unsigned num, Field *fld)
{
  Field *newField = new Field(*fld);
  // Add field to list of struct/union fields
  if(num >fieldList.size()+1)
  	num = fieldList.size();
  // Add field to list of struct/union fields
    dyn_c_vector<Field*> newFieldList;
    std::copy(fieldList.begin(), fieldList.begin() + num, back_inserter(newFieldList));
    newFieldList.push_back(newField);
    std::copy(fieldList.begin() + num, fieldList.end(), back_inserter(newFieldList));
    std::swap(fieldList, newFieldList);

  // API defined structs/union's size are defined on the fly.
  postFieldInsert(newField->getSize());
}

//void fieldListType::fixupUnknown(Module *m)
//{
//  type *t = dynamic_cast<Type *>(this);
//  assert(t);
//  t->fixupUnknown(m);
  //((Type *)this)->fixupUnknown(m);
//}


/*
 * DERIVED
 */

derivedType::derivedType(std::string &name, typeId_t id, int size, dataClass typeDes)
   :Type(name, id, typeDes)
{
	baseType_ = NULL; //Symtab::type_Error;
   size_ = size;
}

derivedType::derivedType(std::string &name, int size, dataClass typeDes)
   :Type(name, ::getUniqueTypeId(), typeDes)
{
	baseType_ = NULL; //Symtab::type_Error;
   size_ = size;
}

boost::shared_ptr<Type> derivedType::getConstituentType(Type::do_share_t) const
{
   return baseType_;
}

bool derivedType::operator==(const Type &otype) const {
   try {
      //const derivedType &oderivedtype = dynamic_cast<const derivedType &>(otype);
      return Type::operator==(otype);
   } catch (...) {
      return false;
   }
}

derivedType::~derivedType()
{}

/*
 * RANGED
 */

rangedType::rangedType(std::string &name, typeId_t ID, dataClass typeDes, int size, unsigned long low, unsigned long hi) :
   	Type(name, ID, typeDes), 
	low_(low), 
	hi_(hi) 
{
   size_ = size;
}

rangedType::rangedType(std::string &name, dataClass typeDes, int size, unsigned long low, unsigned long hi) :
    Type(name, ::getUniqueTypeId(), typeDes),
	low_(low), 
	hi_(hi)
{
   size_ = size;
}

/*
rangedType::rangedType(const char *_name, int _ID, dataClass _class, int _size, const char *_low, const char *_hi) 
   : Type(_name, _ID, _class) {

   low = strdup(_low);
   hi = strdup(_hi);

   size = _size;
}
*/

rangedType::~rangedType() {
}

bool rangedType::operator==(const Type &otype) const 
{
   try 
   {
      const rangedType &oRangedtype = dynamic_cast<const rangedType &>(otype);
      return (low_ == oRangedtype.low_ && hi_ == oRangedtype.hi_ &&
              Type::operator==(otype));
   } 
   catch (...) 
   {
      return false;
   }
}

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

static int findIntrensicType(std::string &name)
{
    struct intrensicTypes_ *curr;

    for (curr = intrensicTypes; curr->name; curr++) {
	if ((name != "")&& curr->name && !strcmp(name.c_str(), curr->name)) {
	    return curr->tid;
	}
    }

    return 0;
}


Field::Field() :
	FIELD_ANNOTATABLE_CLASS(),
	type_(NULL),
	vis_(visUnknown),
	offset_(-1)
{}
/*
 * Field::Field
 *
 * Constructor for Field.  Creates a Field object for 
 * an enumerated type.
 * type = offset = size = 0;
 */
Field::Field(std::string name, boost::shared_ptr<Type> typ, int offsetVal, visibility_t vis) :
	FIELD_ANNOTATABLE_CLASS(),
   fieldName_(name), 
   type_(typ), 
   vis_(vis), 
   offset_(offsetVal)
{}

std::string &Field::getName()
{
   return fieldName_;
}

boost::shared_ptr<Type> Field::getType(Type::do_share_t)
{
   return type_;
}

visibility_t Field::getVisibility()
{
   return vis_;
}

int Field::getOffset()
{
   return offset_;
}

unsigned int Field::getSize()
{
   return type_ ? type_->getSize() : 0;
}

Field::Field(Field &oField) :
	FIELD_ANNOTATABLE_CLASS()
{
   type_ = oField.type_;
   offset_ = oField.offset_;
   fieldName_ = std::string(oField.fieldName_);
   vis_ = oField.vis_;
}

Field::~Field() {}

void Field::fixupUnknown(Module *module) 
{
   if (type_->getDataClass() == dataUnknownType) 
   {
	  typeCollection *tc = typeCollection::getModTypeCollection(module);
	  assert(tc);
      type_ = tc->findType(type_->getID(), Type::share);
   }
}

bool Field::operator==(const Field &f) const
{
	if (type_ && !f.type_) return false;
	if (!type_ && f.type_) return false;
	if (type_)
	{
		if (type_->getID() != f.type_->getID()) return false;
	}
	if (fieldName_ != f.fieldName_) return false;
	if (vis_ != f.vis_) return false;
	if (offset_ != f.offset_) return false;
	return true;
}

/**************************************************************************
 * CBlock
 *************************************************************************/

void CBlock::fixupUnknowns(Module *module) 
{
	for (unsigned int i = 0; i < fieldList.size(); i++) 
	{
		fieldList[i]->fixupUnknown(module);
	}
}

dyn_c_vector<Field *> *CBlock::getComponents()
{
  return &fieldList;
}

dyn_c_vector<Symbol *> *CBlock::getFunctions()
{
  return &functions;
}

Type::Type() : ID_(0), name_(std::string("unnamedType")), size_(0),
               type_(dataUnknownType), updatingSize(false) {}
fieldListType::fieldListType() : derivedFieldList(NULL) {}
rangedType::rangedType() : low_(0), hi_(0) {}
derivedType::derivedType() : baseType_(NULL) {}
typeEnum::typeEnum() {}
typeFunction::typeFunction() : retType_(NULL) {}
typeScalar::typeScalar() : isSigned_(false) {}
typeCommon::typeCommon() {}
typeStruct::typeStruct() {}
typeUnion::typeUnion() {}
typePointer::typePointer() {}
typeTypedef::typeTypedef() : sizeHint_(0) {}
typeRef::typeRef() {}
typeSubrange::typeSubrange() {}
typeArray::typeArray() : arrayElem(NULL), sizeHint_(0) {}
