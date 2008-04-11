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

/* -*- Mode: C; indent-tabs-mode: true; tab-width: 4 -*- */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"

#include "arch.h"
#include "util.h"
#include "process.h"
#include "mapped_object.h"
#include "symtab.h"
#include "instPoint.h"
#include "InstrucIter.h"

#include "BPatch_Set.h"
#include "BPatch_instruction.h"
#include "BPatch_memoryAccess_NP.h"

/* A few utility functions. */
int extractInstructionSlot( Address addr ) {
  return (addr % 16);
} /* end extractInstructionSlot */

bool addressIsValidInsnAddr( Address addr ) {
  switch( extractInstructionSlot( addr ) ) {
  case 0: case 1: case 2:
    return true;
  default:
    return false;
  }
} /* end addressIsValidInsnAddr() */

/* Access the instruction's type through the correct virtual method */
instruction::insnType InstrucIter::getInsnType()
{
  instruction::insnType ret;

  instruction *insn = getInsnPtr();
  ret = insn->getType();

  delete insn;
  return ret;
}

bool InstrucIter::isAReturnInstruction() {

  switch( getInsnType() ) {
  case instruction::RETURN: {
    instruction * itmp = getInsnPtr();
    if( itmp->getPredicate() == 0 ) { return true; }
  } break;
			
  default:
    break;
  } /* end instruction-type switch */
		
  return false;
} /* end isAReturnInstruction() */

/** is the instruction used to return from the functions,
    dependent upon a condition register
    * @param i the instruction value 
    */
bool InstrucIter::isACondReturnInstruction() {
  switch( getInsnType() ) {
  case instruction::RETURN: {
    instruction * itmp = getInsnPtr();
    if( itmp->getPredicate() != 0 ) {
      return true;
    }
  } break;
			
  default:
    break;
  } /* end instruction-type switch */
		
  return false;
} /* end isAReturnInstruction() */

bool InstrucIter::isAIndirectJumpInstruction()
{
  bool ret;
  instruction * itmp = getInsnPtr();
  switch( itmp->getType() ) {
  case instruction::INDIRECT_BRANCH:
    if( itmp->getPredicate() == 0 ) 
      ret = true;
    else 
      ret = false;
    break;
  default:
    ret = false;
    break;
  } /* end instruction-type switch */
  delete itmp;
  return ret;
}

bool InstrucIter::isACondBranchInstruction()
{
  bool ret;
  instruction * itmp = getInsnPtr();
  switch( itmp->getType() ) {
  case instruction::DIRECT_BRANCH:
    /* Not sure if this second case is intended. */
  case instruction::INDIRECT_BRANCH: 
    if( itmp->getPredicate() != 0) 
      ret = true; 
    else
      ret = false;
    break;  
  case instruction::COND_BRANCH:
    ret = true;
    break;
  default:
    ret = false;
    break;
  } /* end instruction-type switch */
  delete itmp;
  return ret;
}

/* We take this to mean a dirct conditional branch which always executes. */
bool InstrucIter::isAJumpInstruction()
{
  bool ret;
  instruction * itmp = getInsnPtr();

  switch( itmp->getType() ) {
  case instruction::DIRECT_BRANCH:
    if( itmp->getPredicate() == 0 ) 
      ret = true;
    else
      ret = false;
    break;
        
  default:
    ret = false;
    break;
  } /* end instruction-type switch */
  delete itmp;
  return ret;
}

bool InstrucIter::isACallInstruction()
{
  return (getInsnType() == instruction::DIRECT_CALL ||
	  getInsnType() == instruction::INDIRECT_CALL);
}

bool InstrucIter::isAnneal()
{
  assert( 0 );
  return false;
}

bool InstrucIter::isAnAbortInstruction()
{
  // FIXME this is sufficient for the common glibc illegal instruction,
  // but should be more general

  // glibc uses break 0, which is illegal

  return (getInsnType() == instruction::BREAK);
}

bool InstrucIter::isAnAllocInstruction()
{
  return (getInsnType() == instruction::ALLOC);
}

bool InstrucIter::isDelaySlot()
{
  return false;
}

Address InstrucIter::getBranchTargetAddress(bool *)
{
  instruction * itmp = getInsnPtr();
  Address rawTargetAddress = itmp->getTargetAddress() + current;
  Address targetAddress = rawTargetAddress - (rawTargetAddress % 0x10);
  // /* DEBUG */ fprintf( stderr, "Instruction at 0x%lx targets 0x%lx\n", currentAddress, targetAddress );
  delete itmp;
  return targetAddress;
}

void initOpCodeInfo() {
  /* I don't need this for anything. */
} /* end initOpCodeInfo() */

BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
  assert(instPtr);
  instruction *insn = getInsnPtr();
  instruction::insnType type = getInsnType();
    
  BPatch_memoryAccess * bpma = NULL;
    
  insn_tmpl tmpl = { insn->getMachineCode() };
  uint8_t size = 0x1 << (tmpl.M1.x6 & 0x3);
    
  switch( type ) {
  case instruction::INTEGER_16_LOAD:
    size = 16;
  case instruction::INTEGER_LOAD:
  case instruction::FP_LOAD:
    bpma = new BPatch_memoryAccess( insn, current, true, false, size, 0, tmpl.M1.r3, -1 );
    assert( bpma != NULL );
    break;
        
  case instruction::INTEGER_16_STORE:
    size = 16;
  case instruction::INTEGER_STORE:
  case instruction::FP_STORE:
    bpma = new BPatch_memoryAccess( insn, current, false, true, size, 0, tmpl.M1.r3, -1 );
    assert( bpma != NULL );
    break;
        
  case instruction::FP_PAIR_LOAD:
  case instruction::INTEGER_PAIR_LOAD:
    /* The load pair instructions encode sizes a little differently. */
    size = (tmpl.M1.x6 & 0x1) ? 16 : 8;
    bpma = new BPatch_memoryAccess( insn, current,true, false, size, 0, tmpl.M1.r3, -1 );
    assert( bpma != NULL );
    break;
        
  case instruction::PREFETCH:
    bpma = new BPatch_memoryAccess( insn, current,false, false, 0, tmpl.M1.r3, -1, 0, 0, -1, -1, 0, -1, false, 0 );
    assert( bpma != NULL );
    break;
                        
  default:
    return BPatch_memoryAccess::none;
  }
    
  return bpma;
}

/* The IA-64 SCRAG defines a pattern similar to POWER's.  At some constant offset
   from the GP, there's a jump table whose 64-bit entries are offsets from the base
   address of the table to the target.
   
   However, gcc and icc both deviate from the standard.  The jump table entries
   generated by icc contain offsets from a label inside the originating function,
   so we look for moves from IP to locate the label.  The entries in a gcc-generated
   jump table are offsets from themselves, not the base of the table.  If we don't
   find a move from IP, we assume gcc-style entries.
   
   However, there is a construct in glibc that jumps to an address calculated as an
   offset from the GP (in vfprintf() and vfwprintf(), for example) using code which
   looks like a jump table computation and follows an unrelated comparison instruction.
   
   We therefore look for a sequence like the following:
   
   (1) add rTAA = r1, constant
   [2] ld8 rTA = [rtAA]
   (3) add rTE = rTA, rVariable ; we also see shladd rTE = rVariable, 3, rTA.
   (4) ld8 rBase = [rTE]
   (5) add rT = rBase + rOffset
   (6) mov brT = rT
   (7) br  brT
		
   where rOffset is a previously-defined move from IP and a
  
   (<4) cmp.ltu pX, pY = rVariable, constant
  		
   occurs anywhere before the second add instruction.  (There should also be a branch
   predicated on pX somewhere, possibly the one last in this sequence.)
  
   rOffset is either rTE or, in icc-style tables, the result of a
  
   [<5] mov.ip rOffset = ip

   instruction.  The final complication is that the first ld8 instruction is
   completely absent in shared objects. 
  
   Note that we would need true DFA (the kill sets) to determine a lower bound
   on the locations of either the cmp.ltu or the mov.ip instructions. */
  
bool InstrucIter::getMultipleJumpTargets( BPatch_Set<Address> & targetAddresses ) {
  // /* DEBUG */ fprintf( stderr, "%s[%d]: examining jump table (@ 0x%lx) in %s \n", __FILE__, __LINE__, current, img_->name().c_str() );

  /* Wind up a new iterator.  This would probably be easier with an explicit
     state table, but for now I don't think a single pattern with many exceptions
     warrants the infastructural investment. */
  InstrucIter iter = * this;

  /* Sanity check. */	
  instruction branchInstruction = iter.getInstruction();
  if( branchInstruction.getType() != instruction::INDIRECT_BRANCH ) {
    fprintf( stderr, "%s[%d]: getMultipleJumpTargets() called on an instruction that is not an indirect branch.\n", __FILE__, __LINE__ );
    return false;
  }

  /* Which branch register are we looking for?  (AFAIK, always b6, but why not check?) */
  insn_tmpl bi_tmpl = { branchInstruction.getMachineCode() };
  uint64_t branchRegister = bi_tmpl.B4.b2;
	
	
  /* Nothing of interest can occur before the previous mov.br instruction;
     extract registerTarget when we see it. */
  bool foundMBTR = false;
  uint64_t registerTarget = 0;
  for( ; iter.hasPrev(); --iter ) {
    instruction moveToBR = iter.getInstruction();
    if( moveToBR.getUnitType() != instruction::I ) { continue; }
		
    insn_tmpl mtbr_tmpl = { moveToBR.getMachineCode() };
    if(	GET_OPCODE( &mtbr_tmpl ) != 0x0
	|| mtbr_tmpl.I21.x3 != 7 
	|| mtbr_tmpl.I21.x != 0 ) { continue; }
		
    if( mtbr_tmpl.I21.b1 != branchRegister ) {
      // /* DEBUG */ fprintf( stderr, "%s[%d]: found move to branch register with wrong branch registers (0x%lx, not 0x%lx)\n", __FILE__, __LINE__, mtbr_tmpl.I21.b1, branchRegister );
      continue;
    }
		
    /* It's the right instruction; extract registerTarget. */
    // /* DEBUG */ fprintf( stderr, "%s[%d]: located at 0x%lx\n", __FILE__, __LINE__, iter.current );
    registerTarget = mtbr_tmpl.I21.r2;
    foundMBTR = true;
    break;
  }	 
  if( ! foundMBTR ) {
    // /* DEBUG */ fprintf( stderr, "%s[%d]: unable to locate move to branch register.\n", __FILE__, __LINE__ );
    return false;
  }
	
  /* Nothing of interest can occur before the previous add instruction; extract
     rBase and rOffset when we find it. */
  bool foundInstruction5 = false;
  uint64_t registerBase = 0, registerOffset = 0;
  for( ; iter.hasPrev(); --iter ) {
    instruction instruction5 = iter.getInstruction();
	
    if( instruction5.getUnitType() != instruction::M && instruction5.getUnitType() != instruction::I ) { continue; }
		
    insn_tmpl i5_tmpl = { instruction5.getMachineCode() };
    if(	GET_OPCODE( &i5_tmpl ) != 0x8
	|| i5_tmpl.A1.x2a != 0
	|| i5_tmpl.A1.ve != 0
	|| i5_tmpl.A1.x4 != 0
	|| i5_tmpl.A1.x2b != 0 ) { continue; }
		
    if( i5_tmpl.A1.r1 != registerTarget ) {
      // /* DEBUG */ fprintf( stderr, "%s[%d]: found an add instruction with the wrong target at 0x%lx (0x%lx, not 0x%lx)\n", __FILE__, __LINE__, iter.current, i5_tmpl.A1.r1, registerTarget );
      continue;
    }
		
    /* It's the right instruction; extract registerBase and registerOffset. */
    // /* DEBUG */ fprintf( stderr, "%s[%d]: located at 0x%lx\n", __FILE__, __LINE__, iter.current );
    registerBase = i5_tmpl.A1.r2;
    registerOffset = i5_tmpl.A1.r3;
    foundInstruction5 = true;
    break;
  }
  if( ! foundInstruction5 ) {
    // /* DEBUG */ fprintf( stderr, "%s[%d]: unable to locate instruction 5.\n", __FILE__, __LINE__ );
    return false;
  }
	

  /* For simplicity, ignore the potential mov.ip and search for it in a second pass. */
  Address movFromIPUpperBound = iter.current;


  /* We don't actually know which one of registerBase and registerOffset is really the base until
     we find the previous ld8 into it, which will also give us registerTableEntry. */
  bool foundInstruction4 = false;
  uint64_t registerTableEntry = 0;
  for( ; iter.hasPrev(); --iter ) {
    instruction instruction4 = iter.getInstruction();
		
    if( instruction4.getUnitType() != instruction::M ) { continue; }
		
    insn_tmpl i4_tmpl = { instruction4.getMachineCode() };
    if(	GET_OPCODE( &i4_tmpl ) != 0x4
	|| i4_tmpl.M1.m != 0x0
	|| i4_tmpl.M1.x != 0x0
	|| (!(i4_tmpl.M1.x6 <= 0x2B && (i4_tmpl.M1.x6 & 0x3) == 0x3)) ) { continue; }

    /* Swap the names, if necessary; see above. */
    if( i4_tmpl.M1.r1 != registerBase && i4_tmpl.M1.r1 == registerOffset ) {
      uint64_t temp = registerBase; registerBase = registerOffset; registerOffset = temp;
    }
			
    if( i4_tmpl.M1.r1 != registerBase ) {
      // /* DEBUG */ fprintf( stderr, "%s[%d]: found ld8 instruction with wrong target at 0x%lx (0x%lx. not 0x%lx or 0x%lx)\n", __FILE__, __LINE__, i4_tmpl.M1.r1, iter.current, registerBase, registerOffset );
      continue;
    }
		
    /* It's the right instruction; extract registerTableEntry. */
    // /* DEBUG */ fprintf( stderr, "%s[%d]: located at 0x%lx\n", __FILE__, __LINE__, iter.current );
    registerTableEntry = i4_tmpl.M1.r3;
    foundInstruction4 = true;
    break;
  }
  if( ! foundInstruction4 ) {
    // /* DEBUG */ fprintf( stderr, "%s[%d]: unable to locate instruction 4.\n", __FILE__, __LINE__ );
    return false;
  }

  /* Again, for simplicity, save the concurrent search for another pass. */
  Address cmpltuUpperBound = iter.current;

  /* Extract registerTableAddress and registerVariable from the previous add or shladd instruction. */
  bool foundInstruction3 = false;
  uint64_t registerTableAddress = 0, registerVariable = 0;
  for( ; iter.hasPrev(); --iter ) {
    instruction instruction3 = iter.getInstruction();
	
    if( instruction3.getUnitType() != instruction::M && instruction3.getUnitType() != instruction::I ) { continue; }
		
    insn_tmpl i3_tmpl = { instruction3.getMachineCode() };
    if(	GET_OPCODE( &i3_tmpl ) != 0x8
	|| i3_tmpl.A1.x2a != 0
	|| i3_tmpl.A1.ve != 0 ) { continue; }	
    if( i3_tmpl.A1.x4 == 0 ) { if( i3_tmpl.A1.x2b != 0 ) { continue; } }
    else if( i3_tmpl.A1.x4 != 4 ) { continue; }
		
    if( i3_tmpl.A1.r1 != registerTableEntry ) {
      // /* DEBUG */ fprintf( stderr, "%s[%d]: found an [shl]add instruction with the wrong target at 0x%lx (0x%lx, not 0x%lx)\n", __FILE__, __LINE__, iter.current, i3_tmpl.A1.r1, registerTarget );
      continue;
    }
		
    /* It's the right instruction; extract registerTableAddress and registerVariable. */
    // /* DEBUG */ fprintf( stderr, "%s[%d]: located at 0x%lx\n", __FILE__, __LINE__, iter.current );
    registerTableAddress = i3_tmpl.A1.r3;
    registerVariable = i3_tmpl.A1.r2; // In the shladd case, this is the correct order.
    foundInstruction3 = true;
    break;
  }
  if( ! foundInstruction3 ) {
    // /* DEBUG */ fprintf( stderr, "%s[%d]: unable to locate instruction 3.\n", __FILE__, __LINE__ );
    return false;
  }

  /* Since we can get false positives looking for the optional ld8 instruction 2, trust to
     the empirical evidence that the double indirection only happens in the a.out instead. */
  bool foundInstruction2 = false;
  uint64_t registerTableAddressAddress = registerTableAddress;
  image* img = dynamic_cast<image*>(instructions_);
  process* proc = dynamic_cast<process*>(instructions_);
  if( (img && img->isAOut()) || (proc && proc->getAOut()->findCodeRangeByAddress( current )) ) {
    /* We don't actually know which of registerTableAddressAddress and registerVariable really is the address
       until we find the previous ld8 or add into it, which will give us registerTableAddressAddress or r1. */		   
    for( ; iter.hasPrev(); --iter ) {
      instruction instruction2 = iter.getInstruction();
		
      if( instruction2.getUnitType() != instruction::M ) { continue; }
		
      insn_tmpl i2_tmpl = { instruction2.getMachineCode() };
      if(	GET_OPCODE( &i2_tmpl ) != 0x4
		|| i2_tmpl.M1.m != 0x0
		|| i2_tmpl.M1.x != 0x0
		|| (!(i2_tmpl.M1.x6 <= 0x2B && (i2_tmpl.M1.x6 & 0x3) == 0x3)) ) { continue; }

      /* Swap the names, if necessary; see above. */
      if( i2_tmpl.M1.r1 != registerTableAddress && i2_tmpl.M1.r1 == registerVariable ) {
	uint64_t temp = registerTableAddress; registerTableAddress = registerVariable; registerVariable = temp;
      }
		
      if( i2_tmpl.M1.r1 != registerTableAddress ) {
	// /* DEBUG */ fprintf( stderr, "%s[%d]: found ld8 instruction with the wrong target at 0x%lx (0x%lx, not 0x%lx or 0x%lx)\n", __FILE__, __LINE__, iter.current, i2_tmpl.M1.r1, registerTableAddress, registerVariable );
	continue;
      }
		
      /* It's the right instruction; extract registerTableAddressAddress. */
      // /* DEBUG */ fprintf( stderr, "%s[%d]: located at 0x%lx\n", __FILE__, __LINE__, iter.current );
      registerTableAddressAddress = i2_tmpl.M1.r3;
      foundInstruction2 = true;
      break;
    }
  } /* end if we're not parsing an a.out. */


  /* Find the add with GP instruction.  NOTE: we only check for the addl form. */
  bool foundAddWithGP = false;
  uint64_t jumpTableOffset = 0;
  bool checkFirstInstruction = true;
  for( ; iter.hasPrev() || checkFirstInstruction; --iter ) {
    if( ! iter.hasPrev() ) { checkFirstInstruction = false; }
	
    instruction addWithGPInstruction = iter.getInstruction();
		
    if( addWithGPInstruction.getUnitType() != instruction::I && addWithGPInstruction.getUnitType() != instruction::M ) { continue; }
		
    insn_tmpl awgpi_tmpl = { addWithGPInstruction.getMachineCode() };
    if(	GET_OPCODE( &awgpi_tmpl ) != 0x9 ) { continue; }
	
    if( awgpi_tmpl.A5.r1 != registerTableAddressAddress ) {
      // /* DEBUG */ fprintf( stderr, "%s[%d]: found add with GP instruction with wrong the wrong target at 0x%lx (0x%lx, not 0x%lx)\n", __FILE__, __LINE__, iter.current, awgpi_tmpl.A5.r1, registerTableAddressAddress );
      continue;
    }
		
    /* It's the right instruction; extract jumpTableOffset. */
    // /* DEBUG */ fprintf( stderr, "%s[%d]: located at 0x%lx\n", __FILE__, __LINE__, iter.current );
    jumpTableOffset = GET_A5_IMM( &awgpi_tmpl );
    foundAddWithGP = true;
    break;
  }		
  if( ! foundAddWithGP ) {
    // /* DEBUG */ fprintf( stderr, "%s[%d]: failed to find the add with GP instruction.\n", __FILE__, __LINE__ );
    return false;
  }
	
	
  /* Calculate the jump table address and extract the targets.
     Start by obtaining the GP. */
  Address gpAddress = 0;
  if( img ) { 
    gpAddress = img->getObject()->getTOCoffset();
  }
  else {
    pdvector< mapped_object * > m_objs = proc->mappedObjects();
    for( unsigned i = 0; i < m_objs.size(); i++ ) {
      void * ptr = m_objs[i]->getPtrToInstruction( current );
      if( ptr ) {
	gpAddress = m_objs[i]->parse_img()->getObject()->getTOCoffset();
	break;
      }
    }
  }
  if( gpAddress == 0 ) {
    // /* DEBUG */ fprintf( stderr, "%s[%d]: unable to find GP for jump table at 0x%lx\n", __FILE__, __LINE__, current );
    return false;
  }

  Address jumpTableAddressAddress = gpAddress + jumpTableOffset;
  Address jumpTableAddress = jumpTableAddressAddress;
  if( img && img->isAOut() && foundInstruction2 ) {
    void *ptr = img->getPtrToData( jumpTableAddressAddress );
    if( ! ptr ) { ptr = img->getPtrToDataInText( jumpTableAddressAddress ); }
    if( !ptr ) {
      // /* DEBUG */ fprintf( stderr, "%s[%d]: unable to access jump table address pointer\n", __FILE__, __LINE__ );
      return false;
    }
    jumpTableAddress = *((Address *)ptr);
  }
  else if( proc && proc->getAOut()->findCodeRangeByAddress( jumpTableAddressAddress ) && foundInstruction2 ) {
    void * ptr = instructions_->getPtrToInstruction( jumpTableAddressAddress );
    if( !ptr ) {
      // /* DEBUG */ fprintf( stderr, "%s[%d]: unable to access jump table address pointer\n", __FILE__, __LINE__ );
      return false;
    }
    jumpTableAddress = *((Address *)ptr);
  }
		
  /* Check if the jump table is IP-relative. */ 
  iter.setCurrentAddress( movFromIPUpperBound );
  bool foundMovFromIP = false;
  Address baseJumpAddress = jumpTableAddress;
  for( ; iter.hasPrev(); --iter ) {
    instruction movFromIP = iter.getInstruction();
		
    if( movFromIP.getUnitType() != instruction::I ) { continue; }
		
    insn_tmpl mfip_tmpl = { movFromIP.getMachineCode() };
    if(	mfip_tmpl.I25.opcode != 0x0
	|| mfip_tmpl.I25.x6 != 0x30
	|| mfip_tmpl.I25.x3 != 0x0 ) { continue; }
			
    /* It's the right instruction; extract baseJumpAddress. */
    // /* DEBUG */ fprintf( stderr, "%s[%d]: mov.ip located at 0x%lx\n", __FILE__, __LINE__, iter.current );
    baseJumpAddress = iter.current & (~0x3);
    foundMovFromIP = true;
    break;
  }
	
  int64_t maxTableLength = -1;
  iter.setCurrentAddress( cmpltuUpperBound );
  for( ; iter.hasPrev(); --iter ) {
    instruction cmpltuInstruction = iter.getInstruction();
		
    if( cmpltuInstruction.getUnitType() != instruction::I && cmpltuInstruction.getUnitType() != instruction::M ) { continue; }
        
    /* If so, is it a cmp.ltu? */
    insn_tmpl cmpltu_tmpl = { cmpltuInstruction.getMachineCode() };
    if(	GET_OPCODE( &cmpltu_tmpl ) != 0xD 
	|| (!( cmpltu_tmpl.A8.x2 == 0x02 || cmpltu_tmpl.A8.x2 == 0x03 ))
	|| cmpltu_tmpl.A8.ta != 0x0 ) { continue; }
        
    /* Extract the immediate. */
    // /* DEBUG */ fprintf( stderr, "%s[%d]: located at 0x%lx\n", __FILE__, __LINE__, iter.current );
    maxTableLength = GET_A8_COUNT( &cmpltu_tmpl );
    break;
  }
  if( maxTableLength <= 0 ) {
    // /* DEBUG */ fprintf( stderr, "%s[%d]: maxTableLength negative, aborting parse.\n", __FILE__, __LINE__ );
    return false;
  }
    	
  // /* DEBUG */ fprintf( stderr, "%s[%d]: parsed jump table (@ 0x%lx) in %s \n", __FILE__, __LINE__, current, img_->name().c_str() );
    	
  /* Extract the jump targets. */
  for( unsigned int i = 0; i < maxTableLength; ++i ) {
    uint64_t finalAddr = baseJumpAddress;
		
    /* Get the jump table entry value. */
    if( img ) {
      /* The jump table may be in the code segment. */
      void * ptr = img->getPtrToData( jumpTableAddress + (sizeof(uint64_t) * i) );
      if( !ptr ) { ptr = img->getPtrToDataInText( jumpTableAddress + (sizeof(uint64_t) * i) ); }
      if( !ptr ) {
	/* DEBUG */ fprintf( stderr, "%s[%d]: unable to read jump target %d\n", __FILE__, __LINE__, i );
	continue;
      }
      finalAddr += *((uint64_t *)ptr);
    }
    else {
      void *ptr = instructions_->getPtrToInstruction( jumpTableAddress + (sizeof(uint64_t) * i) );
      if( ! ptr ) {
	/* DEBUG */ fprintf( stderr, "%s[%d]: unable to read jump target %d\n", __FILE__, __LINE__, i );
	continue;
      }
      finalAddr += *((uint64_t *)ptr);
    }
        
    if( ! foundMovFromIP ) { finalAddr += i * 8; }

    // /* DEBUG */ fprintf( stderr, "%s[%d]: jump table target address 0x%lx\n", __FILE__, __LINE__, finalAddr );
    targetAddresses.insert( finalAddr );
  } /* end jump table iteration */
    
  // /* DEBUG */ fprintf( stderr, "%s[%d]: located jump table for 0x%lx, at GP + 0x%lx %s\n\n", __FILE__, __LINE__, current, jumpTableOffset, foundInstruction2 ? "with double indirection." : "with single indirection." );
  return true;
} /* end of getMultipleJumpTargets(). */


bool InstrucIter::delayInstructionSupported()
{
  return false;
}

Address InstrucIter::peekPrev()
{
  int instructionSlot = extractInstructionSlot( current ); 
  switch( instructionSlot ) {
  case 0: {
    // This is complicated. If we're going onto a bundle
    // with a long insn, skip 2.
    Address temp = current - 16;
    void *tmpPtr;
    assert(instructions_);
    assert(instructions_->isValidAddress(temp));
    tmpPtr = instructions_->getPtrToInstruction(temp);
    ia64_bundle_t *rawBundle = (ia64_bundle_t *)tmpPtr;
    IA64_bundle tmpBundle = IA64_bundle(*rawBundle);
        
    if (tmpBundle.hasLongInstruction())
      return current - 16 + 1;
    else
      return current - 16 + 2;
  }
  case 1: return current - 1;
  case 2: 
    if (bundle.hasLongInstruction()) {
      return current - 2;
    }
    else
      return current - 1;
  default: return 0;
  }
}

Address InstrucIter::peekNext()
{
  int instructionSlot = extractInstructionSlot( current );
  switch( instructionSlot ) {
  case 0: return current + 1;
  case 1: 
    if (bundle.hasLongInstruction())
      return current - 1 + 16;
    else
      return current + 1;
  case 2: return current - 2 + 16;
  default: return 0;
  }
}

void InstrucIter::setCurrentAddress(Address addr)
{
  assert( (current % 0x10) < 3 );
  current = addr;
  initializeInsn();
}

instruction InstrucIter::getInstruction()
{	
  return insn;
}

// Suuuure...
bool InstrucIter::isStackFramePreamble(int &) {
  return true;
}

void InstrucIter::initializeInsn() {
  unsigned short slotno = current % 16;
  Address aligned = current - slotno;
  assert(instructions_);
  assert(instructions_->isValidAddress(aligned));
  instPtr = instructions_->getPtrToInstruction(aligned);
  if (instPtr == NULL) return;
    
  ia64_bundle_t *rawBundle = (ia64_bundle_t *)instPtr;
  bundle = IA64_bundle(*rawBundle);
    
  if (bundle.hasLongInstruction() && 
      (slotno == 2)) {
    cerr << "Attempt to grab the middle of a long instruction!" << endl;
    assert(0);
  }
    
  instruction *newInsn = bundle.getInstruction(slotno);
    
  insn = *newInsn;
    
  // Aaaand delete newInsn...
  delete newInsn;
}

instruction *InstrucIter::getInsnPtr() {
  unsigned short slotno = current % 16;
  Address aligned = current - slotno;
  assert(instructions_);
  assert(instructions_->isValidAddress(aligned));
  instPtr = instructions_->getPtrToInstruction(aligned);
    

  if (instPtr == NULL) return NULL;
    
  ia64_bundle_t *rawBundle = (ia64_bundle_t *)instPtr;
  bundle = IA64_bundle(*rawBundle);
    
  if (bundle.hasLongInstruction() && 
      (slotno == 2)) {
    cerr << "Attempt to grab the middle of a long instruction!" << endl;
    assert(0);
  }
    
  instruction *newInsn = bundle.getInstruction(slotno);

  return newInsn;
}
               
bool InstrucIter::isADynamicCallInstruction()
{
  assert(instPtr);
  return insn.getType() == instruction::INDIRECT_CALL;
}

bool InstrucIter::isALeaveInstruction()
{
  return false;
}

bool InstrucIter::isFrameSetup()
{
  return false;
}

bool InstrucIter::isFramePush()
{
  return false;
}
