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

#define BPATCH_FILE


#include "process.h"
#include "EventHandler.h"
#include "mailbox.h"
#include "signalgenerator.h"
#include "inst.h"
#include "instP.h"
#include "instPoint.h"
#include "function.h" // int_function
#include "codeRange.h"
#include "dyn_thread.h"
#include "miniTramp.h"

#include "mapped_module.h"

#include "BPatch_libInfo.h"
#include "BPatch_asyncEventHandler.h"
#include "BPatch.h"
#include "BPatch_thread.h"
#include "BPatch_function.h"
#include "callbacks.h"

#include "BPatch_private.h"

#include "ast.h"

#include "BPatch_addressSpace.h"


BPatch_addressSpace::BPatch_addressSpace() :
   func_map(NULL),
   instp_map(NULL),
   image(NULL)
{
}

BPatch_addressSpace::~BPatch_addressSpace()
{}


BPatch_function *BPatch_addressSpace::findOrCreateBPFunc(int_function* ifunc,
                                                    BPatch_module *bpmod)
{
  if( func_map->defines(ifunc) ) {
    assert( func_map->get(ifunc) != NULL );
    return func_map->get(ifunc);
  }
  
  // Find the module that contains the function
  if (bpmod == NULL && ifunc->mod() != NULL) {
      bpmod = getImage()->findModule(ifunc->mod()->fileName().c_str());
  }

  // findModule has a tendency to make new function objects... so
  // check the map again
  if (func_map->defines(ifunc)) {
    assert( func_map->get(ifunc) != NULL );
    return func_map->get(ifunc);
  }

  BPatch_function *ret = new BPatch_function(this, ifunc, bpmod);
  assert( ret != NULL );
  return ret;
}




BPatch_point *BPatch_addressSpace::findOrCreateBPPoint(BPatch_function *bpfunc, 
						  instPoint *ip, 
						  BPatch_procedureLocation pointType)
{
    assert(ip);
   if (instp_map->defines(ip)) 
      return instp_map->get(ip);

   if (bpfunc == NULL) 
       bpfunc = findOrCreateBPFunc(ip->func(), NULL);

   
   
   BPatch_point *pt = new BPatch_point(this, bpfunc, ip, pointType);

   instp_map->add(ip, pt);

   return pt;
}



BPatch_function *BPatch_addressSpace::createBPFuncCB(AddressSpace *a, int_function *f)
{
    BPatch_addressSpace *aS = (BPatch_addressSpace *)a->up_ptr();
    assert(aS);
    return aS->findOrCreateBPFunc(f, NULL);
}

BPatch_point *BPatch_addressSpace::createBPPointCB(AddressSpace *a, int_function *f, 
                                              instPoint *ip, int type)
{
    BPatch_addressSpace *aS = (BPatch_addressSpace *)a->up_ptr();
    assert(aS);
    BPatch_function *func = aS->func_map->get(f);
    return aS->findOrCreateBPPoint(func, ip, (BPatch_procedureLocation) type);
}



/***************************************************************************
 * Bpatch_snippetHandle
 ***************************************************************************/

/*
 * BPatchSnippetHandle::BPatchSnippetHandle
 *
 * Constructor for BPatchSnippetHandle.  Delete the snippet instance(s)
 * associated with the BPatchSnippetHandle.
 */
BPatchSnippetHandle::BPatchSnippetHandle(BPatch_addressSpace * addSpace) :
    addSpace_(addSpace)
{
}

/*
 * BPatchSnippetHandle::~BPatchSnippetHandle
 *
 * Destructor for BPatchSnippetHandle.  Delete the snippet instance(s)
 * associated with the BPatchSnippetHandle.
 */
void BPatchSnippetHandle::BPatchSnippetHandle_dtor()
{
   // don't delete inst instances since they are might have been copied
}

BPatch_addressSpace *BPatchSnippetHandle::getAddressSpaceInt()
{
  return addSpace_;
}

BPatch_process *BPatchSnippetHandle::getProcessInt()
{
    return dynamic_cast<BPatch_process *>(addSpace_);
}

BPatch_Vector<BPatch_thread *> &BPatchSnippetHandle::getCatchupThreadsInt()
{
  return catchup_threads;
}


BPatch_image * BPatch_addressSpace::getImageInt()
{
  return image;
}


/*
 * BPatch_addressSpace::deleteSnippet
 * 
 * Deletes an instance of a snippet.
 *
 * handle	The handle returned by insertSnippet when the instance to
 *		deleted was created.
 */

bool BPatch_addressSpace::deleteSnippetInt(BPatchSnippetHandle *handle)
{   
    if (getTerminated()) return true;

    //    if (handle->proc_ == this) {
    if (handle->addSpace_ == this) {  
      for (unsigned int i=0; i < handle->mtHandles_.size(); i++)
	handle->mtHandles_[i]->uninstrument();
      delete handle;
      return true;
    } 
    // Handle isn't to a snippet instance in this process
    cerr << "Error: wrong address space in deleteSnippet" << endl;     
    return false;
}

/*
 * BPatch_addressSpace::replaceCode
 *
 * Replace a given instruction with a BPatch_snippet.
 *
 * point       Represents the instruction to be replaced
 * snippet     The replacing snippet
 */

bool BPatch_addressSpace::replaceCodeInt(BPatch_point *point,
                                    BPatch_snippet *snippet) {
   if (!getMutationsActive())
      return false;

    if (!point) {
        return false;
    }
    if (getTerminated()) {
        return true;
    }

    if (point->edge_) {
        return false;
    }


    return point->point->replaceCode(*(snippet->ast_wrapper));
}



/*
 * BPatch_addressSpace::replaceFunctionCall
 *
 * Replace a function call with a call to a different function.  Returns true
 * upon success, false upon failure.
 * 
 * point	The call site that is to be changed.
 * newFunc	The function that the call site will now call.
 */
bool BPatch_addressSpace::replaceFunctionCallInt(BPatch_point &point,
                                            BPatch_function &newFunc)
{
   // Can't make changes to code when mutations are not active.
   if (!getMutationsActive())
      return false;
   assert(point.point && newFunc.lowlevel_func());
   return getAS()->replaceFunctionCall(point.point, newFunc.lowlevel_func());
}

/*
 * BPatch_addressSpace::removeFunctionCall
 *
 * Replace a function call with a NOOP.  Returns true upon success, false upon
 * failure.
 * 
 * point	The call site that is to be NOOPed out.
 */
bool BPatch_addressSpace::removeFunctionCallInt(BPatch_point &point)
{
   // Can't make changes to code when mutations are not active.
   if (!getMutationsActive())
      return false;   
   assert(point.point);
   return getAS()->replaceFunctionCall(point.point, NULL);
}


/*
 * BPatch_addressSpace::replaceFunction
 *
 * Replace all calls to function OLDFUNC with calls to NEWFUNC.
 * Returns true upon success, false upon failure.
 * 
 * oldFunc	The function to replace
 * newFunc      The replacement function
 */
bool BPatch_addressSpace::replaceFunctionInt(BPatch_function &oldFunc,
                                        BPatch_function &newFunc)
{
#if defined(os_solaris) || defined(os_osf) || defined(os_linux) || \
    defined(os_windows) \

    assert(oldFunc.lowlevel_func() && newFunc.lowlevel_func());
    if (!getMutationsActive())
        return false;
    
    // Self replacement is a nop
    // We should just test direct equivalence here...
    if (oldFunc.lowlevel_func() == newFunc.lowlevel_func()) {
        return true;
    }

   bool old_recursion_flag = BPatch::bpatch->isTrampRecursive();
   BPatch::bpatch->setTrampRecursive( true );
   
   // We replace functions by instrumenting the entry of OLDFUNC with
   // a non-linking jump to NEWFUNC.  Calls to OLDFUNC do actually
   // transfer to OLDFUNC, but then our jump shunts them to NEWFUNC.
   // The non-linking jump ensures that when NEWFUNC returns, it
   // returns directly to the caller of OLDFUNC.
   BPatch_Vector<BPatch_point *> *pts = oldFunc.findPoint(BPatch_entry);
   if (! pts || ! pts->size()) {
      BPatch::bpatch->setTrampRecursive( old_recursion_flag );
      return false;
   }
   BPatch_funcJumpExpr fje(newFunc);
   BPatchSnippetHandle * result = insertSnippet(fje, *pts, BPatch_callBefore);
   
   BPatch::bpatch->setTrampRecursive( old_recursion_flag );
   
   return (NULL != result);
#else
   char msg[2048];
   char buf1[512], buf2[512];
   sprintf(msg, "cannot replace func %s with func %s, not implemented",
           oldFunc.getName(buf1, 512), newFunc.getName(buf2, 512));
    
   BPatch_reportError(BPatchSerious, 109, msg);
   BPatch_reportError(BPatchSerious, 109,
                      "replaceFunction is not implemented on this platform");
   return false;
#endif
}


bool BPatch_addressSpace::getAddressRangesInt( const char * fileName, unsigned int lineNo, std::vector< std::pair< unsigned long, unsigned long > > & ranges ) {
	unsigned int originalSize = ranges.size();

	/* Iteratate over the modules, looking for addr in each. */
	BPatch_Vector< BPatch_module * > * modules = image->getModules();
	for( unsigned int i = 0; i < modules->size(); i++ ) {
		LineInformation *lineInformation = (* modules)[i]->mod->getLineInformation();		
		if(lineInformation)
			lineInformation->getAddressRanges( fileName, lineNo, ranges );
		} /* end iteration over modules */
	if( ranges.size() != originalSize ) { return true; }
	
	return false;
	} /* end getAddressRangesInt() */

bool BPatch_addressSpace::getSourceLinesInt( unsigned long addr, 
                                        BPatch_Vector< BPatch_statement > & lines ) 
{
   return image->getSourceLinesInt(addr, lines);
} /* end getLineAndFile() */


/*
 * BPatch_process::malloc
 *
 * Allocate memory in the thread's address space.
 *
 * n	The number of bytes to allocate.
 *
 * Returns:
 * 	A pointer to a BPatch_variableExpr representing the memory.
 *
 */
BPatch_variableExpr *BPatch_addressSpace::mallocInt(int n)
{
   assert(BPatch::bpatch != NULL);
   void *ptr = (void *) getAS()->inferiorMalloc(n, dataHeap);
   if (!ptr) return NULL;
   return new BPatch_variableExpr(this, ptr, Null_Register, 
                                  BPatch::bpatch->type_Untyped);
}


/*
 * BPatch_process::malloc
 *
 * Allocate memory in the thread's address space for a variable of the given
 * type.
 *
 * type		The type of variable for which to allocate space.
 *
 * Returns:
 * 	A pointer to a BPatch_variableExpr representing the memory.
 *
 * XXX Should return NULL on failure, but the function which it calls,
 *     inferiorMalloc, calls exit rather than returning an error, so this
 *     is not currently possible.
 */
BPatch_variableExpr *BPatch_addressSpace::mallocByType(const BPatch_type &type)
{
   assert(BPatch::bpatch != NULL);
   BPatch_type &t = const_cast<BPatch_type &>(type);
   void *mem = (void *) getAS()->inferiorMalloc(t.getSize(), dataHeap);
   if (!mem) return NULL;
   return new BPatch_variableExpr(this, mem, Null_Register, &t);
}


/*
 * BPatch_process::free
 *
 * Free memory that was allocated with BPatch_process::malloc.
 *
 * ptr		A BPatch_variableExpr representing the memory to free.
 */
bool BPatch_addressSpace::freeInt(BPatch_variableExpr &ptr)
{
   getAS()->inferiorFree((Address)ptr.getBaseAddr());
   return true;
}



/*
 * BPatch_addressSpace::findFunctionByAddr
 *
 * Returns the function that contains the specified address, or NULL if the
 * address is not within a function.
 *
 * addr		The address to use for the lookup.
 */
BPatch_function *BPatch_addressSpace::findFunctionByAddrInt(void *addr)
{
   int_function *func;
   
   codeRange *range = getAS()->findOrigByAddr((Address) addr);
   if (!range)
      return NULL;

   func = range->is_function();
    
   if (!func)
      return NULL;
    
   return findOrCreateBPFunc(func, NULL);
}


/*
 * BPatch_addressSpace::insertSnippet
 *
 * Insert a code snippet at a given instrumentation point.  Upon success,
 * returns a handle to the created instance of the snippet, which can be used
 * to delete it.  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * point	The point at which to insert it.
 */
BPatchSnippetHandle *BPatch_addressSpace::insertSnippetInt(const BPatch_snippet &expr, 
						      BPatch_point &point, 
						      BPatch_snippetOrder order)
{
   BPatch_callWhen when;
   if (point.getPointType() == BPatch_exit)
      when = BPatch_callAfter;
   else
      when = BPatch_callBefore;

   return insertSnippetWhen(expr, point, when, order);
}

/*
 * BPatch_addressSpace::insertSnippet
 *
 * Insert a code snippet at a given instrumentation point.  Upon succes,
 * returns a handle to the created instance of the snippet, which can be used
 * to delete it.  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * point	The point at which to insert it.
 */
// This handles conversion without requiring inst.h in a header file...
extern bool BPatchToInternalArgs(BPatch_point *point,
                                 BPatch_callWhen when,
                                 BPatch_snippetOrder order,
                                 callWhen &ipWhen,
                                 callOrder &ipOrder);
                           

BPatchSnippetHandle *BPatch_addressSpace::insertSnippetWhen(const BPatch_snippet &expr,
						       BPatch_point &point,
						       BPatch_callWhen when,
						       BPatch_snippetOrder order)
{
  BPatch_Vector<BPatch_point *> points;
  points.push_back(&point);
  return insertSnippetAtPointsWhen(expr,
				   points,
				   when,
				   order);
 
}


/*
 * BPatch_addressSpace::insertSnippet
 *
 * Insert a code snippet at each of a list of instrumentation points.  Upon
 * success, Returns a handle to the created instances of the snippet, which
 * can be used to delete them (as a unit).  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * points	The list of points at which to insert it.
 */
// A lot duplicated from the single-point version. This is unfortunate.
BPatchSnippetHandle *BPatch_addressSpace::insertSnippetAtPointsWhen(const BPatch_snippet &expr,
                                                               const BPatch_Vector<BPatch_point *> &points,
                                                               BPatch_callWhen when,
                                                               BPatch_snippetOrder order)
{
    
    if (dyn_debug_inst) {
        BPatch_function *f;
        for (unsigned i=0; i<points.size(); i++) {
            f = points[i]->getFunction();
            const char *sname = f->func->prettyName().c_str();
            inst_printf("[%s:%u] - %d. Insert instrumentation at function %s, "
                        "address %p, when %d, order %d\n",
                        FILE__, __LINE__, i,
                        sname, points[i]->getAddress(), (int) when, (int) order);
            
        }
    }
    
    if (BPatch::bpatch->isTypeChecked()) {
        assert(expr.ast_wrapper);
        if ((*(expr.ast_wrapper))->checkType() == BPatch::bpatch->type_Error) {
            inst_printf("[%s:%u] - Type error inserting instrumentation\n",
                        FILE__, __LINE__);
            return false;
        }
    }
    
    if (!points.size()) {
       inst_printf("%s[%d]:  request to insert snippet at zero points!\n", FILE__, __LINE__);
      return false;
    }
    
    
    batchInsertionRecord *rec = new batchInsertionRecord;
    rec->thread_ = NULL;
    rec->snip = expr;
    rec->trampRecursive_ = BPatch::bpatch->isTrampRecursive();

    BPatchSnippetHandle *ret = new BPatchSnippetHandle(this);
    rec->handle_ = ret;
    
    for (unsigned i = 0; i < points.size(); i++) {
        BPatch_point *point = points[i];

        if (point->addSpace == NULL) {
            fprintf(stderr, "Error: attempt to use point with no process info\n");
            continue;
        }
        if (dynamic_cast<BPatch_addressSpace *>(point->addSpace) != this) {
            fprintf(stderr, "Error: attempt to use point specific to a different process\n");
            continue;
        }        

        callWhen ipWhen;
        callOrder ipOrder;
        
        if (!BPatchToInternalArgs(point, when, order, ipWhen, ipOrder)) {
            inst_printf("[%s:%u] - BPatchToInternalArgs failed for point %d\n",
                        FILE__, __LINE__, i);
            return NULL;
        }

        rec->points_.push_back(point);
        rec->when_.push_back(ipWhen);
        rec->order_ = ipOrder;

        point->recordSnippet(when, order, ret);
    }

    assert(rec->points_.size() == rec->when_.size());

    // Okey dokey... now see if we just tack it on, or insert now.
    if (pendingInsertions) {
        pendingInsertions->push_back(rec);
    }
    else {
        BPatch_process *proc = dynamic_cast<BPatch_process *>(this);
        assert(proc);
        proc->beginInsertionSetInt();
        pendingInsertions->push_back(rec);
        // All the insertion work was moved here...
        proc->finalizeInsertionSetInt(false);
    }
    return ret;
}


/*
 * BPatch_addressSpace::insertSnippet
 *
 * Insert a code snippet at each of a list of instrumentation points.  Upon
 * success, Returns a handle to the created instances of the snippet, which
 * can be used to delete them (as a unit).  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * points	The list of points at which to insert it.
 */

BPatchSnippetHandle *BPatch_addressSpace::insertSnippetAtPoints(
                 const BPatch_snippet &expr,
                 const BPatch_Vector<BPatch_point *> &points,
                 BPatch_snippetOrder order)
{
    return insertSnippetAtPointsWhen(expr,
                                     points,
                                     BPatch_callUnset,
                                     order);
}



