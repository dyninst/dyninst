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

#ifndef _BPatch_addressSpace_h_
#define _BPatch_addressSpace_h_

#include "BPatch_snippet.h"
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_eventLock.h"
#include "BPatch_point.h"

#include "BPatch_callbacks.h"

#include <vector>

#include <stdio.h>
#include <signal.h>

class AddressSpace;
class miniTrampHandle;
class miniTramp;
class BPatch;
class BPatch_funcMap;
class BPatch_instpMap;
class int_function;
struct batchInsertionRecord;

typedef enum{
  TRADITIONAL_PROCESS, STATIC_EDITOR
    } processType;


#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatchSnippetHandle
class BPATCH_DLL_EXPORT BPatchSnippetHandle : public BPatch_eventLock {
    friend class BPatch_point;
    friend class BPatch_image;
    friend class BPatch_process;
    friend class BPatch_binaryEdit;
    friend class BPatch_addressSpace;
    friend class BPatch_thread;

private:    
    // Address Space snippet belogns to
    BPatch_addressSpace *addSpace_;

    // low-level mappings for removal
    BPatch_Vector<miniTramp *> mtHandles_;

    //  a flag for catchuo
    bool catchupNeeded;
    //  and a list of threads to apply catchup to
    BPatch_Vector<BPatch_thread *> catchup_threads;
    
    BPatchSnippetHandle(BPatch_addressSpace * addSpace);

    void addMiniTramp(miniTramp *m) { mtHandles_.push_back(m); }
    
public:

    API_EXPORT_DTOR(_dtor, (),
    ~,BPatchSnippetHandle,());

    // Returns whether the installed miniTramps use traps.
    // Not 100% accurate due to internal Dyninst design; we can
    // have multiple instances of instrumentation due to function
    // relocation.
    API_EXPORT(Int, (), bool, usesTrap, ());

    API_EXPORT(Int, (),
    BPatch_addressSpace *, getAddressSpace, ());

    API_EXPORT(Int, (),
    BPatch_process *, getProcess, ());
    
    API_EXPORT(Int, (),
    BPatch_Vector<BPatch_thread *> &, getCatchupThreads, ());
   
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_addressSpace

class BPATCH_DLL_EXPORT BPatch_addressSpace : public BPatch_eventLock {
    friend class BPatch;
    friend class BPatch_image;
    friend class BPatch_function;
    friend class BPatch_frame;
    friend class BPatch_module;
    friend class BPatch_basicBlock;
    friend class BPatch_flowGraph;
    friend class BPatch_loopTreeNode;
    friend class BPatch_point;
    friend class BPatch_funcCallExpr;
    friend class BPatch_eventMailbox;
    friend class BPatch_instruction;
  
 protected:
    
  BPatch_function *findOrCreateBPFunc(int_function *ifunc, 
				      BPatch_module *bpmod);

  BPatch_point *findOrCreateBPPoint(BPatch_function *bpfunc, instPoint *ip,
					    BPatch_procedureLocation pointType);
  
  
  // These callbacks are triggered by lower-level code and forward
  // calls up to the findOrCreate functions.
  static BPatch_function *createBPFuncCB(AddressSpace *p, int_function *f);
  static BPatch_point *createBPPointCB(AddressSpace *p, int_function *f,
				       instPoint *ip, int type);

  BPatch_Vector<batchInsertionRecord *> *pendingInsertions;

  BPatch_funcMap *func_map;
  BPatch_instpMap *instp_map;
  
  BPatch_image *image;

  //  AddressSpace * as;
  
 
 public:

  BPatch_addressSpace();

  virtual AddressSpace * getAS() = 0;

  virtual ~BPatch_addressSpace();

  // Distinguishes between BPatch_process and BPatch_binaryEdit
  virtual bool getType() = 0;  

  // Returns back bools for variables that are BPatch_process member variables,
  //   the return value is hardcoded for BPatch_binaryEdit
  virtual bool getTerminated() = 0;
  virtual bool getMutationsActive() = 0;

  //  BPatch_addressSpace::insertSnippet
  //  
  //  Insert new code into the mutatee
  API_EXPORT_VIRT(Int, (expr, point, order),
                  BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr, 
                                                       BPatch_point &point,
                                                       BPatch_snippetOrder order = BPatch_firstSnippet));
  
    //BPatch_addressSpace::insertSnippet
      
    //Insert new code into the mutatee, specifying "when" (before/after point)

    API_EXPORT_VIRT(When, (expr, point, when, order),
                    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr, 
                                                         BPatch_point &point,
                                                         BPatch_callWhen when,
                                                         BPatch_snippetOrder order = BPatch_firstSnippet));
    
    //BPatch_addressSpace::insertSnippet
      
    //Insert new code into the mutatee at multiple points

    API_EXPORT_VIRT(AtPoints, (expr, points, order),
                    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr,
                                                         const BPatch_Vector<BPatch_point *> &points,
                                                         BPatch_snippetOrder order = BPatch_firstSnippet));
    
      // BPatch_addressSpace::insertSnippet
      
      //Insert new code into the mutatee at multiple points, specifying "when"

    API_EXPORT_VIRT(AtPointsWhen, (expr, points, when, order),
                    BPatchSnippetHandle *,insertSnippet,(const BPatch_snippet &expr,
                                                         const BPatch_Vector<BPatch_point *> &points,
                                                         BPatch_callWhen when,
                                                         BPatch_snippetOrder order = BPatch_firstSnippet));



  
  virtual void beginInsertionSet() = 0;

  virtual bool finalizeInsertionSet(bool atomic, bool *modified) = 0;
 

  //  BPatch_addressSpace::deleteSnippet
  //  
  //  Remove instrumentation from the mutatee process

  API_EXPORT(Int, (handle),
	     bool,deleteSnippet,(BPatchSnippetHandle *handle));

  //  BPatch_addressSpace::replaceCode
  //
  //  Replace a point (must be an instruction...) with a given BPatch_snippet

    API_EXPORT(Int, (point, snippet), 
    bool, replaceCode, (BPatch_point *point, BPatch_snippet *snippet));

    //  BPatch_addressSpace::replaceFunctionCall
    //  
    //  Replace function call at one point with another

    API_EXPORT(Int, (point, newFunc),
    bool,replaceFunctionCall,(BPatch_point &point, BPatch_function &newFunc));

    //  BPatch_addressSpace::removeFunctionCall
    //  
    //  Remove function call at one point 

    API_EXPORT(Int, (point),
    bool,removeFunctionCall,(BPatch_point &point));

    //  BPatch_addressSpace::replaceFunction
    //  
    //  Replace all calls to a function with calls to another

    API_EXPORT(Int, (oldFunc, newFunc),
    bool,replaceFunction,(BPatch_function &oldFunc, BPatch_function &newFunc));

    //  BPatch_addressSpace::getSourceLines
    //  
    //  Method that retrieves the line number and file name corresponding 
    //  to an address

    API_EXPORT(Int, (addr, lines),
    bool,getSourceLines,(unsigned long addr, BPatch_Vector< BPatch_statement > & lines ));
    
    // BPatch_addressSpace::getAddressRanges
    //
    // Method that retrieves address range(s) for a given filename and line number.
    
    API_EXPORT(Int, (fileName, lineNo, ranges),
    bool,getAddressRanges,(const char * fileName, unsigned int lineNo, std::vector< std::pair< unsigned long, unsigned long > > & ranges ));
	
    //  BPatch_addressSpace::findFunctionByAddr
    //  
    //  Returns the function containing an address

    API_EXPORT(Int, (addr),
    BPatch_function *,findFunctionByAddr,(void *addr));

     //  BPatch_addressSpace::getImage
    //
    //  Obtain BPatch_image associated with this BPatch_addressSpace

    API_EXPORT(Int, (),
    BPatch_image *,getImage,());


    //  BPatch_addressSpace::malloc
    //  
    //  Allocate memory for a new variable in the mutatee process

    API_EXPORT(Int, (n),
    BPatch_variableExpr *,malloc,(int n));

    //  BPatch_addressSpace::malloc
    //  
    //  Allocate memory for a new variable in the mutatee process

    API_EXPORT(ByType, (type),
    BPatch_variableExpr *,malloc,(const BPatch_type &type));

    //  BPatch_addressSpace::free
    //  
    //  Free memory allocated by Dyninst in the mutatee process

    API_EXPORT(Int, (ptr),
    bool,free,(BPatch_variableExpr &ptr));


};

#endif 
