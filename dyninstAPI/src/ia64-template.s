.file "ia64-template.s"
.text
.align 16

##
#  Code template for dynamically creating a canonical set
#  of free registers.
##

# This function just stashes the PFS away in r4 for us.
.global fetch_ar_pfs#
.proc fetch_ar_pfs#
fetch_ar_pfs#:
	alloc r4 = ar.pfs, 0, 0, 0, 0;;
	br.ret.sptk b0;;
.endp fetch_ar_pfs#


# We insert the jump to the first minitramp in this function,
# which also provides the address to which the last minitramp returns.
.proc preservation_function#
preservation_function#:
	# We need to keep this ar.pfs around to make sure the
	# RSE does the right thing when we return.
	alloc r32 = ar.pfs, 0, 8, 8, 0;;
	st8 [r12] = r32,-8;;
	
	# Preserve b0.  When we get to after_instrumention#,
	# r12 will have the value it does after this store,
	# so we can skip adding 8 to it if we don't subtract
	# from it here.  This also means the SP is pointing to
	# a (the last) valid data-word, which is as the
	# runtime conventions desire.  (Except we save
	# an instruction in the function call state preservation
	# by hanging that -8 immediate off the end.)
	mov r32 = b0;;
	st8 [r12] = r32, -8;;

	# We've already saved b0 and the predicate registers.
	# Save the GP, r2, r3, r8 - r11, r14 - r31,
	# b6 and b7,
	# f6-7, f8-15, f32 - f127,
	# ar.ccv, csd, ssd,
	# and FIXME: handle ar.fpsr (status field 1), ar.rsc (mode), and User Mask,
	# so we can make function calls.
	st8 [r12] = r1, -8;;
	st8 [r12] = r2, -8;;
	st8 [r12] = r3, -8;;

	st8 [r12] = r8, -8;;
	st8 [r12] = r9, -8;;
	st8 [r12] = r10, -8;;
	st8 [r12] = r11, -8;;

	st8 [r12] = r14, -8;;
	st8 [r12] = r15, -8;;
	st8 [r12] = r16, -8;;
	st8 [r12] = r17, -8;;
	st8 [r12] = r18, -8;;
	st8 [r12] = r19, -8;;
	st8 [r12] = r20, -8;;
	st8 [r12] = r21, -8;;
	st8 [r12] = r22, -8;;
	st8 [r12] = r23, -8;;
	st8 [r12] = r24, -8;;
	st8 [r12] = r25, -8;;
	st8 [r12] = r26, -8;;
	st8 [r12] = r27, -8;;
	st8 [r12] = r28, -8;;
	st8 [r12] = r29, -8;;
	st8 [r12] = r30, -8;;
	st8 [r12] = r31, -8;;

	mov r16 = b6;;
	st8 [r12] = r16, -8;;
	mov r16 = b7;;
	st8 [r12] = r16, -16;;

.global address_of_jump_to_spills#
address_of_jump_to_spills#:
	{.mlx nop.m 0x0; nop.x 0x0;; }
	
	mov r16 = ar.ccv;;
	st8 [r12] = r16, -8;;
	mov r16 = ar.csd;;
	st8 [r12] = r16, -8;;
	mov r16 = ar.ssd;;	
	st8 [r12] = r16;;

	# FIXME: restoration handling ar.fpsr (status field 1), ar.rsc (mode), and User Mask.

	# r32-r39 are now freely available input registers.
	# r40-r48 are now freely available output registers.
	# Predicate registers are all freely available.
	# r4 holds the address of the first input register.
	# r5 holds the address of the first output register.
	# We can freely make function calls.

.global address_of_instrumentation#
address_of_instrumentation#:
	# This should be replaced with a jump to the first minitramp,
	# which will return to the address of 'after_instrumentation#'.
	# For normal instrumentation, just use this template twice,
	# inserting a jump to the emulation bundles at
	# "address_of_jump_to_emulation#".
	.mii { nop.m 0x00; nop.i 0x00; nop.i 0x00;; }

	# Restore function-call state.

	ld8 r16 = [r12], 8;;
	mov ar.ssd = r16;;
	ld8 r16 = [r12], 8;;
	mov ar.csd = r16;;
	ld8 r16 = [r12], 16;;
	mov ar.ccv = r16;;

.global address_of_jump_to_fills#
address_of_jump_to_fills#:
	{.mlx nop.m 0x0; nop.x 0x0;; }

	ld8 r16 = [r12], 8;;
	mov b7 = r16;;
	ld8 r16 = [r12], 8;;
	mov b6 = r16;;
	
	ld8 r31 = [r12], 8;;
	ld8 r30 = [r12], 8;;
	ld8 r29 = [r12], 8;;
	ld8 r28 = [r12], 8;;
	ld8 r26 = [r12], 8;;
	ld8 r25 = [r12], 8;;
	ld8 r24 = [r12], 8;;
	ld8 r23 = [r12], 8;;
	ld8 r22 = [r12], 8;;
	ld8 r21 = [r12], 8;;
	ld8 r20 = [r12], 8;;
	ld8 r19 = [r12], 8;;
	ld8 r18 = [r12], 8;;
	ld8 r17 = [r12], 8;;
	ld8 r16 = [r12], 8;;
	ld8 r15 = [r12], 8;;
	ld8 r14 = [r12], 8;;

	ld8 r11 = [r12], 8;;
	ld8 r10 = [r12], 8;;
	ld8 r9 = [r12], 8;;
	ld8 r8 = [r12], 8;;

	ld8 r3 = [r12], 8;;
	ld8 r2 = [r12], 8;;
	ld8 r1 = [r12], 8;;

	# Restore b0, assuming by the SCRAG that the stack
	# pointer didn't move.  It doesn't matter what
	# we do to the SP after we load the PFS, because we'll be
	# re-setting it from r4 anyway.
	ld8 r32 = [r12], 8;;
	mov b0 = r32;;

	# Restore the PFS.
	ld8 r32 = [r12];;
	mov ar.pfs = r32;;

	br.ret.sptk b0;;

.endp preservation_function#

# We jump or alter the PC to get to this code block
# (it's not a function, really) when we do instrumentation
# or iRPCs, respectively.
.global ia64_tramp_half#
.proc ia64_tramp_half#
ia64_tramp_half#:

	# Use a trick from HP's Caliper paper to preserve
	# general register r4.

		# Spill a floating-point register to memory.
		adds r12 = -16,r12;;
		stf.spill [r12] = f31, -16;;

		# Copy over the integer register of interest.
		setf.sig f31 = r4;;
		# Spill it.
		stf.spill [r12] = f31, -8;;

	# Preserve the UNAT, so we can spill general
	# registers at will.
	mov r4 = ar.unat;;
	st8 [r12] = r4, -8;;

	# With the UNAT preserved, prepare for the call,
	# using r4 to write ar.pfs and b0 to the memory stack.
	mov r4 = ar.pfs;;
	st8 [r12] = r4, -8;;
	mov r4 = b0;;
	st8 [r12] = r4, -8;;

	# Make the call.
	br.call.sptk b0 = fetch_ar_pfs#

######### r4 now contains the CFM.  Use this to calculate the
	# number of locals and outputs in the current frame. 

	# Preserve r14, r15
	st8.spill [r12] = r14, -8;;
	st8.spill [r12] = r15, -8;;

# You know, some of this (self-modifying) stuff could
# probably be replaced with 'deposit' instructions...
######### Calculate sol in r14, soo in r15.
	mov r14 = 0x3F80;;
	and r14 = r14,r4;;
	shr.u r14 = r14,7;;	# r14 is now the sol
	and r15 = 0x007F,r4;;	# r15 is now the sof
	sub r15 = r15, r14;;	# r15 is now the soo

	# Multiply the sizes by eight to determine
	# offsets from the stack pointer to the first
	# and last output register, respectively.
	shl r14 = r14,3
	shl r15 = r15,3;;

	# We avoid spilling extra registers by overwriting the first
	# such instruction with an indirect jump past the end of the spills.
	# We use an indirect jump so we can just jump to that point
	# without calculating offsets every time.

	# Preserve all the < 32 registers I'll need right here,
	# to avoid worrying about multiple UNATs.
	st8.spill [r12] = r5, -8;;	# The address of out0.
	st8.spill [r12] = r6, -8;;	# The CFM.
	mov r6 = r4;;
	st8.spill [r12] = r16, -8;;	# The address of the last valid output register.

	# Preserve the UNAT.  We may destroy the UNAT bits of the registers 
	# we spilled while spilling input/output registers.  We must
	# spill them (rather than store them) to avoid NaT consumption faults,
	# but we don't actually care about their NaT bits; we _want_
	# to get garbage, rather than faults, if the instrumentation looks at them.
	# (That is, the st8 instruction can cause NaT consumption faults.  See below
	# for why we bother with this at all.)
	mov r16 = ar.unat;;
	st8 [r12]= r16, -8;;

	# Preserve the predicate registers.
	mov r16 = pr;;
	st8 [r12] = r16, -8;;

######### The first input register's address will be stored in r4.
	mov r4 = r12;;
######### Calculate the address of the first output register, store in r5.
	sub r5 = r12, r14;;

	# Calcuate the address of the first invalid register.
	sub r16 = r5, r15;;

	# We preserved the UNAT, so don't worry about overwriting
	# it (its only 64 bits wide) with local/output spills.
	
	# This construction lets us use N bundles to store
	# N registers, (rather than 2N), at the cost of wasting
	# some time loading instructions that won't be executed.
	cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r32, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r33, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r34, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r35, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r36, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r37, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r38, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r39, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r40, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r41, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r42, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r43, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r44, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r45, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r46, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r47, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r48, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r49, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r50, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r51, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r52, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r53, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r54, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r55, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r56, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r57, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r58, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r59, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r60, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r61, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r62, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r63, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r64, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r65, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r66, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r67, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r68, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r69, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r70, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r71, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r72, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r73, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r74, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r75, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r76, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r77, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r78, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r79, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r80, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r81, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r82, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r83, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r84, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r85, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r86, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r87, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r88, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r89, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r90, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r91, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r92, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r93, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r94, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r95, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r96, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r97, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r98, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r99, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r100, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r101, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r102, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r103, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r104, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r105, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r106, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r107, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r108, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r109, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r110, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r111, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r112, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r113, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r114, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r115, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r116, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r117, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r118, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r119, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r120, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r121, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r122, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r123, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r124, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r125, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r126, -8;;

	(p2) cmp.eq p1,p2 = r12, r16;;
	(p2) st8.spill [r12] = r127, -8;;
	
	# We need to preserve all the current registers,
	# so allocate all of them as inputs.  We do this
	# by altering the alloc at address_of_call_alloc#.

	# The reason we save all those registers above
	# and never load them is so that the instrumentation
	# can look at the input and output registers if it
	# ever has to.  Normal (construction-time) preservation
	# routines Just Know where the i/o registers are,
	# so they don't need this hack.  (Hack may be involved
	# in getting code generator to recognize the difference, but...)
		
	add r16 = r14, r15;;
	shl r16 = r16, 22;; # shift past the templateID, too (shifted 3 already)

.global address_of_address_of_call_alloc#	
address_of_address_of_call_alloc#:
	movl r14 = 0x000;;
	ld8 r15 = [r14];;
	or r15 = r15, r16;; # sol
	shr.u r16 = r16, 7;;
	or r15 = r15, r16;; # sof

	# Include sor from the CFM in r6.
	mov r16 = 0x3C000;;
	and r16 = r6, r16;;
	shl r16 = r16, 18;; # shifted 14 already, shift past templateID too
	or r15 = r16, r15;;

	st8 [r14] = r15;;


	# Since we'll return to the bogus frame we
	# set up above, alter the alloc at
	# address_of_return_alloc to set the
	# correct (current) frame.
.global address_of_address_of_return_alloc#
address_of_address_of_return_alloc#:
	movl r14 = 0x000;;
	mov r16 = 0x3FFFF;;
	and r16 = r6, r16;;
	shl r16 = r16, 18;; # shift past the templateID, too
	ld8 r15 = [r14];;
	or r15 = r15, r16;;
	st8 [r14] = r15;;

.global address_of_call_alloc#
address_of_call_alloc#:
	# Alloc doesn't change the rrb's, so the CFM will be
	# correct after we re-set the frame after the fn call.
	.mii { alloc r16 = ar.pfs, 0, 0, 0, 0; nop.i 0x00; nop.i 0x00;; }

	# We need to make a function call to be able to use br.ret
	# to set the CFM.  It will also preserve all the state
	# necessary to make function calls with impunity.  Huzzah.
	br.call.sptk b0 = preservation_function#;;

.global address_of_return_alloc#
address_of_return_alloc#:
	# The RSE will restore the input registers, but it will
	# set the frame incorrectly.  Restore it here.
	.mii { alloc r16 = ar.pfs, 0, 0, 0, 0; nop.i 0x00; nop.i 0x00;; }

	# restore (in reverse order and method of preservation)
	# the predicate registers, the UNAT, r16, r6, r5, r15, r14,
	# b0, ar.pfs, the UNAT, r4, and f31.

	# r4 is the (address of the) first input register; 
	# we want the last thing stored before that.
	adds r12 = 8, r4;;

	ld8 r16 = [r12], 8;;
	mov pr = r16, 0x1FF;;
	
	ld8 r16 = [r12], 8;;
	mov ar.unat = r16;;

	ld8.fill r16 = [r12], 8;;
	ld8.fill r6 = [r12], 8;;
	ld8.fill r5 = [r12], 8;;
	ld8.fill r15 = [r12], 8;;
	ld8.fill r14 = [r12], 8;;

	ld8 r4 = [r12], 8;;
	mov b0 = r4;;

	ld8 r4 = [r12], 16;;
	mov ar.pfs = r4;;

	ldf.fill f31 = [r12], 16;;
	getf.sig r4 = f31;;

	# Last +16 undoes initial -16 at beginning.
	ldf.fill f31 = [r12],16;;

.global address_of_jump_to_emulation#
address_of_jump_to_emulation#:
	.mii { nop.m 0x00; nop.i 0x00; nop.i 0x00;; }
.endp ia64_tramp_half#

##
#  Code template for handling iRPCs "during" system calls.
##

.global syscallPrefix#
syscallPrefix#:
	# We'll insert jumps into these bundles as appropriate for 
	# the ipsr.ri known at construction time.
	.bbb { nop.b 0x0; nop.b 0x0; nop.b 0x0 };;
	.bbb { nop.b 0x0; nop.b 0x0; nop.b 0x0 };;

.global prefixNotInSyscall#
prefixNotInSyscall#:
	adds sp = -16,sp;;
	stf.spill [sp] = f6, -16;;
	setf.sig f6 = r2;;
	# This is the only place where the prefices differ.
	mov r2 = 0;;
	st8 [sp] = r2;;
	getf.sig r2 = f6;;
	adds sp = 16,sp;;
	ldf.fill f6 = [sp];;
	adds sp = -16,sp;;
.global jumpFromNotInSyscallToPrefixCommon
jumpFromNotInSyscallToPrefixCommon:
	.bbb { nop.b 0x0; nop.b 0x0; nop.b 0x0 }

.global prefixInSyscall#
prefixInSyscall#:
	adds sp = -16,sp;;
	stf.spill [sp] = f6, -16;;
	setf.sig f6 = r2;;
	# This is the only place where the prefices differ.
	mov r2 = 1;;
	st8 [sp] = r2;;
	getf.sig r2 = f6;;
	adds sp = 16,sp;;
	ldf.fill f6 = [sp];;
	adds sp = -16,sp;;

.global prefixCommon#
prefixCommon#:
	# This bundle will not be copied by the installation
	# routine; it's just here to ensure spacing.
	.bbb { nop.b 0x0; nop.b 0x0; nop.b 0x0 };;


.global syscallSuffix#
syscallSuffix#:
	adds sp = 16,sp;;
	stf.spill [sp] = f6, -32;;
	setf.sig f6 = r2;;
	mov r2 = pr;;
	st8 [sp] = r2, 16;;
	ld8 r2 = [sp];;
	cmp.eq p1,p2 = 0, r2;;
	
	adds sp = 16,sp;;
	getf.sig r2 = f6;;
	ldf.fill f6 = [sp], 16;;

.global suffixExitPoint#
suffixExitPoint#:
	# This bundle will be overwritten by a bundle
	# with predicated SIGILLS in the right places.
	.mmi { nop.m 0x0; nop.m 0x0; nop.i 0x0 };;
