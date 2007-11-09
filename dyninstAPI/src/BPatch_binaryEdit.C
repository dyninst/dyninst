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
#include "binaryEdit.h"
#include "addressSpace.h"
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
#include "BPatch.h"
#include "BPatch_addressSpace.h"

#include "BPatch_binaryEdit.h"
#include "BPatch_thread.h"
#include "LineInformation.h"
#include "BPatch_function.h"
#include "callbacks.h"

#include "BPatch_private.h"



#include "ast.h"


/*
 * BPatch_binaryEdit::BPatch_binaryEdit
 *
 * Starts a new process and associates it with the BPatch_binaryEdit being
 * constructed.  The new process is placed into a stopped state before
 * executing any code.
 *
 * path		Pathname of the executable to start.
 * argv		A list of pointers to character strings which are the
 *              arguments for the new process, terminated by a NULL pointer.
 * envp		A list of pointers to character strings which are the
 *              environment variables for the new process, terminated by a
 *              NULL pointer.  If NULL, the default environment will be used.
 */
BPatch_binaryEdit::BPatch_binaryEdit(const char *path) :
   BPatch_addressSpace(),
   llBinEdit(NULL),
   creation_error(false)
{
  func_map = new BPatch_funcMap();
  instp_map = new BPatch_instpMap();
  pendingInsertions = new BPatch_Vector<batchInsertionRecord *>;
 
  pdvector<pdstring> argv_vec;
  pdvector<pdstring> envp_vec;
  
  pdstring directoryName = "";
  
  llBinEdit = BinaryEdit::openFile(pdstring(path));
  if (!llBinEdit){
     creation_error = true;
     return;
  }
  
  startup_cerr << "Registering function callback..." << endl;
  llBinEdit->registerFunctionCallback(createBPFuncCB);
  
  startup_cerr << "Registering instPoint callback..." << endl;
  llBinEdit->registerInstPointCallback(createBPPointCB);
  
  llBinEdit->set_up_ptr(this);

  image = new BPatch_image(this);

    // Nothing else to do...

}

void BPatch_binaryEdit::BPatch_binaryEdit_dtor()
{
   if (image) 
      delete image;
   
   image = NULL;

   if (func_map)
      delete func_map;
   func_map = NULL;
   if (instp_map)
      delete instp_map;
   instp_map = NULL;

   if (pendingInsertions) {
     for (unsigned f = 0; f < pendingInsertions->size(); f++) {
       delete (*pendingInsertions)[f];
     }
     delete pendingInsertions;
     pendingInsertions = NULL;
   }

   delete llBinEdit;
   llBinEdit = NULL;

   assert(BPatch::bpatch != NULL);
}

bool BPatch_binaryEdit::writeFileInt(const char * outFile)
{
    assert(pendingInsertions);

    // This should be a parameter...
    bool atomic = false;
   
    
    // Two loops: first addInst, then generate/install/link
    pdvector<miniTramp *> workDone;
    bool err = false;

    for (unsigned i = 0; i < pendingInsertions->size(); i++) {
        batchInsertionRecord *&bir = (*pendingInsertions)[i];
        assert(bir);

        // Don't handle thread inst yet...
        assert(!bir->thread_);

        if (!bir->points_.size()) {
          fprintf(stderr, "%s[%d]:  WARN:  zero points for insertion record\n", FILE__, __LINE__);
          fprintf(stderr, "%s[%d]:  failing to addInst\n", FILE__, __LINE__);
        }

        for (unsigned j = 0; j < bir->points_.size(); j++) {
            BPatch_point *bppoint = bir->points_[j];
            instPoint *point = bppoint->point;
            callWhen when = bir->when_[j];
            
            miniTramp *mini = point->addInst(*(bir->snip.ast_wrapper),
                                             when,
                                             bir->order_,
                                             bir->trampRecursive_,
                                             false);
            if (mini) {
                workDone.push_back(mini);
                // Add to snippet handle
                bir->handle_->addMiniTramp(mini);
            }
            else {
                err = true;
                if (atomic) break;
            }
        }
        if (atomic && err)
            break;
    }
    
   if (atomic && err) goto cleanup;

   // All generation first. Actually, all generation per function...
   // but this is close enough.
   for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
       batchInsertionRecord *&bir = (*pendingInsertions)[i];
       assert(bir);
        if (!bir->points_.size()) {
          fprintf(stderr, "%s[%d]:  WARN:  zero points for insertion record\n", FILE__, __LINE__);
          fprintf(stderr, "%s[%d]:  failing to generateInst\n", FILE__, __LINE__);
        }
       for (unsigned j = 0; j < bir->points_.size(); j++) {
           BPatch_point *bppoint = bir->points_[j];
           instPoint *point = bppoint->point;

           point->optimizeBaseTramps(bir->when_[j]);
           if (!point->generateInst()) {
               err = true;
               if (atomic && err) break;
           }
       }
       if (atomic && err) break;
   }

   if (atomic && err) goto cleanup;

   //  next, all installing 
   for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
       batchInsertionRecord *&bir = (*pendingInsertions)[i];
       assert(bir);
        if (!bir->points_.size()) {
          fprintf(stderr, "%s[%d]:  WARN:  zero points for insertion record\n", FILE__, __LINE__);
          fprintf(stderr, "%s[%d]:  failing to installInst\n", FILE__, __LINE__);
        }
       for (unsigned j = 0; j < bir->points_.size(); j++) {
           BPatch_point *bppoint = bir->points_[j];
           instPoint *point = bppoint->point;
             
           if (!point->installInst()) {
              err = true;
           }

           if (atomic && err) break;
       }
       if (atomic && err) break;
   }

   if (atomic && err) goto cleanup;

   //  finally, do all linking 
   for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
       batchInsertionRecord *&bir = (*pendingInsertions)[i];
       assert(bir);
        if (!bir->points_.size()) {
          fprintf(stderr, "%s[%d]:  WARN:  zero points for insertion record\n", FILE__, __LINE__);
          fprintf(stderr, "%s[%d]:  failing to linklInst\n", FILE__, __LINE__);
        }
       for (unsigned j = 0; j < bir->points_.size(); j++) {
           BPatch_point *bppoint = bir->points_[j];
           instPoint *point = bppoint->point;
             
          if (!point->linkInst()) {
               err = true;
           }

           if (atomic && err) break;
       }
       if (atomic && err) break;
   }

   if (atomic && err) 
      goto cleanup;

  cleanup:
    bool ret = true;

    if (atomic && err) {
        // Something failed...   Cleanup...
        for (unsigned k = 0; k < workDone.size(); k++) {
            workDone[k]->uninstrument();
        }
        ret = false;
    }

    for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
        batchInsertionRecord *&bir = (*pendingInsertions)[i];
        assert(bir);
        delete(bir);
    }

    pendingInsertions->clear();

    llBinEdit->writeFile(outFile);

    return ret;
}



bool BPatch_binaryEdit::getType()
{
  return STATIC_EDITOR;
}

AddressSpace * BPatch_binaryEdit::getAS()
{
  return llBinEdit;
}


// This handles conversion without requiring inst.h in a header file...
extern bool BPatchToInternalArgs(BPatch_point *point,
                                 BPatch_callWhen when,
                                 BPatch_snippetOrder order,
                                 callWhen &ipWhen,
                                 callOrder &ipOrder);


/*
 * BPatch_binaryEdit::insertSnippet
 *
 * Insert a code snippet at a given instrumentation point.  Upon success,
 * returns a handle to the created instance of the snippet, which can be used
 * to delete it.  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * point	The point at which to insert it.
 */
BPatchSnippetHandle *BPatch_binaryEdit::insertSnippetInt(const BPatch_snippet &expr, 
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
 * BPatch_binaryEdit::insertSnippet
 *
 * Insert a code snippet at a given instrumentation point.  Upon succes,
 * returns a handle to the created instance of the snippet, which can be used
 * to delete it.  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * point	The point at which to insert it.
 */

BPatchSnippetHandle *BPatch_binaryEdit::insertSnippetWhen(const BPatch_snippet &expr,
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
 * BPatch_binaryEdit::insertSnippet
 *
 * Insert a code snippet at each of a list of instrumentation points.  Upon
 * success, Returns a handle to the created instances of the snippet, which
 * can be used to delete them (as a unit).  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * points	The list of points at which to insert it.
 */
// A lot duplicated from the single-point version. This is unfortunate.

BPatchSnippetHandle *BPatch_binaryEdit::insertSnippetAtPointsWhen(const BPatch_snippet &expr,
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
		  "address %p, when %d, order %d in static edit\n",
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
    
    callWhen ipWhen;
    callOrder ipOrder;
    
    if (point->addSpace == NULL) {
        fprintf(stderr, "Error: attempt to use point with no process info\n");
        continue;
    }
    if (dynamic_cast<BPatch_binaryEdit *>(point->addSpace) != this) {
        fprintf(stderr, "Error: attempt to use point specific to a different process\n");
        continue;
    }        

#if defined(os_aix) || defined(arch_x86_64)
        // Toss the const; the function _pointer_ doesn't though.
    BPatch_function *func = point->getFunction();
    func->calc_liveness(point);
#endif 

    
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
  assert(pendingInsertions);
  pendingInsertions->push_back(rec);
  return ret; 
}


/*
 * BPatch_binaryEdit::insertSnippet
 *
 * Insert a code snippet at each of a list of instrumentation points.  Upon
 * success, Returns a handle to the created instances of the snippet, which
 * can be used to delete them (as a unit).  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * points	The list of points at which to insert it.
 */

BPatchSnippetHandle *BPatch_binaryEdit::insertSnippetAtPoints(
                 const BPatch_snippet &expr,
                 const BPatch_Vector<BPatch_point *> &points,
                 BPatch_snippetOrder order)
{
    return insertSnippetAtPointsWhen(expr,
                                     points,
                                     BPatch_callUnset,
                                     order);
}



/*
 * BPatch_addressSpace::beginInsertionSet
 * 
 * Starts a batch insertion set; that is, all calls to insertSnippet until
 * finalizeInsertionSet are delayed.
 *
 */

void BPatch_binaryEdit::beginInsertionSetInt() 
{
    return;
}


bool BPatch_binaryEdit::finalizeInsertionSetInt(bool atomic, bool *modified) 
{
    return true;
}
