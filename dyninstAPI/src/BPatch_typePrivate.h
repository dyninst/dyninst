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

#ifndef _BPatch_typePrivate_h_
#define _BPatch_typePrivate_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include <string.h>	
#include "BPatch_type.h"

class BPatch_function;

// Interfaces to be implemented by intermediate subtypes
// We have to do this thanks to reference types and C++'s lovely 
// multiple inheritance

class BPATCH_DLL_EXPORT BPatch_fieldListInterface {
 public:
   virtual ~BPatch_fieldListInterface() {};
   virtual BPatch_Vector<BPatch_field *> * getComponents() const = 0;
};

class BPATCH_DLL_EXPORT BPatch_rangedInterface {
 public:
   virtual ~BPatch_rangedInterface() {};
   virtual const char *getLow() const = 0;
   virtual const char *getHigh() const  = 0;
};  

// Intermediate types (interfaces + BPatch_type)

class BPATCH_DLL_EXPORT BPatch_fieldListType : public BPatch_type, public BPatch_fieldListInterface {
 private:
   void fixupComponents();
 protected:
   BPatch_Vector<BPatch_field *> fieldList;
   BPatch_Vector<BPatch_field *> *derivedFieldList;
   BPatch_fieldListType(const char *name, int _ID, BPatch_dataClass type);
   /* Each subclass may need to update its size after adding a field */
   virtual void postFieldInsert(int offset, int nsize) = 0;
 public:
   ~BPatch_fieldListType();
   bool operator==(const BPatch_type &) const;
   BPatch_Vector<BPatch_field *> * getComponents() const;
   /* Add field for Enum */
   void addField(const char *_fieldname, int value);
   /* Add field for C++(has visibility) Enum */
   void addField(const char * _fieldname,  BPatch_dataClass _typeDes, int value,
                 BPatch_visibility _vis);
   /* Add field for C++ struct or union */
   void addField(const char * _fieldname,  BPatch_dataClass _typeDes, 
                 BPatch_type *_type, int _offset, int _size);
   /* Add field for C++(has visibility) struct or union */
   void addField(const char * _fieldname,  BPatch_dataClass _typeDes, 
                 BPatch_type *_type, int _offset, int _size, BPatch_visibility _vis);
   /* Add field for Function parameter */
   void addField(BPatch_type *_type);
};

class BPATCH_DLL_EXPORT BPatch_rangedType : public BPatch_type, public BPatch_rangedInterface {
 protected:
   char *low;
   char *hi;
 protected:
   BPatch_rangedType(const char *_name, int _ID, BPatch_dataClass _class, int _size, const char *_low, const char *_hi); 
   BPatch_rangedType(const char *_name, int _ID, BPatch_dataClass _class, int _size, int _low, int _hi);
 public:
   ~BPatch_rangedType();
   bool operator==(const BPatch_type &) const;
   const char *getLow() const { return low; }
   const char *getHigh() const { return hi; }
};

// Derived classes from BPatch_type

class BPATCH_DLL_EXPORT BPatch_typeEnum : public BPatch_fieldListType {
 protected:
   void postFieldInsert(int,int) {}
 public:
   BPatch_typeEnum(int _ID, const char *_name = NULL);
   BPatch_typeEnum(const char *_name);
   bool isCompatible(const BPatch_type *otype) const;
};

class BPATCH_DLL_EXPORT BPatch_typePointer : public BPatch_type {
 private:
   BPatch_type *ptr; /* Type that we point to */
 public:
   BPatch_typePointer(int _ID, BPatch_type *_ptr, const char *_name = NULL);
   BPatch_typePointer(BPatch_type *_ptr, const char *_name);
   ~BPatch_typePointer() { ptr->decrRefCount(); }
   BPatch_type *getConstituentType() const { return ptr; }
   bool isCompatible(const BPatch_type *otype) const;
   void fixupUnknowns(BPatch_module *);
};

class BPATCH_DLL_EXPORT BPatch_typeFunction : public BPatch_fieldListType {
 protected:
   void postFieldInsert(int, int) {}
 private:
   BPatch_type *retType; /* Return type of the function */
 public:
   BPatch_typeFunction(int _ID, BPatch_type *_retType, const char *_name = NULL);
   ~BPatch_typeFunction() { retType->decrRefCount(); }
   BPatch_type *getConstituentType() const { return retType; }
   bool isCompatible(const BPatch_type *otype) const;
   void fixupUnknowns(BPatch_module *);
};

class BPATCH_DLL_EXPORT BPatch_typeRange : public BPatch_rangedType {
 public:
   BPatch_typeRange(int _ID, int _size, const char *_low, const char *_hi, const char *_name);
   bool isCompatible(const BPatch_type *otype) const;
};

class BPATCH_DLL_EXPORT BPatch_typeArray : public BPatch_rangedType {
 private:
   BPatch_type *arrayElem;
   unsigned int sizeHint;
 protected:
   void updateSize();
 public:
   bool operator==(const BPatch_type &otype) const;
   void merge(BPatch_type *other);
   BPatch_typeArray(int _ID, BPatch_type *_base, int _low, int _hi, const char *_name, unsigned int sizeHint = 0);
   BPatch_typeArray(BPatch_type *_base, int _low, int _hi, const char *_name);
   ~BPatch_typeArray() { arrayElem->decrRefCount(); }
   BPatch_type *getConstituentType() const { return arrayElem; }
   bool isCompatible(const BPatch_type *otype) const;
   void fixupUnknowns(BPatch_module *);
};

class BPATCH_DLL_EXPORT BPatch_typeStruct : public BPatch_fieldListType {
 protected:
   void updateSize();
   void postFieldInsert(int, int nsize) { size += nsize; }
 public:
   void merge(BPatch_type *other);
   BPatch_typeStruct(int _ID, const char *_name = NULL);
   BPatch_typeStruct(const char *_name);
   bool isCompatible(const BPatch_type *otype) const;
   void fixupUnknowns(BPatch_module *);
};

class BPATCH_DLL_EXPORT BPatch_typeUnion : public BPatch_fieldListType {
 private:
   BPatch_Vector<BPatch_field *> fieldList;
 protected:
   void updateSize();
   void postFieldInsert(int, int nsize) { if ((unsigned int) nsize > size) size = nsize; }
 public:
   void merge(BPatch_type *other);
   BPatch_typeUnion(int _ID, const char *_name = NULL);
   BPatch_typeUnion(const char *name);
   bool isCompatible(const BPatch_type *otype) const;
   void fixupUnknowns(BPatch_module *);
};

class BPATCH_DLL_EXPORT BPatch_typeScalar : public BPatch_type {
 public:
   BPatch_typeScalar(int _ID, unsigned int _size, const char *_name = NULL);
   BPatch_typeScalar(unsigned int _size, const char *_name);
   bool isCompatible(const BPatch_type *otype) const;
};

class BPATCH_DLL_EXPORT BPatch_typeCommon : public BPatch_fieldListType {
 private:
   BPatch_Vector<BPatch_cblock *> cblocks;
 protected:
   void postFieldInsert(int offset, int nsize) { if ((unsigned int) (offset + nsize) > size) size = offset + nsize; }
 public:
   BPatch_typeCommon(int _ID, const char *_name = NULL);
   BPatch_typeCommon(const char *_name);
   BPatch_Vector<BPatch_cblock *> *getCblocks() const { return const_cast<BPatch_Vector<BPatch_cblock*>*>(&cblocks); }
   void beginCommonBlock();
   void endCommonBlock(BPatch_function *, void *baseAddr);
   void fixupUnknowns(BPatch_module *);
};

// Typedefs and eferences are special, because we have to treat 
// them as a union of BPatch_type, BPatch_fieldListType, and BPatch_rangedType

class BPATCH_DLL_EXPORT BPatch_typeTypedef : public BPatch_type, public BPatch_fieldListInterface, public BPatch_rangedInterface {
 private:
   BPatch_type *base; /* The type that we refer to */
   unsigned int sizeHint;
 protected:
   void updateSize() { size = sizeHint ? sizeHint : base->getSize(); }
 public:
   BPatch_typeTypedef(int _ID, BPatch_type *_base, const char *_name = NULL, unsigned int _sizeHint = 0);
   BPatch_typeTypedef(BPatch_type *_base, const char *_name);
   ~BPatch_typeTypedef() { base->decrRefCount(); }
   bool operator==(const BPatch_type &otype) const;
   bool isCompatible(const BPatch_type *otype) const { return base->isCompatible(otype); }
   BPatch_type *getConstituentType() const { return base; }
   // Overriding the interfaces to use the underlying object
   BPatch_Vector<BPatch_field *> * getComponents() const;
   const char *getLow() const;
   const char *getHigh() const;
   void fixupUnknowns(BPatch_module *);
};

class BPATCH_DLL_EXPORT BPatch_typeRef : public BPatch_type, public BPatch_fieldListInterface, public BPatch_rangedInterface {
 private:
   BPatch_type *refType; /* Type that we refer to */
 public:
   BPatch_typeRef(int _ID, BPatch_type *_refType, const char *_name = NULL);
   ~BPatch_typeRef() { refType->decrRefCount(); }
   bool operator==(const BPatch_type &otype) const;
   bool isCompatible(const BPatch_type *otype) const;
   BPatch_type *getConstituentType() const { return refType; }
   // Overriding the interfaces to use the underlying object
   BPatch_Vector<BPatch_field *> * getComponents() const;
   const char *getLow() const;
   const char *getHigh() const;
   void fixupUnknowns(BPatch_module *);
};

#endif
