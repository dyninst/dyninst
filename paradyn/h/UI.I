/* UI.I
     input file for igen for User Interface Manager (UIM) interface to paradyn
*/
/* $Log: UI.I,v $
/* Revision 1.3  1994/06/12 22:39:42  karavan
/* implemented status display service.
/*
# Revision 1.2  1994/04/21  05:03:28  karavan
# Added most DAG services and sync versions of message display services.
#
# Revision 1.1  1994/04/05  19:37:57  karavan
# Initial version.
# */

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
	$version 1;
	//
	// provides user with appropriate choices and passes back selections

	$async void chooseMetricsandResources(chooseMandRCBFunc cb);	
	
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

	$async void showError (char *displayMsg);

	int showErrorWait (char *displayMsg,
				int numChoices,
				char **choices);

	// returns index of user's menu choice
        $async void chooseMenuItem (chooseMenuItemCBFunc cb, 
			char *menuItems,
			char *menuTitle,
			char *options,
			int numMenuItems,
			int flags);

	$upcall $async void chooseMenuItemREPLY (chooseMenuItemCBFunc cb, 
			int userChoice);

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
