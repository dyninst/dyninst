/*
 * trampoline code to get from a code location to an inst. primative.
 *
 *    This code starts life in the controller process and moves into the
 *    appropriate inferior process via ptrace calls.
 *
 * tramp-sparc.s,v
 *
 */
#include "as-power.h"

/*
 * This is the base where a tramp jumps off.
 *
 * - do global before local because global call DYNINSTinit.
 *
 */
.csect .data[RW]
	.globl baseTramp
	.globl _baseTramp
baseTramp:
_baseTramp:
	/* should update cost of base tramp here, but we don't have a
	   register to use!
	*/
	.long	GLOBAL_PRE_BRANCH
	.long	LOCAL_PRE_BRANCH
	.long 	EMULATE_INSN
	.long	GLOBAL_POST_BRANCH
	.long	LOCAL_POST_BRANCH
	/* should update post insn cost of base tramp here */
	.long	RETURN_INSN
	.long	END_TRAMP
