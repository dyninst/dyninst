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


					    
					  
class BPATCH_DLL_EXPORT BPatch_image: public BPatch_sourceObj {
    process	*proc;

    char *defaultNamespacePrefix;

 public:
// The following functions are for internal use by  the library only:
    BPatch_image(process *_proc);
    BPatch_image();
    virtual ~BPatch_image();

    bool                 ModuleListExist();
    void                 addModuleIfExist(BPatch_module *bpmod);

    BPatch_variableExpr	*createVarExprByName(BPatch_module *mod, const char *name);

    void setDefaultNamespacePrefix(char *name) { defaultNamespacePrefix = name; }

// End functions for internal use only

    bool getSourceObj(BPatch_Vector<BPatch_sourceObj *> &);
    BPatch_sourceObj *getObjParent();
    bool getVariables(BPatch_Vector<BPatch_variableExpr *> &);
  
    BPatch_Vector<BPatch_function *> *getProcedures();
    BPatch_Vector<BPatch_module *> *getModules();

    BPatch_Vector<BPatch_variableExpr *> *getGlobalVariables();

    BPatch_point *createInstPointAtAddr(void *address);

    BPatch_point *createInstPointAtAddr(void *address,
                                        BPatch_point** alternative,
					BPatch_function* bpf = NULL);


    BPatch_Vector<BPatch_function*>	*findFunction(const char *name, BPatch_Vector<BPatch_function*> &funcs, bool showError=true, bool regex_case_sensitive=true);

    BPatch_Vector<BPatch_function *> *findFunction(BPatch_Vector<BPatch_function *> &funcs, BPatchFunctionNameSieve bpsieve, void *user_data=NULL, int showError=0);
    BPatch_variableExpr	*findVariable(const char *name, bool showError=true);
    BPatch_variableExpr *findVariable(BPatch_point &scp, const char *nm); 

    BPatch_type		*findType(const char *name);

    //method to retrieve addresses corresponding to a line in a file
    bool getLineToAddr(const char* fileName,unsigned short lineNo,
		       BPatch_Vector<unsigned long>& buffer,
		       bool exactMatch = true);
#ifdef IBM_BPATCH_COMPAT
    BPatch_Vector<BPatch_function*>     *findFunction(const char *name, BPatch_Vector<BPatch_function*> *funcs, bool showError=true, bool regex_case_sensitive=true);

    char *programName(char *name, unsigned int len);
    int  lpType();
#endif
    char *getProgramName(char *name, unsigned int len);
    char *getProgramFileName(char *name, unsigned int len);

  
  

private:
    BPatch_Vector<BPatch_module *> *modlist;
    AddrToVarExprHash *AddrToVarExpr;

    // These private "find" functions convert from internal pd_Function
    // representation to the exported BPatch_Function type
    void findFunctionInImage(const char *name, image *img,
			     BPatch_Vector<BPatch_function*> *funcs);
    
#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // no regex for M$
    void findFunctionPatternInImage(regex_t *comp_pat, image *img, 
				    BPatch_Vector<BPatch_function*> *funcs);
#endif
    void sieveFunctionsInImage(image *img, 
			      BPatch_Vector<BPatch_function *> *funcs,
			      BPatchFunctionNameSieve bpsieve, 
			      void *user_data);
};

#endif /* _BPatch_image_h_ */
