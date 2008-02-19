/* 
 *  
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
//#include "BPatch_typePrivate.h"
#include "BPatch_collections.h"
#include "debug.h"
#include "BPatch_function.h"
#include "BPatch.h"
#include "mapped_module.h"

#ifdef i386_unknown_nt4_0
#define snprintf _snprintf
#endif

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

//static int findIntrensicType(const char *name);

// This is the ID that is decremented for each type a user defines. It is
// Global so that every type that the user defines has a unique ID.
// jdd 7/29/99
//int BPatch_type::USER_BPATCH_TYPE_ID = -1000;


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

BPatch_type::BPatch_type(Type *typ_): ID(typ_->getID()), typ(typ_),
    refCount(1)
{
    typ_->setUpPtr(this);
    type_ = convertToBPatchdataClass(typ_->getDataClass());
}

BPatch_type::BPatch_type(const char *_name, int _ID, BPatch_dataClass _type) :
   ID(_ID), type_(_type), refCount(1)
{
  if (_name != NULL)
     typ = new Type(_name, ID, convertToSymtabType(_type));
  else
     typ = new Type("", ID, convertToSymtabType(_type));
  typ->setUpPtr(this);
}

/* BPatch_type destructor
 * Basic destructor for proper memory management.
 */
BPatch_type::~BPatch_type()
{
}

bool BPatch_type::operator==(const BPatch_type &otype) const 
{
   return (ID==otype.ID && type_ == otype.type_ && typ == otype.typ);
}

unsigned int BPatch_type::getSizeInt()
{
  return typ->getSize();
}

const char *BPatch_type::getName() const { 
   return typ->getName().c_str(); 
}

Type *BPatch_type::getSymtabType() const {
    return typ;
}    

const char *BPatch_type::getLow() const {
    rangedInterface *rangetype = dynamic_cast<rangedInterface *>(typ);
    if(!rangetype)
        return NULL;
    return (const char *)rangetype->getLow(); 
}

bool BPatch_type::isCompatible(BPatch_type *otype) { 
    return typ->isCompatible(otype->typ);
}

bool BPatch_type::isCompatibleInt(BPatch_type *otype) { 
    return typ->isCompatible(otype->typ);
}

BPatch_type *BPatch_type::getConstituentType() const {
    derivedInterface *derivedType = dynamic_cast<derivedInterface *>(typ);
    if(!derivedType)
        return NULL;
    return (BPatch_type *)derivedType->getConstituentType()->getUpPtr();
}

BPatch_Vector<BPatch_field *> *BPatch_type::getComponents() const{
    fieldListInterface *fieldlisttype = dynamic_cast<fieldListInterface *>(typ);
    typeEnum *enumtype = dynamic_cast<typeEnum *>(typ);
    typeTypedef *typedeftype = dynamic_cast<typeTypedef *>(typ);
    if(!fieldlisttype && !enumtype && !typedeftype)
        return NULL;	
    BPatch_Vector<BPatch_field *> *components = new BPatch_Vector<BPatch_field *>();
    if(fieldlisttype) {
        vector<Field *> *comps = fieldlisttype->getComponents();
    	if(!comps){
	        free(components);
	        return NULL;
    	}    
	    for(unsigned i = 0 ; i< comps->size(); i++)
	        components->push_back(new BPatch_field((*comps)[i]));
    	return components;    
    }
    if(enumtype) {
        vector<pair<string, int> *> constants = enumtype->getConstants();
	    for(unsigned i = 0; i < constants.size(); i++){
	        Field *fld = new Field(constants[i]->first.c_str(), NULL);
	        components->push_back(new BPatch_field(fld, BPatch_dataScalar, constants[i]->second, 0));
	    }
	    return components;    
    }
    if(typedeftype)
        return getConstituentType()->getComponents();
}

BPatch_Vector<BPatch_cblock *> *BPatch_type::getCblocks() const {
    typeCommon *commontype = dynamic_cast<typeCommon *>(typ);
    if(!commontype)
	return NULL;
    	
    std::vector<CBlock *> *cblocks = commontype->getCblocks();
    if(!cblocks)
        return NULL;
    BPatch_Vector<BPatch_cblock *> *ret = new BPatch_Vector<BPatch_cblock *>();
    for(unsigned i = 0; i < cblocks->size(); i++)
        ret->push_back((BPatch_cblock *)(*cblocks)[i]->getUpPtr());
    return ret;	
}

const char *BPatch_type::getHigh() const {
    rangedInterface *rangetype = dynamic_cast<rangedInterface *>(typ);
    if(!rangetype)
        return NULL;
    return (const char *)rangetype->getHigh(); 
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

#ifdef IBM_BPATCH_COMPAT
char *BPatch_type::getName(char *buffer, int max) const
{
  const char *name = typ->getName().c_str();
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

#if 0
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
  assert(newField);
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


void BPatch_fieldListType::addField(BPatch_type *_type)
{
  BPatch_field * newField;

  // Create Field for parameter
  newField = new BPatch_field("param", BPatch_dataUnknownType, _type, 0, 0);
  // Add field to list of struct/union fields
  fieldList.push_back(newField);
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
void BPatch_field::BPatch_fieldEnum(const char * fName, BPatch_dataClass _typeDes,
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
#endif

BPatch_field::BPatch_field(Dyninst::SymtabAPI::Field *fld_, BPatch_dataClass typeDescriptor, int value_, int size_) :
    typeDes(typeDescriptor), value(value_), size(size_), fld(fld_)
{
    fld_->setUpPtr(this);
}

void BPatch_field::copy(BPatch_field &oField) 
{
   fld = oField.fld;
   typeDes = oField.typeDes;
   size = oField.size;
   value = oField.value;
}

BPatch_field::BPatch_field(BPatch_field &oField) : BPatch_eventLock()
{
   __LOCK;
   copy(oField);
   __UNLOCK;
}

BPatch_field &BPatch_field::operator_equals(BPatch_field &oField) 
{
   copy(oField);
   return *this;
}

void BPatch_field::BPatch_field_dtor() 
{
}

const char *BPatch_field::getNameInt()
{
  return fld->getName().c_str();
}

BPatch_type *BPatch_field::getTypeInt()
{
  if(!fld->getType()->getUpPtr())
      return new BPatch_type (fld->getType());
  return (BPatch_type *)fld->getType()->getUpPtr();
}

int BPatch_field::getValueInt()
{
  return value;
}

BPatch_visibility BPatch_field::getVisibilityInt()
{
  return (BPatch_visibility)fld->getVisibility();
}

BPatch_dataClass BPatch_field::getTypeDescInt()
{
  return typeDes;
}

int BPatch_field::getSizeInt()
{
  return size;
}

int BPatch_field::getOffsetInt()
{
  return fld->getOffset();
}

void BPatch_field::fixupUnknown(BPatch_module *module) {
   fld->fixupUnknown(module->lowlevel_mod()->pmod()->mod());
}

/**************************************************************************
 * BPatch_localVar
 *************************************************************************/
/*
 * BPatch_localVar Constructor
 *
 */

#if 0
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
#endif

BPatch_localVar::BPatch_localVar(localVar *lVar_) : lVar(lVar_)
{
    type = (BPatch_type *)lVar->getType()->getUpPtr();
    if(!type)
        type = new BPatch_type(lVar->getType());
    type->incrRefCount();
    storageClass = convertToBPatchStorage(lVar_); 
    lVar->setUpPtr(this);
}

BPatch_storageClass BPatch_localVar::convertToBPatchStorage(localVar *lVar)
{
   vector<Dyninst::SymtabAPI::loc_t *> *locs = lVar->getLocationLists();
   if(!locs)
   	return BPatch_storageFrameOffset;
   
   Dyninst::SymtabAPI::storageClass stClass = (*locs)[0]->stClass;
   storageRefClass refClass = (*locs)[0]->refClass;
   if((stClass == storageAddr) && (refClass == storageNoRef))
       return BPatch_storageAddr;
   else if((stClass == storageAddr) && (refClass == storageRef))
       return BPatch_storageAddrRef;
   else if((stClass == storageReg) && (refClass == storageNoRef))
       return BPatch_storageReg;
   else if((stClass == storageReg) && (refClass == storageRef))
       return BPatch_storageRegRef;
   else if((stClass == storageRegOffset) && ((*locs)[0]->reg == -1))
       return BPatch_storageFrameOffset;
   else if((stClass == storageRegOffset))
   	return BPatch_storageRegOffset;
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
    vector<Dyninst::SymtabAPI::loc_t *> *locs = lVar->getLocationLists();
    if(!locs)
        return -1;
    return (*(locs))[0]->frameOffset;
}

int BPatch_localVar::getRegister() { 
    vector<Dyninst::SymtabAPI::loc_t *> *locs = lVar->getLocationLists();
    if(!locs)
        return -1;
    return (*(locs))[0]->reg;
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

BPatch_Vector<BPatch_field *> *BPatch_cblock::getComponentsInt()
{
  BPatch_Vector<BPatch_field *> *components = new BPatch_Vector<BPatch_field *>;
  std::vector<Field *> *vars = cBlk->getComponents();
  if(!vars)
     return NULL;
  for(unsigned i=0; i<vars->size();i++)
      components->push_back((BPatch_field *)(*vars)[i]->getUpPtr());
  return components;
}

BPatch_Vector<BPatch_function *> *BPatch_cblock::getFunctionsInt()
{
  std::vector<Symbol *> *funcs = cBlk->getFunctions();
  if(!funcs)
      return NULL;   
//Return BPatch_functions corresponding to Symbols in SymtabAPI
//Lookup again from BPatch::bpatch with the name. Then return the BPatch_functions
//TODO

}
