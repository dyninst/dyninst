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

#include <stdio.h>
#ifdef rs6000_ibm_aix4_1
#include <memory.h>
#endif

#include "common/h/headers.h"

#define BPATCH_FILE

#include "BPatch_point.h"
#include "BPatch_snippet.h"
#include "BPatch_type.h"
#include "BPatch_image.h"
#include "BPatch_collections.h"
#include "BPatch_asyncEventHandler.h"
#include "BPatch.h"
#include "process.h"
#include "symtab.h"
#include "instPoint.h"
#include "instP.h"

/*
 * Private constructor
 */
BPatch_point::BPatch_point(process *_proc, BPatch_function *_func, instPoint *_point,
	     BPatch_procedureLocation _pointType, BPatch_memoryAccess* _ma) :
  // Note: MIPSPro compiler complains about redefinition of default argument
  proc(_proc), func(_func), point(_point), pointType(_pointType), memacc(_ma)
{
  point->bppoint = this;
  if (_pointType == BPatch_subroutine)
    dynamic_call_site_flag = 2; // dynamic status unknown
  else
    dynamic_call_site_flag = 0; // not a call site, so not a dynamic call site.
  // I'd love to have a "loop" constructor, but the code structure
  // doesn't work right. We create an entry point as a set of edge points,
  // but not all edge points are loop points.
  loop = NULL;
}

/*
 * BPatch_point::setLoop
 *
 * For a BPatch_point representing a loop instrumentation site,
 * set the loop that it represents.
 */

void BPatch_point::setLoop(BPatch_basicBlockLoop *l) {
  // We currently can use a single BPatch_point to represent
  // multiple loops. This is a problem, since we would really
  // like the points to label a unique loop. On the other hand,
  // then multiple points would share the same physical address..
  // not good.


  // Point must be for a loop.
  assert(pointType == BPatch_locLoopEntry ||
	 pointType == BPatch_locLoopExit ||
	 pointType == BPatch_locLoopStartIter ||
	 pointType == BPatch_locLoopEndIter);

  loop = l;
}

/*
 * BPatch_point::getPointType
 *
 * Returns type of BPatch_point
 */

const BPatch_procedureLocation BPatch_point::getPointTypeInt() 
{ 
   return pointType; 
}

/*
 * BPatch_point::getLoop
 *
 * Returns loop if of appropriate type
 */

BPatch_basicBlockLoop *BPatch_point::getLoopInt() 
{ 
   return loop; 
}

/*
 * BPatch_point::getFunction
 *
 * Returns function to which this BPatch_point belongs
 */

const BPatch_function *BPatch_point::getFunctionInt()
{
   return func;
}

/*
 * BPatch_point::getCalledFunction
 *
 * For a BPatch_point representing a call site, returns a pointer to a
 * BPatch_function that represents the function being called.  If the point
 * isn't a call site, returns NULL.
 */
BPatch_function *BPatch_point::getCalledFunctionInt()
{
   BPatch_function *ret;
   
   assert(point);
   
   if (point->getPointType() != callSite)
      return NULL;
   
   int_function *_func;
   
   if (!proc->findCallee(*point, _func))
    	return NULL;
   
   if (_func != NULL)
      ret = proc->PDFuncToBPFuncMap[_func];
   else
      ret = NULL;
    
   return ret;
}

const BPatch_memoryAccess *BPatch_point::getMemoryAccessInt()
{
  return memacc;
}

const BPatch_Vector<BPatchSnippetHandle *> BPatch_point::getCurrentSnippetsInt() 
{
    pdvector<miniTrampHandle *> mt_buf;

    trampTemplate *baseTramp = proc->baseMap[point];

    if (baseTramp && baseTramp->pre_minitramps) {
        List<miniTrampHandle *>::iterator preIter = baseTramp->pre_minitramps->get_begin_iter();
        List<miniTrampHandle *>::iterator preEnd = baseTramp->pre_minitramps->get_end_iter();
        for (; preIter != preEnd; preIter++) {
            mt_buf.push_back(*preIter);
        }
    }
    if (baseTramp && baseTramp->post_minitramps) {
        List<miniTrampHandle *>::iterator postIter = baseTramp->post_minitramps->get_begin_iter();
        List<miniTrampHandle *>::iterator postEnd = baseTramp->post_minitramps->get_end_iter();
        for (; postIter != postEnd; postIter++) {
            mt_buf.push_back(*postIter);
        }
    }
    
    BPatch_Vector<BPatchSnippetHandle *> snippetVec;
    
    for(unsigned i=0; i<mt_buf.size(); i++) {
        BPatchSnippetHandle *handle = new BPatchSnippetHandle(proc);
        handle->add(mt_buf[i]);
        
        snippetVec.push_back(handle);
    }
    return snippetVec;
}

const BPatch_Vector<BPatchSnippetHandle *> 
BPatch_point::getCurrentSnippetsByWhen(BPatch_callWhen when) 
{
    pdvector<miniTrampHandle *> mt_buf;

    trampTemplate *baseTramp = proc->baseMap[point];

    if(when == BPatch_callBefore) {
        List<miniTrampHandle *>::iterator preIter = baseTramp->pre_minitramps->get_begin_iter();
        List<miniTrampHandle *>::iterator preEnd = baseTramp->pre_minitramps->get_end_iter();
        for (; preIter != preEnd; preIter++) {
            mt_buf.push_back(*preIter);
        }
    } else if(when == BPatch_callAfter) {
        List<miniTrampHandle *>::iterator postIter = baseTramp->post_minitramps->get_begin_iter();
        List<miniTrampHandle *>::iterator postEnd = baseTramp->post_minitramps->get_end_iter();
        for (; postIter != postEnd; postIter++) {
            mt_buf.push_back(*postIter);
        }
    }

    BPatch_Vector<BPatchSnippetHandle *> snippetVec;
    
    for(unsigned i=0; i<mt_buf.size(); i++) {
        BPatchSnippetHandle *handle = new BPatchSnippetHandle(proc);
        handle->add(mt_buf[i]);
        
        snippetVec.push_back(handle);
    }

    return snippetVec;
}

/*
 * BPatch_point::getAddress
 *
 * Returns the original address of the first instruction at this point.
 */
void *BPatch_point::getAddressInt()
{
    return (void *)point->absPointAddr(proc);
}


/*
 * BPatch_point::usesTrap_NP
 *
 * Returns true if this point is or would be instrumented with a trap, rather
 * than a jump to the base tramp, false otherwise.  On platforms that do not
 * use traps (everything other than x86), it always returns false;
 */
bool BPatch_point::usesTrap_NPInt()
{
#if defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_nt4_0) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
    assert(point);
    assert(proc);

    return point->usesTrap(proc);
#else
    return false;
#endif
}

/*
 * BPatch_point::isDynamic
 *
 * Returns true if this point is a dynamic call site.
 *
 */
bool BPatch_point::isDynamicInt()
{
#if !defined(ia64_unknown_linux2_4)

    if (!dynamic_call_site_flag) return false;
    if (dynamic_call_site_flag == 1) return true;

    assert(proc);
    assert(point);

    bool is_dyn = proc->isDynamicCallSite(point);
    dynamic_call_site_flag = is_dyn ? 1 : 0;
    return is_dyn;
#else
    fprintf(stderr, "%s[%d]:  Dynamic Call Sites not implemented for ia64 yet\n",
            __FILE__, __LINE__);
    return false;
#endif
}

/*
 * BPatch_point::monitorCalls(BPatch_function *userCBFunc)
 *
 * Insert function call to user-defined callback function
 * at dynamic call site.
 *
 * Returns false if BPatch_point is not a dynamic call site.
 *
 */

void *BPatch_point::monitorCallsInt( BPatch_function * user_cb ) {

  if ( !isDynamic() ) {
    fprintf(stderr, "%s[%d]:  call site is not dynamic, cannot monitor\n", 
            __FILE__, __LINE__ );
    return NULL;
  }

  // The callback takes two arguments: the first is the (address of the) callee,
  // the second the (address of the) callsite. 

  pdvector<AstNode *> args;
  if ( (!proc->getDynamicCallSiteArgs( point,args )) || (args.size() != 2) ) {
     fprintf(stderr,"%s[%d]:  could not get address arguments for dynamic call site\n",  
             __FILE__, __LINE__);
     return NULL;
  }

  // construct function call and insert
  int_function * fb = user_cb->func;

  // Monitoring function
  AstNode * func = new AstNode( fb, args );
  miniTrampHandle * mtHandle = NULL;
  addInstFunc(	proc, mtHandle, point, func, callPreInsn,
					orderLastAtPoint,
					true,	/* noCost flag   */
					false,	/* trampRecursiveDesired flag */
					true );	/* allowTrap */

  if ( ! mtHandle ) {
     fprintf( stderr,"%s[%d]:  insertSnippet failed, cannot monitor call site\n",
               __FILE__, __LINE__ );
     return NULL;
  }

  dynamicMonitoringCalls.push_back(mtHandle);

  //  Return pointer to handle as unique id, user does not need to know its a
  //  miniTrampHandle.

  return (void*) mtHandle;
} /* end monitorCalls() */

bool BPatch_point::stopMonitoringInt(void * handle)
{
  miniTrampHandle *target = NULL, *mtHandle = (miniTrampHandle *) handle;

  for (unsigned int i = 0 ; i < dynamicMonitoringCalls.size(); ++i) {
    if (!target) {
      // haven't found it yet -- keep looking
      if (dynamicMonitoringCalls[i] == mtHandle) {
        target = dynamicMonitoringCalls[i];
      } 
    }
    if (target) {
      //  target is found, shift everthing after it one to the left.
      //  (this removes target from the array)
      if ( (i+1) < dynamicMonitoringCalls.size()) {
        dynamicMonitoringCalls[i] = dynamicMonitoringCalls[i+1];
      }
      else {
        // last element, resize vector (length - 1)
        dynamicMonitoringCalls.resize(dynamicMonitoringCalls.size()-1);
      }
    }    
  }

  if (!target) {
    bperr("%s[%d]:  call site not currently monitored", __FILE__, __LINE__);
    return false;
  }

  bool ret;
  ret = deleteInst(proc, target);

  return ret;
}

//  BPatch_point::registerDynamicCallCallback
//
//  Specifies a user-supplied function to be called when a dynamic call is
//  executed.
//
//  Returns a handle (useful for de-registering callback), NULL if error

void *BPatch_point::registerDynamicCallCallbackInt(BPatchDynamicCallSiteCallback cb)
{
  void *ret = NULL;
  BPatch_asyncEventHandler *eventHandler = BPatch::bpatch->eventHandler;
  ret = eventHandler->registerDynamicCallCallback(cb, this);
  if (ret) BPatch::bpatch->asyncActive = true;
  return ret;
}

//  BPatch_point::removeDynamicCallCallback
//
//  Argument is (void *) handle to previously specified callback function to be
//  de-listed.
//
//  Returns true upon success, false if handle is not currently represented

bool BPatch_point::removeDynamicCallCallbackInt(void *handle)
{
  BPatch_asyncEventHandler *eventHandler = BPatch::bpatch->eventHandler;
  return eventHandler->removeDynamicCallCallback(handle);
}
