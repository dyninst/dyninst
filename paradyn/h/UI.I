/* UI.I
     input file for igen for User Interface Manager (UIM) interface to paradyn
*/
/* $Log: UI.I,v $
/* Revision 1.17  1995/11/08 23:41:36  tamches
/* removed obsolete igen calls showMsg, msgChoice, showMsgWait,
/* chooseMenuItem, and chooseMenuItemREPLY
/*
 * Revision 1.16  1995/10/17 03:00:39  karavan
 * UIstatDisp.h is obsolete
 *
# Revision 1.15  1995/10/05  04:46:22  karavan
# changes to support future Performance Consultant: multiple searches, one
# per phase.  Added PC status window to the interface where it belongs.
# Removed statusDisplayObj class.
# Deleted obsolete commented code.
#
# Revision 1.14  1995/07/17  04:25:50  tamches
# commented out the unused batch calls
#
# Revision 1.13  1995/06/02  20:58:43  newhall
# made code compatable with new datamanager interface
# replaced List and HTable container classes with STL containers
#
# Revision 1.12  1995/01/26  17:59:26  jcargill
# Changed igen-generated include files to new naming convention; fixed
# some bugs compiling with gcc-2.6.3.
#
# Revision 1.11  1994/11/02  04:44:55  karavan
# added new interface service UIM::readStartupFile()
#
# Revision 1.10  1994/09/25  01:49:01  newhall
# UI and VM interface changes to support new representation
# of metric/focus lists as a list of metric/focus pairs (either
# as a string or a list of metric* resourceList* pairs).
#
# Revision 1.9  1994/09/22  00:40:52  markc
# Specified return type for function typedefs
# Added "const" for errString parameter to showError()
# incremented version number
#
# Revision 1.8  1994/08/17  17:55:22  markc
# Incremented version numbers because a new version of igen has been
# committed.
#
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

#include "paradyn/src/DMthread/DMinclude.h"

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

/*
 * igen template for UIM server and client classes 
 */ 

$remote UIM {
	$base 3000;
	$version 9;
	//
	// provides user with appropriate choices and passes back selections

	$async void chooseMetricsandResources(chooseMandRCBFunc cb,
			     vector<metric_focus_pair> *pairList);

	$upcall $async void chosenMetricsandResources (chooseMandRCBFunc, 
			      vector<metric_focus_pair> *pairList);

	
	// message display with "error" visual characteristics
        //  and lookup in error database for explanation

	$async void showError (int errCode, const char *errString);

	// used to tell UI to source a tcl file after initialization
        // (but of course the -s option is for us developers only...)
	$async void readStartupFile(const char *script);

	// 
        // SHG stuff
        //
	int initSHG (const char *phasename, int phaseID);

	// add string to end of PC (scrolling) status display box
	$async void updateStatusDisplay (int token, const char *item);

	int DAGaddNode(int dagID, unsigned nodeID, int styleID, 
			char *label, char *shgname, int root);
	int DAGaddEdge (int dagID, unsigned srcID, 
			unsigned dstID, int styleID);
	int DAGconfigNode (int dagID, unsigned nodeID, int styleID);

// some DAG services are on holiday until further notice.  stay tuned  
//	int DAGconfigEdges (int dagID, int numEntries, int edgeID, 
//                            int styleID);
//	int DAGdestroy (int dagID);
//
	
};
