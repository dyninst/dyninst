/* $Log: UIglobals.h,v $
/* Revision 1.22  1996/01/23 06:53:56  tamches
/* uim_VisiSelections is no longer a ptr
/*
 * Revision 1.21  1995/11/09 02:10:00  tamches
 * removed some obsolete references (e.g. uim_eid, uim_rootRes)
 *
 * Revision 1.20  1995/11/06 02:44:43  tamches
 * removed nodeIdType and numlist (no longer used typedefs)
 *
 * Revision 1.19  1995/10/17 20:43:50  tamches
 * Commented out StrToNodeIdType, dag, and shgDisplay -- things obsoleted
 * by the new search history graph
 *
 * Revision 1.18  1995/10/05 04:30:13  karavan
 * added ActiveDags to dag class.
 * removed SHG_DAGID global (not used).
 * removed SHGwinName global (obsoleted).
 * removed tokenRec struc and tokenHandler class (obsoleted).
 * removed commented obsolete code.
 *
 * Revision 1.17  1995/07/24  21:29:43  tamches
 * removed or commented out resourceDisplayObj, baseWhere, and
 * uim_knownAbstractions, which are all things related to the old where axis.
 *
 * Revision 1.16  1995/06/02  20:50:34  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.15  1995/02/16  08:20:46  markc
 * Changed Boolean to bool
 * Changed wait loop code for igen messages
 *
 * Revision 1.14  1995/01/26  17:58:53  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.13  1994/11/03  19:54:40  karavan
 * changed rDO constructor argument to const char * to eliminate warning
 *
 * Revision 1.12  1994/11/03  06:21:31  karavan
 * oops!  changed debug flag back to 0
 *
 * Revision 1.11  1994/11/03  06:16:12  karavan
 * status display and where axis added to main window and the look cleaned
 * up a little bit.  Added option to ResourceDisplayObj class to specify
 * a parent window for an RDO with the constructor.
 *
 * Revision 1.10  1994/11/01  05:44:27  karavan
 * changed resource selection process to support multiple focus selection
 * on a single display
 *
 * Revision 1.9  1994/10/25  17:57:31  karavan
 * added Resource Display Objects, which support display of multiple resource
 * abstractions.
 *
 * Revision 1.8  1994/10/09  01:24:45  karavan
 * A large number of changes related to the new UIM/visiThread metric&resource
 * selection interface and also to direct selection of resources on the
 * Where axis.
 *
 * Revision 1.7  1994/08/01  20:24:37  karavan
 * new version of dag; new dag support commands
 *
 * Revision 1.6  1994/07/07  15:54:49  jcargill
 * Commit for Karen; added extern defns for UIM_BatchMode & uim_maxError
 *
 * Revision 1.5  1994/05/07  23:26:29  karavan
 * added short explanation feature to SHG.
 *
 * Revision 1.4  1994/05/05  19:53:28  karavan
 * added structure defn
 *
 * Revision 1.3  1994/05/05  02:13:28  karavan
 * moved CmdTabEntry definition from paradyn.tcl.C to UIglobals.h
 *
 * Revision 1.2  1994/04/06  17:39:00  karavan
 * changed interp to global
 *
 * Revision 1.1  1994/04/05  04:42:34  karavan
 * initial version of UI thread code and tcl paradyn command
 * */

/* UIglobals.h 
     definitions used by UI thread */

#ifndef _ui_globals_h
#define _ui_globals_h

#define UIM_DEBUG 0

#include "dataManager.thread.CLNT.h"
#include "performanceConsultant.thread.CLNT.h"
#include "UI.thread.SRVR.h"
#include "thread/h/thread.h"
#include "paradyn/src/DMthread/DMinclude.h"

#include "tkclean.h"

struct cmdTabEntry 
{
  const char *cmdname;
  int (*func)(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);
};

typedef struct UIMReplyRec {
  void *cb;
  thread_t tid;
} UIMReplyRec;

extern List<metricInstInfo *>    uim_enabled;
extern perfStreamHandle          uim_ps_handle;
extern UIM                       *uim_server;

// callback pointers stored here for lookup after return from tcl/tk routines
//  int tokens are used within tcl/tk code since we can't use pointers
extern Tcl_HashTable UIMMsgReplyTbl;
extern int UIMMsgTokenID;

// this tcl interpreter used for entire UI
extern Tcl_Interp *interp;   

// set for batch mode; clear for normal
extern int UIM_BatchMode;    

#include "Status.h"
extern status_line *ui_status;

// value of highest valid error index
extern int uim_maxError;     

//// metric-resource selection 
extern vector<metric_focus_pair> uim_VisiSelections;
extern char **uim_AvailMets;
extern int uim_AvailMetsSize;
extern metricHandle *uim_AvailMetHandles;

int TclTunableCommand(ClientData cd, Tcl_Interp *interp,
                      int argc, char **argv);

#endif
