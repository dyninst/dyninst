/* UI.I
     input file for igen for User Interface Manager (UIM) interface to paradyn
*/
/* $Log: UI.I,v $
/* Revision 1.30  1996/05/07 18:06:28  newhall
/* added threadExiting, this routine must be synchrounous
/*
# Revision 1.29  1996/05/01  20:57:20  tamches
# added DAGinactivateEntireSearch
#
 * Revision 1.28  1996/05/01 14:03:48  naim
 * Adding async. call to requestNodeInfoCallback to avoid deadlock between UI
 * and PC - naim
 *
 * Revision 1.27  1996/04/19  18:26:37  naim
 * Adding a procedure that will be called from the DM to add a new process,
 * as it is done using the "paradyn process" command - naim
 *
 * Revision 1.26  1996/04/16  18:39:05  karavan
 * changed updateStatusDisplay to take a string
 *
 * Revision 1.25  1996/04/13 04:40:24  karavan
 * new interface for batching edge requests
 *
 * Revision 1.24  1996/04/09 19:24:22  karavan
 * added back batch mode to cut down on shg redraw time
 *
 * Revision 1.23  1996/04/07 21:19:37  karavan
 * added newPhaseNotification.
 *
 * Revision 1.22  1996/02/23 22:09:16  naim
 * Adding igen call to display status line (fixes a deadlock situation) - naim
 *
 * Revision 1.21  1996/02/07  19:02:09  tamches
 * got rid of initSHG
 *
 * Revision 1.20  1996/02/02 02:17:50  karavan
 * Changes for new performance consultant.
 *
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

#include "paradyn/src/DMthread/DMinclude.h"
#include "paradyn/src/UIthread/Status.h"
#include "performanceConsultant.thread.h"

/* 
 * Handler that gets called when user-selected metric and focus choices
 * are passed back to the requesting visualization thread
 */

typedef int (*chooseMandRCBFunc) (vector<metric_focus_pair> *pairList);

/* 
 * Handler that gets called when user choice passed back to requesting 
 *  thread after message display
 */
typedef int (*showMsgCBFunc) (int userChoice);

/* 
 * Handler that gets called when user choice passed back to requesting 
 *  thread after error message display
 */
typedef int (*showErrorCBFunc) (int userChoice);

/*
 * Handler that gets called when user choice passed back to requesting 
 *  thread after menu display.  userChoice is index of selected menu 
 *  item in original list.
 */
typedef int (*chooseMenuItemCBFunc) (int userChoice);

//
// used in batched shg add requests
//
struct uiSHGrequest {
  unsigned srcNodeID;
  unsigned dstNodeID;
  int styleID;
  const char *label;
};

/*
 * igen template for UIM server and client classes 
 */ 

$remote UIM {
	$base 3000;
	$version 18;
	//
	// provides user with appropriate choices and passes back selections

	$async void chooseMetricsandResources(chooseMandRCBFunc cb,
			     vector<metric_focus_pair> *pairList);

	$upcall $async void chosenMetricsandResources (chooseMandRCBFunc, 
			      vector<metric_focus_pair> *pairList);

        //
	// called by an exiting thread to notify the UI that it is exiting
	// this is so that the UI does not try to send a metrics menuing 
	// response to a dead tid
	//
	void threadExiting();

	// message display with "error" visual characteristics
        //  and lookup in error database for explanation

	$async void showError (int errCode, const char *errString);

	// update status display

	$async void updateStatus(status_line *status, const char *msg);

	// used to tell UI to source a tcl file after initialization
        // (but of course the -s option is for us developers only...)
	$async void readStartupFile(const char *script);

        // This procedure is used when paradyn create a process after 
	// reading a configuration file (using option -f).
	$async void ProcessCmd(string *arguments);

	// 
        // SHG stuff
        //

	// add string to end of PC (scrolling) status display box
	void updateStatusDisplay (int token, string *item);
	// phaseID here is PC/UI version
	$async void newPhaseNotification (unsigned phaseID, const char *phname, 
					  bool with_new_pc);
	int DAGaddNode(int dagID, unsigned nodeID, int styleID, 
			const char *label, const char *shgname, int root);
	int DAGaddEdge (int dagID, unsigned srcID, 
			unsigned dstID,
			int styleID, // why vs. where refinement
			const char *label // only used for shadow nodes; else NULL
			);
	$async void DAGaddBatchOfEdges(int dagID, vector<uiSHGrequest> *requests,
				       unsigned numRequests);
	int DAGconfigNode (int dagID, unsigned nodeID, int styleID);
	$async void DAGinactivateEntireSearch(int dagID);
        
        $async void requestNodeInfoCallback(unsigned phaseID, int nodeID, 
                                            shg_node_info *theInfo, bool ok);
};
