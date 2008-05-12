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

#include <assert.h>
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

typedef int typeId_t;

typedef enum {visPrivate, visProtected, visPublic,
              visUnknown} visibility_t;
/*
 * visibility: Accessibility of member data and functions
 * These values follow the 'fieldname:' after the '/' identifier.
 * visPrivate   == 0 gnu Sun -- private
 * visProtected == 1 gnu Sun -- protected
 * visPublic    == 2 gnu Sun -- public
 * visUnknown visibility not known or doesn't apply(ANSIC), the default
 *
 */

typedef enum {
	storageAddr,
	storageReg,
	storageRegOffset
} storageClass;

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

class DLLEXPORT Field{
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
				  
class Type{
   friend class typeCollection;
   friend std::string parseStabString(Module *, int linenum, char *, int, 
                          typeCommon *);
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
   DLLEXPORT virtual void updateSize() {}

   // Simple Destructor
   DLLEXPORT virtual ~Type();

   DLLEXPORT virtual void merge( Type * /* other */ ) { }

public:
   DLLEXPORT virtual bool operator==(const Type &) const;
   DLLEXPORT virtual bool isCompatible(Type *oType);
   DLLEXPORT virtual void fixupUnknowns(Module *);

   DLLEXPORT Type(std::string name, typeId_t ID, dataClass dataTyp = dataNullType);
   DLLEXPORT Type(std::string name, dataClass dataTyp = dataNullType);

   // A few convenience functions
   DLLEXPORT static Type *createFake(std::string name);
   /* Placeholder for real type, to be filled in later */
   DLLEXPORT static Type *createPlaceholder(typeId_t ID, std::string name = "");
   
   DLLEXPORT typeId_t getID() const;
   DLLEXPORT unsigned int getSize();
   DLLEXPORT bool setSize(unsigned int size);
   DLLEXPORT std::string &getName();
   DLLEXPORT bool setName(std::string);

   DLLEXPORT bool setUpPtr(void *);
   DLLEXPORT void *getUpPtr() const;

   DLLEXPORT dataClass getDataClass() const;

   // INTERNAL METHODS
   DLLEXPORT void incrRefCount();
   DLLEXPORT void decrRefCount(); 
   
   
   //Methods to dynamically cast generic Type Object to specific types.
   
   DLLEXPORT typeEnum *getEnumType();
   DLLEXPORT typePointer *getPointerType();
   DLLEXPORT typeFunction *getFunctionType();
   DLLEXPORT typeSubrange *getSubrangeType();
   DLLEXPORT typeArray *getArrayType();
   DLLEXPORT typeStruct *getStructType();
   DLLEXPORT typeUnion *getUnionType();
   DLLEXPORT typeScalar *getScalarType();
   DLLEXPORT typeCommon *getCommonType();
   DLLEXPORT typeTypedef *getTypedefType();
   DLLEXPORT typeRef *getRefType();
};

// Interfaces to be implemented by intermediate subtypes
// We have to do this thanks to reference types and C++'s lovely 
// multiple inheritance

class fieldListInterface {
 public:
   DLLEXPORT virtual ~fieldListInterface() {};
   DLLEXPORT virtual std::vector<Field *> *getComponents() const = 0;
};

class rangedInterface {
 public:
   DLLEXPORT virtual ~rangedInterface() {};
   DLLEXPORT virtual int getLow() const = 0;
   DLLEXPORT virtual int getHigh() const  = 0;
};  

class derivedInterface{
 public:
   DLLEXPORT virtual ~derivedInterface() {};
   DLLEXPORT virtual Type *getConstituentType() const = 0;
};

// Intermediate types (interfaces + Type)

class fieldListType : public Type, public fieldListInterface {
 private:
   void fixupComponents();
 protected:
   std::vector<Field *> fieldList;
   std::vector<Field *> *derivedFieldList;
   DLLEXPORT fieldListType(std::string &name, typeId_t ID, dataClass typeDes);
   /* Each subclass may need to update its size after adding a field */
 public:
   DLLEXPORT ~fieldListType();
   DLLEXPORT bool operator==(const Type &) const;
   DLLEXPORT std::vector<Field *> *getComponents() const;
   
   DLLEXPORT std::vector<Field *> *getFields() const;
   
   DLLEXPORT virtual void postFieldInsert(int nsize) = 0;
   
   /* Add field for C++ struct or union */
   DLLEXPORT void addField(std::string fieldname, Type *type, int offsetVal = -1, visibility_t vis = visUnknown);
   DLLEXPORT void addField(unsigned num, std::string fieldname, Type *type, int offsetVal = -1, visibility_t vis = visUnknown);
   DLLEXPORT void addField(Field *fld);
   DLLEXPORT void addField(unsigned num, Field *fld);
  
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
   DLLEXPORT rangedType(std::string &name, typeId_t ID, dataClass typeDes, int size, int low, int hi);
   DLLEXPORT rangedType(std::string &name, dataClass typeDes, int size, int low, int hi);
 public:
   DLLEXPORT ~rangedType();
   DLLEXPORT bool operator==(const Type &) const;
   DLLEXPORT int getLow() const { return low_; }
   DLLEXPORT int getHigh() const { return hi_; }
};

class derivedType : public Type, public derivedInterface {
 protected:
   Type *baseType_;
 protected:
   DLLEXPORT derivedType(std::string &name, typeId_t id, int size, dataClass typeDes);
   DLLEXPORT derivedType(std::string &name, int size, dataClass typeDes);
 public:
   DLLEXPORT ~derivedType();
   DLLEXPORT bool operator==(const Type &) const;
   DLLEXPORT Type *getConstituentType() const;
};

// Derived classes from Type

class typeEnum : public Type {
 private:  
	std::vector<std::pair<std::string, int> *> consts;
 public:
   DLLEXPORT typeEnum(typeId_t ID, std::string name = "");
   DLLEXPORT typeEnum(std::string name);
   DLLEXPORT static typeEnum *create(std::string &name, std::vector<std::pair<std::string, int> *>&elements, 
   								Symtab *obj = NULL);
   DLLEXPORT static typeEnum *create(std::string &name, std::vector<std::string> &elementNames,
								Symtab *obj = NULL);
   DLLEXPORT bool addConstant(const std::string &fieldname,int value);
   DLLEXPORT std::vector<std::pair<std::string, int> *> &getConstants();
   DLLEXPORT bool setName(const char *name);
   DLLEXPORT bool isCompatible(Type *otype);
};

class typeFunction : public Type {
 protected:
   DLLEXPORT void fixupUnknowns(Module *);
 private:
   Type *retType_; /* Return type of the function */
   std::vector<Type *> params_; 
 public:
   DLLEXPORT typeFunction(typeId_t ID, Type *retType, std::string name = "");
   DLLEXPORT typeFunction(Type *retType, std::string name = "");
   DLLEXPORT static typeFunction *create(std::string &name, Type *retType, 
   				std::vector<Type *> &paramTypes, Symtab *obj = NULL);
   DLLEXPORT ~typeFunction();
   DLLEXPORT bool addParam( Type *type);
   DLLEXPORT Type *getReturnType() const;
   DLLEXPORT bool setRetType(Type *rtype);

   DLLEXPORT std::vector<Type *> &getParams();
   DLLEXPORT bool isCompatible(Type *otype);
};

class typeScalar : public Type {
 private:
   bool isSigned_;
 public:
   DLLEXPORT typeScalar(typeId_t ID, unsigned int size, std::string name = "", bool isSigned = false);
   DLLEXPORT typeScalar(unsigned int size, std::string name = "", bool isSigned = false);
   DLLEXPORT static typeScalar *create(std::string &name, int size, Symtab *obj = NULL);
   DLLEXPORT bool isSigned();
   DLLEXPORT bool isCompatible(Type *otype);
};

class typeCommon : public fieldListType {
 private:
   std::vector<CBlock *> cblocks;
 protected:
   DLLEXPORT void postFieldInsert(int nsize) { size_ += nsize; }
   //void postFieldInsert(int offset, int nsize) { if ((unsigned int) (offset + nsize) > size_) size_ = offset + nsize; }
   DLLEXPORT void fixupUnknowns(Module *);
 public:
   DLLEXPORT typeCommon(typeId_t ID, std::string name = "");
   DLLEXPORT typeCommon(std::string name);
   DLLEXPORT static typeCommon *create(std::string &name, Symtab *obj = NULL);
   DLLEXPORT std::vector<CBlock *> *getCblocks() const;
   DLLEXPORT void beginCommonBlock();
   DLLEXPORT void endCommonBlock(Symbol *, void *baseAddr);
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
   DLLEXPORT std::vector<Field *> *getComponents();
   DLLEXPORT std::vector<Symbol *> *getFunctions();

   DLLEXPORT void fixupUnknowns(Module *);
   
   DLLEXPORT void *getUpPtr() const;
   DLLEXPORT bool setUpPtr(void *);
};

class typeStruct : public fieldListType {
 protected:
   DLLEXPORT void updateSize();
   DLLEXPORT void postFieldInsert(int nsize);
   DLLEXPORT void fixupUnknowns(Module *);
   DLLEXPORT void merge(Type *other);
 public:
   DLLEXPORT typeStruct(typeId_t ID, std::string name = "");
   DLLEXPORT typeStruct(std::string name);
   DLLEXPORT static typeStruct *create(std::string &name, std::vector< std::pair<std::string, Type *> *> &flds,
   				 				Symtab *obj = NULL);
   DLLEXPORT static typeStruct *create(std::string &name, std::vector<Field *> &fields, 
								Symtab *obj = NULL);

   DLLEXPORT bool isCompatible(Type *otype);
};

class typeUnion : public fieldListType {
 protected:
   DLLEXPORT void updateSize();
   DLLEXPORT void postFieldInsert(int nsize);
   DLLEXPORT void merge(Type *other);
   DLLEXPORT void fixupUnknowns(Module *);
 public:
   DLLEXPORT typeUnion(typeId_t ID, std::string name = "");
   DLLEXPORT typeUnion(std::string name);
   DLLEXPORT static typeUnion *create(std::string &name, std::vector<std::pair<std::string, Type *> *> &fieldNames,
   							Symtab *obj = NULL);
   DLLEXPORT static typeUnion *create(std::string &name, std::vector<Field *> &fields, 
							Symtab *obj = NULL);
   DLLEXPORT bool isCompatible(Type *otype);
};

class typePointer : public derivedType {
 protected: 
   DLLEXPORT void fixupUnknowns(Module *);
 public:
   DLLEXPORT typePointer(typeId_t ID, Type *ptr, std::string name = "");
   DLLEXPORT typePointer(Type *ptr, std::string name = "");
   DLLEXPORT static typePointer *create(std::string &name, Type *ptr, Symtab *obj = NULL);
   DLLEXPORT static typePointer *create(std::string &name, Type *ptr, int size, 
   							Symtab *obj = NULL);
   DLLEXPORT bool isCompatible(Type *otype);
   DLLEXPORT bool setPtr(Type *ptr);
};

class typeTypedef: public derivedType {
 private:
   unsigned int sizeHint_;
 
 protected:
   DLLEXPORT void updateSize();
   DLLEXPORT void fixupUnknowns(Module *);
      
 public:
   DLLEXPORT typeTypedef(typeId_t ID, Type *base, std::string name, unsigned int sizeHint = 0);
   DLLEXPORT typeTypedef(Type *base, std::string name, unsigned int sizeHint = 0);
   
   DLLEXPORT static typeTypedef *create(std::string &name, Type *ptr, Symtab *obj = NULL);
   DLLEXPORT bool isCompatible(Type *otype);
   DLLEXPORT bool operator==(const Type &otype) const;
};

class typeRef : public derivedType {
 protected:
   DLLEXPORT void fixupUnknowns(Module *);
 public:
   DLLEXPORT typeRef(typeId_t ID, Type *refType, std::string name);
   DLLEXPORT typeRef(Type *refType, std::string name);
   DLLEXPORT static typeRef *create(std::string &name, Type *ptr, Symtab * obj = NULL);
   DLLEXPORT bool isCompatible(Type *otype);
   DLLEXPORT bool operator==(const Type &otype) const;
};

class typeSubrange : public rangedType {
 private:
   //typeSubrange(int ID, int size, const char *low, const char *hi, const char *name);
 public:
   DLLEXPORT typeSubrange(typeId_t ID, int size, int low, int hi, std::string name);
   DLLEXPORT typeSubrange( int size, int low, int hi, std::string name);
   DLLEXPORT static typeSubrange *create(std::string &name, int size, int low, int hi, Symtab *obj = NULL);
   DLLEXPORT bool isCompatible(Type *otype);
};

class typeArray : public rangedType {
 private:
   Type *arrayElem;
   unsigned int sizeHint_;
 protected:
   DLLEXPORT void updateSize();
   DLLEXPORT void merge(Type *other); 
 public:
   DLLEXPORT typeArray(typeId_t ID, Type *base, int low, int hi, std::string name, unsigned int sizeHint = 0);
   DLLEXPORT typeArray(Type *base, int low, int hi, std::string name, unsigned int sizeHint = 0);
   DLLEXPORT static typeArray *create(std::string &name, Type *typ,  int low, int hi, Symtab *obj = NULL);
   DLLEXPORT Type *getBaseType() const;
   DLLEXPORT bool isCompatible(Type *otype);
   DLLEXPORT bool operator==(const Type &otype) const;
   DLLEXPORT void fixupUnknowns(Module *);
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

class DLLEXPORT localVar
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
