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
#ifdef rs6000_ibm_aix4_1
#include <memory.h>
#endif

#define BPATCH_FILE

#include "BPatch_point.h"
#include "BPatch_snippet.h"
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

    if (baseTramp->pre_minitramps) {
        List<miniTrampHandle *>::iterator preIter = baseTramp->pre_minitramps->get_begin_iter();
        List<miniTrampHandle *>::iterator preEnd = baseTramp->pre_minitramps->get_end_iter();
        for (; preIter != preEnd; preIter++) {
            mt_buf.push_back(*preIter);
        }
    }
    if (baseTramp->post_minitramps) {
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
