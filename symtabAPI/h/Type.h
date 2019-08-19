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
			     
#ifndef __Type_h__
#define __Type_h__

#include "Serialization.h"
#include "Annotatable.h"
#include "symutil.h"
#include "concurrent.h"

#include <boost/atomic.hpp>
#include <mutex>

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
class rangedType;
class derivedType;
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

class SYMTAB_EXPORT Field : public Serializable, public FIELD_ANNOTATABLE_CLASS 
{
   friend class typeStruct;
   friend class typeUnion;
   friend class typeCommon;
   friend class CBlock;
   
   std::string fieldName_;
   boost::shared_ptr<Type> type_;
   visibility_t  vis_;
   int offset_;

   /* Method vars */
 protected:
   void copy(Field &);

 public:
   Field(); 
   Field(std::string name, boost::shared_ptr<Type> typ, int offsetVal = -1, 
		   visibility_t vis = visUnknown);
   
   // Copy constructor
   Field(Field &f);
   ~Field();

   std::string &getName();
   boost::shared_ptr<Type> getType();
   visibility_t getVisibility();
   unsigned int getSize();
   int getOffset();
   
   void fixupUnknown(Module *);
   Serializable * serialize_impl(SerializerBase *sb, 
		   const char *tag="Field") THROW_SPEC(SerializerError);
   virtual bool operator==(const Field &) const;
};
				  
#define TYPE_ANNOTATABLE_CLASS AnnotatableDense

class SYMTAB_EXPORT Type : public Serializable, public  TYPE_ANNOTATABLE_CLASS 
{
   friend class typeCollection;
   friend std::string parseStabString(Module *, int linenum, char *, int, 
				      typeCommon*);
   static Type* upgradePlaceholder(Type *placeholder, Type *new_type);

   public:

   virtual void serialize_specific(SerializerBase *) 
	   THROW_SPEC(SerializerError) {}

   Serializable * serialize_impl(SerializerBase *, 
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

   static boost::atomic<typeId_t> USER_TYPE_ID;

   // INTERNAL DATA MEMBERS

protected:
   virtual void updateSize() {}

   virtual void merge( Type * /* other */ ) { }

public:
   virtual bool operator==(const Type &) const;
   bool isCompatible(boost::shared_ptr<Type> x) { return isCompatible(x.get()); };
   virtual bool isCompatible(Type *oType);
   virtual void fixupUnknowns(Module *);

   Type(std::string name, typeId_t ID, dataClass dataTyp = dataNullType);
   Type(std::string name, dataClass dataTyp = dataNullType);

   Type();
   virtual ~Type();

   // Fake unique_ptr type. TODO: Replace with std::unique_ptr for C++11
   class unique_ptr_Type {
      Type* ptr;
   public:
      unique_ptr_Type(Type* p) : ptr(p) {};
      operator boost::shared_ptr<Type>() {
         return boost::shared_ptr<Type>(ptr);
      }
   };
   // A few convenience functions
   static unique_ptr_Type createFake(std::string name);
   /* Placeholder for real type, to be filled in later */
   static unique_ptr_Type createPlaceholder(typeId_t ID, std::string name = "");
   
   typeId_t getID() const;
   unsigned int getSize();
   bool setSize(unsigned int size);
   std::string &getName();
   bool setName(std::string);
   dataClass getDataClass() const;

   //Methods to dynamically cast generic Type Object to specific types.
   
   typeEnum *getEnumType();
   typePointer *getPointerType();
   typeFunction *getFunctionType();
   typeSubrange *getSubrangeType();
   typeArray *getArrayType();
   typeStruct *getStructType();
   typeUnion *getUnionType();
   typeScalar *getScalarType();
   typeCommon *getCommonType();
   typeTypedef *getTypedefType();
   typeRef *getRefType();
   std::string specificType();
   
   inline fieldListType& asFieldListType();
   inline bool isFieldListType();

   inline rangedType& asRangedType();
   inline bool isRangedType();
   
   inline derivedType& asDerivedType();
   inline bool isDerivedType();
   
   inline typeCommon& asCommonType();
   inline bool isCommonType();
   
   inline bool isStructType();
   
   inline typeFunction& asFunctionType();
   
   inline typeEnum& asEnumType();
   inline bool isEnumType();
   
   inline typeArray& asArrayType();
   inline bool isArrayType();

   //Helper Functions for getting & updating unique USER_TYPE_ID
   typeId_t getUniqueTypeId();
   void updateUniqueTypeId(typeId_t);
};

// Interfaces to be implemented by intermediate subtypes
// We have to do this thanks to reference types and C++'s lovely 
// multiple inheritance

class SYMTAB_EXPORT fieldListInterface {
 public:
   virtual ~fieldListInterface() {};
   virtual dyn_c_vector<Field *> *getComponents() const = 0;
};

class SYMTAB_EXPORT rangedInterface {
 public:
   virtual ~rangedInterface() {};
   virtual unsigned long getLow() const = 0;
   virtual unsigned long getHigh() const  = 0;
};  

class SYMTAB_EXPORT derivedInterface{
 public:
   virtual ~derivedInterface() {};
   virtual boost::shared_ptr<Type> getConstituentType() const = 0;
};

// Intermediate types (interfaces + Type)

class SYMTAB_EXPORT fieldListType : public Type, public fieldListInterface 
{
 private:
   void fixupComponents();
 protected:
   dyn_c_vector<Field *> fieldList;
   dyn_c_vector<Field *> *derivedFieldList;
   fieldListType(std::string &name, typeId_t ID, dataClass typeDes);
   /* Each subclass may need to update its size after adding a field */
 public:
   fieldListType();
   ~fieldListType();
   bool operator==(const Type &) const;
   dyn_c_vector<Dyninst::SymtabAPI::Field*> *getComponents() const;
   
   dyn_c_vector<Dyninst::SymtabAPI::Field*> *getFields() const;
   
   virtual void postFieldInsert(int nsize) = 0;
   
   /* Add field for C++ struct or union */
   void addField(std::string fieldname, boost::shared_ptr<Type> type, int offsetVal = -1, visibility_t vis = visUnknown);
   void addField(unsigned num, std::string fieldname, boost::shared_ptr<Type> type, int offsetVal = -1, visibility_t vis = visUnknown);
   void addField(Field *fld);
   void addField(unsigned num, Field *fld);
  
   void serialize_fieldlist(Dyninst::SerializerBase *, 
		   const char * = "fieldListType") THROW_SPEC (SerializerError);
  // void addField(const std::string &fieldname,  dataClass typeDes, 
  //               Type *type, int offset, int size, visibility_t vis = visUnknown);
};
fieldListType& Type::asFieldListType() { return dynamic_cast<fieldListType&>(*this); }
bool Type::isFieldListType() { return dynamic_cast<fieldListType*>(this) != NULL; }

class SYMTAB_EXPORT rangedType : public Type, public rangedInterface {
 protected:
   unsigned long low_;
   unsigned long hi_;
 protected:
   //rangedType(const std::string &name, typeId_t ID, dataClass typeDes, int size, const char *low, const char *hi); 
   rangedType(std::string &name, typeId_t ID, dataClass typeDes, int size, unsigned long low, unsigned long hi);
   rangedType(std::string &name, dataClass typeDes, int size, unsigned long low, unsigned long hi);
   void serialize_ranged(SerializerBase *, 
		   const char * = "rangedType") THROW_SPEC (SerializerError);
 public:
   rangedType();
   ~rangedType();
   bool operator==(const Type &) const;
   unsigned long getLow() const { return low_; }
   unsigned long getHigh() const { return hi_; }
};
rangedType& Type::asRangedType() { return dynamic_cast<rangedType&>(*this); }
bool Type::isRangedType() { return dynamic_cast<rangedType*>(this) != NULL; }

class SYMTAB_EXPORT derivedType : public Type, public derivedInterface {
 protected:
   boost::shared_ptr<Type> baseType_;
 protected:
   derivedType(std::string &name, typeId_t id, int size, dataClass typeDes);
   derivedType(std::string &name, int size, dataClass typeDes);
 public:
   derivedType();
   ~derivedType();
   bool operator==(const Type &) const;
   boost::shared_ptr<Type> getConstituentType() const;
   void serialize_derived(SerializerBase *, 
		   const char * = "derivedType") THROW_SPEC(SerializerError);
};
derivedType& Type::asDerivedType() { return dynamic_cast<derivedType&>(*this); }
bool Type::isDerivedType() { return dynamic_cast<derivedType*>(this) != NULL; }

// Derived classes from Type

class SYMTAB_EXPORT typeEnum : public Type {
 private:  
   dyn_c_vector<std::pair<std::string, int> > consts;
 public:
   typeEnum();
   typeEnum(typeId_t ID, std::string name = "");
   typeEnum(std::string name);
   static typeEnum *create(std::string &name, dyn_c_vector<std::pair<std::string, int> *>&elements,
   								Symtab *obj = NULL);
   static typeEnum *create(std::string &name, dyn_c_vector<std::string> &constNames, Symtab *obj);
   bool addConstant(const std::string &fieldname,int value);
   dyn_c_vector<std::pair<std::string, int> > &getConstants();
   bool setName(const char *name);
   bool isCompatible(boost::shared_ptr<Type> x) { return isCompatible(x.get()); };
   bool isCompatible(Type *otype);
   void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};
typeEnum& Type::asEnumType() { return dynamic_cast<typeEnum&>(*this); }
bool Type::isEnumType() { return dynamic_cast<typeEnum*>(this) != NULL; }

class SYMTAB_EXPORT typeFunction : public Type {
 protected:
   void fixupUnknowns(Module *);
 private:
   boost::shared_ptr<Type> retType_; /* Return type of the function */
   dyn_c_vector<boost::shared_ptr<Type>> params_;
 public:
   typeFunction();
   typeFunction(typeId_t ID, boost::shared_ptr<Type> retType, std::string name = "");
   typeFunction(boost::shared_ptr<Type> retType, std::string name = "");
   static typeFunction *create(std::string &name, boost::shared_ptr<Type> retType, 
                               dyn_c_vector<boost::shared_ptr<Type>> &paramTypes, Symtab *obj = NULL);
   ~typeFunction();
   bool addParam(boost::shared_ptr<Type> type);
   boost::shared_ptr<Type> getReturnType() const;
   bool setRetType(boost::shared_ptr<Type> rtype);

   dyn_c_vector<boost::shared_ptr<Type>> &getParams();
   bool isCompatible(boost::shared_ptr<Type> x) { return isCompatible(x.get()); };
   bool isCompatible(Type *otype);
   void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};
typeFunction& Type::asFunctionType() { return dynamic_cast<typeFunction&>(*this); }

class SYMTAB_EXPORT typeScalar : public Type {
 private:
   bool isSigned_;
 public:
   typeScalar();
   typeScalar(typeId_t ID, unsigned int size, std::string name = "", bool isSigned = false);
   typeScalar(unsigned int size, std::string name = "", bool isSigned = false);
   static typeScalar *create(std::string &name, int size, Symtab *obj = NULL);
   bool isSigned();
   bool isCompatible(boost::shared_ptr<Type> x) { return isCompatible(x.get()); };
   bool isCompatible(Type *otype);
   void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class SYMTAB_EXPORT typeCommon : public fieldListType {
 private:
   dyn_c_vector<CBlock *> cblocks;
 protected:
   void postFieldInsert(int nsize) { size_ += nsize; }
   //void postFieldInsert(int offset, int nsize) { if ((unsigned int) (offset + nsize) > size_) size_ = offset + nsize; }
   void fixupUnknowns(Module *);
 public:
   typeCommon();
   typeCommon(typeId_t ID, std::string name = "");
   typeCommon(std::string name);
   static typeCommon *create(std::string &name, Symtab *obj = NULL);
   dyn_c_vector<CBlock *> *getCblocks() const;
   void beginCommonBlock();
   void endCommonBlock(Symbol *, void *baseAddr);
   void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};
typeCommon& Type::asCommonType() { return dynamic_cast<typeCommon&>(*this); }
bool Type::isCommonType() { return dynamic_cast<typeCommon*>(this) != NULL; }

class SYMTAB_EXPORT CBlock : public Serializable, public AnnotatableSparse
{
   friend class typeCommon;
 private:
   // the list of fields
   dyn_c_vector<Field *> fieldList;

   // which functions use this list
   //  Should probably be updated to use aggregates
   dyn_c_vector<Symbol *> functions;

 public:
   dyn_c_vector<Field *> *getComponents();
   dyn_c_vector<Symbol *> *getFunctions();

   void fixupUnknowns(Module *);
   
   Serializable * serialize_impl(SerializerBase *,
		   const char *tag="CBlock") THROW_SPEC(SerializerError);
};

class SYMTAB_EXPORT typeStruct : public fieldListType {
 protected:
   void updateSize();
   void postFieldInsert(int nsize);
   void fixupUnknowns(Module *);
   void merge(Type *other);
 public:
   typeStruct();
   typeStruct(typeId_t ID, std::string name = "");
   typeStruct(std::string name);
   static typeStruct *create(std::string &name, dyn_c_vector< std::pair<std::string, boost::shared_ptr<Type> > *> &flds,
                             Symtab *obj = NULL);
   static typeStruct *create(std::string &name, dyn_c_vector<Field *> &fields,
                             Symtab *obj = NULL);

   bool isCompatible(boost::shared_ptr<Type> x) { return isCompatible(x.get()); };
   bool isCompatible(Type *otype);
   void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};
bool Type::isStructType() { return dynamic_cast<typeStruct*>(this) != NULL; }

class SYMTAB_EXPORT typeUnion : public fieldListType {
 protected:
   void updateSize();
   void postFieldInsert(int nsize);
   void merge(Type *other);
   void fixupUnknowns(Module *);
 public:
   typeUnion();
   typeUnion(typeId_t ID, std::string name = "");
   typeUnion(std::string name);
   static typeUnion *create(std::string &name, dyn_c_vector<std::pair<std::string, boost::shared_ptr<Type>> *> &fieldNames,
   							Symtab *obj = NULL);
   static typeUnion *create(std::string &name, dyn_c_vector<Field *> &fields, 
							Symtab *obj = NULL);
   bool isCompatible(boost::shared_ptr<Type> x) { return isCompatible(x.get()); };
   bool isCompatible(Type *otype);
   void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class SYMTAB_EXPORT typePointer : public derivedType {
 protected: 
   void fixupUnknowns(Module *);
 public:
   typePointer();
   typePointer(typeId_t ID, boost::shared_ptr<Type> ptr, std::string name = "");
   typePointer(boost::shared_ptr<Type> ptr, std::string name = "");
   static typePointer *create(std::string &name, boost::shared_ptr<Type> ptr, Symtab *obj = NULL);
   static typePointer *create(std::string &name, boost::shared_ptr<Type> ptr, int size, 
   							Symtab *obj = NULL);
   bool isCompatible(boost::shared_ptr<Type> x) { return isCompatible(x.get()); };
   bool isCompatible(Type *otype);
   bool setPtr(boost::shared_ptr<Type> ptr);
   void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class SYMTAB_EXPORT typeTypedef: public derivedType {
 private:
   unsigned int sizeHint_;
 
 protected:
   void updateSize();
   void fixupUnknowns(Module *);
      
 public:
   typeTypedef();
   typeTypedef(typeId_t ID, boost::shared_ptr<Type> base, std::string name, unsigned int sizeHint = 0);
   typeTypedef(boost::shared_ptr<Type> base, std::string name, unsigned int sizeHint = 0);
   
   static typeTypedef *create(std::string &name, boost::shared_ptr<Type> ptr, Symtab *obj = NULL);
   bool isCompatible(boost::shared_ptr<Type> x) { return isCompatible(x.get()); };
   bool isCompatible(Type *otype);
   bool operator==(const Type &otype) const;
   void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class SYMTAB_EXPORT typeRef : public derivedType {
 protected:
   void fixupUnknowns(Module *);
 public:
   typeRef();
   typeRef(typeId_t ID, boost::shared_ptr<Type> refType, std::string name);
   typeRef(boost::shared_ptr<Type> refType, std::string name);
   static typeRef *create(std::string &name, boost::shared_ptr<Type> ptr, Symtab * obj = NULL);
   bool isCompatible(boost::shared_ptr<Type> x) { return isCompatible(x.get()); };
   bool isCompatible(Type *otype);
   bool operator==(const Type &otype) const;
   void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class SYMTAB_EXPORT typeSubrange : public rangedType {
 private:
   //typeSubrange(int ID, int size, const char *low, const char *hi, const char *name);
 public:
   typeSubrange();
   typeSubrange(typeId_t ID, int size, long low, long hi, std::string name);
   typeSubrange( int size, long low, long hi, std::string name);
   static typeSubrange *create(std::string &name, int size, long low, long hi, Symtab *obj = NULL);
   bool isCompatible(boost::shared_ptr<Type> x) { return isCompatible(x.get()); };
   bool isCompatible(Type *otype);
   void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};

class SYMTAB_EXPORT typeArray : public rangedType {
 private:
   boost::shared_ptr<Type> arrayElem;
   unsigned int sizeHint_;
 protected:
   void updateSize();
   void merge(Type *other); 
 public:
   typeArray();
   typeArray(typeId_t ID, boost::shared_ptr<Type> base, long low, long hi, std::string name, unsigned int sizeHint = 0);
   typeArray(boost::shared_ptr<Type> base, long low, long hi, std::string name, unsigned int sizeHint = 0);
   static typeArray *create(std::string &name, boost::shared_ptr<Type> typ,  long low, long hi, Symtab *obj = NULL);
   boost::shared_ptr<Type> getBaseType() const;
   bool isCompatible(boost::shared_ptr<Type> x) { return isCompatible(x.get()); };
   bool isCompatible(Type *otype);
   bool operator==(const Type &otype) const;
   void fixupUnknowns(Module *);
   void serialize_specific(SerializerBase *) THROW_SPEC(SerializerError);
};
typeArray& Type::asArrayType() { return dynamic_cast<typeArray&>(*this); }
bool Type::isArrayType() { return dynamic_cast<typeArray*>(this) != NULL; }

} // namespace SymtabAPI
} // namespace Dyninst
#endif

