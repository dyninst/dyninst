/* uimpd.C
   this file contains implementation of "uimpd" tcl command.  This command
   is used internally by the UIM.
*/
/* $Log: uimpd.tcl.C,v $
/* Revision 1.1  1994/05/05 19:54:03  karavan
/* initial version.
/* */
 
extern "C" {
  #include "/usr/home/paradyn/packages/tk3.6/src/tk.h"
}
#include "paradyn.h"
#include "UIglobals.h"
extern resourceList *build_resource_list (Tcl_Interp *interp, char *list);

/* arguments:
       0: gotMetrics
       1: msgID
       2: list of selected metrics
       3: number of selected metrics
       4: selected focus
*/
int gotMetricsCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  int numMetrics;
  char **chosenMets;
  resourceList *focus = NULL;
  Tcl_HashEntry *entry;
  UIMReplyRec *msgRec;
  int retVal;
  chooseMandRCBFunc mcb;
  int msgID;

  numMetrics = atoi(argv[3]);
  Tcl_SplitList (interp, argv[2], &numMetrics, &chosenMets);
  focus = build_resource_list (interp, argv[4]);

  // get callback and thread id for this msg
  msgID = atoi(argv[1]);
  if (!(entry = Tcl_FindHashEntry (&UIMMsgReplyTbl, (char *) msgID))) {
    Tcl_AppendResult (interp, "invalid message ID!", (char *) NULL);
    return TCL_ERROR;
  }
  
  msgRec = (UIMReplyRec *) Tcl_GetHashValue(entry);
  Tcl_GetInt (interp, argv[3], &retVal);

     /* set thread id for return */
  uim_server->setTid(msgRec->tid);
  mcb = (chooseMandRCBFunc) msgRec->cb;
  Tcl_DeleteHashEntry (entry);   // cleanup hash table record

     /* send reply */
  uim_server->chosenMetricsandResources(mcb, chosenMets, numMetrics, focus);
  return TCL_OK;
}
  

int compare_visi_names (void *viptr1, void *viptr2) {
  const VM_visiInfo *p1 = (VM_visiInfo *)viptr1;
  const VM_visiInfo *p2 = (VM_visiInfo *)viptr2;
  return strcmp (p1->name, p2->name);
}

int drawStartVisiMenuCmd (ClientData clientData, 
                Tcl_Interp *interp, 
                int argc, 
                char *argv[])
{
  int i;
  VM_visiInfo_Array via;
  int count;
  VM_visiInfo *vinfo;
  Tcl_DString namelist;
  Tcl_DString numlist;
  char num[8];

  via = vmMgr->VMAvailableVisis();
  count = via.count;
  vinfo = via.data;
  
  qsort (vinfo, count, sizeof(VM_visiInfo), compare_visi_names);
  Tcl_DStringInit(&namelist);
  Tcl_DStringInit(&numlist);
  
  for (i = 0; i < count; i++) {
    Tcl_DStringAppendElement(&namelist, vinfo[i].name);
    sprintf (num, "%d", vinfo[i].visiTypeId);
    Tcl_DStringAppendElement(&numlist, num);
  }
  Tcl_SetVar (interp, "vnames", Tcl_DStringValue(&namelist), 0);
  Tcl_SetVar (interp, "vnums", Tcl_DStringValue(&numlist), 0);
  Tcl_DStringFree (&namelist);
  Tcl_DStringFree (&numlist);
  sprintf (num, "%d", count);
  Tcl_SetVar (interp, "vcount", num, 0);
  Tcl_SetVar (interp, "title", argv[1], 0);

  if (Tcl_VarEval (interp, "drawVisiMenu $title $vnames $vnums $vcount",
		   0) == TCL_ERROR) {
    printf ("%s", interp->result);
    return TCL_ERROR;
  }
  return TCL_OK;
}
  
static struct cmdTabEntry uimpd_Cmds[] = {
  {"drawStartVisiMenu", drawStartVisiMenuCmd},
  {"gotMetrics", gotMetricsCmd},
  {NULL, NULL}
};

int UimpdCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  int i;

  if (argc < 2) {
    sprintf(interp->result,"USAGE: %s <cmd>", argv[0]);
    return TCL_ERROR;
  }

  for (i = 0; uimpd_Cmds[i].cmdname; i++) {
    if (strcmp(uimpd_Cmds[i].cmdname,argv[1]) == 0) {
      return ((uimpd_Cmds[i].func)(clientData,interp,argc-1,argv+1));      
      }
  }

  sprintf(interp->result,"unknown UIM cmd '%s'",argv[1]);
  return TCL_ERROR;  
}

