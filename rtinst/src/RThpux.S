
        .SPACE $PRIVATE$
	        .SUBSPA $DATA$,QUAD=1,ALIGN=8,ACCESS=31
	        .SUBSPA $BSS$,QUAD=1,ALIGN=8,ACCESS=31,ZERO,SORT=82
	        .SPACE $TEXT$
	        .SUBSPA $LIT$,QUAD=0,ALIGN=8,ACCESS=44
	        .SUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	        .IMPORT $global$,DATA
	        .IMPORT $$dyncall,MILLICODE
	;;  gcc_compiled.:
	        .SPACE $TEXT$
	        .SUBSPA $CODE$
	        .align 4
	        .EXPORT baseCall,ENTRY,PRIV_LEV=3,ARGW0=GR,ARGW1=GR,RTNVAL=GR
baseCall
_baseCall		  
		ldw -36(%r30), %r26
		ldw -40(%r30), %r25
		ldw -44(%r30), %r24
		ldw -48(%r30), %r23 
		copy %r30,%r3
		stwm %r3,128(%sr0,%r30)
		stw %r26, -36(%r30)
		stw %r25, -40(%r30)
		stw %r24, -44(%r30)
		stw %r23, -48(%r30)
		ldw -180(%r30), %r2
		stw %r2, -52(%r30)
		ldw -184(%r30), %r2 
		stw %r2, -56(%r30)	
		ldw -188(%r30), %r2
		stw %r2, -60(%r30)
		ldw -192(%r30), %r2
		stw %r2, -64(%r30)
		stw %r31,-124(%sr0,%r30)
		ble 0(%sr4,%r28)
		copy %r31,%r2
		nop
		ldw -124(%sr0,%r30),%r31
		ldo 64(%r3),%r30
		ldwm -64(%sr0,%r30),%r3
		be 0(%sr5,%r31)
		nop
                .align 4
                .EXPORT miniCall,ENTRY,PRIV_LEV=3,ARGW0=GR,ARGW1=GR,RTNVAL=GR
miniCall
_miniCall
                copy %r30,%r3
                stwm %r3,128(%sr0,%r30)
		stw %r31,-124(%sr0,%r30)
                ble 0(%sr4,%r28)
                copy %r31,%r2
                nop
                ldw -124(%sr0,%r30),%r31
                ldo 64(%r3),%r30
                ldwm -64(%sr0,%r30),%r3
                be 0(%sr5,%r31)
                nop


