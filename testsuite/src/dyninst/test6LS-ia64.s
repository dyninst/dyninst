.file	"test6LS.s"
.text
.align 16

	.global loadsnstores#
	.proc loadsnstores#
loadsnstores:
	alloc r32 = ar.pfs, 4, 0, 0, 0;;
	mov r2 = r12;;
	adds r12 = -16, r12;;

	adds r14 = -16, r2;;
	st8 [r14] = r32;;
	
	adds r14 = -8, r2;;
	st8 [r14] = r33;; 
	
	mov r14 = r2;;
	st8 [r14] = r34;;
	
	adds r16 = -16, r2;;
	ld8 r32 = [r16];;
	
	adds r14 = -8, r2;;
	ld8 r33 = [r14];;
	
	mov r15 = r2;;
	ld8 r34 = [r15];;
		
	adds r14 = 32, r2;;
	{ ldfps f10,f11 = [r14];; }
	{ ldfpd f10,f11 = [r14];; }
	{ ldfp8 f10,f11 = [r14];; }
	
	{ lfetch [r14];; }
	{ lfetch [r14], r0;; }
	{ lfetch [r14], 16;; }
	
	;;
	mov r8 = r0;
	mov r12 = r2
	br.ret.sptk.many b0
	;;
	.endp loadsnstores#
