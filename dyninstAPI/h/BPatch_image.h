/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

#include "BPatch_dll.h"
#include "BPatch_sourceObj.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"
#include "BPatch_module.h"
#include "BPatch_type.h"

class process;
class image;

class AddrToVarExprHash;

class BPATCH_DLL_EXPORT BPatch_image: public BPatch_sourceObj {
    process	*proc;

    void findFunctionInImage(const char *name, image *img,
			     BPatch_Vector<BPatch_function*>& funcs);
public:
// The following functions are for internal use by  the library only:
    BPatch_image(process *_proc);
    BPatch_image();
    virtual ~BPatch_image();

    bool                 ModuleListExist();
    void                 addModuleIfExist(BPatch_module *bpmod);

    BPatch_variableExpr	*createVarExprByName(BPatch_module *mod, const char *name);

// End functions for internal use only

    bool getSourceObj(BPatch_Vector<BPatch_sourceObj *> &);
    BPatch_sourceObj *getObjParent();
    bool getVariables(BPatch_Vector<BPatch_variableExpr *> &);
  
    BPatch_Vector<BPatch_function *> *getProcedures();
    BPatch_Vector<BPatch_module *> *getModules();

    BPatch_Vector<BPatch_variableExpr *> *getGlobalVariables();

    BPatch_Vector<BPatch_point *> *findProcedurePoint(const char *name,
	    const BPatch_procedureLocation loc);

    BPatch_point *createInstPointAtAddr(void *address);

    BPatch_point *createInstPointAtAddr(void *address,
					BPatch_point** alternative);

    BPatch_function	*findFunction(const char *name);
    BPatch_function	*findBPFunction(const char *name);

    BPatch_variableExpr	*findVariable(const char *name, bool showError=true);
    BPatch_variableExpr *findVariable(BPatch_point &scp, const char *nm); 

    BPatch_type		*findType(const char *name);

    //method to retrieve addresses corresponding to a line in a file
    bool getLineToAddr(const char* fileName,unsigned short lineNo,
		       BPatch_Vector<unsigned long>& buffer,
		       bool exactMatch = true);
#ifdef IBM_BPATCH_COMPAT
    char *programName(char *name, unsigned int len);
    char *getProgramName(char *name, unsigned int len);
    int  lpType();
#endif

    BPatch_Vector<BPatch_function*>	*findFunction(const char *name, BPatch_Vector<BPatch_function*> &funcs);

private:
    BPatch_Vector<BPatch_module *> *modlist;
    AddrToVarExprHash *AddrToVarExpr;
};

#endif /* _BPatch_image_h_ */
