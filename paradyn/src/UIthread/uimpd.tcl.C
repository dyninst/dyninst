/* uimpd.C
   this file contains implementation of "uimpd" tcl command.  This command
   is used internally by the UIM.
*/
/* $Log: uimpd.tcl.C,v $
/* Revision 1.15  1995/06/02 20:50:44  newhall
/* made code compatable with new DM interface
/*
 * Revision 1.14  1994/11/07  00:34:09  karavan
 * Added default node to root for each subtree if number of user selections
 * is 0 within that subtree.  This change plus elimination of default clearing
 * of the axis on the screen (mets.tcl) implements new selection semantics.
 *
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
#include "../DMthread/DMinclude.h"
#include "dag.h"

extern int getDagToken ();
extern int initWhereAxis (dag *wheredag, stringHandle abs); 

void printMFPlist (vector<metric_focus_pair> *list) 
{
  for (int i = 0; i < list->size(); i++) {
    cout << "   metric: " <<  
      dataMgr->getMetricName (((*list)[i]).met) << "||| focus: ";
    for (int j = 0; j < ((*list)[i]).res.size(); j++)
      cout << dataMgr->getResourceName (((*list)[i]).res[j]);
    cout << endl;
  }
}

/* 
   Sends metric-focus pair representation of menu selections to requesting
   visi thread.  Pairs are collected in global list uim_VisiSelections, 
   number of pairs is in uim_VisiSelectionsSize.
   arguments:
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
  int msgID;

#if UIM_DEBUG
  printf ("sendVisiSelectionsCmd: %s %s\n", argv[1], argv[2]);
#endif

#ifdef n_def
  cancelFlag = atoi(argv[2]);
  if (cancelFlag == 1) {
    uim_VisiSelections->resize(0);
  }    
#endif
#if UIM_DEBUG
  cout << "processing " << uim_VisiSelections.size() << " visiselections...\n";
  if (uim_VisiSelections->size() > 0) {
    printMFPlist (uim_VisiSelections);
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

  // if cancel was selected invoke callback with null list
  int cancelFlag = atoi(argv[2]);
  if (cancelFlag == 1) {
    uim_server->chosenMetricsandResources(mcb, 0);
  }    
  else {
#ifdef n_def      
      printf("uim_VisiSelections.size() = %d\n",uim_VisiSelections->size());
      for(unsigned l = 0; l < uim_VisiSelections->size(); l++){
          printf("metric %d: %d\n",l,(*uim_VisiSelections)[l].met);
          for(unsigned blah =0; blah < (*uim_VisiSelections)[l].res.size(); 
	      blah++){
	      printf("resource %d:%d:  %d\n",
			l,blah,(*uim_VisiSelections)[l].res[blah]);
          }
      }
#endif
      uim_server->chosenMetricsandResources(mcb, uim_VisiSelections);
  }

  // cleanup data structures
  Tcl_DeleteHashEntry (entry);   // cleanup hash table record
  delete uim_AvailMets;;
  delete uim_AvailMetHandles;
  return TCL_OK;
}

void printResSelectList (vector<numlist> *v, char *name)
{
  cout << "[Focus List " << name << "|" << v->size() << "] "; 
  for (int i = 0; i < v->size(); i++) {
    numlist temp = (*v)[i];
    cout << "                    ";
    for (int j = 0; j < temp.size(); j++) 
      cout << temp[j] << " ";
    cout << endl;
  }
  cout << endl;
}

#ifdef n_def
/* parseSelections
 * takes a list with one resourceHandle vector per resource hierarchy; result 
 * is the cross product list of valid focii, represented as vectors of 
 * nodeID vectors; each element on resulting list can be converted into 
 * one focus.
 */
vector<numlist> *
parseSelections (vector<numlist> *bytree) {

  vector<numlist> *result2;

  int totsize = 1;
  for (int m = 0; m < bytree->size(); m++) {
    totsize = totsize * (*bytree)[m].size();
  }
  result2 = new vector<numlist> (totsize);
  int y;
  int mul = 1;


  for (int v = 0; v < bytree->size(); v++) {
    vector<unsigned> *tmp = &(*bytree)[v];

    for (int z = 0; z < tmp->size(); z++) {
      y = z * mul;
      while (y < totsize) {
	int u = y;
	while ((u < totsize) && (u < (y + mul))) {
	  (*result2)[u] += (*tmp)[z];
	  u++;
	}
	y = y + (mul * tmp->size());
      }
    }

    mul = mul * 2;
  }
  /*
  for(unsigned i = 0; i < result2->size(); i++){
      printf("focus %d:\n",i);
      for(unsigned j = 0; j < (*result2)[i].size(); j++){
	  printf("     part %d:%d\n",j,(*result2)[i][j]);
      }
  }
  */
  return result2;
}
#endif


/* parseSelections
 * takes a list with one resourceHandle vector per resource hierarchy; result 
 * is the cross product list of valid focii, represented as vectors of 
 * nodeID vectors; each element on resulting list can be converted into 
 * one focus.
 */
vector<numlist> *
parseSelections (vector<numlist> *bytree) {

  // figure out size of result vector 
  unsigned totsize = 1;
  for(unsigned i=0; i < bytree->size(); i++){
    totsize = totsize * (*bytree)[i].size();
  }
  vector<numlist> *result = new vector<numlist> (totsize);

  // create the cross product of all elements in bytree
  unsigned iterations = 1;
  for(i=0; i < bytree->size(); i++){
      unsigned element_size = (*bytree)[i].size();
      unsigned r_index = 0;
      totsize = totsize / element_size; 

      // distribute the elements of the ith list over the results vector
      for(unsigned j=0; j < iterations; j++){
          for(unsigned k = 0; k < element_size; k++){
              for(unsigned m = 0; m < totsize; m++){
                  (*result)[r_index] += (*bytree)[i][k];
	          r_index++;
              }
	  }
      }
      iterations = iterations*element_size;
  }

  /*
  for(i = 0; i < result->size(); i++){
      printf("focus %d:\n",i);
      for(unsigned j = 0; j < (*result)[i].size(); j++){
	  printf("     part %d:%d\n",j,(*result)[i][j]);
      }
  }
  */
  return result;
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
  resourceDisplayObj *currRDO;
  int metcnt, metindx;
  char **metlst;
  metric_focus_pair *currpair;

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

    vector<numlist> *allsels = currDag->listAllHighlightedNodes ();

#if UIM_DEBUG
  cout << allsels->size() << endl;
  for (int i = 0; i < allsels->size(); i++) {
    cout << "resHierarchy " << i << " Selections: ";
    vector<nodeIdType> tmp = (*allsels)[i];
    for (int j = 0; j < tmp.size(); j++) {
      cout << " " << tmp[j];
    }
  }
  cout << endl;
#endif

  vector<numlist> *retList = parseSelections (allsels);

#if UIM_DEBUG
  printResSelectList(retList, "list of selected focii");
#endif

//** and, list of metric indices from selections put into metlst

  if (Tcl_SplitList (interp, argv[2], &metcnt, &metlst) == TCL_OK) {
    metricHandle currmet;
    
    uim_VisiSelections = new vector<metric_focus_pair>;
    for (unsigned i = 0; i < metcnt; i++) {
      metindx = atoi(metlst[i]);
      currmet = uim_AvailMetHandles[metindx];
      for (unsigned j = 0; j < retList->size(); j++) {
	currpair = new metric_focus_pair;
	currpair->met = currmet;
	currpair->res = (*retList)[j];
	*uim_VisiSelections += *currpair;
      }
    }
    free (metlst);   // cleanup after Tcl_SplitList
    delete allsels;
    delete retList;
  }

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
  nodeIdType nodeID;
  nodeID = StrToNodeIdType(argv[1]);
/** need error handling for new type
  if (nodeID < 0) {
    sprintf (interp->result, "no selection currently defined\n");
    return TCL_ERROR;
  }
*/
//** note: need to change PC interface to nodeIdType
  perfConsult->setCurrentSHGnode ((int)nodeID);
  if (dataMgr->applicationDefined() != True) {
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
  int dagID;
  nodeIdType nodeID;
  char *currNode;
  dag *currDag;
  currNode = Tcl_GetVar (interp, argv[2], TCL_GLOBAL_ONLY);
  if (currNode == NULL)
    return TCL_ERROR;
  nodeID = StrToNodeIdType (currNode);
/** need error handling for new type
  if (nodeID < 0) {
    sprintf (interp->result, "no selection currently defined\n");
    return TCL_ERROR;
  }
*/
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
  int dagID;
  nodeIdType nodeID;
  dag *currDag;

  // get string for this nodeID
  nodeID = StrToNodeIdType(argv[1]);
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
  int dagID;
  nodeIdType nodeID;
  dag *currDag;

  // get string for this nodeID
  nodeID = StrToNodeIdType(argv[1]);
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
int compare_visi_names (const void *viptr1, const void *viptr2) {
  const VM_visiInfo *p1 = (const VM_visiInfo *)viptr1;
  const VM_visiInfo *p2 = (const VM_visiInfo *)viptr2;
  return strcmp (p1->name.string_of(), p2->name.string_of());
}

int drawStartVisiMenuCmd (ClientData clientData, 
                Tcl_Interp *interp, 
                int argc, 
                char *argv[])
{
  int i;
  vector<VM_visiInfo> *via;
  int count;
  Tcl_DString namelist;
  Tcl_DString numlist;
  char num[8];

  via = vmMgr->VMAvailableVisis();
  count = via->size();
  
  via->sort (compare_visi_names);
  Tcl_DStringInit(&namelist);
  Tcl_DStringInit(&numlist);
  
  for (i = 0; i < count; i++) {
    Tcl_DStringAppendElement(&namelist, ((*via)[i]).name.string_of());
    sprintf (num, "%d", ((*via)[i]).visiTypeId);
    Tcl_DStringAppendElement(&numlist, num);
  }
  Tcl_SetVar (interp, "vnames", Tcl_DStringValue(&namelist), 0);
  Tcl_SetVar (interp, "vnums", Tcl_DStringValue(&numlist), 0);
  Tcl_DStringFree (&namelist);
  Tcl_DStringFree (&numlist);
  sprintf (num, "%d", count);
  Tcl_SetVar (interp, "vcount", num, 0);
  Tcl_SetVar (interp, "title", argv[1], 0);

  delete via;
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

