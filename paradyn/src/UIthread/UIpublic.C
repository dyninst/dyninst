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

/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

/* $Log: UIpublic.C,v $
/* Revision 1.9  1994/07/08 04:03:18  karavan
/* changed showMsg to async function
/*
 * Revision 1.8  1994/07/07  05:58:05  karavan
 * added UI error service function
 *
 * Revision 1.7  1994/06/12  22:38:27  karavan
 * implemented status display service.
 *
 * Revision 1.6  1994/05/12  23:34:15  hollings
 * made path to paradyn.h relative.
 *
 * Revision 1.5  1994/05/07  23:26:30  karavan
 * added short explanation feature to SHG.
 *
 * Revision 1.4  1994/05/05  19:52:46  karavan
 * changed chooseMetricsandResources
 *
 * Revision 1.3  1994/04/21  05:18:17  karavan
 * Implemented DAG functions.
 *
 * Revision 1.2  1994/04/06  17:41:19  karavan
 * added working versions of getMetricsandResources, showError, showMessage
 * */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "UI.CLNT.h"
#include "UI.SRVR.h"
#include "UI.h"
#include "thread/h/thread.h"
#include "UIglobals.h"
#include "../pdMain/paradyn.h"
extern "C" {
  #include "tk.h"
}

#define PCSTATUSOBJECT 1
#define GENSTATUSOBJECT 2

class statusDisplayObject;

#define SHG_DAGID 1
     /** note: right now there is a hard limit of 20 dags total; numbers 
              are never re-used.  This should be changed eventually.  
      */
#define MAXNUMACTIVEDAGS 20
char *ActiveDags [] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
		       '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
		       '\0', '\0', '\0', '\0'};

Tcl_HashTable shgNamesTbl;   /* store full pathname for SHG nodes */
 
extern void initSHGStyles();

void 
UIMUser::chooseMenuItemREPLY(chooseMenuItemCBFunc cb, int userChoice)
{
  (cb) (userChoice);
}

void 
UIMUser::msgChoice(showMsgCBFunc cb,int userChoice)
{
  (cb) (userChoice);
}

void
UIMUser::chosenMetricsandResources
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

// ****************************************************************
// Display Objects 
// ****************************************************************

statusDisplayObj::statusDisplayObj (int type)
{
  if (type == PCSTATUSOBJECT)
    wname = ".shg.status.txt";
  else {
    wname = "notawindow";
    printf ("only PC supported so far!\n");
  }
}

void
statusDisplayObj::updateStatusDisplay (int displayCode, 
				       const char *fmt ...)
{
  char newItem[256];
  va_list ap;
  va_start(ap, fmt);
  char c;
  char *curr = newItem;
  
  while (c = *fmt++) {
    if (c == '%') {
      switch (c = *fmt++) {
      case 'c': 
	*curr = c;
	curr++;
	break;
      case 'd':
	sprintf (curr, "%d", va_arg(ap, int));
	curr = curr + strlen(curr);
	break;
      case 'f':
      case 'g':
	sprintf (curr, "%g", va_arg(ap, double));
	curr = curr + strlen(curr);
	break;
      case 's':
	strcpy (curr, va_arg(ap, char *));
	curr = curr + strlen(curr);
	break;
      }
    } else {
      *curr = c;
      curr++;
    }
    va_end (ap);
    *curr = '\0';
  }

  if (Tcl_VarEval (interp, "shgUpdateStatusLine .shg.status.txt {",
		   newItem, "}", (char *) NULL) == TCL_ERROR)
    fprintf (stderr, "status insert error: %s\n", interp->result);

}

statusDisplayObj *
UIM::initStatusDisplay (int type)
{
  statusDisplayObj *token;
  token = new statusDisplayObj (type);
  return token;
}

// ****************************************************************
// Batch Mode 
// ****************************************************************

void
UIM::startBatchMode ()
{
  UIM_BatchMode++;
}

void 
UIM::endBatchMode ()
{
  UIM_BatchMode--;
  if (UIM_BatchMode < 0)
    UIM_BatchMode = 0;
}

// ****************************************************************
// Error Message Display Service 
// ****************************************************************

void
UIM::showError(int errCode, char *errString)
{
  char errNum[10];
  int maxError;
  sprintf (errNum, "%d", errCode);
  if (errCode <= uim_maxError) {

  // custom error info string to be printed
    if (Tcl_VarEval (interp, "showError ", errNum, " \{", errString, 
		     "\}", (char *) NULL) == TCL_ERROR)
      printf ("newShowError: Tcl call to showError fails, %s\n", 
	      interp->result);
  }
}

// ****************************************************************
//  Standard Message Display Service Routines
// ****************************************************************

/* 
 * showMsg
 * This is an asynchronous UIM call.  A message is displayed with 
 * a set of user choice buttons, as specified in choices.  The array 
 * index of the user's choice is returned to the registered callback 
 * function.  If an error occurs during the message display, -1 is 
 * returned.
 */
void 
UIM::showMsg(showMsgCBFunc cb,
	char *displayMsg,
	int numChoices,
	char **choices)
{
  thread_t client;
  char *clist;
  int retVal;
  UIMReplyRec *reply;
  Tcl_HashEntry *entryPtr;
  char token[16];
  int newptr;

/*** get token*****/
  UIMMsgTokenID++;
  entryPtr = Tcl_CreateHashEntry (&UIMMsgReplyTbl, (char *)UIMMsgTokenID, 
				  &newptr);
  if (newptr) {
    reply = new UIMReplyRec;
      /* grab thread id of requesting thread */
    reply->tid = getRequestingThread();
    reply->cb = cb;
    Tcl_SetHashValue (entryPtr, reply);
  }

  clist = Tcl_Merge (numChoices, choices);
  Tcl_SetVar (interp, "choices", clist, 0);
  Tcl_SetVar (interp, "msg", displayMsg, 0);
  sprintf (token, "%d", UIMMsgTokenID);
  Tcl_SetVar (interp, "id", token, 0);
  retVal = Tcl_EvalFile (interp, "msg.tcl");
  if (retVal == TCL_ERROR) {
    printf ("error showing Message:\n");
    printf ("%s\n", interp->result);
  }
}

int uimMsgReplyCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  Tcl_HashEntry *entry;
  UIMReplyRec *msgRec;
  int retVal;
  showMsgCBFunc mcb;
  int msgID;

  if (argc < 3) {
    Tcl_AppendResult(interp, "Usage: uimMsgReply msg# choice\n", 
		     (char *) NULL);
    return TCL_ERROR;
  }
  printf ("in uimMsgReplyCmd w/ id = %s; ret = %s\n", argv[1], argv[2]);
  // get callback and thread id for this msg
  Tcl_GetInt (interp, argv[1], &msgID);
  if (!(entry = Tcl_FindHashEntry (&UIMMsgReplyTbl, (char *) msgID))) {
    Tcl_AppendResult (interp, "invalid message ID!", (char *) NULL);
    return TCL_ERROR;
  }
  
  msgRec = (UIMReplyRec *) Tcl_GetHashValue(entry);
  Tcl_GetInt (interp, argv[2], &retVal);

     /* set thread id for return */
  uim_server->setTid(msgRec->tid);
  mcb = (showMsgCBFunc) msgRec->cb;
  Tcl_DeleteHashEntry (entry);   // cleanup hash table record

     /* send reply */
  uim_server->msgChoice (mcb, retVal);
  return TCL_OK;
}

/*
 *  showMsgWait
 *  This is a synchronous version of showMsg; the index of the user 
 *  menu choice is returned directly to the calling thread.  
 */
int 
UIM::showMsgWait(char *displayMsg,
	int numChoices,
	char **choices)
{
  char *clist;
  int retVal;

  clist = Tcl_Merge (numChoices, choices);
  Tcl_SetVar (interp, "choices", clist, 0);
  Tcl_SetVar (interp, "msg", displayMsg, 0);
  retVal = Tcl_EvalFile (interp, "msg.tcl");
  if (retVal == TCL_ERROR) {
    printf ("error showing Message\n");
    return -1;
  }
     /* send reply */
  Tcl_GetInt (interp, interp->result, &retVal);
  return retVal;
}

// ****************************************************************
// Metrics and Resources 
// ****************************************************************

void 
UIM::chooseMetricsandResources(chooseMandRCBFunc cb)
{
  char *ml;
  int retVal;
  String_Array availMets;
  char ctr[16];
  UIMReplyRec *reply;
  Tcl_HashEntry *entryPtr;
  int newptr;

  
      // store record with unique id and callback function
  UIMMsgTokenID++;
  entryPtr = Tcl_CreateHashEntry (&UIMMsgReplyTbl, (char *)UIMMsgTokenID, 
				  &newptr);
  if (newptr) {
    reply = new UIMReplyRec;
      /* grab thread id of requesting thread */
    reply->tid = getRequestingThread();
    reply->cb = (showMsgCBFunc) cb;
    Tcl_SetHashValue (entryPtr, reply);
  }

     // initialize metric menu 
  availMets = dataMgr->getAvailableMetrics(context);

  ml = Tcl_Merge (availMets.count, availMets.data);
  ml = Tcl_SetVar (interp, "metList", ml, 0);
  sprintf (ctr, "%d", availMets.count);
  Tcl_SetVar (interp, "metCount", ctr, 0);
  sprintf (ctr, "%d", UIMMsgTokenID);
  Tcl_SetVar (interp, "metsAndResID", ctr, 0);

      // tcl proc draws window & gets metrics and resources from user 
  retVal = Tcl_VarEval (interp, "getMetsAndRes", 0);
  if (retVal == TCL_ERROR)  {
    printf ("%s\n", interp->result);
  }
}
// ****************************************************************
//  DAG/SHG Display Service Routines
// ****************************************************************


int 
UIM::initSHG() 
{
  static int nextActiveDag = 0;
  int token;
  char longshot[] = ".shg";
  if (nextActiveDag == MAXNUMACTIVEDAGS) {
    printf ("initSHG error: max no. active dags exceeded\n");
    return -1;
  }
  token = nextActiveDag;
  nextActiveDag++;

      /* set this variable to unique window name */
  Tcl_SetVar (interp, "SHGname", longshot, 0);
  
  if (Tcl_VarEval (interp, "initSHG", (char *) NULL) == TCL_ERROR)
    printf ("initSHG error: %s\n", interp->result);
      /** ooops! need to move this out of shg! */ 
  ActiveDags[token] = ".shg.d01";
      /* hash table for long node names */
  Tcl_InitHashTable (&shgNamesTbl, TCL_ONE_WORD_KEYS);

  return token;
}

/*  flags: 1 = root
 *         2 = shg name handling
 */

int 
UIM::DAGaddNode(int dagID, int nodeID, int styleID, 
			char *label, char *shgname, int flags)
{
  char *dagName;
  char tcommand[200];
  Tcl_HashEntry *entryPtr;
  Tk_Uid token;
  int added;
  
  dagName = ActiveDags[dagID];
  if (flags) {
    sprintf (tcommand, "%s addNode %d -root yes -style %d -label \"root\"",
	     dagName, nodeID, styleID);
  }
  else {
    sprintf (tcommand, "%s addNode %d -root no -style %d -label \"%s\"",
	     dagName, nodeID, styleID, label);
  }
  if (Tcl_VarEval (interp, tcommand, (char *) NULL) == TCL_ERROR) {
    printf ("DAGaddNodeError: %s\n", interp->result);
    return 0;
  }
     // store pointer to full node pathname and use last piece as label
  sprintf (tcommand, "%d", nodeID);
  token = Tk_GetUid (tcommand);
  entryPtr = Tcl_CreateHashEntry (&shgNamesTbl, token, &added);
  if (added) {
    Tcl_SetHashValue (entryPtr, shgname);
  }
  return 1;
}

int 
UIM::DAGaddEdge (int dagID, int srcID, 
		 int dstID, int styleID)
{
  char *dagName;
  char tcommand[200];

  dagName = ActiveDags[dagID];
  sprintf (tcommand, "%s addEdge %d %d -style %d ",
	   dagName, srcID, dstID, styleID);
  if (Tcl_VarEval (interp, tcommand, (char *) NULL) == TCL_ERROR) {
    printf ("DAGaddEdgeError: %s\n", interp->result);
    return 0;
  }
  else
    return 1;

}

  
int 
UIM::DAGconfigNode (int dagID, int nodeID, int styleID)
{
  char *dagName;
  char tcommand[200];

  dagName = ActiveDags[dagID];
  sprintf (tcommand, "%s nodeconfigure %d -style %d", 
	   dagName, nodeID, styleID);
  if (Tcl_VarEval (interp, tcommand, (char *) NULL) == TCL_ERROR) {
    printf ("DAGconfigNode: %s\n", interp->result);
    return 0;
  }
  else
    return 1;
}
  
int 
UIM::DAGcreateNodeStyle(int dagID, int styleID, char *options)

{
  char *dagName;
  char tcommand[200];
  dagName = ActiveDags[dagID];
  sprintf (tcommand, "%s addNstyle %d %s", dagName, styleID, options);

  if (Tcl_VarEval (interp, tcommand, (char *) NULL) == TCL_ERROR) {
    printf ("DAGcreateNodeStyle: %s\n", interp->result);
    return 0;
  }
  else
    return 1;
}

int 
UIM::DAGcreateEdgeStyle(int dagID, int styleID, char *options)

{
  char *dagName;
  char tcommand[200];
  dagName = ActiveDags[dagID];
  sprintf (tcommand, "%s addEstyle %d %s", dagName, styleID, options);

  if (Tcl_VarEval (interp, tcommand, (char *) NULL) == TCL_ERROR) {
    printf ("DAGcreateNodeStyle: %s\n", interp->result);
    return 0;
  }
  else
    return 1;
}


