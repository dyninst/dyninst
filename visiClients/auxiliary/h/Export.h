#ifndef _export_h
#define _export_h

#if defined(__cplusplus)
extern "C" {
#endif

extern int DoExport(ClientData, Tcl_Interp *, int, Tcl_Obj *CONST*);
extern int get_subscribed_mrpairs(ClientData, Tcl_Interp *, int, Tcl_Obj *CONST*);

#if defined(__cplusplus)
};
#endif /* defined(__cplusplus) */

#endif
