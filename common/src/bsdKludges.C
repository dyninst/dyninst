
#include "util/h/headers.h"

typedef int (*intKludge)();

void   P_xdr_destroy(XDR *x) { XDR_DESTROY(x);}
void P_xdrrec_create(XDR *x, const u_int send_sz, const u_int rec_sz,
		     const caddr_t handle, 
		     xdr_rd_func read_r, xdr_wr_func write_f) {
  xdrrec_create(x, send_sz, rec_sz, handle, (intKludge) read_r,
		(intKludge) write_f);} 
bool_t P_xdrrec_endofrecord(XDR *x, int now) {
  return (xdrrec_endofrecord(x, now));}
bool_t P_xdrrec_skiprecord(XDR *x) { return (xdrrec_skiprecord(x));}
