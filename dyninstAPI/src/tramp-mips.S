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

/* $Id: tramp-mips.S,v 1.9 2003/10/21 17:22:35 bernat Exp $ */

/*  base trampoline template layout: 
 *
 *  pre-instrumentation
 *    - save registers
 *    - [slot to skip pre-instr]
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

/* mihai Mon Feb 21 14:34:06 CST 2000
 *
 * WARNING! WARNING! WARNING! WARNING!
 *
 * If you modify the base tramp layout
 * or contents in any way, make sure you synchronize
 * the changes with the baseNonRecursiveTramp
 * below.
 */
	
/* code directives */
.text
.set noreorder  /* no delay slot scheduling */
.set nomacro    /* no code expansion */

/* exported symbols */
.globl baseTramp
.globl baseTramp_savePreInsn
.globl baseTramp_skipPreInsn
.globl baseTramp_globalPreBranch
.globl baseTramp_localPreBranch
.globl baseTramp_localPreReturn
.globl baseTramp_updateCostInsn
.globl baseTramp_restorePreInsn
.globl baseTramp_emulateInsn
.globl baseTramp_skipPostInsn
.globl baseTramp_savePostInsn
.globl baseTramp_globalPostBranch
.globl baseTramp_localPostBranch
.globl baseTramp_localPostReturn
.globl baseTramp_restorePostInsn
.globl baseTramp_returnInsn
.globl baseTramp_endTramp

.globl _baseTramp
.globl _baseTramp_savePreInsn
.globl _baseTramp_skipPreInsn
.globl _baseTramp_globalPreBranch
.globl _baseTramp_localPreBranch
.globl _baseTramp_localPreReturn
.globl _baseTramp_updateCostInsn
.globl _baseTramp_restorePreInsn
.globl _baseTramp_emulateInsn
.globl _baseTramp_skipPostInsn
.globl _baseTramp_savePostInsn
.globl _baseTramp_globalPostBranch
.globl _baseTramp_localPostBranch
.globl _baseTramp_localPostReturn
.globl _baseTramp_restorePostInsn
.globl _baseTramp_returnInsn
.globl _baseTramp_endTramp

/* function body */
.ent baseTramp
baseTramp:
_baseTramp:	

/* save registers */
baseTramp_savePreInsn:	
_baseTramp_savePreInsn:
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
baseTramp_restorePreInsn:	
_baseTramp_restorePreInsn:
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
baseTramp_savePostInsn:	
_baseTramp_savePostInsn:
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
baseTramp_restorePostInsn:	
_baseTramp_restorePostInsn:
	daddiu	$sp,$sp,512
	
baseTramp_returnInsn:	
_baseTramp_returnInsn:	
	nop
	nop

baseTramp_endTramp:	
_baseTramp_endTramp:	
/* no code needed here */
	
.end baseTramp

/* mihai Mon Feb 21 14:13:00 CST 2000
 * This base trampoline is a copy of the base trampoling above
 * with the recursive guard code inserted. Such a trampoline
 * prevents execution to entry a mini-tramp once inside a
 * mini-tramp.
 */

/*  base trampoline with no recursion template layout: 
 *
 *  pre-instrumentation
 *    - save registers
 *    - [slot to skip pre-instr]
 *    - turn on recursive guard flag
 *    - [slot for global pre-instr]
 *    - [slot for local pre-instr]
 *    - turn off recursive guard flag
 *    - update instr cost
 *    - restore registers
 *  displaced insns
 *    - emulate insns
 *  post-instrumentation
 *    - [slot to skip post-instr]
 *    - save registers
 *    - turn on recursive guard flag
 *    - [slot for global post-instr]
 *    - [slot for local post-instr]
 *    - turn off recursive guard flag
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
.globl baseNonRecursiveTramp
.globl baseNonRecursiveTramp_savePreInsn
.globl baseNonRecursiveTramp_skipPreInsn
.globl baseNonRecursiveTramp_globalPreBranch
.globl baseNonRecursiveTramp_localPreBranch
.globl baseNonRecursiveTramp_localPreReturn
.globl baseNonRecursiveTramp_updateCostInsn
.globl baseNonRecursiveTramp_restorePreInsn
.globl baseNonRecursiveTramp_emulateInsn
.globl baseNonRecursiveTramp_skipPostInsn
.globl baseNonRecursiveTramp_savePostInsn
.globl baseNonRecursiveTramp_globalPostBranch
.globl baseNonRecursiveTramp_localPostBranch
.globl baseNonRecursiveTramp_localPostReturn
.globl baseNonRecursiveTramp_restorePostInsn
.globl baseNonRecursiveTramp_returnInsn
.globl baseNonRecursiveTramp_endTramp

.globl baseNonRecursiveTramp_guardOnPre_begin
.globl baseNonRecursiveTramp_guardOffPre_begin
.globl baseNonRecursiveTramp_guardOnPost_begin
.globl baseNonRecursiveTramp_guardOffPost_begin
.globl baseNonRecursiveTramp_guardOnPre_end
.globl baseNonRecursiveTramp_guardOffPre_end
.globl baseNonRecursiveTramp_guardOnPost_end
.globl baseNonRecursiveTramp_guardOffPost_end

.globl _baseNonRecursiveTramp
.globl _baseNonRecursiveTramp_savePreInsn
.globl _baseNonRecursiveTramp_skipPreInsn
.globl _baseNonRecursiveTramp_globalPreBranch
.globl _baseNonRecursiveTramp_localPreBranch
.globl _baseNonRecursiveTramp_localPreReturn
.globl _baseNonRecursiveTramp_updateCostInsn
.globl _baseNonRecursiveTramp_restorePreInsn
.globl _baseNonRecursiveTramp_emulateInsn
.globl _baseNonRecursiveTramp_skipPostInsn
.globl _baseNonRecursiveTramp_savePostInsn
.globl _baseNonRecursiveTramp_globalPostBranch
.globl _baseNonRecursiveTramp_localPostBranch
.globl _baseNonRecursiveTramp_localPostReturn
.globl _baseNonRecursiveTramp_restorePostInsn
.globl _baseNonRecursiveTramp_returnInsn
.globl _baseNonRecursiveTramp_endTramp

.globl _baseNonRecursiveTramp_guardOnPre_begin
.globl _baseNonRecursiveTramp_guardOffPre_begin
.globl _baseNonRecursiveTramp_guardOnPost_begin
.globl _baseNonRecursiveTramp_guardOffPost_begin
.globl _baseNonRecursiveTramp_guardOnPre_end
.globl _baseNonRecursiveTramp_guardOffPre_end
.globl _baseNonRecursiveTramp_guardOnPost_end
.globl _baseNonRecursiveTramp_guardOffPost_end

/* function body */
.ent baseNonRecursiveTramp
baseNonRecursiveTramp:
_baseNonRecursiveTramp:	

/* save registers */
baseNonRecursiveTramp_savePreInsn:	
_baseNonRecursiveTramp_savePreInsn:
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
		
baseNonRecursiveTramp_skipPreInsn:	
_baseNonRecursiveTramp_skipPreInsn:
	nop
	nop
	
baseNonRecursiveTramp_guardOnPre_begin:
_baseNonRecursiveTramp_guardOnPre_begin:
	nop			/* lui $t0, bits 63-48 of address      */
	nop			/* ori $t0, $t0, bits 47-32 of address */
	nop			/* lui $t1, bits 31-16 of address      */
	nop			/* ori $t1, $t1, bits 15- 0 of address */
	dsll32 $12, $12, 0	/* dsll32 $t0, $t0, 0		       */
	or $13, $12, $13	/* or $t1, $t0, $t1                    */
	lw $12, 0($13)		/* lw $t0, 0($t1)                      */
	nop			/* beq $t0, $zero, guardOffPre_end     */
	sw $0, 0($13)		/* sw $zero, 0($t1)                    */
baseNonRecursiveTramp_guardOnPre_end:
_baseNonRecursiveTramp_guardOnPre_end:

baseNonRecursiveTramp_localPreBranch:	
_baseNonRecursiveTramp_localPreBranch:	
/* just in case, leave enough room for a long jump (8-insn sequence) */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

baseNonRecursiveTramp_localPreReturn:	
_baseNonRecursiveTramp_localPreReturn:	

baseNonRecursiveTramp_guardOffPre_begin:
_baseNonRecursiveTramp_guardOffPre_begin:
	nop			/* lui $t0, bits 63-48 of address      */
	nop			/* ori $t0, $t0, bits 47-32 of address */
	nop			/* lui $t1, bits 31-16 of address      */
	nop			/* ori $t1, $t1, bits 15- 0 of address */
	dsll32 $12, $12, 0	/* dsll32 $t0, $t0, 0                  */
	or $13, $12, $13	/* or $t1, $t0, $t1                    */
	addi $12, $0, 1		/* addi $t0, $zero, 1                  */
	sw $12, 0($13)		/* sw $zero, 0($t1)                    */
baseNonRecursiveTramp_updateCostInsn:
_baseNonRecursiveTramp_updateCostInsn:
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
baseNonRecursiveTramp_restorePreInsn:	
_baseNonRecursiveTramp_restorePreInsn:
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
	
baseNonRecursiveTramp_emulateInsn:	
_baseNonRecursiveTramp_emulateInsn:	
	nop	/* displaced insn (jump) */
	nop	/* displaced insn (delay slot) */
/* TODO - this should be big enough to accomodate
   the largest possible instPoint footprint (= ???) */
	
baseNonRecursiveTramp_skipPostInsn:	
_baseNonRecursiveTramp_skipPostInsn:	
	nop
        nop

/* save registers */
baseNonRecursiveTramp_savePostInsn:	
_baseNonRecursiveTramp_savePostInsn:
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

baseNonRecursiveTramp_guardOnPost_begin:
_baseNonRecursiveTramp_guardOnPost_begin:
	nop			/* lui $t0,      bits 63-48 of address */
	nop			/* ori $t0, $t0, bits 47-32 of address */
	nop			/* lui $t1,      bits 31-16 of address */
	nop			/* ori $t1, $t1, bits 15- 0 of address */
	dsll32 $12, $12, 0	/* dsll32 $t0, $t0, 0                  */
	or $13, $12, $13	/* or $t1, $t0, $t1                    */
	lw $12, 0($13)		/* lw $t0, 0($t1)                      */
	nop			/* beq $t0, $zero, guardOffPost_end    */
	sw $0, 0($13)		/* sw $zero, 0($t1)                    */
baseNonRecursiveTramp_guardOnPost_end:
_baseNonRecursiveTramp_guardOnPost_end:

baseNonRecursiveTramp_localPostBranch:	
_baseNonRecursiveTramp_localPostBranch:	
/* just in case, leave enough room for a long jump (8-insn sequence) */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

baseNonRecursiveTramp_localPostReturn:	
_baseNonRecursiveTramp_localPostReturn:	

baseNonRecursiveTramp_guardOffPost_begin:
_baseNonRecursiveTramp_guardOffPost_begin:
	nop			/* lui $t0,      bits 63-48 of address */
	nop			/* ori $t0, $t0, bits 47-32 of address */
	nop			/* lui $t1,      bits 31-16 of address */
	nop			/* ori $t1, $t1, bits 15- 0 of address */
	dsll32 $12, $12, 0	/* dsll32 $t0, $t0, 0                  */
	or $13, $12, $13	/* or $t1, $t0, $t1                    */
	addi $12, $0, 1		/* addi $t0, $zero, 1                  */
	sw $12, 0($13)		/* sw 1, 0($t1)                        */
/* restore registers */
baseNonRecursiveTramp_restorePostInsn:	
_baseNonRecursiveTramp_restorePostInsn:
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
	
baseNonRecursiveTramp_returnInsn:	
_baseNonRecursiveTramp_returnInsn:	
	nop
	nop

baseNonRecursiveTramp_endTramp:	
_baseNonRecursiveTramp_endTramp:	
/* no code needed here */
	
.end baseNonRecursiveTramp


/* 
 * This is the conservative base trampoline for use at arbitrary
 * instrumentation points.  It saves additional registers over those saved by
 * the "normal" base trampoline.
 */
/* code directives */
.text
.set noreorder  /* no delay slot scheduling */
.set nomacro    /* no code expansion */

/* exported symbols */
.globl conservativeTramp
.globl conservativeTramp_savePreInsn
.globl conservativeTramp_skipPreInsn
.globl conservativeTramp_globalPreBranch
.globl conservativeTramp_localPreBranch
.globl conservativeTramp_localPreReturn
.globl conservativeTramp_updateCostInsn
.globl conservativeTramp_restorePreInsn
.globl conservativeTramp_emulateInsn
.globl conservativeTramp_skipPostInsn
.globl conservativeTramp_savePostInsn
.globl conservativeTramp_globalPostBranch
.globl conservativeTramp_localPostBranch
.globl conservativeTramp_localPostReturn
.globl conservativeTramp_restorePostInsn
.globl conservativeTramp_returnInsn
.globl conservativeTramp_endTramp
.globl _conservativeTramp
.globl _conservativeTramp_savePreInsn
.globl _conservativeTramp_skipPreInsn
.globl _conservativeTramp_globalPreBranch
.globl _conservativeTramp_localPreBranch
.globl _conservativeTramp_localPreReturn
.globl _conservativeTramp_updateCostInsn
.globl _conservativeTramp_restorePreInsn
.globl _conservativeTramp_emulateInsn
.globl _conservativeTramp_skipPostInsn
.globl _conservativeTramp_savePostInsn
.globl _conservativeTramp_globalPostBranch
.globl _conservativeTramp_localPostBranch
.globl _conservativeTramp_localPostReturn
.globl _conservativeTramp_restorePostInsn
.globl _conservativeTramp_returnInsn
.globl _conservativeTramp_endTramp

/* function body */
.ent conservativeTramp
conservativeTramp:
_conservativeTramp:	

/* save registers */
conservativeTramp_savePreInsn:	
_conservativeTramp_savePreInsn:
	daddiu	$sp,$sp,-544
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

	mfhi     $12		/* Save HI register. */
	sd       $12,512($sp)
	mflo	 $12		/* Save LO register. */
	sd	 $12,520($sp)
	cfc1	 $12,$31	/* Save floating point control/status reg. */
	sd       $12,528($sp)

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
		
conservativeTramp_skipPreInsn:	
_conservativeTramp_skipPreInsn:
	nop
	nop
	
conservativeTramp_globalPreBranch:	
_conservativeTramp_globalPreBranch:	
/* what is this slot for? */

conservativeTramp_localPreBranch:	
_conservativeTramp_localPreBranch:	
/* just in case, leave enough room for a long jump (8-insn sequence) */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

conservativeTramp_localPreReturn:	
_conservativeTramp_localPreReturn:	

conservativeTramp_updateCostInsn:	
_conservativeTramp_updateCostInsn:	
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

	ld       $12,512($sp)   /* Restore HI register. */
	mthi     $12
	ld	 $12,520($sp)	/* Restore LO register. */
	mtlo	 $12
	ld       $12,528($sp)	/* Restore floating point control/status reg. */
	ctc1	 $12,$31

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
conservativeTramp_restorePreInsn:	
_conservativeTramp_restorePreInsn:
	daddiu	$sp,$sp,544
	
conservativeTramp_emulateInsn:	
_conservativeTramp_emulateInsn:	
	nop	/* displaced insn (jump) */
	nop	/* displaced insn (delay slot) */
/* TODO - this should be big enough to accomodate
   the largest possible instPoint footprint (= ???) */
	
conservativeTramp_skipPostInsn:	
_conservativeTramp_skipPostInsn:	
	nop
        nop

/* save registers */
conservativeTramp_savePostInsn:	
_conservativeTramp_savePostInsn:
	daddiu	 $sp,$sp,-544
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

	mfhi     $12		/* Save HI register. */
	sd       $12,512($sp)
	mflo	 $12		/* Save LO register. */
	sd	 $12,520($sp)
	cfc1	 $12,$31	/* Save floating point control/status reg. */
	sd       $12,528($sp)

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

conservativeTramp_globalPostBranch:	
_conservativeTramp_globalPostBranch:	
/* what is this slot for? */

conservativeTramp_localPostBranch:	
_conservativeTramp_localPostBranch:	
/* just in case, leave enough room for a long jump (8-insn sequence) */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

conservativeTramp_localPostReturn:	
_conservativeTramp_localPostReturn:	

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

	ld       $12,512($sp)   /* Restore HI register. */
	mthi     $12
	ld	 $12,520($sp)	/* Restore LO register. */
	mtlo	 $12
	ld       $12,528($sp)	/* Restore floating point control/status reg. */
	ctc1	 $12,$31

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
conservativeTramp_restorePostInsn:	
_conservativeTramp_restorePostInsn:
	daddiu	$sp,$sp,544
	
conservativeTramp_returnInsn:	
_conservativeTramp_returnInsn:	
	nop
	nop

conservativeTramp_endTramp:	
_conservativeTramp_endTramp:	
/* no code needed here */
	
.end conservativeTramp
