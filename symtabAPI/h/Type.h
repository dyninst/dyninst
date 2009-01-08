/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
			     
#ifndef Type_h_
#define Type_h_

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
class localVarCollection;

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

const char *dataClass2Str(dataClass dc);

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
 
const char *visibility2Str(visibility_t v);

typedef enum {
	storageAddr,
	storageReg,
	storageRegOffset
} storageClass;

const char *storageClass2Str(storageClass sc);
/*
 * storageClass: Encodes how a variable is stored.
 *
 * storageAddr           - Absolute address of variable.
 * storageReg            - Register which holds variable value.
 * storageRegOffset      - Address of variable = $reg + address.
 */

typedef enum {
    storageRef,
    storageNoRef
} storageRefClass;
	
/*
 * storageRefClass: Encodes if a variable can be accessed through a register/address.
 *
 * storageRef        - There is a pointer to variable.
 * storageNoRef      - No reference. Value can be obtained using storageClass.
 */

class SYMTABEXPORT Field{
   friend class typeStruct;
   friend class typeUnion;
   friend class typeCommon;
   friend class CBlock;
   
   std::string fieldName_;
   Type *type_;
   visibility_t  vis_;
   int offset_;
   void *upPtr_;

   /* Method vars */
 protected:
   void copy(Field &);

 public:
   Field(std::string name, Type *typ, int offsetVal = -1, visibility_t vis = visUnknown);
   
   // Copy constructor
   Field(Field &f);
   ~Field();

   std::string &getName();
   Type *getType();
   visibility_t getVisibility();
   unsigned int getSize();
   int getOffset();
   void *getUpPtr() const;
   bool setUpPtr(void *);
   
   void fixupUnknown(Module *);
};
				  
class Type : public Serializable, public AnnotatableSparse  {
   friend class typeCollection;
   friend std::string parseStabString(Module *, int linenum, char *, int, 
                          typeCommon *);
   public:
  SYMTABEXPORT void serialize(SerializerBase *, const char *);
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

   //Extend Type
   void *upPtr_;
 
protected:
   SYMTABEXPORT virtual void updateSize() {}

   // Simple Destructor
   SYMTABEXPORT virtual ~Type();

   SYMTABEXPORT virtual void merge( Type * /* other */ ) { }

public:
   SYMTABEXPORT virtual bool operator==(const Type &) const;
   SYMTABEXPORT virtual bool isCompatible(Type *oType);
   SYMTABEXPORT virtual void fixupUnknowns(Module *);

   SYMTABEXPORT Type(std::string name, typeId_t ID, dataClass dataTyp = dataNullType);
   SYMTABEXPORT Type(std::string name, dataClass dataTyp = dataNullType);

   // A few convenience functions
   SYMTABEXPORT static Type *createFake(std::string name);
   /* Placeholder for real type, to be filled in later */
   SYMTABEXPORT static Type *createPlaceholder(typeId_t ID, std::string name = "");
   
   SYMTABEXPORT typeId_t getID() const;
   SYMTABEXPORT unsigned int getSize();
   SYMTABEXPORT bool setSize(unsigned int size);
   SYMTABEXPORT std::string &getName();
   SYMTABEXPORT bool setName(std::string);

   SYMTABEXPORT bool setUpPtr(void *);
   SYMTABEXPORT void *getUpPtr() const;

   SYMTABEXPORT dataClass getDataClass() const;

   // INTERNAL METHODS
   SYMTABEXPORT void incrRefCount();
   SYMTABEXPORT void decrRefCount(); 
   Type () {}
   
   
   //Methods to dynamically cast generic Type Object to specific types.
   
   SYMTABEXPORT typeEnum *getEnumType();
   SYMTABEXPORT typePointer *getPointerType();
   SYMTABEXPORT typeFunction *getFunctionType();
   SYMTABEXPORT typeSubrange *getSubrangeType();
   SYMTABEXPORT typeArray *getArrayType();
   SYMTABEXPORT typeStruct *getStructType();
   SYMTABEXPORT typeUnion *getUnionType();
   SYMTABEXPORT typeScalar *getScalarType();
   SYMTABEXPORT typeCommon *getCommonType();
   SYMTABEXPORT typeTypedef *getTypedefType();
   SYMTABEXPORT typeRef *getRefType();
};

// Interfaces to be implemented by intermediate subtypes
// We have to do this thanks to reference types and C++'s lovely 
// multiple inheritance

class fieldListInterface {
 public:
   SYMTABEXPORT virtual ~fieldListInterface() {};
   SYMTABEXPORT virtual std::vector<Field *> *getComponents() const = 0;
};

class rangedInterface {
 public:
   SYMTABEXPORT virtual ~rangedInterface() {};
   SYMTABEXPORT virtual int getLow() const = 0;
   SYMTABEXPORT virtual int getHigh() const  = 0;
};  

class derivedInterface{
 public:
   SYMTABEXPORT virtual ~derivedInterface() {};
   SYMTABEXPORT virtual Type *getConstituentType() const = 0;
};

// Intermediate types (interfaces + Type)

class fieldListType : public Type, public fieldListInterface {
 private:
   void fixupComponents();
 protected:
   std::vector<Field *> fieldList;
   std::vector<Field *> *derivedFieldList;
   SYMTABEXPORT fieldListType(std::string &name, typeId_t ID, dataClass typeDes);
   /* Each subclass may need to update its size after adding a field */
 public:
   SYMTABEXPORT ~fieldListType();
   SYMTABEXPORT bool operator==(const Type &) const;
   SYMTABEXPORT std::vector<Field *> *getComponents() const;
   
   SYMTABEXPORT std::vector<Field *> *getFields() const;
   
   SYMTABEXPORT virtual void postFieldInsert(int nsize) = 0;
   
   /* Add field for C++ struct or union */
   SYMTABEXPORT void addField(std::string fieldname, Type *type, int offsetVal = -1, visibility_t vis = visUnknown);
   SYMTABEXPORT void addField(unsigned num, std::string fieldname, Type *type, int offsetVal = -1, visibility_t vis = visUnknown);
   SYMTABEXPORT void addField(Field *fld);
   SYMTABEXPORT void addField(unsigned num, Field *fld);
  
  // void addField(const std::string &fieldname,  dataClass typeDes, 
  //               Type *type, int offset, int size, visibility_t vis = visUnknown);
};

class rangedType : public Type, public rangedInterface {
 protected:
   int low_;
   int hi_;
   //char *low;
   //char *hi;
 protected:
   //rangedType(const std::string &name, typeId_t ID, dataClass typeDes, int size, const char *low, const char *hi); 
   SYMTABEXPORT rangedType(std::string &name, typeId_t ID, dataClass typeDes, int size, int low, int hi);
   SYMTABEXPORT rangedType(std::string &name, dataClass typeDes, int size, int low, int hi);
 public:
   SYMTABEXPORT ~rangedType();
   SYMTABEXPORT bool operator==(const Type &) const;
   SYMTABEXPORT int getLow() const { return low_; }
   SYMTABEXPORT int getHigh() const { return hi_; }
};

class derivedType : public Type, public derivedInterface {
 protected:
   Type *baseType_;
 protected:
   SYMTABEXPORT derivedType(std::string &name, typeId_t id, int size, dataClass typeDes);
   SYMTABEXPORT derivedType(std::string &name, int size, dataClass typeDes);
 public:
   SYMTABEXPORT ~derivedType();
   SYMTABEXPORT bool operator==(const Type &) const;
   SYMTABEXPORT Type *getConstituentType() const;
};

// Derived classes from Type

class typeEnum : public Type {
 private:  
	std::vector<std::pair<std::string, int> *> consts;
 public:
   SYMTABEXPORT typeEnum(typeId_t ID, std::string name = "");
   SYMTABEXPORT typeEnum(std::string name);
   SYMTABEXPORT static typeEnum *create(std::string &name, std::vector<std::pair<std::string, int> *>&elements, 
   								Symtab *obj = NULL);
   SYMTABEXPORT static typeEnum *create(std::string &name, std::vector<std::string> &elementNames,
								Symtab *obj = NULL);
   SYMTABEXPORT bool addConstant(const std::string &fieldname,int value);
   SYMTABEXPORT std::vector<std::pair<std::string, int> *> &getConstants();
   SYMTABEXPORT bool setName(const char *name);
   SYMTABEXPORT bool isCompatible(Type *otype);
};

class typeFunction : public Type {
 protected:
   SYMTABEXPORT void fixupUnknowns(Module *);
 private:
   Type *retType_; /* Return type of the function */
   std::vector<Type *> params_; 
 public:
   SYMTABEXPORT typeFunction(typeId_t ID, Type *retType, std::string name = "");
   SYMTABEXPORT typeFunction(Type *retType, std::string name = "");
   SYMTABEXPORT static typeFunction *create(std::string &name, Type *retType, 
   				std::vector<Type *> &paramTypes, Symtab *obj = NULL);
   SYMTABEXPORT ~typeFunction();
   SYMTABEXPORT bool addParam( Type *type);
   SYMTABEXPORT Type *getReturnType() const;
   SYMTABEXPORT bool setRetType(Type *rtype);

   SYMTABEXPORT std::vector<Type *> &getParams();
   SYMTABEXPORT bool isCompatible(Type *otype);
};

class typeScalar : public Type {
 private:
   bool isSigned_;
 public:
   SYMTABEXPORT typeScalar(typeId_t ID, unsigned int size, std::string name = "", bool isSigned = false);
   SYMTABEXPORT typeScalar(unsigned int size, std::string name = "", bool isSigned = false);
   SYMTABEXPORT static typeScalar *create(std::string &name, int size, Symtab *obj = NULL);
   SYMTABEXPORT bool isSigned();
   SYMTABEXPORT bool isCompatible(Type *otype);
};

class typeCommon : public fieldListType {
 private:
   std::vector<CBlock *> cblocks;
 protected:
   SYMTABEXPORT void postFieldInsert(int nsize) { size_ += nsize; }
   //void postFieldInsert(int offset, int nsize) { if ((unsigned int) (offset + nsize) > size_) size_ = offset + nsize; }
   SYMTABEXPORT void fixupUnknowns(Module *);
 public:
   SYMTABEXPORT typeCommon(typeId_t ID, std::string name = "");
   SYMTABEXPORT typeCommon(std::string name);
   SYMTABEXPORT static typeCommon *create(std::string &name, Symtab *obj = NULL);
   SYMTABEXPORT std::vector<CBlock *> *getCblocks() const;
   SYMTABEXPORT void beginCommonBlock();
   SYMTABEXPORT void endCommonBlock(Symbol *, void *baseAddr);
};

class CBlock{
   friend class typeCommon;
 private:
   // the list of fields
   std::vector<Field *> fieldList;

   // which functions use this list
   std::vector<Symbol *> functions;

   void *upPtr_;
 
 public:
   SYMTABEXPORT std::vector<Field *> *getComponents();
   SYMTABEXPORT std::vector<Symbol *> *getFunctions();

   SYMTABEXPORT void fixupUnknowns(Module *);
   
   SYMTABEXPORT void *getUpPtr() const;
   SYMTABEXPORT bool setUpPtr(void *);
};

class typeStruct : public fieldListType {
 protected:
   SYMTABEXPORT void updateSize();
   SYMTABEXPORT void postFieldInsert(int nsize);
   SYMTABEXPORT void fixupUnknowns(Module *);
   SYMTABEXPORT void merge(Type *other);
 public:
   SYMTABEXPORT typeStruct(typeId_t ID, std::string name = "");
   SYMTABEXPORT typeStruct(std::string name);
   SYMTABEXPORT static typeStruct *create(std::string &name, std::vector< std::pair<std::string, Type *> *> &flds,
   				 				Symtab *obj = NULL);
   SYMTABEXPORT static typeStruct *create(std::string &name, std::vector<Field *> &fields, 
								Symtab *obj = NULL);

   SYMTABEXPORT bool isCompatible(Type *otype);
};

class typeUnion : public fieldListType {
 protected:
   SYMTABEXPORT void updateSize();
   SYMTABEXPORT void postFieldInsert(int nsize);
   SYMTABEXPORT void merge(Type *other);
   SYMTABEXPORT void fixupUnknowns(Module *);
 public:
   SYMTABEXPORT typeUnion(typeId_t ID, std::string name = "");
   SYMTABEXPORT typeUnion(std::string name);
   SYMTABEXPORT static typeUnion *create(std::string &name, std::vector<std::pair<std::string, Type *> *> &fieldNames,
   							Symtab *obj = NULL);
   SYMTABEXPORT static typeUnion *create(std::string &name, std::vector<Field *> &fields, 
							Symtab *obj = NULL);
   SYMTABEXPORT bool isCompatible(Type *otype);
};

class typePointer : public derivedType {
 protected: 
   SYMTABEXPORT void fixupUnknowns(Module *);
 public:
   SYMTABEXPORT typePointer(typeId_t ID, Type *ptr, std::string name = "");
   SYMTABEXPORT typePointer(Type *ptr, std::string name = "");
   SYMTABEXPORT static typePointer *create(std::string &name, Type *ptr, Symtab *obj = NULL);
   SYMTABEXPORT static typePointer *create(std::string &name, Type *ptr, int size, 
   							Symtab *obj = NULL);
   SYMTABEXPORT bool isCompatible(Type *otype);
   SYMTABEXPORT bool setPtr(Type *ptr);
};

class typeTypedef: public derivedType {
 private:
   unsigned int sizeHint_;
 
 protected:
   SYMTABEXPORT void updateSize();
   SYMTABEXPORT void fixupUnknowns(Module *);
      
 public:
   SYMTABEXPORT typeTypedef(typeId_t ID, Type *base, std::string name, unsigned int sizeHint = 0);
   SYMTABEXPORT typeTypedef(Type *base, std::string name, unsigned int sizeHint = 0);
   
   SYMTABEXPORT static typeTypedef *create(std::string &name, Type *ptr, Symtab *obj = NULL);
   SYMTABEXPORT bool isCompatible(Type *otype);
   SYMTABEXPORT bool operator==(const Type &otype) const;
};

class typeRef : public derivedType {
 protected:
   SYMTABEXPORT void fixupUnknowns(Module *);
 public:
   SYMTABEXPORT typeRef(typeId_t ID, Type *refType, std::string name);
   SYMTABEXPORT typeRef(Type *refType, std::string name);
   SYMTABEXPORT static typeRef *create(std::string &name, Type *ptr, Symtab * obj = NULL);
   SYMTABEXPORT bool isCompatible(Type *otype);
   SYMTABEXPORT bool operator==(const Type &otype) const;
};

class typeSubrange : public rangedType {
 private:
   //typeSubrange(int ID, int size, const char *low, const char *hi, const char *name);
 public:
   SYMTABEXPORT typeSubrange(typeId_t ID, int size, int low, int hi, std::string name);
   SYMTABEXPORT typeSubrange( int size, int low, int hi, std::string name);
   SYMTABEXPORT static typeSubrange *create(std::string &name, int size, int low, int hi, Symtab *obj = NULL);
   SYMTABEXPORT bool isCompatible(Type *otype);
};

class typeArray : public rangedType {
 private:
   Type *arrayElem;
   unsigned int sizeHint_;
 protected:
   SYMTABEXPORT void updateSize();
   SYMTABEXPORT void merge(Type *other); 
 public:
   SYMTABEXPORT typeArray(typeId_t ID, Type *base, int low, int hi, std::string name, unsigned int sizeHint = 0);
   SYMTABEXPORT typeArray(Type *base, int low, int hi, std::string name, unsigned int sizeHint = 0);
   SYMTABEXPORT static typeArray *create(std::string &name, Type *typ,  int low, int hi, Symtab *obj = NULL);
   SYMTABEXPORT Type *getBaseType() const;
   SYMTABEXPORT bool isCompatible(Type *otype);
   SYMTABEXPORT bool operator==(const Type &otype) const;
   SYMTABEXPORT void fixupUnknowns(Module *);
};

//location for a variable
typedef struct{
   storageClass stClass;
   storageRefClass refClass;
   int reg;
   long frameOffset;
   Address lowPC;
   Address hiPC;
} loc_t;


class SYMTABEXPORT localVar
{
   friend class typeCommon;
   friend class localVarCollection;
 
    std::string name_;
    Type *type_;
    std::string fileName_;
    int lineNum_;
    std::vector<loc_t *> *locs_;
    void *upPtr_;
    
    // scope_t scope;
  
  public:
    localVar() {}
      //  Internal use only
      localVar(std::string name,  Type *typ, std::string fileName, int lineNum, std::vector<loc_t *> *locs = NULL);
      // Copy constructor
      localVar(localVar &lvar);
      bool addLocation(loc_t *location);
      bool setLocation(std::vector<loc_t *> *locs);
      ~localVar();
      void fixupUnknown(Module *);
 public:
      //  end of functions for internal use only
      std::string &getName();
      Type *getType();
      bool setType(Type *newType);
      int  getLineNum();
      std::string &getFileName();
      std::vector<loc_t *> *getLocationLists();

      void *getUpPtr() const;
      bool setUpPtr(void *);
};

} // namespace SymtabAPI
} // namespace Dyninst
#endif
