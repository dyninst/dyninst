/* $Log: UIglobals.h,v $
/* Revision 1.2  1994/04/06 17:39:00  karavan
/* changed interp to global
/*
 * Revision 1.1  1994/04/05  04:42:34  karavan
 * initial version of UI thread code and tcl paradyn command
 * */

/* UIglobals.h 
     definitions used by UI thread */

#ifndef _ui_globals_h
#define _ui_globals_h

#include "dataManager.CLNT.h"
#include "performanceConsultant.CLNT.h"
#include "UI.SRVR.h"
extern "C" {
 #include "tk.h"
}
#define UIMBUFFSIZE 256


extern resource                  *uim_rootRes;
extern int                       uim_eid;
extern List<metricInstance*>     uim_enabled;
extern performanceStream         *uim_defaultStream;
extern UIM                       *uim_server;

extern Tcl_Interp *interp;

#endif
