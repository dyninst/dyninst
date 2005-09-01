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

// $Id: mapped_module.h,v 1.1 2005/09/01 22:18:30 bernat Exp $

#if !defined(mapped_module_h)
#define mapped_module_h

class mapped_module;
class mapped_object;

class int_function;
class int_variable;

class parse_image;
class pdmodule;
class image;

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


// pdmodule equivalent The internals tend to use images, while the
// BPatch layer uses modules. On the other hand, "module" means
// "compilation unit for the a.out, or the entire image for a
// library". At some point this will need to be fixed, which will be a
// major pain.

class mapped_module {
 public:
    static mapped_module *createMappedModule(mapped_object *obj,
                                             pdmodule *pdmod);

    mapped_object *obj() const;
    pdmodule *pmod() const;

    const pdstring &fileName() const;
    const pdstring &fullName() const;

    process *proc() const;

    //bool analyze();
    
    // A lot of stuff shared with the internal module
    // Were we compiled with the native compiler?
    bool isNativeCompiler() const;

    supportedLanguages language() const;
    
    const pdvector<int_function *> &getAllFunctions();
    
    bool findFuncVectorByPretty(const pdstring &funcname,
                                pdvector<int_function *> &funcs);
    // Yeah, we can have multiple mangled matches -- for libraries there
    // is a single module. Even if we went multiple, we might not have
    // module information, and so we can get collisions.
    bool findFuncVectorByMangled(const pdstring &funcname,
                                 pdvector<int_function *> &funcs);
    
    int_function *findFuncByAddr(const Address &address);
    codeRange *findCodeRangeByAddress(const Address &address);


    void dumpMangled(pdstring prefix) const;
    
    /////////////////////////////////////////////////////
    // Line information
    /////////////////////////////////////////////////////
    // Line info is something we _definitely_ don't want multiple copies
    // of. So instead we provide pass-through functions that handle
    // things like converting absolute addresses (external) into offsets
    // (internal).
    
    pdstring* processDirectories(pdstring* fn) const;

    // Have we parsed line information yet?
    bool lineInformation() const { return lineInfoValid_; }

#if defined(arch_power)
    void parseLineInformation(	process * proc,
                                pdstring * currentSourceFile,
                                char * symbolName,
                                SYMENT * sym,
                                Address linesfdptr,
                                char * lines,
                                int nlines );
#endif

    // We're not generic-asizing line information yet, so this
    // calls into the pdmodule class to do the work.
    LineInformation &getLineInformation();
    // Given a line in the module, get the set of addresses that it maps
    // to. Calls the internal getAddrFromLine and then adds the base
    // address to the returned list of offsets.
    bool getAddrFromLine(unsigned lineNum,
                         pdvector<Address> &addresses,
                         bool exactMatch);
    
    void addFunction(int_function *func);
    void addVariable(int_variable *var);

#ifndef BPATCH_LIBRARY
   resource *getResource() { return modResource; }
#endif

#ifndef BPATCH_LIBRARY
   resource *modResource;
#endif

 private:
   void parseFileLineInfo();

   pdmodule *internal_mod_;
   mapped_object *obj_;
   
   mapped_module();
   mapped_module(mapped_object *obj,
                 pdmodule *pdmod);
   
   pdvector<int_function *> everyUniqueFunction;
   pdvector<int_variable *> everyUniqueVariable;
   bool lineInfoValid_;
   LineInformation lineInfo_;
};

#endif
