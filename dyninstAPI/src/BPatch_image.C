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

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "process.h"
#include "symtab.h"

#include "BPatch.h"
#include "BPatch_image.h"
#include "BPatch_type.h"


/*
 * BPatch_image::BPatch_image
 *
 * Construct a BPatch_image for the given process.
 */

BPatch_image::BPatch_image(process *_proc) : proc(_proc)
{
}


/*
 * BPatch_image::getProcedures
 *
 * Returns a list of all procedures in the image upon success, and NULL
 * upon failure.
 */
BPatch_Vector<BPatch_function *> *BPatch_image::getProcedures()
{
    BPatch_Vector<BPatch_function *> *proclist =
	new BPatch_Vector<BPatch_function *>;

    if (proclist == NULL) return NULL;

    // XXX Maybe getIncludedFunction instead?  Depends on what we want
    //     to do to support the MDL exclude_lib function.
    // XXX Also, what should we do about getting rid of this?  Should
    //     the BPatch_functions already be made and kept around as long
    //     as the process is, so the user doesn't have to delete them?
    vector<function_base *> *funcs = proc->getAllFunctions();

    for (unsigned int f = 0; f < funcs->size(); f++) {
	BPatch_function *bpfunc = new BPatch_function((*funcs)[f]);
	proclist->push_back(bpfunc);
    }

    return proclist;
}


/*
 * BPatch_image::findProcedurePoint
 *
 * Returns a vector of the instrumentation points from a procedure that are
 * identified by the paramteres.
 *
 * name		The name of the procedure in which to look up the points.
 * loc		The points within the procedure to return.  The following
 *		values are valid for this parameter:
 * 		  BPatch_entry         The function's entry point.
 * 		  BPatch_exit          The function's exit point(s).
 * 		  BPatch_subroutine    The points at which the procedure calls
 * 		                       other procedures.
 * 		  BPatch_longJump      The points at which the procedure make
 * 		                       long jump calls.
 *		  BPatch_allLocations  All of the points described above.
 */
BPatch_Vector<BPatch_point*> *BPatch_image::findProcedurePoint(
	const char *name, const BPatch_procedureLocation loc)
{
    /*
     * XXXX We'd like to get a list of the functions with the name:
    vector<function_base *> flist;

    if (!proc->findFunction(name, flist))
	return NULL;
     * but we don't seem to have a way to do that right now.
     */

    function_base *func = proc->findOneFunction(name);

    BPatch_Vector<BPatch_point*> *result = new BPatch_Vector<BPatch_point *>;

//    for (unsigned func_num = 0; func_num < flist.size(); func_num++) {
	if (loc == BPatch_entry || loc == BPatch_allLocations) {
	    BPatch_point *new_point = new BPatch_point(func->funcEntry(proc));
	    result->push_back(new_point);
	}
	if (loc ==  BPatch_exit || loc == BPatch_allLocations) {
	    const vector<instPoint *> &points = func->funcExits(proc);
	    for (unsigned i = 0; i < points.size(); i++) {
	       	BPatch_point *new_point = new BPatch_point(points[i]);
    		result->push_back(new_point);
	    }
	}
	if (loc ==  BPatch_subroutine || loc == BPatch_allLocations) {
	    const vector<instPoint *> &points = func->funcCalls(proc);
	    for (unsigned i = 0; i < points.size(); i++) {
	       	BPatch_point *new_point = new BPatch_point(points[i]);
		result->push_back(new_point);
	    }
	}
	if (loc ==  BPatch_longJump /* || loc == BPatch_allLocations */) {
	    /* XXX Not yet implemented */
	    assert( 0 );
	}
//    }

    return result;
}


/*
 * BPatch_image::findFunction
 *
 * Returns a BPatch_function* representing the named function upon success,
 * and NULL upon failure.
 *
 * name		The name of function to look up.
 */
BPatch_function *BPatch_image::findFunction(const char *name)
{
    function_base *func = proc->findOneFunction(name);

    if (func == NULL)
	return NULL;

    return new BPatch_function(func);
}


/*
 * BPatch_image::findVariable
 *
 * Returns a BPatch_variableExpr* representing the given variable in the
 * application image.  If no such variable exists, returns NULL.
 *
 * name		The name of the variable to look up.
 *
 * First look for the name with an `_' prepended to it, and if that is not
 *   found try the original name.
 */
BPatch_variableExpr *BPatch_image::findVariable(const char *name)
{
    string full_name = string("_") + string(name);

    Symbol syminfo;
    if (!proc->getSymbolInfo(full_name, syminfo)) {
	string short_name(name);
	if (!proc->getSymbolInfo(short_name, syminfo)) {
	    return NULL;
	}
    }

    // XXX Need to find out type and use it
    BPatch_type *type = BPatch::bpatch->type_Untyped;

    return new BPatch_variableExpr((void *)syminfo.addr(), type);
}

/*
 * BPatch_image::findType
 *
 * Returns a BPatch_type* representing the named type.  If no such type
 * exists, returns NULL.
 *
 * name		The name of type to look up.
 */
BPatch_type *BPatch_image::findType(const char *name)
{
    assert(BPatch::bpatch != NULL);
    return BPatch::bpatch->stdTypes->findType(name);

    /*
     * XXX When we have the info we need from the symbol table, we'll add some
     * more code here that checks it if "name" isn't found in the standard
     * types.
     */
}
