/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
			     
#ifndef __Type_h__
#define __Type_h__

#include "Serialization.h"
#include "Annotatable.h"
#include "symutil.h"

namespace Dyninst{
namespace SymtabAPI{

class Module;
class Symtab;
class Symbol;
class Type;
class typeEnum;
class typePointer;
class typeFunction;
class typeSubrange;
class typeArray;
class typeStruct;
class typeUnion;
class typeScalar;
class typeCommon;
class typeTypedef;
class typeRef;
class CBlock;
class typeCollection;
class fieldListType;
class TypeMemManager;

//TODO?? class BPatch(to be  ??)function;


typedef enum {dataEnum,
              dataPointer,
              dataFunction,
	      dataSubrange,
              dataArray,
              dataStructure,
              dataUnion,
              dataCommon,
              dataScalar,
              dataTypedef,
              dataReference,
              dataUnknownType,
              dataNullType,
	      dataTypeClass
} dataClass;

SYMTAB_EXPORT const char *dataClass2Str(dataClass dc);

typedef int typeId_t;

typedef enum {
   visPrivate, 
   visProtected, 
   visPublic,
   visUnknown
} visibility_t;

/*
 * visibility: Accessibility of member data and functions
 * These values follow the 'fieldname:' after the '/' identifier.
 * visPrivate   == 0 gnu Sun -- private
 * visProtected == 1 gnu Sun -- protected
 * visPublic    == 2 gnu Sun -- public
 * visUnknown visibility not known or doesn't apply(ANSIC), the default
 *
 */
 
SYMTAB_EXPORT const char *visibility2Str(visibility_t v);

#define FIELD_ANNOTATABLE_CLASS AnnotatableDense

class Field : public Serializable, public FIELD_ANNOTATABLE_CLASS 
{
   friend class typeStruct;
   friend class typeUnion;
   friend class typeCommon;
   friend class CBlock;
   
   std::string fieldName_;
   Type *type_;
   visibility_t  vis_;
   int offset_;

   /* Method vars */
 protected:
   void copy(Field &);

 public:
   SYMTAB_EXPORT Field(); 
   SYMTAB_EXPORT Field(std::string name, Type *typ, int offsetVal = -1, 
		   visibility_t vis = visUnknown);
   
   // Copy constructor
   SYMTAB_EXPORT Field(Field &f);
   SYMTAB_EXPORT ~Field();

   SYMTAB_EXPORT std::string &getName();
   SYMTAB_EXPORT Type *getType();
   SYMTAB_EXPORT visibility_t getVisibility();
   SYMTAB_EXPORT unsigned int getSize();
   SYMTAB_EXPORT int getOffset();
   
   SYMTAB_EXPORT void fixupUnknown(Module *);
   SYMTAB_EXPORT Serializable * serialize_impl(SerializerBase *sb, 
		   const char *tag="Field") THROW_SPEC(SerializerError);
   SYMTAB_EXPORT virtual bool operator==(const Field &) const;
};
				  
#define TYPE_ANNOTATABLE_CLASS AnnotatableDense

class Type : public Serializable, public  TYPE_ANNOTATABLE_CLASS 
{
   friend class typeCollection;
   friend std::string parseStabString(Module *, int linenum, char *, int, 
				      typeCommon *);
   static Type* upgradePlaceholder(Type *placeholder, Type *new_type);

   public:

   SYMTAB_EXPORT virtual void serialize_specific(SerializerBase *) 
	   THROW_SPEC(SerializerError) {}

   SYMTAB_EXPORT Serializable * serialize_impl(SerializerBase *, 
		   const char * = "Type") THROW_SPEC (SerializerError);

 protected:
   typeId_t ID_;           /* unique ID of type */
   std::string name_;
   unsigned int  size_;    /* size of type */
   dataClass   type_;
   
   /**
    * We set updatingSize to true when we're in the process of
    * calculating the size of container structures.  This helps avoid
    * infinite recursion for self referencial types.  Getting a
    * self-referencial type probably signifies an error in another
    * part of the type processing code (or an error in the binary).
    **/
   bool updatingSize;
   
   static typeId_t USER_TYPE_ID;

   // INTERNAL DATA MEMBERS
   unsigned int refCount;

protected:
   SYMTAB_EXPORT virtual void updateSize() {}

   SYMTAB_EXPORT virtual void merge( Type * /* other */ ) { }

public:
   SYMTAB_EXPORT virtual bool operator==(const Type &) const;
   SYMTAB_EXPORT virtual bool isCompatible(Type *oType);
   SYMTAB_EXPORT virtual void fixupUnknowns(Module *);

   SYMTAB_EXPORT Type(std::string name, typeId_t ID, dataClass dataTyp = dataNullType);
   SYMTAB_EXPORT Type(std::string name, dataClass dataTyp = dataNullType);

   SYMTAB_EXPORT Type();
   SYMTAB_EXPORT virtual ~Type();

   // A few convenience functions
   SYMTAB_EXPORT static Type *createFake(std::string name);
   /* Placeholder for real type, to be filled in later */
   SYMTAB_EXPORT static Type *createPlaceholder(typeId_t ID, std::string name = "");
   
   SYMTAB_EXPORT typeId_t getID() const;
   SYMTAB_EXPORT unsigned int getSize();
   SYMTAB_EXPORT bool setSize(unsigned int size);
   SYMTAB_EXPORT std::string &getName();
   SYMTAB_EXPORT bool setName(std::string);
   SYMTAB_EXPORT dataClass getDataClass() const;

   // INTERNAL METHODS
   SYMTAB_EXPORT void incrRefCount();
   SYMTAB_EXPORT void decrRefCount(); 
   //Methods to dynamically cast generic Type Object to specific types.
   
   SYMTAB_EXPORT typeEnum *getEnumType();
   SYMTAB_EXPORT typePointer *getPointerType();
   SYMTAB_EXPORT typeFunction *getFunctionType();
   SYMTAB_EXPORT typeSubrange *getSubrangeType();
   SYMTAB_EXPORT typeArray *getArrayType();
   SYMTAB_EXPORT typeStruct *getStructType();
   SYMTAB_EXPORT typeUnion *getUnionType();
   SYMTAB_EXPORT typeScalar *getScalarType();
   SYMTAB_EXPORT typeCommon *getCommonType();
   SYMTAB_EXPORT typeTypedef *getTypedefType();
   SYMTAB_EXPORT typeRef *getRefType();
   SYMTAB_EXPORT std::string specificType();
};

// Interfaces to be implemented by intermediate subtypes
// We have to do this thanks to reference types and C++'s lovely 
// multiple inheritance

class fieldListInterface {
 public:
   SYMTAB_EXPORT virtual ~fieldListInterface() {};
   SYMTAB_EXPORT virtual std::vector<Field *> *getComponents() const = 0;
};

class rangedInterface {
 public:
   SYMTAB_EXPORT virtual ~rangedInterface() {};
   SYMTAB_EXPORT virtual unsigned long getLow() const = 0;
   SYMTAB_EXPORT virtual unsigned long getHigh() const  = 0;
};  

class derivedInterface{
 public:
   SYMTAB_EXPORT virtual ~derivedInterface() {};
   SYMTAB_EXPORT virtual Type *getConstituentType() const = 0;
};

// Intermediate types (interfaces + Type)

class fieldListType : public Type, public fieldListInterface 
{
 private:
   void fixupComponents();
 protected:
   std::vector<Field *> fieldList;
   std::vector<Field *> *derivedFieldList;
   SYMTAB_EXPORT fieldListType(std::string &name, typeId_t ID, dataClass typeDes);
   /* Each subclass may need to update its size after adding a field */
 public:
   SYMTAB_EXPORT fieldListType();
   SYMTAB_EXPORT ~fieldListType();
   SYMTAB_EXPORT bool operator==(const Type &) const;
   SYMTAB_EXPORT std::vector<Field *> *getComponents() const;
   
   SYMTAB_EXPORT std::vector<Field *> *getFields() const;
   
   SYMTAB_EXPORT virtual void postFieldInsert(int nsize) = 0;
   
   /* Add field for C++ struct or union */
   SYMTAB_EXPORT void addField(std::string fieldname, Type *type, int offsetVal = -1, visibility_t vis = visUnknown);
   SYMTAB_EXPORT void addField(unsigned num, std::string fieldname, Type *type, int offsetVal = -1, visibility_t vis = visUnknown);
   SYMTAB_EXPORT void addField(Field *fld);
   SYMTAB_EXPORT void addField(unsigned num, Field *fld);
  
   SYMTAB_EXPORT void serialize_fieldlist(Dyninst::SerializerBase *, 
		   const char * = "fieldListType") THROW_SPEC (SerializerError);
  // void addField(const std::string &fieldname,  dataClass typeDes, 
  //               Type *type, int offset, int size, visibility_t vis = visUnknown);
};

class rangedType : public Type, public rangedInterface {
 protected:
   unsigned long low_;
   unsigned long hi_;
 protected:
   //rangedType(const std::string &name, typeId_t ID, dataClass typeDes, int size, const char *low, const char *hi); 
   SYMTAB_EXPORT rangedType(std::string &name, typeId_t ID, dataClass typeDes, int size, unsigned long low, unsigned long hi);
   SYMTAB_EXPORT rangedType(std::string &name, dataClass typeDes, int size, unsigned long low, unsigned long hi);
   SYMTAB_EXPORT void serialize_ranged(SerializerBase *, 
		   const char * = "rangedType") THROW_SPEC (SerializerError);
 public:
   SYMTAB_EXPORT rangedType();
   SYMTAB_EXPORT ~rangedType();
   SYMTAB_EXPORT bool operator==(const Type &) const;
   SYMTAB_EXPORT unsigned long getLow() const { return low_; }
   SYMTAB_EXPORT unsigned long getHigh() const { return hi_; }
};

class derivedType : public Type, public derivedInterface {
 protected:
   Type *baseType_;
 protected:
   SYMTAB_EXPORT derivedType(std::string &name, typeId_t id, int size, dataClass typeDes);
   SYMTAB_EXPORT derivedType(std::string &name, int size, dataClass typeDes);
 public:
   SYMTAB_EXPORT derivedType();
   SYMTAB_EXPORT ~derivedType();
   SYMTAB_EXPORT bool operator==(const Type &) const;
   SYMTAB_EXPORT Type *getConstituentType() const;
   SYMTAB_EXPORT void serialize_derived(SerializerBase *, 
		   const char * = "derivedType") THROW_SPEC(SerializerError);
};

// Derived classes from Type

class typeEnum : public Type {
 private:  
	std::vector<std::pair<std::string, int> > consts;
 public:
   SYMTAB_EXPORT typeEnum();
   SYMTAB_EXPORT typeEnum(typeId_t ID, std::string name = "");
   SYMTAB_EXPORT typeEnum(std::string name);
   SYMTAB_EXPORT static typeEnum *create(std::string &name, std::vector<std::pair<std::string, int> *>&elements, 
   								Symtab *obj = NULL);
   SYMTAB_EXPORT static typeEnum *create(std::string &name, std::vector<std::string> &constNames, Symtab *obj);
   SYMTAB_EXPORT bool addConstant(const std::string &fieldname,int value);
   SYMTAB_EXPORT std::vector<std::pair<std::string, int> > &getConstants();
   SYMTAB_EXPORT bool setName(const char *name);
   SYMTAB_EXPORT bool isCompatible(Type *otype);
   SYMTAB_EXPORT void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class typeFunction : public Type {
 protected:
   SYMTAB_EXPORT void fixupUnknowns(Module *);
 private:
   Type *retType_; /* Return type of the function */
   std::vector<Type *> params_; 
 public:
   SYMTAB_EXPORT typeFunction();
   SYMTAB_EXPORT typeFunction(typeId_t ID, Type *retType, std::string name = "");
   SYMTAB_EXPORT typeFunction(Type *retType, std::string name = "");
   SYMTAB_EXPORT static typeFunction *create(std::string &name, Type *retType, 
   				std::vector<Type *> &paramTypes, Symtab *obj = NULL);
   SYMTAB_EXPORT ~typeFunction();
   SYMTAB_EXPORT bool addParam( Type *type);
   SYMTAB_EXPORT Type *getReturnType() const;
   SYMTAB_EXPORT bool setRetType(Type *rtype);

   SYMTAB_EXPORT std::vector<Type *> &getParams();
   SYMTAB_EXPORT bool isCompatible(Type *otype);
   SYMTAB_EXPORT void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class typeScalar : public Type {
 private:
   bool isSigned_;
 public:
   SYMTAB_EXPORT typeScalar();
   SYMTAB_EXPORT typeScalar(typeId_t ID, unsigned int size, std::string name = "", bool isSigned = false);
   SYMTAB_EXPORT typeScalar(unsigned int size, std::string name = "", bool isSigned = false);
   SYMTAB_EXPORT static typeScalar *create(std::string &name, int size, Symtab *obj = NULL);
   SYMTAB_EXPORT bool isSigned();
   SYMTAB_EXPORT bool isCompatible(Type *otype);
   SYMTAB_EXPORT void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class typeCommon : public fieldListType {
 private:
   std::vector<CBlock *> cblocks;
 protected:
   SYMTAB_EXPORT void postFieldInsert(int nsize) { size_ += nsize; }
   //void postFieldInsert(int offset, int nsize) { if ((unsigned int) (offset + nsize) > size_) size_ = offset + nsize; }
   SYMTAB_EXPORT void fixupUnknowns(Module *);
 public:
   SYMTAB_EXPORT typeCommon();
   SYMTAB_EXPORT typeCommon(typeId_t ID, std::string name = "");
   SYMTAB_EXPORT typeCommon(std::string name);
   SYMTAB_EXPORT static typeCommon *create(std::string &name, Symtab *obj = NULL);
   SYMTAB_EXPORT std::vector<CBlock *> *getCblocks() const;
   SYMTAB_EXPORT void beginCommonBlock();
   SYMTAB_EXPORT void endCommonBlock(Symbol *, void *baseAddr);
   SYMTAB_EXPORT void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class CBlock : public Serializable, public AnnotatableSparse
{
   friend class typeCommon;
 private:
   // the list of fields
   std::vector<Field *> fieldList;

   // which functions use this list
   //  Should probably be updated to use aggregates
   std::vector<Symbol *> functions;

 public:
   SYMTAB_EXPORT std::vector<Field *> *getComponents();
   SYMTAB_EXPORT std::vector<Symbol *> *getFunctions();

   SYMTAB_EXPORT void fixupUnknowns(Module *);
   
   SYMTAB_EXPORT Serializable * serialize_impl(SerializerBase *,
		   const char *tag="CBlock") THROW_SPEC(SerializerError);
};

class typeStruct : public fieldListType {
 protected:
   SYMTAB_EXPORT void updateSize();
   SYMTAB_EXPORT void postFieldInsert(int nsize);
   SYMTAB_EXPORT void fixupUnknowns(Module *);
   SYMTAB_EXPORT void merge(Type *other);
 public:
   SYMTAB_EXPORT typeStruct();
   SYMTAB_EXPORT typeStruct(typeId_t ID, std::string name = "");
   SYMTAB_EXPORT typeStruct(std::string name);
   SYMTAB_EXPORT static typeStruct *create(std::string &name, std::vector< std::pair<std::string, Type *> *> &flds,

   				 				Symtab *obj = NULL);
   SYMTAB_EXPORT static typeStruct *create(std::string &name, std::vector<Field *> &fields, 
								Symtab *obj = NULL);

   SYMTAB_EXPORT bool isCompatible(Type *otype);
   SYMTAB_EXPORT void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class typeUnion : public fieldListType {
 protected:
   SYMTAB_EXPORT void updateSize();
   SYMTAB_EXPORT void postFieldInsert(int nsize);
   SYMTAB_EXPORT void merge(Type *other);
   SYMTAB_EXPORT void fixupUnknowns(Module *);
 public:
   SYMTAB_EXPORT typeUnion();
   SYMTAB_EXPORT typeUnion(typeId_t ID, std::string name = "");
   SYMTAB_EXPORT typeUnion(std::string name);
   SYMTAB_EXPORT static typeUnion *create(std::string &name, std::vector<std::pair<std::string, Type *> *> &fieldNames,
   							Symtab *obj = NULL);
   SYMTAB_EXPORT static typeUnion *create(std::string &name, std::vector<Field *> &fields, 
							Symtab *obj = NULL);
   SYMTAB_EXPORT bool isCompatible(Type *otype);
   SYMTAB_EXPORT void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class typePointer : public derivedType {
 protected: 
   SYMTAB_EXPORT void fixupUnknowns(Module *);
 public:
   SYMTAB_EXPORT typePointer();
   SYMTAB_EXPORT typePointer(typeId_t ID, Type *ptr, std::string name = "");
   SYMTAB_EXPORT typePointer(Type *ptr, std::string name = "");
   SYMTAB_EXPORT static typePointer *create(std::string &name, Type *ptr, Symtab *obj = NULL);
   SYMTAB_EXPORT static typePointer *create(std::string &name, Type *ptr, int size, 
   							Symtab *obj = NULL);
   SYMTAB_EXPORT bool isCompatible(Type *otype);
   SYMTAB_EXPORT bool setPtr(Type *ptr);
   SYMTAB_EXPORT void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class typeTypedef: public derivedType {
 private:
   unsigned int sizeHint_;
 
 protected:
   SYMTAB_EXPORT void updateSize();
   SYMTAB_EXPORT void fixupUnknowns(Module *);
      
 public:
   SYMTAB_EXPORT typeTypedef();
   SYMTAB_EXPORT typeTypedef(typeId_t ID, Type *base, std::string name, unsigned int sizeHint = 0);
   SYMTAB_EXPORT typeTypedef(Type *base, std::string name, unsigned int sizeHint = 0);
   
   SYMTAB_EXPORT static typeTypedef *create(std::string &name, Type *ptr, Symtab *obj = NULL);
   SYMTAB_EXPORT bool isCompatible(Type *otype);
   SYMTAB_EXPORT bool operator==(const Type &otype) const;
   SYMTAB_EXPORT void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class typeRef : public derivedType {
 protected:
   SYMTAB_EXPORT void fixupUnknowns(Module *);
 public:
   SYMTAB_EXPORT typeRef();
   SYMTAB_EXPORT typeRef(typeId_t ID, Type *refType, std::string name);
   SYMTAB_EXPORT typeRef(Type *refType, std::string name);
   SYMTAB_EXPORT static typeRef *create(std::string &name, Type *ptr, Symtab * obj = NULL);
   SYMTAB_EXPORT bool isCompatible(Type *otype);
   SYMTAB_EXPORT bool operator==(const Type &otype) const;
   SYMTAB_EXPORT void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class typeSubrange : public rangedType {
 private:
   //typeSubrange(int ID, int size, const char *low, const char *hi, const char *name);
 public:
   SYMTAB_EXPORT typeSubrange();
   SYMTAB_EXPORT typeSubrange(typeId_t ID, int size, long low, long hi, std::string name);
   SYMTAB_EXPORT typeSubrange( int size, long low, long hi, std::string name);
   SYMTAB_EXPORT static typeSubrange *create(std::string &name, int size, long low, long hi, Symtab *obj = NULL);
   SYMTAB_EXPORT bool isCompatible(Type *otype);
   SYMTAB_EXPORT void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class typeArray : public rangedType {
 private:
   Type *arrayElem;
   unsigned int sizeHint_;
 protected:
   SYMTAB_EXPORT void updateSize();
   SYMTAB_EXPORT void merge(Type *other); 
 public:
   SYMTAB_EXPORT typeArray();
   SYMTAB_EXPORT typeArray(typeId_t ID, Type *base, long low, long hi, std::string name, unsigned int sizeHint = 0);
   SYMTAB_EXPORT typeArray(Type *base, long low, long hi, std::string name, unsigned int sizeHint = 0);
   SYMTAB_EXPORT static typeArray *create(std::string &name, Type *typ,  long low, long hi, Symtab *obj = NULL);
   SYMTAB_EXPORT Type *getBaseType() const;
   SYMTAB_EXPORT bool isCompatible(Type *otype);
   SYMTAB_EXPORT bool operator==(const Type &otype) const;
   SYMTAB_EXPORT void fixupUnknowns(Module *);
   SYMTAB_EXPORT void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

} // namespace SymtabAPI
} // namespace Dyninst
#endif

