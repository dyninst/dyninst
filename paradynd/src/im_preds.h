
/*
 * $Log: im_preds.h,v $
 * Revision 1.2  1995/08/24 15:03:49  hollings
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
#ifndef _IM_PREDS_HDR
#define _IM_PREDS_HDR

// TODO -- a quick fix
// For internal metrics -- these have not needed refinement below the top level
// If that is needed, this needs to be modified
enum im_pred_type { pred_null, pred_invalid };

typedef struct im_pred_struct {
  im_pred_type machine;
  im_pred_type procedure;
  im_pred_type process;
  im_pred_type sync;
} im_pred_struct;

#endif
