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
#include "BPatch_image.h"
#include "BPatch_thread.h"
#include "BPatch_function.h"
#include "callbacks.h"

#include "BPatch_private.h"
#include <queue>


#include "ast.h"

#include "sys/stat.h"

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
BPatch_binaryEdit::BPatch_binaryEdit(const char *path, bool openDependencies) :
   BPatch_addressSpace(),
   creation_error(false)
{
  bool has_thread_lib = false;
  pendingInsertions = new BPatch_Vector<batchInsertionRecord *>;
 
  pdvector<std::string> argv_vec;
  pdvector<std::string> envp_vec;
  
  std::string directoryName = "";

  startup_printf("[%s:%u] - Opening original file %s\n", 
                 FILE__, __LINE__, path);
  origBinEdit = BinaryEdit::openFile(std::string(path));
  if (!origBinEdit){
     startup_printf("[%s:%u] - Creation error opening %s\n", 
                    FILE__, __LINE__, path);
     creation_error = true;
     return;
  }
  llBinEdits[path] = origBinEdit;
  
  std::queue<std::string> files;
  origBinEdit->getAllDependencies(files);

  while (files.size()) {
     string s = files.front(); 
     files.pop();

     if (strstr(s.c_str(), "libpthread") ||
         strstr(s.c_str(), "libthread")) {
        has_thread_lib = true;
     }

     if (llBinEdits.count(s))
        continue;
     if (!openDependencies)
        continue;
     
     startup_printf("[%s:%u] - Opening dependant file %s\n", 
                    FILE__, __LINE__, s.c_str());
     BinaryEdit *lib = BinaryEdit::openFile(s);
     if (!lib) {
        startup_printf("[%s:%u] - Creation error opening %s\n", 
                       FILE__, __LINE__, s.c_str());
        creation_error = true;
        return;
     }
     llBinEdits[s] = lib;
     lib->getAllDependencies(files);
  }

  origBinEdit->getDyninstRTLibName();
  std::string rt_name = origBinEdit->dyninstRT_name;
  rtLib = BinaryEdit::openFile(rt_name);
  if (!rtLib) {
     startup_printf("[%s:%u] - ERROR.  Could not open RT library\n",
                    __FILE__, __LINE__);
     creation_error = true;
     return;
  }

  std::map<std::string, BinaryEdit*>::iterator i, j;
  for(i = llBinEdits.begin(); i != llBinEdits.end(); i++) {
     (*i).second->setupRTLibrary(rtLib);
  }

  int_variable* masterTrampGuard = origBinEdit->createTrampGuard();
  assert(masterTrampGuard);
  
  for(i = llBinEdits.begin(); i != llBinEdits.end(); i++) {
     BinaryEdit *llBinEdit = (*i).second;
     llBinEdit->registerFunctionCallback(createBPFuncCB);
     llBinEdit->registerInstPointCallback(createBPPointCB);
     llBinEdit->set_up_ptr(this);
     llBinEdit->setupRTLibrary(rtLib);
     llBinEdit->setTrampGuard(masterTrampGuard);
     llBinEdit->setMultiThreadCapable(has_thread_lib);
     for (j = llBinEdits.begin(); j != llBinEdits.end(); j++) {
        llBinEdit->addSibling((*j).second);
     }
  }

  image = new BPatch_image(this);
}

BPatch_image * BPatch_binaryEdit::getImageInt() {
	return image;
}

void BPatch_binaryEdit::BPatch_binaryEdit_dtor()
{
   if (image) 
      delete image;
   
   image = NULL;

   if (pendingInsertions) {
     for (unsigned f = 0; f < pendingInsertions->size(); f++) {
       delete (*pendingInsertions)[f];
     }
     delete pendingInsertions;
     pendingInsertions = NULL;
   }

  std::map<std::string, BinaryEdit*>::iterator i = llBinEdits.begin();
  for(; i != llBinEdits.end(); i++) {
     delete (*i).second;
  }
  llBinEdits.clear();
  origBinEdit = NULL;
  
  assert(BPatch::bpatch != NULL);
}

bool BPatch_binaryEdit::writeFileInt(const char * outFile)
{
    assert(pendingInsertions);

    // This should be a parameter...
    bool atomic = false;
   
    // Define up here so we don't have gotos causing issues
    std::set<int_function *> instrumentedFunctions;
    
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
            
            miniTramp *mini = point->addInst(bir->snip.ast_wrapper,
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

   // Having inserted the requests, we hand things off to functions to 
   // actually do work. First, develop a list of unique functions. 

   for (unsigned i = 0; i < pendingInsertions->size(); i++) {
       batchInsertionRecord *&bir = (*pendingInsertions)[i];
       for (unsigned j = 0; j < bir->points_.size(); j++) {
           BPatch_point *bppoint = bir->points_[j];
           instPoint *point = bppoint->point;
           point->optimizeBaseTramps(bir->when_[j]);

           instrumentedFunctions.insert(point->func());
       }
   }

   for (std::set<int_function *>::iterator funcIter = instrumentedFunctions.begin();
        funcIter != instrumentedFunctions.end();
        funcIter++) {

       pdvector<instPoint *> failedInstPoints;
       (*funcIter)->performInstrumentation(atomic,
                                           failedInstPoints); 
       if (failedInstPoints.size() && atomic) {
           err = true;
           goto cleanup;
       }
   }

   
   // Now that we've instrumented we can see if we need to replace the
   // trap handler.
   replaceTrapHandler();


   if (atomic && err) 
      goto cleanup;

   for(std::map<std::string, BinaryEdit*>::iterator i = llBinEdits.begin();
       i != llBinEdits.end(); i++) 
   {
      (*i).second->trapMapping.flush();
   }

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

    origBinEdit->writeFile(outFile);

    std::map<std::string, BinaryEdit *>::iterator i;
    for (i = llBinEdits.begin(); i != llBinEdits.end(); i++) {
       BinaryEdit *bin = (*i).second;
       if (bin == origBinEdit)
          continue;
       if (!bin->isDirty())
          continue;
       
       std::string newname = bin->getMappedObject()->fileName();
       bin->writeFile(newname);
    }


    return ret;
}

bool BPatch_binaryEdit::getType()
{
  return STATIC_EDITOR;
}

void BPatch_binaryEdit::getAS(std::vector<AddressSpace *> &as)
{
   std::map<std::string, BinaryEdit*>::iterator i = llBinEdits.begin();
   as.push_back(origBinEdit);
   for(; i != llBinEdits.end(); i++) {
      if ((*i).second == origBinEdit)
         continue;
      as.push_back((*i).second);
   }
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


bool BPatch_binaryEdit::finalizeInsertionSetInt(bool /*atomic*/, bool * /*modified*/) 
{
    return true;
}

bool BPatch_binaryEdit::loadLibraryInt(const char *libname, bool deps)
{
   std::queue<std::string> libs;
   libs.push(libname);
   std::map<string, BinaryEdit*> newlibs;
   while (libs.size()) {
      string s = libs.front(); 
      libs.pop();
      
      if (llBinEdits.count(s) || newlibs.count(s))
         continue;
      BinaryEdit *file = BinaryEdit::openFile(s);
      if (!file) {
         std::map<string, BinaryEdit*>::iterator i;
         for (i = newlibs.begin(); i != newlibs.end(); i++)
            delete (*i).second;
         return false;
      }
      newlibs[s] = file;

      if (deps)
         file->getAllDependencies(libs);
   }
   
   std::map<string, BinaryEdit*>::iterator i;
   for (i = newlibs.begin(); i != newlibs.end(); i++)
      llBinEdits[(*i).first] = (*i).second;

   return true;
}

// Here's the story. We may need to install a trap handler for instrumentation
// to work in the rewritten binary. This doesn't play nicely with trap handlers
// that the binary itself registers. So we're going to replace every call to
// sigaction in the binary with a call to our wrapper. This wrapper:
//   1) Ignores attempts to register a SIGTRAP
//   2) Passes everything else through to sigaction
// It's called "dyn_sigaction".
// This is just a multiplexing function over each child binaryEdit
// object because they're all individual.

bool BPatch_binaryEdit::replaceTrapHandler() {
    // Did we use a trap?

    bool usedATrap = false;

    std::map<std::string, BinaryEdit *>::iterator iter = llBinEdits.begin();
    for (; iter != llBinEdits.end(); iter++) {
        if (iter->second == rtLib) continue;
        if (iter->second->usedATrap()) {
            usedATrap = true;
        }
    }

    if (!usedATrap) return true;

    // We used a trap, so go through and set up the replacement instrumentation.
    // However, don't let this be the first piece of instrumentation put into
    // a library.

    bool success = true;
    iter = llBinEdits.begin();
    for (; iter != llBinEdits.end(); iter++) {
        BinaryEdit *binEd = iter->second;
        if (binEd == rtLib) {
            continue;
        }

        // Did we instrument this already?
        if (!binEd->isDirty()) {
            continue;
        }

        // Okay, replace trap handler
        if (!binEd->replaceTrapHandler()) {
            success = false;
        }
    }
    return success;
}
    

