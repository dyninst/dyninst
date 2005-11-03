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

#ifndef _BPatch_thread_h_
#define _BPatch_thread_h_

#include <stdio.h>

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_image.h"
#include "BPatch_snippet.h"
#include "BPatch_eventLock.h"
#include "BPatch_process.h"
#include "BPatch_frame.h"

class process;
class BPatch;
class BPatch_thread;
class BPatch_process;
class dyn_thread;

typedef long dynthread_t;

/*
 * Represents a thread of execution.
 */
#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_thread

class BPATCH_DLL_EXPORT BPatch_thread : public BPatch_eventLock {
    friend class BPatch_frame;
    friend class BPatch_process;
    friend bool pollForStatusChange();
    friend class BPatch_asyncEventHandler;
    friend class process;

    bool legacy_destructor;
    BPatch_process *proc;
    dyn_thread *llthread;
    unsigned index;
    bool updated;

 protected:
    BPatch_thread(BPatch_process *parent, int ind, int lwp_id);
    BPatch_thread(BPatch_process *parent, dyn_thread *dthr);

    void updateValues(dynthread_t tid, unsigned long stack_start, 
                      BPatch_function *initial_func, int lwp_id);
 public:

    /**
     * The following function are all deprecated.  They've been replaced
     * by equivently named functions in BPatch_process.  See BPatch_process.h
     * for documentation.
     **/  
    BPatch_image *getImage() { return proc->getImage(); }
    int getPid() { return proc->getPid(); }
    bool stopExecution() { return proc->stopExecution(); }
    bool continueExecution() { return proc->continueExecution(); }
    bool terminateExecution() { return proc->terminateExecution(); }
    bool isStopped() { return proc->isStopped(); }
    int stopSignal() { return proc->stopSignal(); }
    bool isTerminated() { return proc->isTerminated(); }
    BPatch_exitType terminationStatus() { return proc->terminationStatus(); }
    int getExitCode() { return proc->getExitCode(); }
    int getExitSignal() { return proc->getExitSignal(); }
    bool detach(bool cont) { return proc->detach(cont); }
    bool isDetached() { return proc->isDetached(); }
    bool dumpCore(const char *file, bool terminate) 
       { return proc->dumpCore(file, terminate); }
    bool dumpImage(const char *file) { return proc->dumpImage(file); }
    char *dumpPatchedImage(const char *file) 
       { return proc->dumpPatchedImage(file); }
    BPatch_variableExpr *malloc(int n) { return proc->malloc(n); }
    BPatch_variableExpr *malloc(const BPatch_type &type) 
       { return proc->malloc(type); }
    bool free(BPatch_variableExpr &ptr) { return proc->free(ptr); }
    BPatch_variableExpr *getInheritedVariable(BPatch_variableExpr &pVar)
       { return proc->getInheritedVariable(pVar); }
    BPatchSnippetHandle *getInheritedSnippet(BPatchSnippetHandle &parentSnippet)
       { return proc->getInheritedSnippet(parentSnippet); }
    BPatchSnippetHandle *insertSnippet(const BPatch_snippet &expr,
          BPatch_point &point, BPatch_snippetOrder order = BPatch_firstSnippet)
       { return proc->insertSnippet(expr, point, order); }
    BPatchSnippetHandle *insertSnippet(const BPatch_snippet &expr,
          BPatch_point &point, BPatch_callWhen when, 
          BPatch_snippetOrder order = BPatch_firstSnippet)
       { return proc->insertSnippet(expr, point, when, order); }
    BPatchSnippetHandle *insertSnippet(const BPatch_snippet &expr,
          const BPatch_Vector<BPatch_point *> &points, 
          BPatch_snippetOrder order = BPatch_firstSnippet)
       { return proc->insertSnippet(expr, points, order); }
    BPatchSnippetHandle *insertSnippet(const BPatch_snippet &expr,
          const BPatch_Vector<BPatch_point *> &points, BPatch_callWhen when,
          BPatch_snippetOrder order = BPatch_firstSnippet)
       { return proc->insertSnippet(expr, points, when, order); }
    bool deleteSnippet(BPatchSnippetHandle *handle) 
       { return proc->deleteSnippet(handle); }
    bool setMutationsActive(bool activate) 
       { return proc->setMutationsActive(activate); }
    bool replaceFunctionCall(BPatch_point &point, BPatch_function &newFunc)
       { return proc->replaceFunctionCall(point, newFunc); }
    bool removeFunctionCall(BPatch_point &point) 
       { return proc->removeFunctionCall(point); }
    bool replaceFunction(BPatch_function &oldFunc, BPatch_function &newFunc)
       { return proc->replaceFunction(oldFunc, newFunc); }
    void *oneTimeCode(const BPatch_snippet &expr)
       { return proc->oneTimeCode(expr); }
    bool oneTimeCodeAsync(const BPatch_snippet &expr, void *userData = NULL)
       { return proc->oneTimeCodeAsync(expr, userData); }
    bool loadLibrary(const char *libname, bool reload = false)
       { return proc->loadLibrary(libname, reload); }
    bool getSourceLines( unsigned long addr, std::vector< std::pair< const char *, unsigned int > > & lines )
       { return proc->getSourceLines( addr, lines ); }
    BPatch_function *findFunctionByAddr(void *addr)
       { return proc->findFunctionByAddr(addr); }
    void enableDumpPatchedImage() { proc->enableDumpPatchedImage(); }
#ifdef IBM_BPATCH_COMPAT
    bool addSharedObject(const char *name, const unsigned long loadaddr)
       { return proc->addSharedObject(name, loadaddr); }
#endif

    //  BPatch_thread::getCallStack
    //  
    //  Returns a vector of BPatch_frame, representing the current call stack
    API_EXPORT(Int, (stack),
    bool,getCallStack,(BPatch_Vector<BPatch_frame>& stack));
    //  BPatch_thread::getProcess
    //  Returns a pointer to the process that owns this thread
    API_EXPORT(Int, (),
    BPatch_process *, getProcess, ());

    API_EXPORT(Int, (),
    dynthread_t, getTid, ());

    API_EXPORT(Int, (),
    int, getLWP, ());

    API_EXPORT(Int, (),
    unsigned, getBPatchID, ());

    API_EXPORT(Int, (),
    BPatch_function *, getInitialFunc, ());
    
    API_EXPORT(Int, (),
    unsigned long, getStackTopAddr, ());

    API_EXPORT_DTOR(_dtor, (),
    ~,BPatch_thread,());

    API_EXPORT(Int, (),
    unsigned long, os_handle, ());

    // DO NOT USE
    // this function should go away as soon as Paradyn links against Dyninst
    process *lowlevel_process() { return proc->llproc; }

    // DO NOT USE
    // this function should go away as soon as Paradyn links against Dyninst
    process *PDSEP_process() { return proc->llproc; }
};

#endif /* BPatch_thread_h_ */
