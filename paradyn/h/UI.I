/* UI.I
     input file for igen for User Interface Manager (UIM) interface to paradyn
*/
/* $Log: UI.I,v $
/* Revision 1.1  1994/04/05 19:37:57  karavan
/* Initial version.
/* */

#include "dataManager.CLNT.h"

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
	$version 1;
	//
	// provides user with appropriate choices and passes back selections

	$async void chooseMetricsandResources(chooseMandRCBFunc cb);	
	
	$upcall $async void chooseMetricsandResourcesREPLY ( 					chooseMandRCBFunc, 
		 char **metricNames,
		 int numMetrics,
	 	 resourceList* focusChoice);	

	
	// generic message display; returns index of user choice if applic.

	$async void showMsg (showMsgCBFunc cb,
			char *displayMsg, 
			int numChoices, 
			char **choices);
	$upcall $async void showMsgREPLY (showMsgCBFunc cb, 
			int userChoice);


	// message display with "error" visual characteristics
	$async void showError (showErrorCBFunc cb, 
			char *displayMsg);
	$upcall $async void showErrorREPLY (showErrorCBFunc cb, 
			int userChoice);


	// returns index of user's menu choice
        $async void chooseMenuItem (chooseMenuItemCBFunc cb, 
			char *menuItems,
			char *menuTitle,
			char *options,
			int numMenuItems,
			int flags);

	$upcall $async void chooseMenuItemREPLY (chooseMenuItemCBFunc cb, 
			int userChoice);

	// 
	// in emergency, break glass; valid tcl command will go right to 
        //   tcl interpreter
        //     (not currently implemented)
	// $async void tclRequest (char *command);
	// $upcall $async void tclRequestREPLY (char *tclResult);

	// 
        // DAG services 
        //
// DAG services are on holiday until further notice.  stay tuned
  
//	int DAGaddNodes(int dagID, int numEntries, int nodeID, int styleID, 
//			char *label, char *tags, int root);
//	int DAGaddEdges (int dagID, int numEntries, int edgeID, int srcID, 
//			int dstID, int styleID, char *tags);
//	int DAGconfigNodes (int dagID, int numEntries, int nodeID, 
//                            int styleID);
//	int DAGconfigEdges (int dagID, int numEntries, int edgeID, 
//                            int styleID);
//	int DAGcreateEdgeStyle();
//	int DAGcreateNodeStyle();
//	int DAGdestroy (int dagID);
//
	
};
