/*
 * input file for igen for User Interface Manager (UIM) interface to Paradyn
 * $Id: UI.I,v 1.44 2003/07/15 22:45:15 schendel Exp $
 */

#include "paradyn/src/DMthread/DMinclude.h"
#include "paradyn/src/UIthread/Status.h"
#include "performanceConsultant.thread.h"
#include "paradyn/src/VMthread/VMinclude.h"

/* 
 * Handler that gets called when user-selected metric and focus choices
 * are passed back to the requesting visualization thread
 */

typedef int (*chooseMandRCBFunc) (pdvector<metric_focus_pair> *pairList);

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
	$version 26;
	//
	// provides user with appropriate choices and passes back selections

	$async void chooseMetricsandResources(chooseMandRCBFunc cb,
			     pdvector<metric_focus_pair> *pairList);

	$upcall $async void chosenMetricsandResources (chooseMandRCBFunc, 
			      pdvector<metric_focus_pair> *pairList);

	$async void registerValidVisis (pdvector<VM_visiInfo> *allVisis);

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

	$async void updateStatus(status_line* sl, const char *msg);

	$async void updateStatusLine(const char* sl_name, const char *msg);
	
	$async void createStatusLine(const char* sl_name);
	$async void createProcessStatusLine(const char* sl_name);

	$async void destroyStatusLine(const char* sl_name);

	$async void enablePauseOrRun();

	// used to tell UI to source a tcl file after initialization
        // (but of course the -s option is for us developers only...)
	$async void readStartupFile(const char *script);

        // This procedure is used when paradyn create a process after 
	// reading a configuration file (using option -f).
	$async void ProcessCmd(pdstring *arguments);

	$async void allDataSaved(bool succeeded);
	$async void resourcesSaved(bool succeeded);
        $async void shgSaved (bool succeeded);

	// 
        // SHG stuff
        //

	// add string to end of PC (scrolling) status display box
	void updateStatusDisplay (int token, pdstring *item);
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
	$async void DAGaddBatchOfEdges(int dagID, pdvector<uiSHGrequest> *requests,
				       unsigned numRequests);
	int DAGconfigNode (int dagID, unsigned nodeID, int styleID);
	$async void DAGinactivateEntireSearch(int dagID);
        
        $async void requestNodeInfoCallback(unsigned phaseID, int nodeID, 
                                            shg_node_info *theInfo, bool ok);

	//
	//CallGraph Stuff
	//
	
	$async void CGaddNode(int pid, resourceHandle parent, 
	                      resourceHandle newNode, const char *shortName, 
                              const char *fullName, bool isRecursive,
                              bool shadowNode);
	$async void callGraphProgramAddedCB(unsigned programId,
                                            resourceHandle newId,
                                            const char *executableName,
                                            const char *shortName,  
                                            const char *fullName); 
	$async void CGDoneAddingNodesForNow(int programId);

	// tell the UI thread to close its GUI connection
	// used in cases when the front-end forks to spawn a new process
	$async void CloseTkConnection();

	// support for configuring Paradyn windows programmatically
	// (e.g., in response to programmatic tunable constant changes)
	$async void showWhereAxisTips( bool val );
	$async void showSHGKey( bool val );
	$async void showSHGTips( bool val );
	$async void showSHGTrueNodes( bool val );
	$async void showSHGFalseNodes( bool val );
	$async void showSHGUnknownNodes( bool val );
	$async void showSHGNeverExpandedNodes( bool val );
	$async void showSHGActiveNodes( bool val );
	$async void showSHGInactiveNodes( bool val );
	$async void showSHGShadowNodes( bool val );
	$async void setDeveloperMode( bool val );
	$async void showTclPrompt( bool val );
};

