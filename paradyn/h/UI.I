/* UI.I
     input file for igen for User Interface Manager (UIM) interface to paradyn
*/
/* $Log: UI.I,v $
/* Revision 1.8  1994/08/17 17:55:22  markc
/* Incremented version numbers because a new version of igen has been
/* committed.
/*
# Revision 1.7  1994/08/09  18:14:58  karavan
# Expanded GetMetricsandResources to handle retries.
#
# Revision 1.6  1994/07/08  04:04:15  karavan
# changed showError function to async
#
# Revision 1.5  1994/07/07  05:59:02  karavan
# updated version number (finally!)
#
# Revision 1.4  1994/07/07  05:57:42  karavan
# added UIM error service function
#
# Revision 1.3  1994/06/12  22:39:42  karavan
# implemented status display service.
#
# Revision 1.2  1994/04/21  05:03:28  karavan
# Added most DAG services and sync versions of message display services.
#
# Revision 1.1  1994/04/05  19:37:57  karavan
# Initial version.
# */

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

#include "dataManager.CLNT.h"
#include "../src/UIthread/UIstatDisp.h"

/* 
 * Handler that gets called when user-selected metric and focus choices
 * are passed back to the requesting visualization thread
 */
typedef (*chooseMandRCBFunc) (char **metricNames, int numMetrics, 
					resourceList *focusChoice);

/* 
 * Handler that gets called when user choice passed back to requesting 
 *  thread after message display
 */
typedef (*showMsgCBFunc) (int userChoice);

/* 
 * Handler that gets called when user choice passed back to requesting 
 *  thread after error message display
 */
typedef (*showErrorCBFunc) (int userChoice);

/*
 * Handler that gets called when user choice passed back to requesting 
 *  thread after menu display.  userChoice is index of selected menu 
 *  item in original list.
 */
typedef (*chooseMenuItemCBFunc) (int userChoice);


/*
 * igen template for UIM server and client classes 
 */ 

$remote UIM {
	$base 3000;
	$version 5;
	//
	// provides user with appropriate choices and passes back selections

	$async void chooseMetricsandResources(chooseMandRCBFunc cb,
		char **metricNames,
		int numMetrics,
		resourceList* focusChoice);	
	
	$upcall $async void chosenMetricsandResources ( 					chooseMandRCBFunc, 
		 char **metricNames,
		 int numMetrics,
	 	 resourceList* focusChoice);	

	
	// generic message display; returns index of user choice if applic.

	$async void showMsg (showMsgCBFunc cb,
			char *displayMsg, 
			int numChoices, 
			char **choices);

	$upcall $async void msgChoice (showMsgCBFunc cb, 
			int userChoice);

	int showMsgWait (char *displayMsg, 
			int numChoices, 
			char **choices);

	// message display with "error" visual characteristics
        //  and lookup in error database for explanation

	$async void showError (int errCode, char *errString);

	// returns index of user's menu choice
        $async void chooseMenuItem (chooseMenuItemCBFunc cb, 
			char *menuItems,
			char *menuTitle,
			char *options,
			int numMenuItems,
			int flags);

	$upcall $async void chooseMenuItemREPLY (chooseMenuItemCBFunc cb, 
			int userChoice);

	$async void startBatchMode ();
	$async void endBatchMode ();

	// add string to end of status display window (PC only for now)
	//	$async void updateStatusDisplay (int token, char *item);
	statusDisplayObj *initStatusDisplay (int type);
	// 
	// in emergency, break glass; valid tcl command will go right to 
        //   tcl interpreter
        //     (not currently implemented)
	// $async void tclRequest (char *command);
	// $upcall $async void tclRequestREPLY (char *tclResult);

	// 
        // DAG services 
        //
	int initSHG ();

	int DAGaddNode(int dagID, int nodeID, int styleID, 
			char *label, char *shgname, int root);
	int DAGaddEdge (int dagID, int srcID, 
			int dstID, int styleID);
	int DAGconfigNode (int dagID, int nodeID, int styleID);

	int DAGcreateEdgeStyle(int dagID, int styleID, char *options);
	int DAGcreateNodeStyle(int dagID, int styleID, char *options);

// some DAG services are on holiday until further notice.  stay tuned  
//	int DAGconfigEdges (int dagID, int numEntries, int edgeID, 
//                            int styleID);
//	int DAGdestroy (int dagID);
//
	
};
