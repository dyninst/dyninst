/* $Log: UIglobals.h,v $
/* Revision 1.11  1994/11/03 06:16:12  karavan
/* status display and where axis added to main window and the look cleaned
/* up a little bit.  Added option to ResourceDisplayObj class to specify
/* a parent window for an RDO with the constructor.
/*
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

#define UIM_DEBUG 1

#include "dataManager.CLNT.h"
#include "performanceConsultant.CLNT.h"
#include "UI.SRVR.h"
#include "thread/h/thread.h"
extern "C" {
 #include "tk.h"
}

#define UIMBUFFSIZE 256
#define MAXNUMACTIVEDAGS 20

#define DISPLAYED 0
#define ICONIFIED 1
#define INACTIVE 2

// typedef Boolean char;
class dag;

class resourceDisplayObj {
 public:
  int getStatus () {return status;}
  char *getParentWindow () {return parentwin;}
  int getToken () {return token;}
  int getSize () {return numdags;}
  dag *getTopDag() {return topdag;}
  resourceDisplayObj (int baseflag, int &success);
  resourceDisplayObj (int baseflag, int &success, char *pwin);
  resourceDisplayObj copy (char *pwin);
  void addResource (resource *newres, resource *parent, char *name, 
		    stringHandle abs);
  dag *addAbstraction (stringHandle newabs);
  int cycle (char *abs);
  void print ();
  friend void resourceAddedCB (performanceStream *ps , resource *parent, 
			       resource *newResource, char *name);
  friend int switchRDOdagCmd (ClientData clientData, Tcl_Interp *interp, 
                int argc, char *argv[]);
  friend int processVisiSelectionCmd(ClientData clientData, Tcl_Interp *interp, 
			    int argc, char *argv[]);
  friend int clearResourceSelectionCmd (ClientData clientData, 
                      Tcl_Interp *interp, int argc, char *argv[]);
  friend void UIM::chooseMetricsandResources(chooseMandRCBFunc cb,
		      metrespair *pairList, int numElements);
 private: 
  int token;
  dag *topdag;
  int numdags;
  char parentwin[15];
  int status;
  int base;
  List<dag *> dags;
  static List<resourceDisplayObj *> allRDOs;
  static int rdoCount;
  char tbuf[300];
};

struct cmdTabEntry 
{
  const char *cmdname;
  int (*func)(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);
};

typedef struct UIMReplyRec {
  void *cb;
  thread_t tid;
} UIMReplyRec;

typedef struct tokenRec {
  int token;
  void *object;
} tokenRec;

class tokenHandler {
 public:
  int getToken(void *obj);
  int reportToken (void *obj);
  tokenRec *translateToken (int token);
  Boolean invalidate (int token);
  tokenHandler () {counter = 1;}
  int getCount () {return counter;}
 private:
  int counter;
  List<tokenRec *> store;
};

// used by paradyn enable command
extern int                       uim_eid;

extern List<metricInstance*>     uim_enabled;
extern performanceStream         *uim_defaultStream;
extern UIM                       *uim_server;

// callback pointers stored here for lookup after return from tcl/tk routines
//  int tokens are used within tcl/tk code since we can't use pointers
extern Tcl_HashTable UIMMsgReplyTbl;
extern int UIMMsgTokenID;
//extern tokenHandler tokenClerk; 
extern Tcl_HashTable shgNamesTbl;
extern char *SHGwinName;

// this tcl interpreter used for entire UI
extern Tcl_Interp *interp;   

// set for batch mode; clear for normal
extern int UIM_BatchMode;    

// value of highest valid error index
extern int uim_maxError;     

// every currently defined dag listed here for lookups by tcl/tk routines
extern dag *ActiveDags[MAXNUMACTIVEDAGS];

// where axes display
extern resource                  *uim_rootRes;
extern dag *baseWhere;  /*** get rid of this from uimpd, UImain,UIpublic */
extern List<stringHandle> uim_knownAbstractions;

// metric-resource selection 
extern int uim_ResourceSelectionStatus;
extern List<resourceList *> uim_CurrentResourceSelections;
extern metrespair *uim_VisiSelections;
extern int uim_VisiSelectionsSize;
extern String_Array uim_AvailMets;
extern resourceList *uim_SelectedFocus;

int TclTunableCommand(ClientData cd, Tcl_Interp *interp,
                      int argc, char **argv);

#endif
