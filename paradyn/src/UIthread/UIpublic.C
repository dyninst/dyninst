/*
 * UIpublic.C : exported services of the User Interface Manager thread 
 *              of Paradyn
 *
 * This file contains working versions of the following:
 *      UIMUser::
 *               showErrorREPLY
 *               showMsgREPLY
 *               chooseMetricsandResourcesREPLY
 *
 *      UIM:: showError
 *            showMsg
 *            chooseMetricsandResources
 */
/* $Log: UIpublic.C,v $
/* Revision 1.2  1994/04/06 17:41:19  karavan
/* added working versions of getMetricsandResources, showError, showMessage
/* */

#include <stdio.h>
#include "UI.CLNT.h"
#include "UI.SRVR.h"
#include "UI.h"
#include "thread/h/thread.h"
#include "UIglobals.h"

extern resourceList *build_resource_list (Tcl_Interp *interp, char *list);

void 
UIMUser::chooseMenuItemREPLY(chooseMenuItemCBFunc cb, int userChoice)
{
  (cb) (userChoice);
}

void
UIMUser::showErrorREPLY(showErrorCBFunc cb,int userChoice)
{
  (cb) (userChoice);
}

void 
UIMUser::showMsgREPLY(showMsgCBFunc cb,int userChoice)
{
  (cb) (userChoice);
}

void
UIMUser::chooseMetricsandResourcesREPLY
          (chooseMandRCBFunc cb,
	   char **metricNames,
	   int numMetrics,
	   resourceList *focusChoice)
{
  (cb) (metricNames, numMetrics, focusChoice);
}
	
void 
UIM::chooseMenuItem(chooseMenuItemCBFunc cb,
	       char *menuItems,
	       char *menuTitle,
	       char *options,
	       int numMenuItems,
	       int flags)
{
  fprintf (stderr, "chooseMenuItem called\n");

}

void 
UIM::showError(showErrorCBFunc cb,
	  char *displayMsg)
{
  fprintf (stderr, "showError: %s\n", displayMsg);
  Tcl_VarEval (interp, "showError", displayMsg, (char *) NULL); 
}

void 
UIM::showMsg(showMsgCBFunc cb,
	char *displayMsg,
	int numChoices,
	char **choices)
{
  thread_t client;
  char *clist;
  int retVal;

      /* grab thread id of requesting thread */
  client = getRequestingThread();
  fprintf (stderr, "showMsg: %s\n", displayMsg);

  clist = Tcl_Merge (numChoices, choices);
  printf ("clist = %s\n", clist);
  Tcl_SetVar (interp, "choices", clist, 0);
  Tcl_SetVar (interp, "msg", displayMsg, 0);
  retVal = Tcl_EvalFile (interp, "msg.tcl");
  if (retVal == TCL_ERROR)
    printf ("error showing Message\n");
  
     /* set thread id for return */
  uim_server->setTid(client);
     /* send reply */
  uim_server->showMsgREPLY(cb, retVal);

}

void 
UIM::chooseMetricsandResources(chooseMandRCBFunc cb)
{
  char *ml, *rl;
  char **mlist;
  int numMetrics;
  int retVal;
  resourceList *focus;
  thread_t client;

      /* grab thread id of requesting thread */
  client = getRequestingThread();
  fprintf (stderr, "chooseMetricsandResources\n");

      /* tcl script draws window & gets metrics and resources from user */

  retVal = Tcl_EvalFile (interp, "mets.tcl");
    
  if ((retVal == TCL_ERROR) || (interp->result == 0)) {
    printf ("error getting metrics and focus\n");
  }
      /* get back two lists from user input; resource list gets converted */  
  ml = Tcl_GetVar (interp, "metList", 0);
  rl = Tcl_GetVar (interp, "resList", 0);
  Tcl_SplitList (interp, ml, &numMetrics, &mlist);
  focus = build_resource_list (interp, rl);

     /* set thread id for return */
  uim_server->setTid(client);
     /* send reply */
  uim_server->chooseMetricsandResourcesREPLY(cb, mlist, numMetrics, focus);
}






