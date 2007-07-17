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

#ifndef _BPatch_parRegion_h_
#define _BPatch_parRegion_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_type.h"
#include "BPatch_module.h"
#include "BPatch_flowGraph.h"
#include "BPatch_instruction.h"
#include "BPatch_eventLock.h"
#include "BPatch_memoryAccess_NP.h"

class int_parRegion;
class process;
class InstrucIter;
class BPatch_function;

typedef enum{
  OMP_NONE, OMP_PARALLEL, OMP_DO_FOR,OMP_DO_FOR_LOOP_BODY, OMP_SECTIONS, OMP_SINGLE, 
    OMP_PAR_DO, OMP_PAR_SECTIONS, OMP_MASTER, OMP_CRITICAL,
    OMP_BARRIER, OMP_ATOMIC, OMP_FLUSH, OMP_ORDERED, OMP_ANY
    } parRegType;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_parRegion

class BPATCH_DLL_EXPORT BPatch_parRegion: public BPatch_eventLock{
  BPatch_function * func;
  int_parRegion * parReg;
  
 public:
  BPatch_parRegion(int_parRegion * _parReg, BPatch_function *  _func);
  ~BPatch_parRegion();

  int_parRegion *lowlevel_region() const {return parReg;}

  void printDetails();


   /** BPatch_basicBlock::getClause    */
        
   API_EXPORT(Int, (key),
              int, getClause, (const char * key));


   /** BPatch_basicBlock::replaceOMPParameter    */
        
   API_EXPORT(Int, (key, value),
              int, replaceOMPParameter, (const char * key, int value));


  /** BPatch_parRegion::getInstructions   */
  /** return the instructions that belong to the block */

  API_EXPORT(Int, (),
             BPatch_Vector<BPatch_instruction *> *,getInstructions,());


  /** BPatch_parRegion::size   */

   API_EXPORT(Int, (),
              unsigned,size,() CONST_EXPORT);

   /** BPatch_parRegion::getStartAddress   */
   //these always return absolute address

   API_EXPORT(Int, (),
              unsigned long,getStartAddress,() CONST_EXPORT);

   /** BPatch_basicBlock::getEndAddress    */
        
   API_EXPORT(Int, (),
              unsigned long, getEndAddress, () CONST_EXPORT);

   
 private:
   /** the instructions within this region */
   BPatch_Vector<BPatch_instruction*> *instructions;

};

#endif
