
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
