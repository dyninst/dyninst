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

#ifndef _BPatch_module_h_
#define _BPatch_module_h_
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_sourceObj.h"
#include "BPatch_eventLock.h"
#include <vector>

class mapped_module;

class process;
class BPatch_image;
class BPatch_function;
class BPatch_typeCollection;
class BPatch_builtInTypeCollection;
class BPatch_process;
class LineInformation; // PDSEP -- this should probably not be exported

extern BPatch_builtInTypeCollection * builtInTypes;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_module

class BPATCH_DLL_EXPORT BPatch_module: public BPatch_sourceObj, public BPatch_eventLock{

    friend class process;
    friend class BPatch_function;
    friend class BPatch_flowGraph;
    friend class BPatch_image;
    friend class InstrucIter;

    BPatch_process *proc;
    mapped_module      	 *mod;
    BPatch_image	 *img;
    BPatch_Vector<BPatch_function *> * BPfuncs;
    BPatch_Vector<BPatch_function *> * BPfuncs_uninstrumentable;

    bool nativeCompiler;
     
public:

    //  This function should go away when paradyn is on top of dyninst
    mapped_module* getModule() { return mod; }

    // The following functions are for internal use by  the library only:
    BPatch_module(BPatch_process *_proc, mapped_module *_mod, BPatch_image *img);
    BPatch_module() : mod(NULL), img(NULL), BPfuncs(NULL),nativeCompiler(false) {
	_srcType = BPatch_sourceModule;
    };
    virtual ~BPatch_module();
    bool getSourceObj(BPatch_Vector<BPatch_sourceObj *>&);
    BPatch_sourceObj *getObjParent();
    void parseTypes();
    char *parseStabStringSymbol(int line, char *stabstr, void *stabptr);
    void setDefaultNamespacePrefix(char *name);
    // End functions for internal use only
  
    // BPatch_module::getName
    // Returns file name associated with module
    API_EXPORT(Int, (buffer, length),

    char *,getName,(char *buffer, int length));

    // BPatch_module::getFullName
    // Returns full path name of module, when available
    API_EXPORT(Int, (buffer, length),

    char *,getFullName,(char *buffer, int length));

    // BPatch_module::libraryName
    // Returns name if library, if this module is a shared object
    API_EXPORT(Int, (),

    const char *,libraryName,());

    // BPatch_module::getAddressWidth
    // Returns the width (in bytes) of an address in this module
    API_EXPORT(Int, (),

    size_t,getAddressWidth,());

    // BPatch_module::getVariables
    // Fills a vector with the global variables that are specified in this module
    API_EXPORT(Int, (vars),

    bool,getVariables,(BPatch_Vector<BPatch_variableExpr *> &vars));

    // BPatch_module::getProcedures
    // Returns a vector of all functions in this module
    API_EXPORT(Int, (incUninstrumentable),

    BPatch_Vector<BPatch_function *> *,getProcedures,(bool incUninstrumentable = false));

    // BPatch_module::findFunction
    // Returns a vector of BPatch_function *, matching specified <name>
    API_EXPORT(Int, (name, funcs, notify_on_failure,regex_case_sensitive,incUninstrumentable,dont_use_regex),

    BPatch_Vector<BPatch_function *> *,findFunction,(const char *name,
                          BPatch_Vector<BPatch_function *> &funcs,
                          bool notify_on_failure =true,
                          bool regex_case_sensitive =true,
                          bool incUninstrumentable =false,
                          bool dont_use_regex = false));

    // FIXME: This (undocumented) method only works for function entry addresses.
    API_EXPORT(Int, (addr, funcs, notify_on_failure, incUninstrumentable),

    BPatch_Vector<BPatch_function *> *,
      findFunctionByAddress,(void *addr,
                             BPatch_Vector<BPatch_function *> &funcs,
                             bool notify_on_failure = true,
                             bool incUninstrumentable = false));

    // get the module types member (instead of directly accessing)
    API_EXPORT(Int, (), BPatch_typeCollection *, getModuleTypes, ());

    // BPatch_module::findFunctionByMangled
    // Returns a function, if it exits, that matches the provided mangled name
    API_EXPORT(Int, (mangled_name, incUninstrumentable),

    BPatch_function *,findFunctionByMangled,(const char * mangled_name,
                                             bool incUninstrumentable=false));


    // BPatch_module::dumpMangled
    // Prints all <mangled> function names in this module
    API_EXPORT(Int, (prefix),

    bool,dumpMangled,(char *prefix));

    // BPatch_module::isSharedLib
    // Returns true if this module represents a shared library
    API_EXPORT(Int, (),

    bool,isSharedLib,());

    // BPatch_module::isNativeCompiler
    // Returns true if this module was compiled with a native compiler for  
    // the particular platform
    API_EXPORT(Int, (),

    bool,isNativeCompiler,());

    // BPatch_module::getAddressRanges
    // 
    // function to get addresses for a line of the module
    // if fileName is NULL, uses the name of the module
    API_EXPORT(Int, (fileName, lineNo, ranges),

    bool,getAddressRanges,( const char * fileName, unsigned int lineNo, std::vector< std::pair< unsigned long, unsigned long > > & ranges ));

    // BPatch_module::getLineInformation
    // Returns a pointer to LineInformation for this module
    API_EXPORT(Int, (),

    LineInformation &,getLineInformation,());

#ifdef IBM_BPATCH_COMPAT
    API_EXPORT(Int, (start, end),
    bool,getLineNumbers,(unsigned int &start, unsigned int &end));

    API_EXPORT(Int, (start, end),
    bool,getAddressRange,(void * &start, void * &end));

    API_EXPORT(Int, (buffer, length),
    char *,getUniqueString,(char *buffer, int length));   

    char *sharedLibraryName(char *buffer, int length) { getFullName(buffer, length); return buffer;}
    char *getSharedLibName(char *buffer, int length) { getFullName(buffer, length); return buffer;}

    API_EXPORT(Int, (),
    int,getSharedLibType,());

    API_EXPORT(Int, (),
    int,getBindingType,());
#endif


private:
    // Parse wrapper
    void parseTypesIfNecessary();
    BPatch_typeCollection *moduleTypes;

    // In particular, we understand the type information
    // in both DWARF and STABS format.
    void parseStabTypes();
    void parseDwarfTypes();

};

#ifdef IBM_BPATCH_COMPAT
#define	BPatch_sharedPublic	1
#define BPatch_sharedPrivate	2
#define BPatch_nonShared	3

#define BPatch_static		1
#define BPatch_dynamic		2

#endif

#endif /* _BPatch_module_h_ */
