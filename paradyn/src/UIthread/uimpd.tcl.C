/* uimpd.C
   this file contains implementation of "uimpd" tcl command.  This command
   is used internally by the UIM.
*/
/* $Log: uimpd.tcl.C,v $
/* Revision 1.14  1994/11/07 00:34:09  karavan
/* Added default node to root for each subtree if number of user selections
/* is 0 within that subtree.  This change plus elimination of default clearing
/* of the axis on the screen (mets.tcl) implements new selection semantics.
/*
 * Revision 1.13  1994/11/02  23:30:05  karavan
 * added showError command for error access from within tcl code
 *
 * Revision 1.12  1994/11/01  05:44:26  karavan
 * changed resource selection process to support multiple focus selection
 * on a single display
 *
 * Revision 1.11  1994/10/26  23:14:09  tamches
 * Added tclTunable sub-command to command uimpd (see tclTunable.h and .C)
 *
 * Revision 1.10  1994/10/25  17:57:37  karavan
 * added Resource Display Objects, which support display of multiple resource
 * abstractions.
 *
 * Revision 1.9  1994/10/09  02:28:07  karavan
 * Many updates related to the switch to the new UIM/visiThread resource/metric
 * selection interface, and to resource selection directly on the nodes.
 *
 * Revision 1.8  1994/09/25  01:54:14  newhall
 * updated to support changes in VM, and UI interface
 *
 * Revision 1.7  1994/09/24  01:10:00  rbi
 * Added #include of stdlib.h to get correct prototype for atof()
 * and thereby fix the SHG display bug.
 *
 * Revision 1.6  1994/09/22  01:17:26  markc
 * Added const to char* for args in compare function
 *
 * Revision 1.5  1994/09/21  15:35:24  karavan
 * added addNStyle and addEStyle commands
 *
 * Revision 1.4  1994/08/01  20:24:42  karavan
 * new version of dag; new dag support commands
 *
 * Revision 1.3  1994/05/12  23:34:18  hollings
 * made path to paradyn.h relative.
 *
 * Revision 1.2  1994/05/07  23:26:48  karavan
 * added short explanation feature to shg.
 *
 * Revision 1.1  1994/05/05  19:54:03  karavan
 * initial version.
 * */
 
#include <stdlib.h>

extern "C" {
  int atoi(const char*);
}
#include "UIglobals.h"
#include "../pdMain/paradyn.h"
#include "../DMthread/DMresource.h"
#include "../VMthread/metrespair.h"
#include "dag.h"

extern resourceList *build_resource_list (Tcl_Interp *interp, char *list);
extern int getDagToken ();
extern int initWhereAxis (dag *wheredag, stringHandle abs); 
resourceList *uim_SelectedFocus;

void printMFPlist (metrespair *list, int lsize) 
{
  for (int i = 0; i < lsize; i++) {
    printf ("   metric: %s ||| focus: %s\n", 
	    (char *) dataMgr->getMetricName (list[i].met),
	    (char *) (list[i].focus)->getCanonicalName());
  }
}

/* 
   Sends metric-focus pair representation of menu selections to requesting
   visi thread.  Pairs are collected in global list uim_VisiSelections, 
   number of pairs is in uim_VisiSelectionsSize.
   arguments:
       0: sendVisiSelections
       1: msgID
       2: cancelFlag

   note: space allocated for chosenMets and localFocusList must be 
         freed by the visi thread which gets the callback.
*/
int sendVisiSelectionsCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  Tcl_HashEntry *entry;
  UIMReplyRec *msgRec;
  chooseMandRCBFunc mcb;
  int msgID, cancelFlag;

#if UIM_DEBUG
  printf ("sendVisiSelectionsCmd: %s %s\n", argv[1], argv[2]);
#endif

  cancelFlag = atoi(argv[2]);
  if (cancelFlag == 1) {
    uim_VisiSelectionsSize = 0;
  }    
#if UIM_DEBUG
  printf ("processing %d visiselections...\n", uim_VisiSelectionsSize);
  if (uim_VisiSelectionsSize > 0) {
    printMFPlist (uim_VisiSelections, uim_VisiSelectionsSize);
  }
#endif
  
  // get callback and thread id for this msg
  msgID = atoi(argv[1]);
  if (!(entry = Tcl_FindHashEntry (&UIMMsgReplyTbl, (char *) msgID))) {
    Tcl_AppendResult (interp, "invalid message ID!", (char *) NULL);
    return TCL_ERROR;
  }
  msgRec = (UIMReplyRec *) Tcl_GetHashValue(entry);

     /* set thread id for return */
  uim_server->setTid(msgRec->tid);
  mcb = (chooseMandRCBFunc) msgRec->cb;

  uim_server->chosenMetricsandResources(mcb, uim_VisiSelections, 
					uim_VisiSelectionsSize);
  // cleanup data structures
  Tcl_DeleteHashEntry (entry);   // cleanup hash table record
  uim_VisiSelectionsSize = 0;
  free (uim_AvailMets.data);
  return TCL_OK;
}

void printResSelectList(List<resource **>& res, int size) 
{
  List<resource **> tmp;
  resource **thisptr;
  tmp += res;
  
  while (*tmp) {
    thisptr = *tmp;
    for (int i = 0; i < size; i++) {
      printf ("%s ", (char *)(thisptr[i])->getName());
    }
    printf ("\n");
    tmp++;
  }
}

/* parseSelections
 * takes a list with one resourceList per resource hierarchy; result 
 * is the cross product list of valid focii, represented as resourceLists
 */
List<resourceList *> *parseSelections (List<resourceList *>& res, int size)
{
    List<resourceList *> tmpsel, *retList;
    List<resource **> result, resultb, tmpres, tmpres2;
    resource **newentry, **oldentry;
    resourceList *tmp;
    int cnt = 1;

    // parse first resourceList
    tmpsel += res;
    tmp = *tmpsel;
    for (int i = 0; i < tmp->getCount(); i++) {
      if ((newentry = (resource **) calloc (1, sizeof(resource **))) == NULL) {
	printf ("calloc failed in UI\n");
	thr_exit(0);
      }
      newentry[0] = tmp->getNth(i);
      result.add(newentry);
    }

    // parse remaining resourceList's
    tmpsel++;
    cnt++;
    while (*tmpsel) { 
      tmp = *tmpsel;
      tmpres.removeAll();
      tmpres += result;
      while (*tmpres) { // for each partial result on list "result"
	oldentry = *tmpres;
	for (int i = 0; i < tmp->getCount(); i++) { //for each resource
	  if ((newentry = (resource **) calloc (cnt, sizeof(resource **))) 
	      == NULL) {
	    printf ("calloc failed in UI\n");
	    thr_exit(0);
	  }
	  for (int j = 0; j < (cnt - 1); j++) {
	    newentry[j] = oldentry[j];
	  }
	  newentry[cnt-1] = tmp->getNth(i);
	  resultb.add(newentry);
	}
	tmpres++;
      }
      result.removeAll();
      result += resultb;
      resultb.removeAll();
      cnt++;
      tmpsel++;
    }
    cnt--;

#if UIM_DEBUG
    printResSelectList(result, cnt);
#endif
  // result is list of resource * arrays; now 
  // make resourceList's out of these arrays
    retList = new List<resourceList *>;

    tmpres2 += result;
    while (*tmpres2) {
      oldentry = *tmpres2;
      tmp = new resourceList;
      for (int j = 0; j < cnt; j++) {
	tmp->add(oldentry[j]);
      }
      retList->add(tmp);
      tmpres2++;
    }
    return retList;
  }

void getSubtreeSelections (dag *dagptr, rNode curr, resourceList *selection) 
{
  if (dagptr->isHighlighted (curr))
      selection->add ((resource *) curr->aObject);
  for (int i = 0; i < curr->downSize; i++)
    getSubtreeSelections (dagptr, curr->downList[i].dest, selection);
}

/* arguments:
       0: processVisiSelection
       1: rdo token
       2: list of selected metrics
*/
int processVisiSelectionCmd(ClientData clientData, 
			    Tcl_Interp *interp, 
			    int argc, 
			    char *argv[])
{
  int rdoToken;
  dag *currDag;
  resourceList *selection;
  List<resourceList *> allSelections, tmpall, *retList;
  rNode me;
  resourceDisplayObj *currRDO;
  int metcnt, metindx;
  char **metlst;
  int mfpindex, total = 0;

  // get rdo ptr from token
  rdoToken = atoi(argv[1]);
#if UIM_DEBUG
  printf ("processVisiSelection::rdoToken: %d\n", rdoToken);
#endif
  currRDO = resourceDisplayObj::allRDOs.find((void *)rdoToken);
#if UIM_DEBUG
  printf ("processVisiSelection::lookuptoken: %d\n", currRDO->getToken());
#endif

  // get currently displayed dag
  currDag = currRDO->getTopDag();
#if UIM_DEBUG
  printf ("processVisiSelection::lookupDag: %s\n", currDag->getCanvasName());
#endif
  // parse resource selections
  me = currDag->graph->row[0].first;
  while (me != NULL) {
    selection = new resourceList;
    getSubtreeSelections (currDag, me, selection);
    // if no selections highlighted, go back to menu stage
    if (selection->getCount() == 0) {
      /* add this back for auto-clear version
      sprintf (interp->result, "%d", 0);
      return TCL_OK;
      */
      // if nothing selected in entire subtree, use root as default
      selection->add ((resource *) me->aObject); 
    }
    allSelections.add(selection);
    total++;
    me = me->forw;
  }

  retList = parseSelections (allSelections, total);
//  allSelections.removeAll();
  allSelections = *retList;

  if (Tcl_SplitList (interp, argv[2], &metcnt, &metlst) == TCL_OK) {
    metric *currmet;

    // build array of metrespair's
    if ((uim_VisiSelections = new metrespair [metcnt * retList->count()]) 
	== NULL) {
      printf ("malloc error!\n");
      thr_exit(0);
    }
    mfpindex = 0;
    for (int i = 0; i < metcnt; i++) {
      metindx = atoi(metlst[i]);
      currmet = dataMgr->findMetric(context, uim_AvailMets.data[metindx]);
      tmpall += allSelections;
      while (*tmpall) {
	uim_VisiSelections[mfpindex].met = currmet;
	uim_VisiSelections[mfpindex].focus = *tmpall;
	uim_VisiSelectionsSize++;
	tmpall++;
	mfpindex++;
      }
    }
    free (metlst);
  }
//  allSelections.removeAll();
//  tmpall.removeAll();
//  retList->removeAll();
  sprintf (interp->result, "%d", 1);
  return TCL_OK;
}

/*
  closeDAGCmd
  binding set in tcl procedure initWHERE,
  looks up dag * for this display and calls function to close display
  arguments: dagID
*/
int closeDAGCmd (ClientData clientData, 
		   Tcl_Interp *interp, 
		   int argc, 
		   char *argv[])
{
  int dagID;
  dag *currDag;
  dagID = atoi(argv[1]);
  currDag = ActiveDags[dagID];
  currDag->destroyDisplay();
#if UIM_DEBUG
  printf ("dag %d destroyed\n", dagID);
#endif
  return TCL_OK;
}

/*
  addEStyleCmd
  looks up dag * for this display and calls function to add Edge Style
  arguments: dagID, styleID, arrow, fill, capstyle, width
*/
int addEStyleCmd (ClientData clientData, 
		   Tcl_Interp *interp, 
		   int argc, 
		   char *argv[])
{
  int dagID;
  dag *currDag;
  dagID = atoi(argv[1]);
  currDag = ActiveDags[dagID];

  currDag->AddEStyle(atoi(argv[2]), atoi(argv[3]),  0, 0, 0, NULL, 
		     argv[4], argv[5][0], atof(argv[6]));
  return TCL_OK;
}

/*
  addNStyleCmd
  looks up dag * for this display and calls function to add node Style
  arguments: dagID, styleID, bg, outline, font, text, shape, 
             width
*/
int addNStyleCmd (ClientData clientData, 
		   Tcl_Interp *interp, 
		   int argc, 
		   char *argv[])
{
  int dagID;
  dag *currDag;
  dagID = atoi(argv[1]);
#if UIM_DEBUG
  printf ("adding style for dagtoken = %d\n", dagID);
#endif
  currDag = ActiveDags[dagID];
  currDag->AddNStyle(atoi(argv[2]), argv[3], argv[4], NULL,
		     argv[5], argv[6], argv[7][0], atof(argv[8]));
  return TCL_OK;
}

/*
  refineSHGCmd
  binding set in tcl procedure initWHERE,
  looks up dag * for this display and calls function to close display
  arguments: global variable currentSelection$dagID
*/
int refineSHGCmd (ClientData clientData, 
		  Tcl_Interp *interp, 
		  int argc, 
		  char *argv[])
{
  int nodeID;
  nodeID = atoi(argv[1]);
  if (nodeID < 0) {
    sprintf (interp->result, "no selection currently defined\n");
    return TCL_ERROR;
  }
  perfConsult->setCurrentSHGnode (nodeID);
  if (dataMgr->applicationDefined(context) != True) {
    sprintf (interp->result, "no program defined, can't search\n");
    return TCL_ERROR;
  } else {
    perfConsult->search(True, 1);
    return TCL_OK;
  }
}


/*
  hideSubgraphCmd
  binding set in tcl procedure initWHERE,
  looks up dag * for this display and calls function to close display
  arguments: dagID
             currentselection variable name
*/
int hideSubgraphCmd (ClientData clientData, 
		     Tcl_Interp *interp, 
		     int argc, 
		     char *argv[])
{
  int nodeID, dagID;
  char *currNode;
  dag *currDag;
  currNode = Tcl_GetVar (interp, argv[2], TCL_GLOBAL_ONLY);
  if (currNode == NULL)
    return TCL_ERROR;
  nodeID = atoi(currNode);
  if (nodeID < 0) {
    sprintf (interp->result, "no selection currently defined\n");
    return TCL_ERROR;
  }
  dagID = atoi(argv[1]);
  currDag = ActiveDags[dagID];
  currDag->addDisplayOption (SUBTRACT, nodeID);
  return TCL_OK;
}

/*
  showAllNodesCmd
  binding set in tcl procedure initWHERE,
  looks up dag * for this display and calls function to close display
  arguments: dagID
             currentselection variable name
*/
int showAllNodesCmd (ClientData clientData, 
		     Tcl_Interp *interp, 
		     int argc, 
		     char *argv[])
{
  int dagID;
  dag *currDag;
  dagID = atoi(argv[1]);
  currDag = ActiveDags[dagID];
  currDag->clearAllDisplayOptions ();
  return TCL_OK;
}

/*
  shgShortExplain
  called from tcl procedure shgFullName.
  looks up and displays full pathname for node.
  arguments: 0 - cmd name
             1 - nodeID
*/  
int shgShortExplainCmd (ClientData clientData, 
                Tcl_Interp *interp, 
                int argc, 
                char *argv[])
{
  Tcl_HashEntry *entry;
  Tk_Uid nodeID;
  char *nodeExplain;

  // get string for this nodeID
  nodeID = Tk_GetUid (argv[1]);
  if (!(entry = Tcl_FindHashEntry (&shgNamesTbl, nodeID))) {
    Tcl_AppendResult (interp, "invalid message ID!", (char *) NULL);
    return TCL_ERROR;
  }
  nodeExplain = (char *) Tcl_GetHashValue(entry);
    // change variable linked to display window; display window will be 
    //  updated automatically
  Tcl_SetVar (interp, "shgExplainStr", nodeExplain, TCL_GLOBAL_ONLY);
  return TCL_OK;
}

/*
  highlightNodeCmd
  called from tcl procedure updateCurrentSelection.
  looks up and displays full pathname for node.
  arguments: 0 - cmd name
             1 - nodeID
	     2 - dag id
	     3 - selFlag (opt)
*/  
int highlightNodeCmd (ClientData clientData, 
		      Tcl_Interp *interp, 
		      int argc, 
		      char *argv[])
{
  int nodeID, dagID;
  dag *currDag;

  // get string for this nodeID
  nodeID = atoi(argv[1]);
  dagID = atoi (argv[2]);

  currDag = ActiveDags[dagID];
  if (argc > 3) {
    if (currDag->constrHighlightNode (nodeID)) 
      return TCL_OK;    
    else
      return TCL_ERROR;
  }
    
  if (currDag->highlightNode (nodeID)) 
    return TCL_OK;    
  else {
    return TCL_ERROR;
  }
}

/*
  unhighlightNodeCmd
  called from tcl procedure updateCurrentSelection.
  looks up and displays full pathname for node.
  arguments: 0 - cmd name
             1 - nodeID
	     2 - dag id
*/  
int unhighlightNodeCmd (ClientData clientData, 
		      Tcl_Interp *interp, 
		      int argc, 
		      char *argv[])
{
  int nodeID, dagID;
  dag *currDag;

  // get string for this nodeID
  nodeID = atoi(argv[1]);
  dagID = atoi (argv[2]);

  currDag = ActiveDags[dagID];
  if (currDag->unhighlightNode (nodeID)) 
    return TCL_OK;    
  else {
    return TCL_ERROR;
  }
}

int clearResourceSelectionCmd (ClientData clientData, 
                      Tcl_Interp *interp, 
                      int argc, 
                      char *argv[])
{
  int dagID;
  dag *currDag;

  if (!strcmp (argv[1], "rdo")) {
    // need to get dag from rdo
    int rdoID;
    resourceDisplayObj *rdo;
    rdoID = atoi (argv[2]);
    rdo = resourceDisplayObj::allRDOs.find((void *)rdoID);
    currDag = rdo->getTopDag();
  } else {
    // get dag ptr from token
    dagID = atoi (argv[2]);
    currDag = ActiveDags[dagID];
  }
  currDag->clearAllHighlighting();
  currDag->highlightAllRootNodes();
  return TCL_OK;
}
  
/* 
   drawStartVisiMenuCmd
   gets list of currently available visualizations from visi manager
   and displays menu which allows 0 or more selections.
   Tcl command "drawVisiMenu" will return selections to the requesting
   visi thread
*/
int compare_visi_names (void *viptr1, void *viptr2) {
  const VM_visiInfo *p1 = (VM_visiInfo *)viptr1;
  const VM_visiInfo *p2 = (VM_visiInfo *)viptr2;
  return strcmp (p1->name, p2->name);
}


/* 
   drawStartVisiMenuCmd
   gets list of currently available visualizations from visi manager
   and displays menu which allows 0 or more selections.
   Tcl command "drawVisiMenu" will return selections to the requesting
   visi thread
*/
int compare_visi_names (const void *viptr1, const void *viptr2) {
  const VM_visiInfo *p1 = (const VM_visiInfo *)viptr1;
  const VM_visiInfo *p2 = (const VM_visiInfo *)viptr2;
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

  free (via.data);
  if (Tcl_VarEval (interp, "drawVisiMenu $title $vnames $vnums $vcount",
		   0) == TCL_ERROR) {
    printf ("%s", interp->result);
    return TCL_ERROR;
  }
  return TCL_OK;
}

/*
 * argv[1] = rdoToken
 * argv[2] = abstraction
 */
int switchRDOdagCmd (ClientData clientData, 
                Tcl_Interp *interp, 
                int argc, 
                char *argv[])
{
  int rdoToken;
  resourceDisplayObj *trdo;
  rdoToken = atoi(argv[1]);
  trdo = resourceDisplayObj::allRDOs.find((void *)rdoToken);
  trdo->cycle(argv[2]);
  return TCL_OK;
}

/*
 * argv[1] = error number
 * argv[2] = error string
 */
int showErrorCmd (ClientData clientData, 
                Tcl_Interp *interp, 
                int argc, 
                char *argv[])
{
  int code = atoi(argv[1]);
  uim_server->showError (code, argv[2]);
  return TCL_OK;
}


struct cmdTabEntry uimpd_Cmds[] = {
  {"drawStartVisiMenu", drawStartVisiMenuCmd},
  {"sendVisiSelections", sendVisiSelectionsCmd},
  {"shgShortExplain", shgShortExplainCmd},
  {"closeDAG", closeDAGCmd}, 
  {"highlightNode", highlightNodeCmd},
  {"unhighlightNode", unhighlightNodeCmd},
  {"refineSHG", refineSHGCmd},
  {"hideSubgraph", hideSubgraphCmd},
  {"showAllNodes", showAllNodesCmd},
  {"addEStyle", addEStyleCmd},
  {"addNStyle", addNStyleCmd},
  {"processVisiSelection", processVisiSelectionCmd},
  {"clearResourceSelection", clearResourceSelectionCmd},
  {"switchRDOdag", switchRDOdagCmd},
  {"tclTunable", TclTunableCommand},
  {"showError", showErrorCmd},
  { NULL, NULL}
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

