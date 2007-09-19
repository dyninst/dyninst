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

// $Id: addressSpace.h,v 1.2 2007/09/19 19:25:06 bernat Exp $

#ifndef ADDRESS_SPACE_H
#define ADDRESS_SPACE_H



#include "infHeap.h"
#include "codeRange.h"
#include "InstructionSource.h"
#include "ast.h"

class codeRange;
class multiTramp;
class replacedFunctionCall;
class functionReplacement;

class int_function;
class int_variable;
class mapped_module;
class mapped_object;
class instPoint;

class BPatch_process;
class BPatch_function;
class BPatch_point;

class Emitter;
class generatedCodeObject;

class relocationEntry;
class int_function;

class BinaryEdit;

// This file serves to define an "address space", a set of routines that 
// code generation and instrumentation rely on to perform their duties. 
// This was derived from the process class and serves as a parent to that
// class and the static_space class for the rewriter. 

// The methods in this class were determined by what the code currently
// uses, not a particular design. As such, I expect this to change wildly
// by the time the refactoring is complete. 
//
// bernat, 5SEP07

// Note: this is a pure virtual class; it serves as an interface
// specification.

class AddressSpace : public InstructionSource {
 public:
    
    // Down-conversion functions
    process *proc();
    BinaryEdit *edit();

    // Read/write

    // We have read/write for both "text" and "data". This comes in handy,
    // somewhere, I'm sure
    virtual bool readDataSpace(const void *inOther, 
                               u_int amount, 
                               void *inSelf, 
                               bool showError) = 0;
    virtual bool readTextSpace(const void *inOther, 
                               u_int amount, 
                               const void *inSelf) = 0;
    

    virtual bool writeDataSpace(void *inOther,
                                u_int amount,
                                const void *inSelf) = 0;
    virtual bool writeTextSpace(void *inOther,
                                u_int amount,
                                const void *inSelf) = 0;

    // Memory allocation
    // We don't specify how it should be done, only that it is. The model is
    // that you ask for an allocation "near" a point, where "near" has an
    // internal, platform-specific definition. The allocation mechanism does its
    // best to give you what you want, but there are no promises - check the
    // address of the returned buffer to be sure.

    virtual Address inferiorMalloc(unsigned size, inferiorHeapType type=anyHeap,
                                   Address near = 0, bool *err = NULL) = 0;
    void inferiorFree(Address item);
    // And a "constrain" call to free unused memory. This is useful because our
    // instrumentation is incredibly wasteful.
    virtual bool inferiorRealloc(Address item, unsigned newSize);

    bool isInferiorAllocated(Address block);

    // Get the pointer size of the app we're modifying
    virtual unsigned getAddressWidth() const = 0;

    // We need a mechanism to track what exists at particular addresses in the
    // address space - both for lookup and to ensure that there are no collisions.
    // We have a multitude of ways to "muck with" the application (function replacement,
    // instrumentation, function relocation, ...) and they can all stomp on each
    // other. 

    void addOrigRange(codeRange *range);
    void addModifiedRange(codeRange *range);

    void removeOrigRange(codeRange *range);
    void removeModifiedRange(codeRange *range);

    codeRange *findOrigByAddr(Address addr);
    codeRange *findModByAddr(Address addr);

    virtual void *getPtrToInstruction(Address) const;
    virtual bool isValidAddress(const Address &) const;

    // Trap address to base tramp address (for trap instrumentation)
    dictionary_hash<Address, Address> trampTrapMapping;
    
    // Should return iterators
    bool getOrigRanges(pdvector<codeRange *> &);
    bool getModifiedRanges(pdvector<codeRange *> &);

    // Multitramp convenience functions
    multiTramp *findMultiTrampByAddr(Address addr);
    multiTramp *findMultiTrampById(unsigned int id);
    void addMultiTramp(multiTramp *multi);
    void removeMultiTramp(multiTramp *multi);

    // Function replacement (or relocated, actually) convenience functions
    functionReplacement *findFuncReplacement(Address addr);
    void addFuncReplacement(functionReplacement *funcrep);
    void removeFuncReplacement(functionReplacement *funcrep);

    // Function call replacement convenience functions
    replacedFunctionCall *findReplacedCall(Address addr);
    void addReplacedCall(replacedFunctionCall *rep);
    void removeReplacedCall(replacedFunctionCall *rep);

    //////////////////////////////////////////////////////////////
    // Function/variable lookup code
    // Turns out that instrumentation needs this... so the 
    // AddressSpace keeps growing. 
    //////////////////////////////////////////////////////////////

    // findFuncByName: returns function associated with "func_name"
    // This routine checks both the a.out image and any shared object images 
    // for this function
    //int_function *findFuncByName(const pdstring &func_name);
    
    bool findFuncsByAll(const pdstring &funcname,
                        pdvector<int_function *> &res,
                        const pdstring &libname = "");
    
    // Specific versions...
    bool findFuncsByPretty(const pdstring &funcname,
                           pdvector<int_function *> &res,
                           const pdstring &libname = "");
    bool findFuncsByMangled(const pdstring &funcname, 
                            pdvector<int_function *> &res,
                            const pdstring &libname = "");
    
    bool findVarsByAll(const pdstring &varname,
                       pdvector<int_variable *> &res,
                       const pdstring &libname = "");
    
    // And we often internally want to wrap the above to return one
    // and only one func...
    int_function *findOnlyOneFunction(const pdstring &name,
                                      const pdstring &libname = "");

    // getAllFunctions: returns a vector of all functions defined in the
    // a.out and in the shared objects
    void getAllFunctions(pdvector<int_function *> &);
    
    // Find the code sequence containing an address
    // Note: fix the name....
    int_function *findFuncByAddr(Address addr);
    int_basicBlock *findBasicBlockByAddr(Address addr);
    
    // And a lookup by "internal" function to find clones during fork...
    int_function *findFuncByInternalFunc(image_func *ifunc);
    
    //findJumpTargetFuncByAddr Acts like findFunc, but if it fails,
    // checks if 'addr' is a jump to a function.
    int_function *findJumpTargetFuncByAddr(Address addr);
    
    // findModule: returns the module associated with "mod_name" 
    // this routine checks both the a.out image and any shared object 
    // images for this module
    // if check_excluded is true it checks to see if the module is excluded
    // and if it is it returns 0.  If check_excluded is false it doesn't check
    //  if substring_match is true, the first module whose name contains
    //  the provided string is returned.
    // Wildcard: handles "*" and "?"
    mapped_module *findModule(const pdstring &mod_name, bool wildcard = false);
    // And the same for objects
    // Wildcard: handles "*" and "?"
    mapped_object *findObject(const pdstring &obj_name, bool wildcard = false);
    mapped_object *findObject(Address addr);

    mapped_object *getAOut() { assert(mapped_objects.size()); return mapped_objects[0];}
    
    // getAllModules: returns a vector of all modules defined in the
    // a.out and in the shared objects
    void getAllModules(pdvector<mapped_module *> &);

    // return the list of dynamically linked libs
    const pdvector<mapped_object *> &mappedObjects() { return mapped_objects;  } 
    
    // If true is passed for ignore_if_mt_not_set, then an error won't be
    // initiated if we're unable to determine if the program is multi-threaded.
    // We are unable to determine this if the daemon hasn't yet figured out
    // what libraries are linked against the application.  Currently, we
    // identify an application as being multi-threaded if it is linked against
    // a thread library (eg. libpthreads.a on AIX).  There are cases where we
    // are querying whether the app is multi-threaded, but it can't be
    // determined yet but it also isn't necessary to know.
    virtual bool multithread_capable(bool ignore_if_mt_not_set = false) = 0;
    
    // Do we have the RT-side multithread functions available
    virtual bool multithread_ready(bool ignore_if_mt_not_set = false) = 0;

    //////////////////////////////////////////////////////
    // Process-level instrumentation (?)
    /////////////////////////////////////////////////////

    // instPoint isn't const; it may get an updated list of
    // instances since we generate them lazily.
    // Shouldn't this be an instPoint member function?
    bool replaceFunctionCall(instPoint *point,const int_function *newFunc);
    
    // And this....
    bool getDynamicCallSiteArgs(instPoint *callSite, 
                                pdvector<AstNodePtr> &args);

    // Default to "nope"
    virtual bool hasBeenBound(const relocationEntry &, 
                              int_function *&, 
                              Address) { return false; }
    
    // Trampoline guard get/set functions
    Address trampGuardBase(void) { return trampGuardBase_; }
    AstNodePtr trampGuardAST(void);

    // Get the current code generator (or emitter)
    Emitter *getEmitter();

    // Should be easy if the process isn't _executing_ where
    // we're deleting...
    virtual void deleteGeneratedCode(generatedCodeObject *del);

    //////////////////////////////////////////////////////
    // BPatch-level stuff
    //////////////////////////////////////////////////////
    // Callbacks for higher level code (like BPatch) to learn about new 
    //  functions and InstPoints.
 private:
    BPatch_function *(*new_func_cb)(AddressSpace *a, int_function *f);
    BPatch_point *(*new_instp_cb)(AddressSpace *a, int_function *f, instPoint *ip, 
                                  int type);
 public:
    //Trigger the callbacks from a lower level
    BPatch_function *newFunctionCB(int_function *f) 
        { assert(new_func_cb); return new_func_cb(this, f); }
    BPatch_point *newInstPointCB(int_function *f, instPoint *pt, int type)
        { assert(new_instp_cb); return new_instp_cb(this, f, pt, type); }
    
    //Register callbacks from the higher level
    void registerFunctionCallback(BPatch_function *(*f)(AddressSpace *p, 
                                                        int_function *f))
        { new_func_cb = f; };
    void registerInstPointCallback(BPatch_point *(*f)(AddressSpace *p, int_function *f,
                                                      instPoint *ip, int type))
        { new_instp_cb = f; }
    
    
    //Anonymous up pointer to the containing process.  This is BPatch_process
    // in Dyninst.  Currently stored as an void pointer in case we do
    // anything with this during the library split.
    void *up_ptr() { return up_ptr_; }
    void set_up_ptr(void *ptr) { up_ptr_ = ptr; }
    
    //////////////////////////////////////////////////////
    // Internal and cleanup 
    //////////////////////////////////////////////////////

    // Clear things out (e.g., deleteProcess)
    void deleteAddressSpace();
    // Fork psuedo-constructor
    void copyAddressSpace(process *parent);

    // Aaand constructor/destructor
    AddressSpace();
    virtual ~AddressSpace();


    //////////////////////////////////////////////////////
    // Yuck
    //////////////////////////////////////////////////////
    Address getObservedCostAddr() const { return costAddr_; }
    void updateObservedCostAddr(Address addr) { costAddr_ = addr;}


 protected:

    // inferior malloc support functions
    void inferiorFreeCompact();
    int findFreeIndex(unsigned size, int type, Address lo, Address hi);
    void addHeap(heapItem *h);
    void initializeHeap();
    
    // Centralization of certain inferiorMalloc operations
    Address inferiorMallocInternal(unsigned size, Address lo, Address hi, 
                                   inferiorHeapType type);
    void inferiorMallocAlign(unsigned &size);

    bool heapInitialized_;
    inferiorHeap heap_;

    // Text sections (including added - instrumentation)
    codeRangeTree textRanges_;
    // Data sections
    codeRangeTree dataRanges_;
    // And address-space-wide patches that we've dropped in
    codeRangeTree modifiedRanges_;

    // We label multiTramps by ID
    dictionary_hash<int, multiTramp *> multiTrampsById_;

    // Loaded mapped objects (may be just 1)
    pdvector<mapped_object *> mapped_objects;

    Address trampGuardBase_; // Tramp recursion index mapping
    AstNodePtr trampGuardAST_;

    void *up_ptr_;

    Address costAddr_;

    
};

extern int heapItemCmpByAddr(const heapItem **A, const heapItem **B);

#endif // ADDRESS_SPACE_H
