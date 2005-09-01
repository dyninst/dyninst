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

// $Id: mapped_object.h,v 1.4 2005/09/01 22:18:32 bernat Exp $

#if !defined(_mapped_object_h)
#define _mapped_object_h

#include "common/h/String.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/Object.h"
#ifndef BPATCH_LIBRARY
#include "paradynd/src/mdld.h"
#endif

#ifndef BPATCH_LIBRARY
#include "paradynd/src/resource.h"

#define CHECK_ALL_CALL_POINTS  // we depend on this for Paradyn
#endif

class mapped_module;

class int_variable {
    // Should subclass this and function off the same thing...

 private:
    int_variable() {};
 public:
    int_variable(image_variable *var, 
                 Address base,
                 mapped_module *mod);

    int_variable(int_variable *parVar, mapped_module *child);

    Address getAddress() const { return addr_; }
    // Can variables have multiple names?
    const pdvector<pdstring> &prettyNameVector() const;
    const pdvector<pdstring> &symTabNameVector() const;
    mapped_module *mod() const { return mod_; };

    const image_variable *ivar() const { return ivar_; }

    Address addr_;
    unsigned size_;
    // type?
    image_variable *ivar_;

    mapped_module *mod_;
};


/*
 * A class for link map information about a shared object that is mmapped 
 * by the dynamic linker into the applications address space at runtime. 
 */
#define 	SHAREDOBJECT_NOCHANGE	0
#define 	SHAREDOBJECT_ADDED	1
#define 	SHAREDOBJECT_REMOVED	2


// mapped_object represents a file in memory. It will be a collection
// of modules (basically, .o's) that are referred to as a unit and
// loaded as a unit.  The big reason for this is 1) per-process
// specialization and 2) a way to reduce memory; to create objects for
// all functions ahead of time is wasteful and expensive. So
// basically, the mapped_object "wins" if it can return useful
// information without having to allocate memory.

class mapped_object : public codeRange {
    friend class mapped_module; // for findFunction
    friend class int_function;
 private:
    mapped_object();
    mapped_object(fileDescriptor fileDesc, 
                  image *img,
		  process *proc);

 public:
    // We need a way to check for errors; hence a "get" method
    static mapped_object *createMappedObject(fileDescriptor desc,
                                             process *p);

    // Copy constructor: for forks
    mapped_object(const mapped_object *par_obj, process *child);

    // Will delete all int_functions which were originally part of this object; including 
    // any that were relocated (we can always follow the "I was relocated" pointer).
    ~mapped_object();

    bool analyze();

    const fileDescriptor &getFileDesc() const { return desc_; }
    // Full name, including path
    const pdstring &fullName() const { return fullName_; }
    const pdstring &fileName() const { return fileName_; }
    Address codeAbs() const { return codeBase() + codeOffset(); }
    Address codeBase() const { return codeBase_; }
    Address codeOffset() const { return parse_img()->codeOffset(); }
    unsigned codeSize() const { return parse_img()->codeLength(); }

    // Deprecated...
    Address getBaseAddress() const { return codeBase(); }

    Address dataAbs() const { return dataBase() + dataOffset(); }
    Address dataBase() const { return dataBase_; }
    Address dataOffset() const { return parse_img()->dataOffset(); }
    unsigned dataSize() const { return parse_img()->dataLength(); }

    image *parse_img() const { return image_; }
    bool isSharedLib() const;

    // Return an appropriate identification string for debug purposes.
    // Will eventually be required by a debug base class.
    const pdstring debugString() const;

    // Used for codeRange ONLY! DON'T USE THIS! BAD USER!
    Address get_address_cr() const { return codeAbs(); }
    unsigned get_size_cr() const { return codeSize(); }

    process *proc() const;

    mapped_module *findModule(pdstring m_name, bool wildcard = false);
    mapped_module *findModule(pdmodule *mod);

    // This way we can avoid parsing everything as it comes in; we
    // only care about existence and address, and only if they're in
    // the symbol table.
    class foundHeapDesc {
    public:
        pdstring name;
        Address addr;
    };

    void getInferiorHeaps(pdvector<foundHeapDesc> &foundHeaps) const;

    // codeRange method
    void *getPtrToInstruction(Address addr) const;
    void *getPtrToData(Address addr) const;

    // Try to avoid using these if you can, since they'll trigger
    // parsing and allocation. 
    bool getAllFunctions(pdvector<int_function *> &funcs);
    bool getAllVariables(pdvector<int_variable *> &vars);

    const pdvector<mapped_module *> &getModules();

#if defined(cap_save_the_world)
    bool isinText(Address addr){ 
        return ((addr >= codeBase_) && (addr < (codeBase_ + codeSize())));
    }
    void openedWithdlopen() { dlopenUsed = true; }; 
    bool isopenedWithdlopen() { return dlopenUsed; };
#endif

    // Annoying low-level requirement... direct access to the symbol table.
    bool  getSymbolInfo(const pdstring &n,Symbol &info);

    // All name lookup functions are vectorized, because you can have
    // multiple overlapping names for all sorts of reasons.
    // Demangled/"pretty": easy overlap (overloaded funcs, etc.).
    // Mangled: multiple modules with static/private functions and
    // we've lost the module name.

    const pdvector<int_function *> *findFuncVectorByPretty(const pdstring &funcname);
    const pdvector<int_function *> *findFuncVectorByMangled(const pdstring &funcname); 

    int_function *findFuncByAddr(const Address &address);
    codeRange *findCodeRangeByAddress(const Address &address);

    const pdvector<int_variable *> *findVarVectorByPretty(const pdstring &varname);
    const pdvector<int_variable *> *findVarVectorByMangled(const pdstring &varname); 
    

#if defined(cap_save_the_world)
	//this marks the shared object as dirty, mutated
	//so it needs saved back to disk during saveworld 

	void setDirty(){ dirty_=true;}
	bool isDirty() { return dirty_; }


	//ccw 24 jul 2003
	//This marks the shared library as one that contains functions
	//that are called by instrumentation.  These functions, and hence
	//this shared library, MUST be reloaded in the same place.  The
	//shared library is not necessarily mutated itself, so it may not
	//need to be saved (as dirty_ would imply).
	void setDirtyCalled() { dirtyCalled_ = true; }
	bool isDirtyCalled() { return dirtyCalled_; }
#endif


    //
    //     PRIVATE DATA MEMBERS
    //				
private:
    fileDescriptor desc_; // full file descriptor

    pdstring  fullName_;	// full file name of the shared object
    pdstring  fileName_; // name of shared object as it should be identified
			//  in mdl, e.g. as used for "exclude"....
    Address   codeBase_; // The OS offset where the text segment is loaded;
    // there is a corresponding codeOffset_ in the image class.

    // For example, an a.out often has a codeBase of 0, and a
    // codeOffset of 0x<valid>. Libraries are the reverse; codeBase_
    // of <valid>, codeOffset of 0. All of our incoming functions,
    // etc. from the image class have codeOffset built in.

    Address   dataBase_; // Where the data starts...

    void set_short_name();

    pdvector<mapped_module *> everyModule;

    dictionary_hash<const image_func *, int_function *> everyUniqueFunction;
    dictionary_hash<const image_variable *, int_variable *> everyUniqueVariable;

    dictionary_hash< pdstring, pdvector<int_function *> * > allFunctionsByMangledName;
    dictionary_hash< pdstring, pdvector<int_function *> * > allFunctionsByPrettyName;

    dictionary_hash< pdstring, pdvector<int_variable *> * > allVarsByMangledName;
    dictionary_hash< pdstring, pdvector<int_variable *> * > allVarsByPrettyName;

    codeRangeTree codeRangesByAddr_;

    int_function *findFunction(image_func *img_func);
    int_variable *findVariable(image_variable *img_var);
    // And those call...
    void addFunction(int_function *func);
    void addVariable(int_variable *var);

    bool dirty_; // marks the shared object as dirty 
    bool dirtyCalled_;//see comment for setDirtyCalled
    
    image  *image_; // pointer to image if processed is true 
    bool dlopenUsed; //mark this shared object as opened by dlopen
    process *proc_; // Parent process

    bool analyzed_; // Prevent multiple adds

    mapped_module *getOrCreateForkedModule(mapped_module *mod);

    // from a string that is a complete path name to a function in a module
    // (ie. "/usr/lib/libc.so.1/write") return a string with the function
    // part removed.  return 0 on error
    char *getModulePart(pdstring &full_path_name) ;

};

// Aggravation: a mapped object might very well occupy multiple "ranges". 
class mappedObjData : public codeRange {
 public:
    mappedObjData(mapped_object *obj_) : obj(obj_) {};
    Address get_address_cr() const { return obj->dataAbs(); }
    unsigned get_size_cr() const { return obj->dataSize(); }
    mapped_object *obj;
};


#endif
