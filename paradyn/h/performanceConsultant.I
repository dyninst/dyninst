
/*
 * Interface between the Performance Consultant and the rest of paradyn.
 *
 * $Log: performanceConsultant.I,v $
 * Revision 1.26  1996/05/01 14:04:28  naim
 * Adding async. call to requestNodeInfoCallback to avoid deadlock between UI
 * and PC - naim
 *
 * Revision 1.25  1996/04/21  21:37:18  newhall
 * removed getPredictedDataCostCallbackPC
 *
 * Revision 1.24  1996/04/18  22:00:18  naim
 * Changes to make getPredictedDataCost asynchronous - naim
 *
 * Revision 1.23  1996/04/07  21:20:47  karavan
 * combined newSearch and activateSearch and got rid of newSearch.
 *
 * Revision 1.22  1996/03/18 07:11:12  karavan
 * added new TC, PCcollectInstrTimings
 *
 * Revision 1.21  1996/02/09 20:56:11  karavan
 * Added globalRawDataServer and currentRawDataServer to keep the new data
 * path efficient.
 *
 * Revision 1.20  1996/02/09 18:01:28  karavan
 * Changes to support phase-level searching
 *
 * Revision 1.19  1996/02/08 20:09:49  karavan
 * added PC-global variables to store current TC values.
 *
 * Revision 1.18  1996/02/02 02:17:52  karavan
 * Changes for new performance consultant.
 *
 * Revision 1.17  1996/01/09 21:43:17  tamches
 * incremented version number to 7
 *
 * Revision 1.16  1996/01/09 00:45:15  tamches
 * added phaseID argument to getNodeInfo
 *
 * Revision 1.15  1996/01/08 22:07:15  tamches
 * Added igen call getNodeInof() and related struct shg_node_info
 *
 * Revision 1.14  1995/10/17 03:21:05  tamches
 * (really Karen)
 * Bumped version number from 5 to 6
 *
 * Revision 1.13  1995/10/17 03:11:54  tamches
 * (really Karen)
 * Added endSearch() igen call
 *
 * Revision 1.12  1995/10/05 04:48:24  karavan
 * changes to support multiple searches, one per phase.
 *
 * Revision 1.11  1995/02/16  08:01:45  markc
 * Modified interfaces:
 *   1) XDR interfaces use strings and vectors instead of igen arrays and char*'s
 *   2) No sync upcalls are allowed by the server in xdr interfaces
 *   3) Redefined igen arrays that were used in thread code to use manually defined
 *      structures rather than igen-defined, since igen did not need to know about
 *      these structures.
 *   4) Upped all of the version numbers since all new interface code is generated
 *   5) Async calls are no longer handled while waiting for sync responses, these
 *      requests are buffered.
 *
 * Revision 1.10  1995/01/26  17:59:31  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.9  1994/09/22  00:45:15  markc
 * Made array declarations explicit
 * Incremented version number
 *
 * Revision 1.8  1994/08/17  17:55:29  markc
 * Incremented version numbers because a new version of igen has been
 * committed.
 *
 * Revision 1.7  1994/05/09  20:53:36  hollings
 * added changeState to dataManager, and clearSHG to performanceConsultant.
 *
 * Revision 1.6  1994/05/06  06:50:45  karavan
 * added startSHG().
 *
 * Revision 1.5  1994/05/02  20:36:50  hollings
 * Added pause and shg get node routines.
 *
 * Revision 1.4  1994/04/27  22:54:52  hollings
 * Merged refine auto and search.
 *
 * Revision 1.3  1994/04/21  05:01:21  karavan
 * changed search and autoRefine to async to avoid deadlock with UIM.
 *
 * Revision 1.2  1994/02/02  00:43:20  hollings
 * Added Auto Refinement options.
 *
 * Revision 1.1  1994/02/01  18:47:35  hollings
 * Upgrades for performanceConsultant thread.
 *
 */
#include "dataManager.thread.h"

//
// Forward decl of classes that are passed out the interface.
//
class searchHistoryNode;
class filteredDataServer;
class PCmetricInstServer;

typedef struct int_Array {
  int count;
  int *data;
} int_Array;

// Note: the following structure (used by the "getNodeInfo" igen call) is
//       subject to fairly frequent change, as we thing of more goodies.
struct shg_node_info {
   string why;
   string where;
   bool currentConclusion;
   timeStamp timeTrueFalse;
   sampleValue currentValue;
   timeStamp startTime;
   timeStamp endTime;
   sampleValue estimatedCost;
   bool persistent;
};

$remote performanceConsultant {
    $base 6000;
    $version 15;

    // If search is paused, resumes search.  Otherwise, performs
    // all initialization for new search; initializes root node and 
    // messages in search display
    // Assumes phaseID is valid, defined as: DM's phaseID plus one for 
    // current search,  0 for global search.
    $async void activateSearch(unsigned phaseID);

    // stop the automated search process in its tracks.
    $async void pauseSearch(unsigned phaseID);

    // Completely terminate a search; remove all instrumentation.
    $async void endSearch(unsigned phaseID);

    // DM tells the PC the new value for predicted data cost
    // $async void getPredictedDataCostCallbackPC(int dummy,float val);

    // Get loads of information about a node:
    // client's responsibility to provide valid shg_node_info struct pointer
    $async void requestNodeInfo(unsigned phaseID, int nodeID);

    $signore
    // these globals hold values updated by tunable constant callbacks
    static float hysteresisRange;
    static float predictedCostLimit;
    static float minObservationTime;
    static float sufficientTime;
    static bool printDataTrace;
    static bool printTestResults;
    static bool printSearchChanges;
    static bool printDataCollection;
    static bool collectInstrTimings;
    static bool useIndividualThresholds;
    // other PC-globals
    static unsigned DMcurrentPhaseToken;
    static unsigned currentPhase;
    // for quick handling of new data
    static filteredDataServer *globalRawDataServer;
    static filteredDataServer *currentRawDataServer;
    static PCmetricInstServer *globalPCMetricServer;
    $signore    
};
