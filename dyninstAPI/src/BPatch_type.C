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

#include <stdio.h>

#define BPATCH_FILE


#include "util.h"
#include "BPatch_Vector.h"
#include "BPatch_collections.h"
#include "debug.h"
#include "BPatch_function.h"
#include "BPatch.h"
#include "mapped_module.h"
#include "RegisterConversion.h"
#include "registers/abstract_regs.h"
//#include "Annotatable.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

AnnotationClass<BPatch_cblock> CommonBlockUpPtrAnno("CommonBlockUpPtr", NULL);
AnnotationClass<BPatch_localVar> LocalVarUpPtrAnno("LocalVarUpPtrAnno", NULL);
AnnotationClass<BPatch_field> FieldUpPtrAnno("FieldUpPtrAnno", NULL);
AnnotationClass<BPatch_type> TypeUpPtrAnno("TypeUpPtr", NULL);
//static int findIntrensicType(const char *name);

// This is the ID that is decremented for each type a user defines. It is
// Global so that every type that the user defines has a unique ID.
// jdd 7/29/99
//int BPatch_type::USER_BPATCH_TYPE_ID = -1000;


std::map<Dyninst::SymtabAPI::Type*, BPatch_type *> BPatch_type::type_map;

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
 * BPatch_type::BPatch_type
 *
 * EMPTY Constructor for BPatch_type.  
 * 
 */

BPatch_type::BPatch_type(boost::shared_ptr<Type> typ_): ID(typ_->getID()), typ(typ_),
    refCount(1)
{
	// if a derived type, make sure the upPtr is set for the base type.
	// if it is not set, create a new BPatch_type for the upPtr

	if (typ_->isDerivedType()) 
	{
		auto base = typ_->asDerivedType().getConstituentType(Dyninst::SymtabAPI::Type::share);

		assert(base);
		BPatch_type *bpt = NULL;

		if (!base->getAnnotation(bpt, TypeUpPtrAnno))
		{
			//fprintf(stderr, "%s[%d]:  failed to get up ptr here\n", FILE__, __LINE__);

			//BPatch_type* dyninstType = new BPatch_type(base);
			// We might consider registering this new type in BPatch.
			// For now, just silence the warning:
			//(void) dyninstType;
		}
		else
		{
			assert (bpt);
		}
	}

	assert(typ_);
	typ_->addAnnotation(this, TypeUpPtrAnno);

	type_ = convertToBPatchdataClass(typ_->getDataClass());
	type_map[typ.get()] = this;
}

BPatch_type::BPatch_type(const char *_name, int _ID, BPatch_dataClass _type) :
   ID(_ID), type_(_type), typ(NULL), refCount(1)
{
	if (_name != NULL)
		typ = Type::make_shared<Type>(_name, ID, convertToSymtabType(_type));
	else
		typ = Type::make_shared<Type>("", ID, convertToSymtabType(_type));
	assert(typ);

	typ->addAnnotation(this, TypeUpPtrAnno);

	type_map[typ.get()] = this;
}

BPatch_type *BPatch_type::findOrCreateType(boost::shared_ptr<Dyninst::SymtabAPI::Type> type)  
{
   std::map<Dyninst::SymtabAPI::Type*, BPatch_type *>::iterator elem = type_map.find(type.get());
   if (elem != type_map.end()) {
      return (*elem).second;
   }
   
   BPatch_type *bptype = new BPatch_type(type);
   assert(bptype);
   return bptype;
}

/* BPatch_type destructor
 * Basic destructor for proper memory management.
 */
BPatch_type::~BPatch_type() {}

bool BPatch_type::operator==(const BPatch_type &otype) const 
{
   return (ID==otype.ID && type_ == otype.type_ && typ == otype.typ);
}

unsigned int BPatch_type::getSize()
{
  return typ->getSize();
}

const char *BPatch_type::getName() const
{
   return typ->getName().c_str(); 
}

boost::shared_ptr<Type> BPatch_type::getSymtabType(Type::do_share_t) const 
{
    return typ;
}    

boost::shared_ptr<Type> SymtabAPI::convert(const BPatch_type *t, Type::do_share_t) {
	return t->getSymtabType(Type::share);
}

unsigned long BPatch_type::getLow() const 
{
    return typ->isRangedType() ? typ->asRangedType().getLow() : 0;
}

bool BPatch_type::isCompatible(BPatch_type *otype) 
{
    return typ->isCompatible(otype->typ);
}

BPatch_type *BPatch_type::getConstituentType() const 
{
   boost::shared_ptr<Type> ctype;

   // Pointer, reference, typedef
   if (typ->isDerivedType()) ctype = typ->asDerivedType().getConstituentType(Type::share);
   else if(typ->isArrayType()) ctype = typ->asArrayType().getBaseType(Type::share);
   else return NULL;
   
   BPatch_type *bpt = NULL;
   
   if (!ctype->getAnnotation(bpt, TypeUpPtrAnno))
   {
      bpt = new BPatch_type(ctype);
   }
   else
   {
      assert(bpt);
   }
   return bpt;
}

BPatch_Vector<BPatch_field *> *BPatch_type::getComponents() const{
    if(typ->isFieldListType()) {
      auto comps = typ->asFieldListType().getComponents();
      if(!comps) return NULL;
      BPatch_Vector<BPatch_field *> *components = new BPatch_Vector<BPatch_field *>();
      for(unsigned i = 0 ; i< comps->size(); i++)
         components->push_back(new BPatch_field((*comps)[i]));
    	return components;    
    }

    if (typ->isEnumType()) 
	{
        auto constants = typ->asEnumType().getConstants();
        BPatch_Vector<BPatch_field *> *components = new BPatch_Vector<BPatch_field *>();
	    for (unsigned i = 0; i < constants.size(); i++)
		{
	        Field *fld = new Field(constants[i].first.c_str(), NULL);
	        components->push_back(new BPatch_field(fld, BPatch_dataScalar, constants[i].second, 0));
	    }
	    return components;    
    }

    if(typ->isDerivedType())
        return getConstituentType()->getComponents();
    return NULL;
}

BPatch_Vector<BPatch_cblock *> *BPatch_type::getCblocks() const 
{
	if (!typ->isCommonType())
		return NULL;

	auto cblocks = typ->asCommonType().getCblocks();

	if (!cblocks)
		return NULL;

	BPatch_Vector<BPatch_cblock *> *ret = new BPatch_Vector<BPatch_cblock *>();

	for (unsigned i = 0; i < cblocks->size(); i++)
	{
		BPatch_cblock *bpcb = NULL;
		CBlock *cb = (*cblocks)[i];
		assert(cb);
		if (!cb->getAnnotation(bpcb, CommonBlockUpPtrAnno))
		{
			fprintf(stderr, "%s[%d]:  WARN:  No Common Block UpPtr\n", FILE__, __LINE__);
		}
		else
		{
			assert(bpcb);
			ret->push_back(bpcb);
		}
	}
	return ret;	
}

unsigned long BPatch_type::getHigh() const {
    if(!typ->isRangedType())
        return 0;
    return typ->asRangedType().getHigh(); 
}

BPatch_dataClass BPatch_type::convertToBPatchdataClass(dataClass type) {
    switch(type){
      case dataEnum:
          return BPatch_dataEnumerated;
      case dataPointer:
          return BPatch_dataPointer;
      case dataFunction:
          return BPatch_dataMethod;
      case dataSubrange:
          return BPatchSymTypeRange;
      case dataArray:
          return BPatch_dataArray;
      case dataStructure:
          return BPatch_dataStructure;
      case dataUnion:
          return BPatch_dataUnion;
      case dataCommon:
          return BPatch_dataCommon;
      case dataScalar:
          return BPatch_dataScalar;
      case dataTypedef:
          return BPatch_dataTypeDefine;
      case dataReference:
          return BPatch_dataReference;
      case dataUnknownType:
          return BPatch_dataUnknownType;
      case dataNullType:
          return BPatch_dataNullType;
      case dataTypeClass:
          return BPatch_dataTypeClass;
      default:
          return BPatch_dataNullType;
    }  
}

Dyninst::SymtabAPI::dataClass BPatch_type::convertToSymtabType(BPatch_dataClass type){
    switch(type){
      case BPatch_dataScalar:
          return dataScalar;
      case BPatch_dataEnumerated:
          return dataEnum;
      case BPatch_dataTypeClass:
          return dataTypeClass;
      case BPatch_dataStructure:
          return dataStructure;
      case BPatch_dataUnion:
          return dataUnion;
      case BPatch_dataArray:
      	  return dataArray;
      case BPatch_dataPointer:
          return dataPointer;
      case BPatch_dataReferance:
          return dataReference;
      case BPatch_dataFunction:
          return dataFunction;
      case BPatch_dataReference:	// NOT sure-- TODO
          return dataNullType;
      case BPatch_dataUnknownType:
          return dataUnknownType;
      case BPatchSymTypeRange:
          return dataSubrange;
      case BPatch_dataMethod:
          return dataFunction;
      case BPatch_dataCommon:
          return dataCommon;
      case BPatch_dataTypeDefine:
          return dataTypedef;
      case BPatch_dataTypeAttrib:	//Never used
      case BPatch_dataTypeNumber:
      case BPatch_dataPrimitive:
      case BPatch_dataNullType:
      default:
          return dataNullType;
    }	  
}

BPatch_field::BPatch_field(Dyninst::SymtabAPI::Field *fld_, BPatch_dataClass typeDescriptor, int value_, int size_) :
    typeDes(typeDescriptor), value(value_), size(size_), fld(fld_)
{
	if (!fld_->addAnnotation(this, FieldUpPtrAnno))
	{
		fprintf(stderr, "%s[%d]: failed to add field list anno here\n", FILE__, __LINE__);
	}
}

void BPatch_field::copy(BPatch_field &oField) 
{
   fld = oField.fld;
   typeDes = oField.typeDes;
   size = oField.size;
   value = oField.value;
}

BPatch_field::BPatch_field(BPatch_field &oField)
{
   copy(oField);
}

BPatch_field &BPatch_field::operator=(BPatch_field &oField) 
{
   copy(oField);
   return *this;
}

BPatch_field::~BPatch_field() 
{
}

const char *BPatch_field::getName()
{
  return fld->getName().c_str();
}

BPatch_type *BPatch_field::getType()
{
	BPatch_type *bpt= NULL;
	assert(fld);
    assert(fld->getType(Type::share));
	if (!fld->getType(Type::share)->getAnnotation(bpt, TypeUpPtrAnno))
	{
		//fprintf(stderr, "%s[%d]:  failed to get up ptr here\n", FILE__, __LINE__);
		return new BPatch_type(fld->getType(Type::share));
	}

	assert(bpt);
	return bpt;
}

int BPatch_field::getValue()
{
  return value;
}

BPatch_visibility BPatch_field::getVisibility()
{
  return (BPatch_visibility)fld->getVisibility();
}

BPatch_dataClass BPatch_field::getTypeDesc()
{
  return typeDes;
}

int BPatch_field::getSize()
{
  return size;
}

int BPatch_field::getOffset()
{
  return fld->getOffset();
}

void BPatch_field::fixupUnknown(BPatch_module *module) {
   fld->fixupUnknown(module->lowlevel_mod()->pmod()->mod());
}

BPatch_localVar::BPatch_localVar(localVar *lVar_) : lVar(lVar_)
{
	assert(lVar);

	auto t = lVar->getType(Type::share);
	assert(t);
	
	if (!t->getAnnotation(type, TypeUpPtrAnno))
	{
		//fprintf(stderr, "%s[%d]:  failed to get up ptr here\n", FILE__, __LINE__);
		type = new BPatch_type(t);
	}
	else
	{
		assert(type);
	}

	type->incrRefCount();

        vector<Dyninst::VariableLocation> &locs = lVar_->getLocationLists();

    if (!locs.size())
       storageClass = BPatch_storageFrameOffset;
    else
       storageClass = convertToBPatchStorage(& locs[0]);



	if (!lVar->addAnnotation(this, LocalVarUpPtrAnno))
	{
		fprintf(stderr, "%s[%d]:  failed to add annotation here\n", FILE__, __LINE__);
	}

}

BPatch_storageClass BPatch_localVar::convertToBPatchStorage(Dyninst::VariableLocation *loc)
{
   Dyninst::storageClass stClass = loc->stClass;
   storageRefClass refClass = loc->refClass;
   if((stClass == storageAddr) && (refClass == storageNoRef))
      return BPatch_storageAddr;
   else if((stClass == storageAddr) && (refClass == storageRef))
      return BPatch_storageAddrRef;
   else if((stClass == storageReg) && (refClass == storageNoRef))
      return BPatch_storageReg;
   else if((stClass == storageReg) && (refClass == storageRef))
      return BPatch_storageRegRef;
   else if((stClass == storageRegOffset) && ((loc->mr_reg == Dyninst::InvalidReg) ||
                                             (loc->mr_reg == Dyninst::FrameBase) ||
                                             (loc->mr_reg == Dyninst::CFA)))
      return BPatch_storageFrameOffset;
   else if(stClass == storageRegOffset)
      return BPatch_storageRegOffset;
   else {
      assert(0);
      return (BPatch_storageClass) -1;
   }
}

localVar *BPatch_localVar::getSymtabVar(){
    return lVar;
}
				      
const char *BPatch_localVar::getName() { 
    return lVar->getName().c_str();
}

BPatch_type *BPatch_localVar::getType() { 
    return type; 
}

int BPatch_localVar::getLineNum() { 
    return lVar->getLineNum(); 
}

//TODO?? - get the first frame offset
long BPatch_localVar::getFrameOffset() {

   vector<Dyninst::VariableLocation> &locs = lVar->getLocationLists();

   if (locs.empty())
      return -1;

   return locs[0].frameOffset;
}

int BPatch_localVar::getRegister() {

    vector<Dyninst::VariableLocation> &locs = lVar->getLocationLists();

    if (!locs.size())
        return -1;

    bool ignored;
    return convertRegID(locs[0].mr_reg, ignored);
}

BPatch_storageClass BPatch_localVar::getStorageClass() {
    return storageClass; 
}

/*
 * BPatch_localVar destructor
 *
 */
BPatch_localVar::~BPatch_localVar()
{
    //XXX jdd 5/25/99 More to do later
    if(type)
        type->decrRefCount();
}

void BPatch_localVar::fixupUnknown(BPatch_module *module) {
   if (type->getDataClass() == BPatch_dataUnknownType) {
      BPatch_type *otype = type;
      type = module->getModuleTypes()->findType(type->getID());
      type->incrRefCount();
      otype->decrRefCount();
   }
}

/**************************************************************************
 * BPatch_cblock
 *************************************************************************/

BPatch_cblock::BPatch_cblock(CBlock *cBlk_) : cBlk(cBlk_) {
//TODO construct components here
}

void BPatch_cblock::fixupUnknowns(BPatch_module *module) {
   cBlk->fixupUnknowns(module->lowlevel_mod()->pmod()->mod());
}

BPatch_Vector<BPatch_field *> *BPatch_cblock::getComponents()
{
	BPatch_Vector<BPatch_field *> *components = new BPatch_Vector<BPatch_field *>;
	auto vars = cBlk->getComponents();

	if (!vars)
		return NULL;

	for (unsigned i=0; i<vars->size();i++)
	{
		Field *f = (*vars)[i];
		assert(f);
		BPatch_field *bpf = NULL;

		if (!f->getAnnotation(bpf, FieldUpPtrAnno))
		{
			fprintf(stderr, "%s[%d]:  no up ptr anno here\n", FILE__, __LINE__);
		}
		else
		{
			assert (bpf);
			components->push_back(bpf);
		}
	}

	return components;
}

BPatch_Vector<BPatch_function *> *BPatch_cblock::getFunctions()
{
  auto funcs = cBlk->getFunctions();
  if(!funcs)
      return NULL;   
  assert(0);
//Return BPatch_functions corresponding to Symbols in SymtabAPI
//Lookup again from BPatch::bpatch with the name. Then return the BPatch_functions
//TODO
  return NULL;
}

