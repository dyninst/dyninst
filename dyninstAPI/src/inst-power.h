

#ifndef INST_POWER_H
#define INST_POWER_H

/*
 * $Log: inst-power.h,v $
 * Revision 1.1  1995/08/24 15:03:58  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 *
 */

/*
 * inst-power.h - Common definitions to the POWER specific instrumentation code.
 *
 * inst-power.h,v
 */


#include "ast.h"
#include "as-power.h"

/* "pseudo" instructions that are placed in the tramp code for the inst funcs
 *   to patch up.   This must be invalid instructions (any instruction with
 *   its top 10 bits as 0 is invalid (technically UNIMP).
 *
 */

extern registerSpace *regSpace;

extern trampTemplate baseTemplate;
extern trampTemplate noArgsTemplate;
extern trampTemplate withArgsTemplate;

#endif
