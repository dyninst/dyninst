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
 
// $Id: function.C,v 1.6 2005/02/15 17:43:53 legendre Exp $

#include "function.h"
#include "BPatch_flowGraph.h"
#include "func-reloc.h"
#include "process.h"
#include "instPoint.h"

pdstring int_function::emptyString("");


// Verify that this is code
// Find the call points
// Find the return address
// TODO -- use an instruction object to remove
// Sets err to false on error, true on success
//
// Note - this must define funcEntry and funcReturn
// 
int_function::int_function(const pdstring &symbol,
			   Address offset, 
			   const unsigned size,
			   pdmodule *m) :
  line_(0),
  offset_(offset),
  size_(size),
  mod_(m),
  parsed_(false),
  blockList(NULL),
  flowGraph(NULL),
  noStackFrame(false),
  makesNoCalls_(false),
  savesFP_(false),
  call_points_have_been_checked(false),
  funcEntry_(NULL),
  isTrap(false),
  isInstrumentable_(false),
  has_jump_to_first_five_bytes(false),
#if defined(arch_ia64)
  framePointerCalculator(NULL),
  usedFPregs(NULL),
#endif
  needs_relocation_(false),
  mayNeedRelocation_(false),
  canBeRelocated_(false),
  relocatedCode(NULL),
  originalCode(NULL),
  numInstructions(0),
  instructions(NULL),
#if !defined(BPATCH_LIBRARY)
  funcResource(NULL),
#endif
  o7_live(false)
{
  symTabName_.push_back(symbol);
  blockList = new pdvector<BPatch_basicBlock *>;
#ifdef BPATCH_LIBRARY
  if (BPatch::bpatch->hasForcedRelocation_NPInt()) 
    {
      needs_relocation_ = true;
    }
#endif
}


int_function::~int_function() { 
  // delete the rewritten version of the function if it was relocated
  if (blockList) delete blockList;
  if (relocatedCode) delete relocatedCode;  
  if (originalCode) delete originalCode;   
  if (instructions) delete instructions;   
  /* TODO */ 
}


// coming to dyninstAPI/src/symtab.hC
// needed in metric.C
bool int_function::match(int_function *fb)
{
  if (this == fb)
    return true;
  else
    return ((symTabName_ == fb->symTabName_) &&
	    (prettyName_ == fb->prettyName_) &&
	    (line_       == fb->line_) &&
	    (offset_     == fb->offset_) &&
	    (size_       == fb->size_));
}

void int_function::changeModule(pdmodule *mod) {
  // Called from buildFunctionLists, so we aren't entered in any 
  // module-level data structures. If this changes, UPDATE THE
  // FUNCTION.

  mod_ = mod;
}

#ifdef DEBUG
/*
  Debuggering info for function_prototype....
 */
ostream & function_prototype::operator<<(ostream &s) const {
  
    unsigned i=0;
    s << "Mangled name(s): " << symTabName_[0];
    for(i = 1; i < symTabName_.size(); i++) {
        s << ", " << symTabName_[i];
    }

    s << "\nPretty name(s): " << prettyName_[0];
    for(i = 1; i < prettyName_.size(); i++) {
        s << ", " << prettyName_[i];
    }
      s << "\nline_ = " << line_ << " addr_ = "<< addr_ << " size_ = ";
      s << size_ << endl;
  
    return s;
}

ostream &operator<<(ostream &os, function_prototype &f) {
    return f.operator<<(os);
}

#endif


BPatch_flowGraph *
int_function::getCFG(process * proc)
{
  assert(parsed_);
  if (!flowGraph) { 
       bool valid;
       flowGraph = new BPatch_flowGraph(this, proc, pdmod(), valid);
       assert (valid);
  }
  return flowGraph;
}

BPatch_loopTreeNode * 
int_function::getLoopTree(process * proc)
{
   assert(parsed_);
   BPatch_flowGraph *fg = getCFG(proc);
   return fg ? fg->getLoopTree() : NULL;
}

bool int_function::isInstrumentableByFunctionName()
{
#if defined(i386_unknown_solaris2_5)
    /* On Solaris, this function is called when a signal handler
       returns.  If it requires trap-based instrumentation, it can foul
       the handler return mechanism.  So, better exclude it.  */
    if (prettyName() == "_setcontext" || prettyName() == "setcontext")
        return false;
#endif /* i386_unknown_solaris2_5 */
    
    if( prettyName() == "__libc_free" )
        return false;
    
    // XXXXX kludge: these functions are called by DYNINSTgetCPUtime, 
    // they can't be instrumented or we would have an infinite loop
    if (prettyName() == "gethrvtime" || prettyName() == "_divdi3"
        || prettyName() == "GetProcessTimes")
        return false;
    return true;
}

void int_function::addArbitraryPoint(instPoint* insp, process* p) {
  if (insp) arbitraryPoints.push_back(insp);

#if defined(cap_relocation)
  if(insp && p && needs_relocation_)
    for(u_int i=0; i < relocatedByProcess.size(); i++)
      if(relocatedByProcess[i] &&
	 (relocatedByProcess[i])->getProcess() == p) {
	addArbitraryPoint(insp, p, relocatedByProcess[i]);
	return;
      }
#endif
}

// passing in a value of 0 for p will return the original address
// otherwise, if the process is relocated it will return the new address
Address int_function::getAddress(const process *p) const{
  assert(parsed_);
  if(p && needs_relocation_) { 
    for(u_int i=0; i < relocatedByProcess.size(); i++){
      if(relocatedByProcess[i] &&
	 (relocatedByProcess[i])->getProcess() == p)
	return (relocatedByProcess[i])->get_address();
    } }
  return get_address();
}

// This method returns the address at which this function resides
// in the process P, even if it is dynamic, even if it has been
// relocated.  getAddress() (see symtab.h) does not always do this:
// If the function is in a shlib and it has not been relocated,
// getAddress() only returns the relative offset of the function
// within its shlib.  We think that getAddress() should be fixed
// to always return the effective address, but we are not sure
// whether that would boggle any of its 75 callers.  Until that is
// cleared up, call this method. -zandy, Apr-26-1999
Address int_function::getEffectiveAddress(const process *p) const {
  assert(parsed_);
     assert(p);
     // Even if the function has been relocated, call it at its
     // original address since the call will be redirected to the
     // right place anyway.
     Address base;
     if (!p->getBaseAddress(pdmod()->exec(), base)) {
         cerr << "Error: couldn't find base address for func "
              << prettyName();
     }
     return base + get_address();
}

instPoint *int_function::funcEntry(const process *p) const {
  assert(parsed_);
  if(p && needs_relocation_) { 
    for(u_int i=0; i < relocatedByProcess.size(); i++){
      if(relocatedByProcess[i] &&
	 (relocatedByProcess[i])->getProcess() == p) {
	return (relocatedByProcess[i])->funcEntry();
      }
    }       
  }
  return funcEntry_;
}

const pdvector<instPoint*> &int_function::funcExits(const process *p) const {
  assert(parsed_);
  if(p && needs_relocation_) {
    for(u_int i=0; i < relocatedByProcess.size(); i++){
      if(relocatedByProcess[i] &&
	 (relocatedByProcess[i])->getProcess() == p)
	return (relocatedByProcess[i])->funcReturns();
    } 
  }
  return funcReturns;
}
const pdvector<instPoint*> &int_function::funcArbitraryPoints(const process *p) const {
  assert(parsed_);
  if(p && needs_relocation_) {
    for(u_int i=0; i < relocatedByProcess.size(); i++){
      if(relocatedByProcess[i] &&
	 (relocatedByProcess[i])->getProcess() == p)
	return (relocatedByProcess[i])->funcArbitraryPoints();
    } }
  return arbitraryPoints;
}

const pdvector<instPoint*> &int_function::funcCalls(const process *p) {
  assert(parsed_);

  if (!call_points_have_been_checked) checkCallPoints();  

  if(p && needs_relocation_) {
    for(u_int i=0; i < relocatedByProcess.size(); i++){
      if(relocatedByProcess[i] &&
	 (relocatedByProcess[i])->getProcess() == p)
	return (relocatedByProcess[i])->funcCallSites();
    } }
  return calls;
}

bool int_function::hasBeenRelocated(const process *p) const{
  assert(parsed_);

  if(p && needs_relocation_) {
    for(u_int i=0; i < relocatedByProcess.size(); i++) {
      if(relocatedByProcess[i] &&
	 (relocatedByProcess[i])->getProcess() == p)
	return true;
    }
  }
  return false;
}



codeRange *int_function::copy() const { 
  return new int_function(*this);
}

bool int_function::is_on_stack(process *proc,
                              const pdvector<pdvector<Frame> > &stackWalks) {
   // for every vectors of frame, ie. thread stack walk, make sure can do
   // relocation
   for (unsigned walk_itr = 0; walk_itr < stackWalks.size(); walk_itr++) {
      pdvector<int_function *> stack_funcs =
         proc->pcsToFuncs(stackWalks[walk_itr]);

      // for every frame in thread stack walk
      for(unsigned i=0; i<stack_funcs.size(); i++) {
         int_function *stack_func = stack_funcs[i];
          
         if( stack_func == this ) {
            return true;
         }
      }
   }

   return false;
}

bool loop_above_func(process *proc, int_function *instrumented_func,
                     const pdvector<Frame> &stackwalk)
{
   bool in_a_loop = false;
   bool func_on_stack = false;

   pdvector<int_function *> stack_funcs = proc->pcsToFuncs(stackwalk);

   // for every frame in thread stack walk
   for(unsigned i=0; i<stack_funcs.size(); i++) {
      int_function *stack_func = stack_funcs[i];
      
      Address pc = stackwalk[i].getPC();
      
      /*
      cerr << "  -------------------------------------------------\n";
      cerr << "  [" << i+1 << "] looking at function: ("
           << (void*)stack_func << ") "
           << stack_func->prettyName().c_str() << ", pc: "
           << (void*)pc << endl;
      if(func_on_stack)
         cerr << "    FUNC ON STACK\n";
      */

      if(func_on_stack) {
         BPatch_flowGraph *fg = stack_func->getCFG(proc);
         BPatch_Vector<BPatch_basicBlockLoop*> outerLoops;
         fg->getOuterLoops(outerLoops);
         for(unsigned i=0; i<outerLoops.size(); i++) {
            BPatch_basicBlockLoop *curloop = outerLoops[i];
            if(curloop->containsAddress(pc)) {
               in_a_loop = true;
               //cerr << "     func " << stack_func->prettyName().c_str()
               //     << " has loop that CONTAINS pc\n";
            }
            
            BPatch_Vector<BPatch_basicBlockLoop*> containedLoops;
            curloop->getContainedLoops(containedLoops);
            //cerr << "         # contained loops: " << containedLoops.size()
            //     << endl;
            for(unsigned j=0; j<containedLoops.size(); j++) {
               BPatch_basicBlockLoop *childloop = containedLoops[j];
               if(childloop->containsAddress(pc)) {
                  in_a_loop = true;
                  // cerr << "     func " <<stack_func->prettyName().c_str()
                  //      << " has child loop that CONTAINS pc\n";
               }
            }
         }
      }

      if(in_a_loop)  break;
      
      if( stack_func == instrumented_func ) {
         func_on_stack = true;
      }
   }

   if(func_on_stack)
      return in_a_loop;
   else
      return false;
}                    

// Our current heuristic for calling a function long running is whether it's
// in a loop or one of it's parent functions is in a loop.  If the function
// call is in a loop at some level then it's likely short running.  If it's
// not in a loop at any level, it's more likely to be a long running
// function.

// from 2/27/04 commit
// There is one difficulty though of relocating and instrumenting functions
// that are on the stack.  If the function is long-running or never exiting,
// then the instrumentation will rarely, or never be executed, since the
// instrumentation is in the relocated instance of the function.  This
// problem manifests itself in Paradyn by catchup not starting timers for
// functions that are relocated and instrumented.  This leads to the
// performance consultant in certain cases deferring cpu_time metrics for
// bottleneck functions, thereby preventing child experiments from occurring.
// At this time, I have solved this problem by instrumenting the original
// function if we think the function is long-running.  The current method I
// have implemented for deciding whether a function is long-running is if
// it's not in a loop and it has no parent function that's in a loop.  A
// different heuristic can be easily swapped in if this one is found
// undesirable.  A full solution to this problem might instrument both the
// original function and the relocated function.  Perhaps this could be
// looked into after the release sometime when the changes needed wouldn't
// cause such a disruption.

bool int_function::think_is_long_running(process *proc,
                           const pdvector<pdvector<Frame> > &stackWalks)
{
   // for every vectors of frame, ie. thread stack walk
   for (unsigned walk_itr = 0; walk_itr < stackWalks.size(); walk_itr++) {
      bool loop_above = loop_above_func(proc, this, stackWalks[walk_itr]);

      // if this function is in a loop or has a parent func in a loop
      // on any thread, then NO it's likely short running ... return false
      if(loop_above) {
         return false;
      }
   }
   
   // not in a loop, so it's likely long running
   return true;
}

void int_function::updateForFork(process *childProcess, 
				const process *parentProcess) {
  if(needs_relocation_) {
    for(u_int i=0; i < relocatedByProcess.size(); i++) {
      if(relocatedByProcess[i] &&
	 (relocatedByProcess[i])->getProcess() == parentProcess) {
	relocatedFuncInfo *childRelocInfo = 
	  new relocatedFuncInfo(*relocatedByProcess[i]);
	childRelocInfo->setProcess(childProcess);
	relocatedByProcess.push_back(childRelocInfo);
	// And add the relocation to the process' address tree
	childProcess->addCodeRange(childRelocInfo->get_address(), childRelocInfo);
      }
    }
  }
}

void int_function::cleanProcessSpecific(process *p) {
  // Relocated func info needs to go
  for (unsigned i = 0; i < relocatedByProcess.size(); i++) {
    if (relocatedByProcess[i] && 
	relocatedByProcess[i]->getProcess() == p) {
      delete relocatedByProcess[i];
      relocatedByProcess[i] = NULL;
    }
  }      
}



void print_func_vector_by_pretty_name(pdstring prefix,
				      pdvector<int_function *>*funcs) {
    unsigned int i;
    int_function *func;
    for(i=0;i<funcs->size();i++) {
      func = ((*funcs)[i]);
      cerr << prefix << func->prettyName() << endl;
    }
}

