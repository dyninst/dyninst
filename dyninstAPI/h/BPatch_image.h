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

#ifndef _BPatch_image_h_
#define _BPatch_image_h_


#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // no regex for M$
#include <regex.h>
#endif 

#include "BPatch_dll.h"
#include "BPatch_sourceObj.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"
#include "BPatch_module.h"
#include "BPatch_type.h"
#include "BPatch_eventLock.h"

typedef bool (*BPatchFunctionNameSieve)(const char *test,void *data);
class process;
class image;

class AddrToVarExprHash;

#ifdef IBM_BPATCH_COMPAT

typedef enum BPatch_LpModel {
    LP32,      /* 32 bit image */
    LP64,      /* 64 bit image */
    UNKNOWN_LP /* cannot be determined */
};

#endif

class ThreadLibrary;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_image

class BPATCH_DLL_EXPORT BPatch_image: public BPatch_sourceObj, public BPatch_eventLock {
    friend class ThreadLibrary;
    process	*proc; // get rid of this in favor of BPatch_thread
    BPatch_thread *appThread;

    char *defaultNamespacePrefix;

 public:

    // The following functions are for internal use by  the library only:
    // As such, these functions are not locked.
    BPatch_image(BPatch_thread *_thr);
    BPatch_image();
    virtual ~BPatch_image();
    bool                 ModuleListExist();
    void                 addModuleIfExist(BPatch_module *bpmod);
    BPatch_variableExpr	*createVarExprByName(BPatch_module *mod, const char *name);
    void setDefaultNamespacePrefix(char *name) { defaultNamespacePrefix = name; }
    // End functions for internal use only

    //  BPatch_image::getThr
    //  
    //  return the BPatch_thread associated with this image
    API_EXPORT(Int, (),

    BPatch_thread *,getThr,());


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

    BPatch_Vector<BPatch_function *> *,
    getProcedures,(bool incUninstrumentable = false));

    //  BPatch_image::getModules
    //  
    //  Returns a vector of all modules in this image
    API_EXPORT(Int, (),

    BPatch_Vector<BPatch_module *> *,getModules,());

    //  BPatch_image::findModule
    //  
    //  Returns a module matching <name> if present in image, NULL if not found
    API_EXPORT(Int, (name),

    BPatch_module *,findModule,(const char *name));

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
                                                    
    //  BPatch_image::BPatch_image::findFunction
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

    //  BPatch_image::findVariable
    //  
    //  Returns global variable matching <name> in the image.  NULL if not found.
    API_EXPORT(Int, (name, showError),

    BPatch_variableExpr *,findVariable,(const char *name, bool showError=true));

    //  BPatch_image::findVariable
    //  
    //  Returns local variable matching name <nm> in function scope of 
    //  provided BPatch_point.
    API_EXPORT(InScope, (scp,nm),

    BPatch_variableExpr *,findVariable,(BPatch_point &scp, const char *nm));

    //  BPatch_image::findType
    //  
    //  Returns a BPatch_type corresponding to <name>, if exists, NULL if not found
    API_EXPORT(Int, (name),

    BPatch_type *,findType,(const char *name));

    //  BPatch_image::getLineToAddr
    //  
    //  method to retrieve addresses corresponding to a line in a file
    API_EXPORT(Int, (fileName, lineNo, buffer, exactMatch),

    bool,getLineToAddr,(const char* fileName, unsigned short lineNo,
                        BPatch_Vector<unsigned long>& buffer, bool exactMatch = true));

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
    BPatch_Vector<BPatch_module *> *modlist;
    AddrToVarExprHash *AddrToVarExpr;

    // These private "find" functions convert from internal int_function
    // representation to the exported BPatch_Function type
    void findFunctionInImage(const char *name, image *img,
			     BPatch_Vector<BPatch_function*> *funcs,
			     bool incUninstrumentable = false);
    
#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // no regex for M$
    void findFunctionPatternInImage(regex_t *comp_pat, image *img, 
				    BPatch_Vector<BPatch_function*> *funcs,
				    bool incUninstrumentable = false);
#endif
    void sieveFunctionsInImage(image *img, 
			       BPatch_Vector<BPatch_function *> *funcs,
			       BPatchFunctionNameSieve bpsieve, 
			       void *user_data,
			       bool incUninstrumentable = false);
};

#endif /* _BPatch_image_h_ */
