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

#endif /* !defined(hppa1_1_hp_hpux) */
