
/*
 * Interface between the Performance Consultant and the rest of paradyn.
 *
 * $Log: performanceConsultant.I,v $
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
typedef searchHistoryNode *SHNptr;

typedef struct int_Array {
  int count;
  int *data;
} int_Array;

typedef struct SHNptr_Array {
  int count;
  SHNptr *data;
} SHNptr_Array;

$remote performanceConsultant {
    $base 6000;
    $version 5;

    // for all of the configured tests print their status.
    //  This is intended for debugging only.
    void printTestStatus();

    // enumerate all of the refinements.
    SHNptr_Array getWhyRefinements(searchHistoryNode *);
    SHNptr_Array getWhenRefinements(searchHistoryNode *);
    SHNptr_Array getWhereRefinements(searchHistoryNode *);
    SHNptr_Array getAllRefinements(searchHistoryNode *);

    // return the current refinement node.
    searchHistoryNode *getCurrentRefinement();
    int getCurrentNodeId();

    // all initialization for new search; checks first if search already
    // active for specified phase, to enforce 1 search/phase rule
    // this calls ui->initSHG which opens new display
    $async void newSearch(phaseType pt);

    void printSHGNode(searchHistoryNode *);
    void printSHGList();

    // try to make automated refinements.  Up to limit number of steps.
    $async void search(bool, int limit, int phaseID);

    // stop the automated search process in its tracks.
    $async void pauseSearch(int phaseID);

    // change the current refinement node.
    //  NOTE:  The node must be true and an ancestral path to the root must
    //         also be true or set will fail.
    //         As a side effect, searching is paused!!
    int setCurrentSHGnode(int node);

    // This doesn't work yet !!
    void doRefine(int_Array ids);

    // Really just setCurrentSHGnode(id of root node).
    $async void resetRefinement();
};
