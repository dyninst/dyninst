/*
 * Copyright (c) 1998 Barton P. Miller
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

/*  base trampoline template layout: 
 *
 *  pre-instrumentation
 *    - [slot to skip pre-instr]
 *    - save registers
 *    - [slot for global pre-instr]
 *    - [slot for local pre-instr]
 *    - update instr cost
 *    - restore registers
 *  displaced insns
 *    - emulate insns
 *  post-instrumentation
 *    - [slot to skip post-instr]
 *    - save registers
 *    - [slot for global post-instr]
 *    - [slot for local post-instr]
 *    - restore registers
 *  return to app
 *    - jump to user code
 *  (end of trampoline)      
 */


	
/* code directives */
.text
.set noreorder  /* no delay slot scheduling */
.set nomacro    /* no code expansion */

/* exported symbols */
.globl baseTramp
.globl baseTramp_skipPreInsn
.globl baseTramp_globalPreBranch
.globl baseTramp_localPreBranch
.globl baseTramp_localPreReturn
.globl baseTramp_updateCostInsn
.globl baseTramp_emulateInsn
.globl baseTramp_skipPostInsn
.globl baseTramp_globalPostBranch
.globl baseTramp_localPostBranch
.globl baseTramp_localPostReturn
.globl baseTramp_returnInsn
.globl baseTramp_endTramp
.globl _baseTramp
.globl _baseTramp_skipPreInsn
.globl _baseTramp_globalPreBranch
.globl _baseTramp_localPreBranch
.globl _baseTramp_localPreReturn
.globl _baseTramp_updateCostInsn
.globl _baseTramp_emulateInsn
.globl _baseTramp_skipPostInsn
.globl _baseTramp_globalPostBranch
.globl _baseTramp_localPostBranch
.globl _baseTramp_localPostReturn
.globl _baseTramp_returnInsn
.globl _baseTramp_endTramp

/* function body */
.ent baseTramp
baseTramp:
_baseTramp:	

/* save registers */
	daddiu	$sp,$sp,-512
	sdc1	 $f0,504($sp)
	sdc1	 $f1,496($sp)
	sdc1	 $f2,488($sp)
	sdc1	 $f3,480($sp)
	sdc1	 $f4,472($sp)
	sdc1	 $f5,464($sp)
	sdc1	 $f6,456($sp)
	sdc1	 $f7,448($sp)
	sdc1	 $f8,440($sp)
	sdc1	 $f9,432($sp)
	sdc1	$f10,424($sp)
	sdc1	$f11,416($sp)
	sdc1	$f12,408($sp)
	sdc1	$f13,400($sp)
	sdc1	$f14,392($sp)
	sdc1	$f15,384($sp)
	sdc1	$f16,376($sp)
	sdc1	$f17,368($sp)
	sdc1	$f18,360($sp)
	sdc1	$f19,352($sp)
	sdc1	$f20,344($sp)
	sdc1	$f21,336($sp)
	sdc1	$f22,328($sp)
	sdc1	$f23,320($sp)
	sdc1	$f24,312($sp)
	sdc1	$f25,304($sp)
	sdc1	$f26,296($sp)
	sdc1	$f27,288($sp)
	sdc1	$f28,280($sp)
	sdc1	$f29,272($sp)
	sdc1	$f30,264($sp)
	sdc1	$f31,256($sp)
	sd	  $0,248($sp)
	sd	  $1,240($sp)
	sd	  $2,232($sp)
	sd	  $3,224($sp)
	sd	  $4,216($sp)
	sd	  $5,208($sp)
	sd	  $6,200($sp)
	sd	  $7,192($sp)
	sd	  $8,184($sp)
	sd	  $9,176($sp)
	sd	 $10,168($sp)
	sd	 $11,160($sp)
	sd	 $12,152($sp)
	sd	 $13,144($sp)
	sd	 $14,136($sp)
	sd	 $15,128($sp)
	sd	 $16,120($sp)
	sd	 $17,112($sp)
	sd	 $18,104($sp)
	sd	 $19, 96($sp)
	sd	 $20, 88($sp)
	sd	 $21, 80($sp)
	sd	 $22, 72($sp)
	sd	 $23, 64($sp)
	sd	 $24, 56($sp)
	sd	 $25, 48($sp)
	sd	 $26, 40($sp)
	sd	 $27, 32($sp)
	sd	 $28, 24($sp)
	sd	 $29, 16($sp)
	sd	 $30,  8($sp)
	sd	 $31,  0($sp)
		
baseTramp_skipPreInsn:	
_baseTramp_skipPreInsn:
	nop
	nop
	
baseTramp_globalPreBranch:	
_baseTramp_globalPreBranch:	
/* what is this slot for? */

baseTramp_localPreBranch:	
_baseTramp_localPreBranch:	
/* just in case, leave enough room for a long jump (8-insn sequence) */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

baseTramp_localPreReturn:	
_baseTramp_localPreReturn:	

baseTramp_updateCostInsn:	
_baseTramp_updateCostInsn:	
/* must be big enough for emitted "updateCost" code */
/* worst case: genLoadConst + load + genLoadConst + add + store */
/* worst case: 6 + 1 + 6 + 1 + 1 = 15 */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

/* restore registers */
	ldc1	 $f0,504($sp)
	ldc1	 $f1,496($sp)
	ldc1	 $f2,488($sp)
	ldc1	 $f3,480($sp)
	ldc1	 $f4,472($sp)
	ldc1	 $f5,464($sp)
	ldc1	 $f6,456($sp)
	ldc1	 $f7,448($sp)
	ldc1	 $f8,440($sp)
	ldc1	 $f9,432($sp)
	ldc1	$f10,424($sp)
	ldc1	$f11,416($sp)
	ldc1	$f12,408($sp)
	ldc1	$f13,400($sp)
	ldc1	$f14,392($sp)
	ldc1	$f15,384($sp)
	ldc1	$f16,376($sp)
	ldc1	$f17,368($sp)
	ldc1	$f18,360($sp)
	ldc1	$f19,352($sp)
	ldc1	$f20,344($sp)
	ldc1	$f21,336($sp)
	ldc1	$f22,328($sp)
	ldc1	$f23,320($sp)
	ldc1	$f24,312($sp)
	ldc1	$f25,304($sp)
	ldc1	$f26,296($sp)
	ldc1	$f27,288($sp)
	ldc1	$f28,280($sp)
	ldc1	$f29,272($sp)
	ldc1	$f30,264($sp)
	ldc1	$f31,256($sp)
	ld	  $0,248($sp)
	ld	  $1,240($sp)
	ld	  $2,232($sp)
	ld	  $3,224($sp)
	ld	  $4,216($sp)
	ld	  $5,208($sp)
	ld	  $6,200($sp)
	ld	  $7,192($sp)
	ld	  $8,184($sp)
	ld	  $9,176($sp)
	ld	 $10,168($sp)
	ld	 $11,160($sp)
	ld	 $12,152($sp)
	ld	 $13,144($sp)
	ld	 $14,136($sp)
	ld	 $15,128($sp)
	ld	 $16,120($sp)
	ld	 $17,112($sp)
	ld	 $18,104($sp)
	ld	 $19, 96($sp)
	ld	 $20, 88($sp)
	ld	 $21, 80($sp)
	ld	 $22, 72($sp)
	ld	 $23, 64($sp)
	ld	 $24, 56($sp)
	ld	 $25, 48($sp)
	ld	 $26, 40($sp)
	ld	 $27, 32($sp)
	ld	 $28, 24($sp)
	ld	 $29, 16($sp)
	ld	 $30,  8($sp)
	ld	 $31,  0($sp)
	daddiu	$sp,$sp,512
	
baseTramp_emulateInsn:	
_baseTramp_emulateInsn:	
	nop	/* displaced insn (jump) */
	nop	/* displaced insn (delay slot) */
/* TODO - this should be big enough to accomodate
   the largest possible instPoint footprint (= ???) */
	
baseTramp_skipPostInsn:	
_baseTramp_skipPostInsn:	
	nop
        nop

/* save registers */
	daddiu	$sp,$sp,-512
	sdc1	 $f0,504($sp)
	sdc1	 $f1,496($sp)
	sdc1	 $f2,488($sp)
	sdc1	 $f3,480($sp)
	sdc1	 $f4,472($sp)
	sdc1	 $f5,464($sp)
	sdc1	 $f6,456($sp)
	sdc1	 $f7,448($sp)
	sdc1	 $f8,440($sp)
	sdc1	 $f9,432($sp)
	sdc1	$f10,424($sp)
	sdc1	$f11,416($sp)
	sdc1	$f12,408($sp)
	sdc1	$f13,400($sp)
	sdc1	$f14,392($sp)
	sdc1	$f15,384($sp)
	sdc1	$f16,376($sp)
	sdc1	$f17,368($sp)
	sdc1	$f18,360($sp)
	sdc1	$f19,352($sp)
	sdc1	$f20,344($sp)
	sdc1	$f21,336($sp)
	sdc1	$f22,328($sp)
	sdc1	$f23,320($sp)
	sdc1	$f24,312($sp)
	sdc1	$f25,304($sp)
	sdc1	$f26,296($sp)
	sdc1	$f27,288($sp)
	sdc1	$f28,280($sp)
	sdc1	$f29,272($sp)
	sdc1	$f30,264($sp)
	sdc1	$f31,256($sp)
	sd	  $0,248($sp)
	sd	  $1,240($sp)
	sd	  $2,232($sp)
	sd	  $3,224($sp)
	sd	  $4,216($sp)
	sd	  $5,208($sp)
	sd	  $6,200($sp)
	sd	  $7,192($sp)
	sd	  $8,184($sp)
	sd	  $9,176($sp)
	sd	 $10,168($sp)
	sd	 $11,160($sp)
	sd	 $12,152($sp)
	sd	 $13,144($sp)
	sd	 $14,136($sp)
	sd	 $15,128($sp)
	sd	 $16,120($sp)
	sd	 $17,112($sp)
	sd	 $18,104($sp)
	sd	 $19, 96($sp)
	sd	 $20, 88($sp)
	sd	 $21, 80($sp)
	sd	 $22, 72($sp)
	sd	 $23, 64($sp)
	sd	 $24, 56($sp)
	sd	 $25, 48($sp)
	sd	 $26, 40($sp)
	sd	 $27, 32($sp)
	sd	 $28, 24($sp)
	sd	 $29, 16($sp)
	sd	 $30,  8($sp)
	sd	 $31,  0($sp)

baseTramp_globalPostBranch:	
_baseTramp_globalPostBranch:	
/* what is this slot for? */

baseTramp_localPostBranch:	
_baseTramp_localPostBranch:	
/* just in case, leave enough room for a long jump (8-insn sequence) */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

baseTramp_localPostReturn:	
_baseTramp_localPostReturn:	

/* restore registers */
	ldc1	 $f0,504($sp)
	ldc1	 $f1,496($sp)
	ldc1	 $f2,488($sp)
	ldc1	 $f3,480($sp)
	ldc1	 $f4,472($sp)
	ldc1	 $f5,464($sp)
	ldc1	 $f6,456($sp)
	ldc1	 $f7,448($sp)
	ldc1	 $f8,440($sp)
	ldc1	 $f9,432($sp)
	ldc1	$f10,424($sp)
	ldc1	$f11,416($sp)
	ldc1	$f12,408($sp)
	ldc1	$f13,400($sp)
	ldc1	$f14,392($sp)
	ldc1	$f15,384($sp)
	ldc1	$f16,376($sp)
	ldc1	$f17,368($sp)
	ldc1	$f18,360($sp)
	ldc1	$f19,352($sp)
	ldc1	$f20,344($sp)
	ldc1	$f21,336($sp)
	ldc1	$f22,328($sp)
	ldc1	$f23,320($sp)
	ldc1	$f24,312($sp)
	ldc1	$f25,304($sp)
	ldc1	$f26,296($sp)
	ldc1	$f27,288($sp)
	ldc1	$f28,280($sp)
	ldc1	$f29,272($sp)
	ldc1	$f30,264($sp)
	ldc1	$f31,256($sp)
	ld	  $0,248($sp)
	ld	  $1,240($sp)
	ld	  $2,232($sp)
	ld	  $3,224($sp)
	ld	  $4,216($sp)
	ld	  $5,208($sp)
	ld	  $6,200($sp)
	ld	  $7,192($sp)
	ld	  $8,184($sp)
	ld	  $9,176($sp)
	ld	 $10,168($sp)
	ld	 $11,160($sp)
	ld	 $12,152($sp)
	ld	 $13,144($sp)
	ld	 $14,136($sp)
	ld	 $15,128($sp)
	ld	 $16,120($sp)
	ld	 $17,112($sp)
	ld	 $18,104($sp)
	ld	 $19, 96($sp)
	ld	 $20, 88($sp)
	ld	 $21, 80($sp)
	ld	 $22, 72($sp)
	ld	 $23, 64($sp)
	ld	 $24, 56($sp)
	ld	 $25, 48($sp)
	ld	 $26, 40($sp)
	ld	 $27, 32($sp)
	ld	 $28, 24($sp)
	ld	 $29, 16($sp)
	ld	 $30,  8($sp)
	ld	 $31,  0($sp)
	daddiu	$sp,$sp,512
	
baseTramp_returnInsn:	
_baseTramp_returnInsn:	
	nop
	nop

baseTramp_endTramp:	
_baseTramp_endTramp:	
/* no code needed here */
	
.end baseTramp
