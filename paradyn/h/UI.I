/*
 * input file for igen for User Interface Manager (UIM) interface to Paradyn
 * $Id: UI.I,v 1.46 2005/01/11 22:47:15 legendre Exp $
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
	$version 27;
	//
	// provides user with appropriate choices and passes back selections

	$virtual $async void chooseMetricsandResources(chooseMandRCBFunc cb,
			     pdvector<metric_focus_pair> *pairList);

	$upcall $async void chosenMetricsandResources (chooseMandRCBFunc, 
			      pdvector<metric_focus_pair> *pairList);

	$virtual $async void registerValidVisis (pdvector<VM_visiInfo> *allVisis);

        //
	// called by an exiting thread to notify the UI that it is exiting
	// this is so that the UI does not try to send a metrics menuing 
	// response to a dead tid
	//
	$virtual void threadExiting();

	// message display with "error" visual characteristics
        //  and lookup in error database for explanation

	$virtual $async void showError (int errCode, const char *errString);

	// update status display
	$virtual $async void updateStatusLine(const char* sl_name, const char *msg);
	$virtual $async void createStatusLine(const char* sl_name);
	$virtual $async void createProcessStatusLine(const char* sl_name);
	$virtual $async void destroyStatusLine(const char* sl_name);

	$virtual $async void enablePauseOrRun();

	// used to tell UI to source a tcl file after initialization
        // (but of course the -s option is for us developers only...)
	$virtual $async void readStartupFile(const char *script);

        // This procedure is used when paradyn create a process after 
	// reading a configuration file (using option -f).
	$virtual $async void ProcessCmd(pdstring *arguments);

	$virtual $async void allDataSaved(bool succeeded);
	$virtual $async void resourcesSaved(bool succeeded);
    $virtual $async void shgSaved (bool succeeded);

	// 
        // SHG stuff
        //

	// add string to end of PC (scrolling) status display box
	$virtual void updateStatusDisplay (int token, pdstring *item);

	// phaseID here is PC/UI version
	$virtual $async void newPhaseNotification (unsigned phaseID, const char *phname, 
					  bool with_new_pc);
	$virtual void DAGaddNode(int dagID, unsigned nodeID, int styleID, 
			const char *label, const char *shgname, int root);
	$virtual void DAGaddEdge (int dagID, unsigned srcID, 
			unsigned dstID,
			int styleID, // why vs. where refinement
			const char *label // only used for shadow nodes; else NULL
			);
	$virtual $async void DAGaddBatchOfEdges(int dagID, pdvector<uiSHGrequest> *requests,
				       unsigned numRequests);
	$virtual void DAGconfigNode (int dagID, unsigned nodeID, int styleID);
	$virtual $async void DAGinactivateEntireSearch(int dagID);
        
    $virtual $async void requestNodeInfoCallback(unsigned phaseID, int nodeID, 
                                            shg_node_info *theInfo, bool ok);

	//
	//CallGraph Stuff
	//
	
	$virtual $async void CGaddNode(int pid, resourceHandle parent, 
	                      resourceHandle newNode, const char *shortName, 
                              const char *fullName, bool isRecursive,
                              bool shadowNode);
	$virtual $async void callGraphProgramAddedCB(unsigned programId,
                                            resourceHandle newId,
                                            const char *executableName,
                                            const char *shortName,  
                                            const char *fullName); 
	$virtual $async void CGDoneAddingNodesForNow(int programId);

	// tell the UI thread to close its GUI connection
	// used in cases when the front-end forks to spawn a new process
	$virtual $async void CloseTkConnection();

	// support for configuring Paradyn windows programmatically
	// (e.g., in response to programmatic tunable constant changes)
	$virtual $async void showWhereAxisTips( bool val );
	$virtual $async void showSHGKey( bool val );
	$virtual $async void useLoops( bool val );
	$virtual $async void showSHGTips( bool val );
	$virtual $async void showSHGTrueNodes( bool val );
	$virtual $async void showSHGFalseNodes( bool val );
	$virtual $async void showSHGUnknownNodes( bool val );
	$virtual $async void showSHGNeverExpandedNodes( bool val );
	$virtual $async void showSHGActiveNodes( bool val );
	$virtual $async void showSHGInactiveNodes( bool val );
	$virtual $async void showSHGShadowNodes( bool val );
	$virtual $async void setDeveloperMode( bool val );
	$virtual $async void showTclPrompt( bool val );

    // call to let UI thread know data manager is ready to accept calls
    $virtual $async void DMready();
};

