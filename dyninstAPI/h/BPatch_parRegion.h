/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _BPatch_parRegion_h_
#define _BPatch_parRegion_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_eventLock.h"

class int_parRegion;
class process;
class InstrucIter;
class BPatch_function;
class BPatch_instruction;
#include "Instruction.h"

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

  API_EXPORT(Int, (insns),
	     bool, getInstructions, (std::vector<Dyninst::InstructionAPI::Instruction::Ptr>& insns));


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
