#if !defined(hppa1_1_hp_hpux)
#error "invalid architecture-os inclusion"
#endif /* !defined(hppa1_1_hp_hpux) */


#if !defined(_inst_hppa_h_)
#define _inst_hppa_h_


#include "ast.h"
#include "as-hppa.h"

/* "pseudo" instructions that are placed in the tramp code for the inst funcs
 *   to patch up.   This must be invalid instructions.
 */

extern registerSpace *regSpace;

extern trampTemplate baseTemplate;
extern trampTemplate noArgsTemplate;
extern trampTemplate withArgsTemplate;

#define REG_MT                3   /* register saved to keep the address of */
                                  /* the current vector of counter/timers  */
                                  /* for each thread.                      */
#define NUM_INSN_MT_PREAMBLE  9   /* number of instructions required for   */
                                  /* the MT preamble.                      */ 

#endif /* !defined(hppa1_1_hp_hpux) */
