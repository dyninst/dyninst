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

#ifndef _BPatch_image_h_
#define _BPatch_image_h_

#include "BPatch_dll.h"
#include "BPatch_sourceObj.h"
#include "BPatch_Vector.h"
//#include "BPatch_addressSpace.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"
#include "BPatch_module.h"
#include "BPatch_type.h"
#include "BPatch_eventLock.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_parRegion.h"
#include "dyntypes.h"

#include <vector>
#include <map>

typedef bool (*BPatchFunctionNameSieve)(const char *test,void *data);
class process;
class image;
class int_variable;

#ifdef IBM_BPATCH_COMPAT

typedef enum BPatch_LpModel {
    LP32,      /* 32 bit image */
    LP64,      /* 64 bit image */
    UNKNOWN_LP /* cannot be determined */
};

#endif

class BPatch_statement;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_image

class BPATCH_DLL_EXPORT BPatch_image: public BPatch_sourceObj, public BPatch_eventLock {
    friend class BPatch; // registerLoaded... callbacks
    friend class BPatch_module; // access to findOrCreate...
    friend class process; // Which also needs findOrCreate because we upcall when a library is loaded.
    friend class BPatch_process;
    friend class BPatch_addressSpace;
    friend class BPatch_binaryEdit;

    BPatch_variableExpr *findOrCreateVariable(int_variable *);
 public:

    // The following functions are for internal use by  the library only:
    // As such, these functions are not locked.
    //BPatch_image(BPatch_process *_proc);
    BPatch_image(BPatch_addressSpace *addSpace);
    BPatch_image();
    virtual ~BPatch_image();
    void getNewCodeRegions
        (std::vector<BPatch_function*>&newFuncs, 
         std::vector<BPatch_function*>&modFuncs);
    void clearNewCodeRegions();
    // End functions for internal use only

    //  BPatch_image::getThr
    //  
    //  return the BPatch_thread associated with this image
    API_EXPORT(Int, (),
    BPatch_thread *,getThr,());


    // BPatch_image::getAddressSpace()
    //
    //  return the BPatch_addressSpace associated with this image
    API_EXPORT(Int, (),
    BPatch_addressSpace *,getAddressSpace,());
    

    //  BPatch_image::getProcess
    //  
    //  return the BPatch_process associated with this image
    API_EXPORT(Int, (),
    BPatch_process *,getProcess,());


    //  BPatch_image::getSourceObj
    //  
    //  fill a vector with children source objects (modules)
    API_EXPORT(Int, (sources),

    bool,getSourceObj,(BPatch_Vector<BPatch_sourceObj *> &sources));

    //  BPatch_image::getObjParent
    //  
    //  Return the parent of this image (always NULL since this is the top level)
    API_EXPORT(Int, (),

    BPatch_sourceObj *,getObjParent,());

    //  BPatch_image::getVariables
    //  
    //  Returns the global variables defined in this image
    API_EXPORT(Int, (vars),

    bool,getVariables,(BPatch_Vector<BPatch_variableExpr *> &vars));

    //  BPatch_image::getProcedures
    //  
    //  Returns a list of all procedures in the image upon success,
    //  NULL upon failure
    API_EXPORT(Int, (incUninstrumentable),
    BPatch_Vector<BPatch_function *> *,getProcedures,(bool incUninstrumentable = false));
    
    API_EXPORT(Int, (procs, incUninstrumentable),
    bool,getProcedures,(BPatch_Vector<BPatch_function*> &procs, bool incUninstrumentable = false));

    //  BPatch_image::getParRegions
    //  
    //  Returns a list of all procedures in the image upon success,
    //  NULL upon failure
    API_EXPORT(Int, (incUninstrumentable),

    BPatch_Vector<BPatch_parRegion *> *,
    getParRegions,(bool incUninstrumentable = false));

    //  BPatch_image::getModules
    //  
    //  Returns a vector of all modules in this image
    API_EXPORT(Int, (),
    BPatch_Vector<BPatch_module *> *,getModules,());

    API_EXPORT(Int, (mods),
    bool,getModules,(BPatch_Vector<BPatch_module*> &mods));

    //  BPatch_image::findModule
    //  
    //  Returns a module matching <name> if present in image, NULL if not found
    //  if <substring_match> is set, the first module that has <name> as a substring
    //  of its name is returned (eg, to find "libpthread.so.1", search for "libpthread" 
    //  with substring_match set to true)
    API_EXPORT(Int, (name, substring_match),

    BPatch_module *,findModule,(const char *name, bool substring_match = false));

    //  BPatch_image::getGlobalVariables
    //  
    //  Returns the global variables defined in this image
    API_EXPORT(Int, (),

    BPatch_Vector<BPatch_variableExpr *> *,getGlobalVariables,());

    //  BPatch_image::createInstPointAtAddr
    //  
    //  Returns a pointer to a BPatch_point object representing an
    //  instrumentation point at the given address.
    //  Returns NULL on failure.
    API_EXPORT(Int, (address),

    BPatch_point *,createInstPointAtAddr,(void *address));

    //  BPatch_image::createInstPointAtAddr
    //  
    //  Returns a pointer to a BPatch_point object representing an
    //  instrumentation point at the given address. If the BPatch_function
    //  argument is given it has to be the function that address belongs to or NULL.
    //  The function is used to bypass the function that the address belongs to
    //  The alternative argument is used to retrieve the point if the new point
    //  intersects with another already existing one.
    //  Returns NULL on failure.
    API_EXPORT(WithAlt, (address, alternative, bpf),

    BPatch_point *,createInstPointAtAddr,(void *address,
                                          BPatch_point** alternative, 
                                          BPatch_function* bpf = NULL)); 

    //  BPatch_image::findFunction
    //  
    //  Returns a vector of functions matching <name>, if <name> is a regular
    //  expression, a (slower) regex search will be performed.  
    //  Returns NULL on failure.
    API_EXPORT(Int, (name, funcs, showError, regex_case_sensitive, incUninstrumentable),

    BPatch_Vector<BPatch_function*> *,findFunction,(const char *name,
                                                    BPatch_Vector<BPatch_function*> &funcs, 
                                                    bool showError=true,
                                                    bool regex_case_sensitive=true,
                                                    bool incUninstrumentable = false));
                                                    
    //  BPatch_image::findFunction
    //  
    //  Returns a vector of functions matching criterion specified by user defined
    //  callback function bpsieve.
    API_EXPORT(WithSieve, (funcs, bpsieve, user_data, showError, incUninstrumentable),

    BPatch_Vector<BPatch_function *> *,
       findFunction,(BPatch_Vector<BPatch_function *> &funcs,
                     BPatchFunctionNameSieve bpsieve,
                     void *user_data=NULL,
                     int showError=0,
                     bool incUninstrumentable = false));

    //  BPatch_image::findFunction(Address)
    //
    //  Returns a function at a specified address
    API_EXPORT(Int, (addr),
               BPatch_function *, findFunction,(unsigned long addr));

    API_EXPORT(Int, (addr, funcs),
               bool, findFunction,(Dyninst::Address addr, 
                                   BPatch_Vector<BPatch_function *> &funcs));

    //  BPatch_image::findVariable
    //  
    //  Returns global variable matching <name> in the image.  NULL if not found.
    API_EXPORT(Int, (name, showError),

    BPatch_variableExpr *,findVariable,(const char *name, bool showError=true));

    //  BPatch_image::findVariable
    //  
    //  Returns local variable matching name <nm> in function scope of 
    //  provided BPatch_point.
    API_EXPORT(InScope, (scp, nm, showError),

               BPatch_variableExpr *,findVariable,(BPatch_point &scp, const char *nm, bool showError=true));

    //  BPatch_image::findType
    //  
    //  Returns a BPatch_type corresponding to <name>, if exists, NULL if not found
    API_EXPORT(Int, (name),

    BPatch_type *,findType,(const char *name));

    //  BPatch_image::getAddressRanges
    //  
    //  method to retrieve addresses corresponding to a line in a file
    API_EXPORT(Int, (fileName, lineNo, ranges),

    bool,getAddressRanges,( const char * fileName, unsigned int lineNo, 
                            std::vector<std::pair<unsigned long, unsigned long> > & ranges ));
    
    API_EXPORT(Int, (addr, lines),
    bool,getSourceLines,( unsigned long addr, BPatch_Vector<BPatch_statement> & lines ));

    //  BPatch_image::getProgramName
    //  
    //  fills provided buffer <name> with the program's name, up to <len> chars
    API_EXPORT(Int, (name, len),

    char *,getProgramName,(char *name, unsigned int len));

    //  BPatch_image::getProgramFileName
    //  
    //  fills provided buffer <name> with the program's file name, 
    //  which may include path information.
    API_EXPORT(Int, (name, len),

    char *,getProgramFileName,(char *name, unsigned int len));

    /* BPatch_image::parseNewFunctions
     *
     * This function uses function entry addresses to find and parse
     * new functions using our control-flow traversal parsing. 
     *
     * funcEntryAddrs: this is a vector of function start addresses
     * that seed the control-flow-traversal parsing.  If they lie in
     * an existing module they are parsed in that module, otherwise a
     * new module is created.  In both cases the modules are added to
     * affectedModules
     *
     * affectedModules: BPatch_modules will be added to this vector if no
     * existing modules bounded the specified function entry points.
     * Unfortunately, new modules will also sometimes have to be created
     * for dynamically created code in memory that does not map to the
     * file version of the binary.  
     *
     * Return value: This value is true if a new module was created or if
     * new code was parsed in an existing module
     */
    API_EXPORT(Int, (affectedModules, funcEntryAddrs),
    bool ,parseNewFunctions, 
    (BPatch_Vector<BPatch_module*> &affectedModules, 
     const BPatch_Vector<Dyninst::Address> &funcEntryAddrs));

    //
    //  Reads a string from the target process
    API_EXPORT(Int, (addr, str, size_limit),
               bool, readString,(Dyninst::Address addr, std::string &str, 
                                 unsigned size_limit = 0));

    API_EXPORT(Int, (expr, str, size_limit),
               bool, readString,(BPatch_variableExpr *expr, std::string &str, 
                                 unsigned size_limit = 0));

#ifdef IBM_BPATCH_COMPAT
    API_EXPORT(Ptr, (name, funcs, showError, regex_case_sensitive, incUninstrumentable),

    BPatch_Vector<BPatch_function*> *,
      findFunction,(const char *name,
                    BPatch_Vector<BPatch_function*> *funcs,
                    bool showError=true,
                    bool regex_case_sensitive=true,
                    bool incUninstrumentable = false));

    API_EXPORT(Int, (name, len),

    char *,programName,(char *name, unsigned int len));

    API_EXPORT(Int, (),
    int,lpType,());
#endif

private:
    BPatch_addressSpace *addSpace;
    BPatch_Vector<BPatch_module *> modlist;
    BPatch_Vector<BPatch_module *> removed_list;
    BPatch_module *defaultModule;

    BPatch_module *findModule(mapped_module *base);
    BPatch_module *findOrCreateModule(mapped_module *base);
    void removeModule(BPatch_module *mod);
    void removeAllModules();

    BPatch_Vector<BPatch_point *> unresolvedCF;

    // These private "find" functions convert from internal int_function
    // representation to the exported BPatch_Function type
    void findFunctionInImage(const char *name, image *img,
			     BPatch_Vector<BPatch_function*> *funcs,
			     bool incUninstrumentable = false);
    void sieveFunctionsInImage(image *img, 
			       BPatch_Vector<BPatch_function *> *funcs,
			       BPatchFunctionNameSieve bpsieve, 
			       void *user_data,
			       bool incUninstrumentable = false);

    static bool setFuncModulesCallback(BPatch_function *bpf, void *data);
};

#endif /* _BPatch_image_h_ */
