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

#define BPATCH_FILE

#include "BPatch_point.h"
#include "BPatch_snippet.h"
#include "BPatch_type.h"
#include "BPatch_image.h"
#include "BPatch_collections.h"
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
  proc(_proc), func(_func), point(_point), pointType(_pointType), memacc(_ma),
  dynamicMonitoringCall(NULL)
{
  point->bppoint = this;
  if (_pointType == BPatch_subroutine)
    dynamic_call_site_flag = 2; // dynamic status unknown
  else
    dynamic_call_site_flag = 0; // not a call site, so not a dynamic call site.
}

/*
 * BPatch_point::getCalledFunction
 *
 * For a BPatch_point representing a call site, returns a pointer to a
 * BPatch_function that represents the function being called.  If the point
 * isn't a call site, returns NULL.
 */
BPatch_function *BPatch_point::getCalledFunction()
{
   BPatch_function *ret;
   
   assert(point);
   
   if (point->getPointType() != callSite)
      return NULL;
   
   function_base *_func;
   
   if (!proc->findCallee(*point, _func))
    	return NULL;
   
   if (_func != NULL)
      ret = proc->PDFuncToBPFuncMap[_func];
   else
      ret = NULL;
    
   return ret;
}

const BPatch_Vector<BPatchSnippetHandle *> BPatch_point::getCurrentSnippets() {
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

const BPatch_Vector<BPatchSnippetHandle *> BPatch_point::getCurrentSnippets(BPatch_callWhen when) {
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
void *BPatch_point::getAddress()
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
bool BPatch_point::usesTrap_NP()
{
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)
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
bool BPatch_point::isDynamic()
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

bool BPatch_point::monitorCalls(BPatch_function *user_cb)
{
#if !defined(ia64_unknown_linux2_4)
   if (!isDynamic()) {
     fprintf(stderr, "%s[%d]:  call site is not dynamic, cannot monitor\n",
             __FILE__, __LINE__);
     return false;
   }
   if (dynamicMonitoringCall) {
     // we are already monitoring this site.
     // we could allow more than 1 function to monitor a site, but
     // unless someone really needs this behavior, lets just keep it simple
     //bperr("%s[%d]:  call site already being monitored", __FILE__, __LINE__);
     fprintf(stderr, "%s[%d]:  call site already being monitored", __FILE__, __LINE__);
     return false;
   }

#ifdef NOTDEF // PDSEP -- egad, type checking is gonna be tricky
   //  Make sure user_cb has 2 paramaters, and they are both 'void *'

   BPatch_Vector<BPatch_localVar *> *params = user_cb->getParams();
   if ((!params) || !(params->size() == 2)) {
     char fname[2048];
     user_cb->getName(fname, 2048);
     /*
     bperr("%s[%d]:  wrong number of args (%d != 2) for function %s, %s", 
           __FILE__, __LINE__, params ? params->size() : 0, fname,
           "cannot monitor dynamic calls using this function.");
     */
     fprintf(stderr,"%s[%d]:  wrong number of args (%d != 2) for function %s, %s",
           __FILE__, __LINE__, params ? params->size() : 0, fname,
           "cannot monitor dynamic calls using this function.");

     return false;
   }

   BPatch_type *vstype = BPatch::bpatch->stdTypes->findType("void *");
   if (!vstype) {
     //bperr("%s[%d]:  cannot find internal type 'void *'.", __FILE__, __LINE__);
     fprintf(stderr,"%s[%d]:  cannot find internal type 'void *'.", __FILE__, __LINE__);
     return false;
   }

   for (unsigned int i = 0; i < params->size(); ++i) {
     BPatch_type *param_type = (*params)[i]->getType();
     if (!param_type || !(*param_type == *vstype)) {
       char fname[2048];
       user_cb->getName(fname, 2048);
       /*
       bperr("%s[%d]: bad type '%s' for param %d of function %s", 
             __FILE__, __LINE__, 
             param_type ? "<no param type>" : param_type->getName(),
             i, fname);
        */
       fprintf(stderr,"%s[%d]: bad type '%s' for param %d of function %s",
             __FILE__, __LINE__,
             param_type ? "<no param type>" : param_type->getName(),
             i, fname);

       return false;
     }
   }
#endif
   // get address arguments to user_cb (platform dep)

   pdvector<AstNode *> args;
   if ((!proc->getDynamicCallSiteArgs(point,args)) || (args.size() != 2)) {

     //bperr("%s[%d]:  could not get address arguments for dynamic call site",
      //     __FILE__, __LINE__);
     fprintf(stderr,"%s[%d]:  could not get address arguments for dynamic call site",
           __FILE__, __LINE__);

     return false;
   }

   // construct function call and insert
   function_base *fb = user_cb->func;

   // Monitoring function
   AstNode *func = new AstNode(fb, args);
   miniTrampHandle *mtHandle = NULL;
   addInstFunc(proc, mtHandle, point, func, callPreInsn,
               //orderFirstAtPoint,
               orderLastAtPoint,
               true,                        /* noCost flag   */
               false,                       /* trampRecursiveDesired flag */
               true);                       /* allowTrap */
   dynamicMonitoringCall = mtHandle;

#ifdef NOTDEF // PDSEP

   BPatch_funcCallExpr *fs = new BPatch_funcCallExpr(*user_cb, args);
   BPatch_thread *appThread = proc->bpatch_thread;
   dynamicMonitoringCall = appThread->insertSnippet(*fs, *this, 
                                                    BPatch_callBefore, 
                                                    BPatch_lastSnippet);
#endif
   if (!dynamicMonitoringCall) {
    // bperr("%s[%d]:  insertSnippet failed, cannot monitor call site", 
     //       __FILE__, __LINE__);
     fprintf(stderr,"%s[%d]:  insertSnippet failed, cannot monitor call site", 
            __FILE__, __LINE__);
     return false;
   }

   return true;
#else
    fprintf(stderr, "%s[%d]:  Dynamic Call Sites not implemented for ia64 yet\n",
            __FILE__, __LINE__);
    return false;
#endif

}

bool BPatch_point::stopMonitoring()
{
  if (!dynamicMonitoringCall) {
    bperr("%s[%d]:  call site not currently monitored", __FILE__, __LINE__);
    return false;
  }
  bool ret;
  ret = deleteInst(proc, dynamicMonitoringCall);
  dynamicMonitoringCall = NULL;

#ifdef NOTDEF // PDSEP
  ret = proc->bpatch_thread->deleteSnippet(dynamicMonitoringCall);
#endif
  return ret;
}
