/*
 * trampoline code to get from a code location to an inst. primative.
 *
 *    This code starts life in the controller process and moves into the
 *    appropriate inferior process via ptrace calls.
 *
 * $Log: tramp-sparc.s,v $
 * Revision 1.10  1996/06/20 21:35:07  naim
 * Adding a nop at the delay slot to avoid "illegal instruction" bug - naim
 *
# Revision 1.9  1995/08/24  15:04:45  hollings
# AIX/SP-2 port (including option for split instruction/data heaps)
# Tracing of rexec (correctly spawns a paradynd if needed)
# Added rtinst function to read getrusage stats (can now be used in metrics)
# Critical Path
# Improved Error reporting in MDL sematic checks
# Fixed MDL Function call statement
# Fixed bugs in TK usage (strings passed where UID expected)
#
# Revision 1.8  1994/11/02  19:01:26  hollings
# Made the observed cost model use a normal variable rather than a reserved
# register.
#
# Revision 1.7  1994/11/02  11:18:32  markc
# Commented out the cost model.
#
# Revision 1.6  1994/10/13  07:25:08  krisna
# solaris porting and updates
#
# Revision 1.5  1994/07/26  19:58:03  hollings
# removed slots executed counter.
#
# Revision 1.4  1994/07/14  23:30:33  hollings
# Hybrid cost model added.
#
# Revision 1.3  1994/07/06  00:35:45  hollings
# Added code to handle SPARC ABI aggregate return type calling convention
# of using the instruction after the call's delay slot to indicate aggregate
# size.  We treat this as an extra delay slot and relocate it to the
# base tramp as needed.
#
# Revision 1.2  1994/07/05  03:26:20  hollings
# observed cost model
#
# Revision 1.1  1994/01/27  20:31:47  hollings
# Iinital version of paradynd speaking dynRPC igend protocol.
#
 * Revision 1.6  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.5  1993/09/10  20:33:44  hollings
 * moved tramps to data area so the dyninst code can be run under qpt.
 *
 * Revision 1.4  1993/08/11  01:36:10  hollings
 * fixed include files.
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/03/19  22:52:54  hollings
 * fixed comment character.
 *
 * Revision 1.1  1993/03/19  22:51:50  hollings
 * Initial revision
 *
 */
#include "as-sparc.h"

/*
 * This is the base where a tramp jumps off.
 *
 * - do global before local because global call DYNINSTinit.
 *
 */
.data
	.global baseTramp
	.global	_baseTramp
baseTramp:
_baseTramp:
	/* should update cost of base tramp here, but we don't have a
	   register to use!
	*/
	.word	GLOBAL_PRE_BRANCH
	.word	LOCAL_PRE_BRANCH
	nop
	.word 	EMULATE_INSN
	nop			/* delay slot */
	nop			/* extra nop for aggregate size */
	.word	GLOBAL_POST_BRANCH
	.word	LOCAL_POST_BRANCH
	/* should update post insn cost of base tramp here */
	.word	RETURN_INSN
	nop			/* see if this prevents crash jkh 4/4/95 */
	.word	END_TRAMP
