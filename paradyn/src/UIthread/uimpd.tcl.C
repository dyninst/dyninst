/* uimpd.C
   this file contains implementation of "uimpd" tcl command.  This command
   is used internally by the UIM.
*/
/* $Log: uimpd.tcl.C,v $
/* Revision 1.34  1996/05/07 18:06:00  newhall
/* added threadExiting routine
/*
 * Revision 1.33  1996/04/01  22:31:47  tamches
 * refs to uim_AvailMets etc. gone
 * added UI_all_metric_names, UI_all_metrics_set_yet
 *
 * Revision 1.32  1996/02/07 18:50:35  tamches
 * added uimpd_startPhaseCmd
 * made copy of uim_visiSelections before calling chosenMetricsAndResources
 *
 * Revision 1.31  1996/02/02 18:55:19  tamches
 * removed a lot of obsolete code that had been commented out
 *
 * Revision 1.30  1996/02/02 01:01:36  karavan
 * Changes to support the new PC/UI interface
 *
 * Revision 1.29  1996/02/01 01:39:23  tamches
 * oops, fixed a minor vrble name misspelling
 *
 * Revision 1.28  1996/02/01 01:35:33  tamches
 * suppressed duplicate of "Whole Program" which was sometimes being generated
 *
 * Revision 1.27  1996/01/30 23:04:01  tamches
 * removed include to obsolete file shgDisplay.h
 *
 * Revision 1.26  1996/01/23 07:11:25  tamches
 * uim_VisiSelections no longer a ptr
 *
 * Revision 1.25  1996/01/11 04:43:04  tamches
 * replaced parseSelections - memory leaks gone, runs more efficiently,
 * and doesn't add spurious metric/focus pairs when Whole Program is chosen
 * Changed processVisiSelection to use the new parseSelections and to reduce
 * memory leaks
 *
 * Revision 1.24  1995/11/08 06:26:11  tamches
 * removed some warnings
 *
 * Revision 1.23  1995/11/06 02:35:05  tamches
 * removed some warnings w/g++2.7
 *
 * Revision 1.22  1995/10/17 22:12:24  tamches
 * commented out closeDAGCmd, addEStyleCmd, addNStyleCmd,
 * refineSHGCmd, hideSubgraphCmd, showAllNodesCmd, shgShortExplainCmd,
 * highlightNodeCmd, and unhighlightNodeCmd.  These things were either
 * obsoleted for the new shg or (as in the case of refine) haven't
 * been implemented yet.
 *
 * Revision 1.21  1995/10/13 22:08:07  newhall
 * added call to dataManager::getResourceLabelName
 *
 * Revision 1.20  1995/10/05  04:35:06  karavan
 * changes to support search display changes including igen interfaces and
 * paradyn search and paradyn shg commands.
 * removed obsolete and commented code.
 *
 * Revision 1.19  1995/09/08  19:51:16  krisna
 * stupid way to avoid the for-scope problem
 *
 * Revision 1.18  1995/08/01 02:18:33  newhall
 * changes to support phase interface
 *
 * Revision 1.17  1995/07/17 05:09:06  tamches
 * Many changes related to the new where axis.  Some code was
 * no longer needed and hence commented out.  Other code unrelated
 * to the where axis was left alone.  But nothing much was added to
 * this file
 *
 * Revision 1.16  1995/06/11  22:59:47  karavan
 * changed error handling for new node type.
 *
 * Revision 1.15  1995/06/02  20:50:44  newhall
 * made code compatable with new DM interface
 *
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
#include "tcl.h"
#include "tk.h"
#include "util/h/odometer.h"

extern "C" {
  int atoi(const char*);
}
#include "UIglobals.h"
#include "../pdMain/paradyn.h"
#include "../DMthread/DMinclude.h"
#include "abstractions.h"

void printMFPlist (vector<metric_focus_pair> *list) 
{
  for (unsigned i = 0; i < list->size(); i++) {
    cout << "   metric: " <<  
      dataMgr->getMetricName (((*list)[i]).met) << "||| focus: ";
    for (unsigned j = 0; j < ((*list)[i]).res.size(); j++)
      cout << dataMgr->getResourceLabelName (((*list)[i]).res[j]);
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
int sendVisiSelectionsCmd(ClientData,
		Tcl_Interp *interp, 
		int,
		char *argv[])
{
  Tcl_HashEntry *entry;
  UIMReplyRec *msgRec;

#if UIM_DEBUG
  cout << "processing " << uim_VisiSelections.size() << " visiselections...\n";
  if (uim_VisiSelections->size() > 0) {
    printMFPlist (uim_VisiSelections);
  }
#endif
  
  // get callback and thread id for this msg
  int msgID = atoi(argv[1]);
  if (!(entry = Tcl_FindHashEntry (&UIMMsgReplyTbl, (char *) msgID))) {
    // this case can occur if a thread has exited between making the
    // menuing request and choosing accept on the menu...ignore it
    return TCL_OK;
  }
  msgRec = (UIMReplyRec *) Tcl_GetHashValue(entry);

     /* set thread id for return */
  uim_server->setTid(msgRec->tid);
  chooseMandRCBFunc mcb = (chooseMandRCBFunc) msgRec->cb;

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

      // Since the following igen call is async, we must unfortunately make
      // a copy of uim_VisiSelections, and pass that in.  The consumer
      // will deallocate the memory.
      vector<metric_focus_pair> *temp_igen_vec = new vector<metric_focus_pair> (uim_VisiSelections);
      assert(temp_igen_vec);
      uim_server->chosenMetricsandResources(mcb, temp_igen_vec);
  }

  // cleanup data structures
  Tcl_DeleteHashEntry (entry);   // cleanup hash table record

  return TCL_OK;
}

typedef vector<unsigned> numlist;
void printResSelectList (vector<numlist> *v, char *name)
{
  cout << "[Focus List " << name << "|" << v->size() << "] "; 
  for (unsigned i = 0; i < v->size(); i++) {
    numlist temp = (*v)[i];
    cout << "                    ";
    for (unsigned j = 0; j < temp.size(); j++) 
      cout << temp[j] << " ";
    cout << endl;
  }
  cout << endl;
}

/* parseSelections
 * takes a list with one resourceHandle vector per resource hierarchy; result 
 * is the cross product list of valid foci, represented as vectors of 
 * nodeID vectors; each element on resulting list can be converted into 
 * one focus.
 */
vector<numlist> parseSelections(vector<numlist> &theHierarchy,
				bool plusWholeProgram,
				numlist wholeProgramFocus) {
   // how many resources are in each hierarchy?:
   vector<unsigned> numResourcesByHierarchy(theHierarchy.size());
   for (unsigned hier=0; hier < theHierarchy.size(); hier++)
      numResourcesByHierarchy[hier] = theHierarchy[hier].size();

   odometer theOdometer(numResourcesByHierarchy);
   vector<numlist> result;

   while (!theOdometer.done()) {
      // create a focus using the odometer's current setting
      // Make a note if we have added the equivalent of "Whole Program"

      numlist theFocus(theHierarchy.size());

      bool addedEquivOfWholeProgram = plusWholeProgram; // so far...

      for (unsigned hier=0; hier < theHierarchy.size(); hier++) {
         theFocus[hier] = theHierarchy[hier][theOdometer[hier]];
         if (addedEquivOfWholeProgram)
	    addedEquivOfWholeProgram = (theFocus[hier] == wholeProgramFocus[hier]);
      }

      if (!addedEquivOfWholeProgram)
         result += theFocus;
//      else
//         cout << "Suppressing duplicate of whole program" << endl;

      theOdometer++;
   }

   // There is one final thing to check for: whole-program
   if (plusWholeProgram)
      result += wholeProgramFocus;

//   cout << "parseSelections: returning result of " << result.size() << " foci" << endl;
   return result;
}

/* arguments:
       0: "processVisiSelection"
       1: list of selected metrics
*/
extern dictionary_lite<unsigned, string> UI_all_metric_names;
extern bool UI_all_metrics_set_yet;

int processVisiSelectionCmd(ClientData,
			    Tcl_Interp *interp, 
			    int,
			    char *argv[])
{
   extern abstractions *theAbstractions;
   bool wholeProgram;
   numlist wholeProgramFocus;
   vector< vector<resourceHandle> > theHierarchySelections = theAbstractions->getCurrAbstractionSelections(wholeProgram, wholeProgramFocus);

#if UIM_DEBUG
   for (int i=0; i < theHierarchySelections.size(); i++) {
      cout << "ResHierarchy " << i << " selections: ";
      for (int j=0; j < theHierarchySelections[i].size(); j++)
         cout << " " << theHierarchySelections[i][j];
      cout << endl;
   }
#endif

  vector<numlist> fociList = parseSelections (theHierarchySelections,
					      wholeProgram, wholeProgramFocus);

#if UIM_DEBUG
  printResSelectList(fociList, "list of selected focii");
#endif

//** and, list of metric indices from selections put into metlst

  assert(UI_all_metrics_set_yet);

  int metcnt;
  char **metlst;
  // reminder: argv[1] is the list of selected metrics (each is an integer id)
  assert(TCL_OK == Tcl_SplitList (interp, argv[1], &metcnt, &metlst));

//   cout << "Here are the selections (in metric-ids)" << endl;
//   for (unsigned i=0; i < metcnt; i++)
//      cout << metlst[i] << " ";
//   cout << endl;

   uim_VisiSelections.resize(0);
   
   for (unsigned i = 0; i < metcnt; i++) {
      unsigned metric_id = atoi(metlst[i]);
      assert(UI_all_metric_names.defines(metric_id)); // just a sanity check

      for (unsigned focuslcv = 0; focuslcv < fociList.size(); focuslcv++)
	 uim_VisiSelections += metric_focus_pair(metric_id, fociList[focuslcv]);
   }

   free (metlst);   // cleanup after Tcl_SplitList

   sprintf (interp->result, "%d", 1);
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

int drawStartVisiMenuCmd (ClientData,
                Tcl_Interp *interp, 
                int,
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
    Tcl_DStringAppendElement(&namelist, (*via)[i].name.string_of());
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
  if (Tcl_VarEval (interp, "StartVisi $title $vnames $vnums $vcount",
		   0) == TCL_ERROR) {
    printf ("%s", interp->result);
    return TCL_ERROR;
  }
  return TCL_OK;
}


/*
 * argv[1] = error number
 * argv[2] = error string
 */
int showErrorCmd (ClientData,
                Tcl_Interp *,
                int,
                char *argv[])
{
  int code = atoi(argv[1]);
  uim_server->showError (code, argv[2]);
  return TCL_OK;
}

int uimpd_startPhaseCmd(ClientData, Tcl_Interp *,
			int argc, char **argv) {
   assert(argc == 2);
   if (0==strcmp(argv[1], "plain")) {
      dataMgr->StartPhase(-1, NULL,
			      false, false);
      return TCL_OK;
   }
   else if (0==strcmp(argv[1], "pc")) {
      dataMgr->StartPhase(-1, NULL,
			      true, false);
      return TCL_OK;
   }
   else if (0==strcmp(argv[1], "visis")) {
      dataMgr->StartPhase(-1, NULL,
			      false, true);
      return TCL_OK;
   }
   else if (0==strcmp(argv[1], "both")) {
      dataMgr->StartPhase(-1, NULL,
			      true, true);
      return TCL_OK;
   }
   else {
      cerr << "uimpd_startPhaseCmd: unknown cmd " << argv[1] << endl;
      return TCL_ERROR;
   }
}

struct cmdTabEntry uimpd_Cmds[] = {
  {"drawStartVisiMenu", drawStartVisiMenuCmd},
  {"sendVisiSelections", sendVisiSelectionsCmd},
  {"processVisiSelection", processVisiSelectionCmd},
  {"tclTunable", TclTunableCommand},
  {"showError", showErrorCmd},
  {"startPhase", uimpd_startPhaseCmd},
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
