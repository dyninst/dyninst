/*
 * Interface between the Performance Consultant and the rest of paradyn.
 * $Id: performanceConsultant.I,v 1.41 2005/01/11 22:44:59 legendre Exp $
 */
#include "paradyn/src/DMthread/DMinclude.h"
#include "dataManager.thread.h"
#include "common/h/Time.h"
#include "pdutil/h/pdSample.h"
#include "pdutil/h/pdRate.h"
//
// Forward decl of classes that are passed out the interface.
//
class searchHistoryNode;
class filteredDataServer;
class PCmetricInstServer;

typedef struct {
  int count;
  int *data;
} int_Array;


typedef enum {
  tfalse, ttrue, tunknown
} testResult; 

// Note: the following structure (used by the "getNodeInfo" igen call) is
//       subject to fairly frequent change, as we think of more goodies.
struct shg_node_info {
   pdstring why;
   pdstring where;
   bool active;                 // new
   testResult currentConclusion;
   timeLength timeTrueFalse;
   pdRate currentValue;    // changed to reflect "pure" value
   pdRate adjustedValue;   // what was previously currentValue
   pdRate lastThreshold;   // new
   float hysConstant;      // new
   relTimeStamp startTime;
   relTimeStamp endTime;
   float estimatedCost;
   bool persistent;
};

$remote performanceConsultant {
    $base 6000;
    $version 22;

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

    // Dump out contents of search history graph into a file
    // valid flag values:  1 = global 2 = phase 3 = all
    $async void saveSHG(const char *filename, int flag);

    //Notify the PC that function <parent> has an extra child,
    //so that the PC can revise its search strategy appropriately.
    $async void notifyDynamicChild(resourceHandle parent,
                                   resourceHandle child);

    $signore
    // these globals hold values updated by tunable constant callbacks
    static float hysteresisRange;
    static float predictedCostLimit;
    static timeLength minObservationTime;
    static timeLength sufficientTime;
    static bool printDataTrace;
    static bool printTestResults;
    static bool printSearchChanges;
    static bool printDataCollection;
    static bool collectInstrTimings;
    static bool useIndividualThresholds;
    static bool useCallGraphSearch;
    static bool searchMachineSync;
    static bool useLoops;
    // other PC-globals
    static unsigned DMcurrentPhaseToken;
    static unsigned currentPhase;
    static perfStreamHandle pstream;
    static metricHandle normalMetric;
    static bool PChyposDefined;
    static unsigned numMetrics;
    // for quick handling of new data
    static filteredDataServer *globalRawDataServer;
    static filteredDataServer *currentRawDataServer;
    static PCmetricInstServer *globalPCMetricServer;
    $signore    
};
