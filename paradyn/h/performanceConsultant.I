
/*
 * Interface between the Performance Consultant and the rest of paradyn.
 *
 * $Log: performanceConsultant.I,v $
 * Revision 1.6  1994/05/06 06:50:45  karavan
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
#include "dataManager.h"

//
// Forward decl of classes that are passed out the interface.
//
class searchHistoryNode;
typedef searchHistoryNode *SHNptr;

$remote performanceConsultant {
    $base 6000;
    $version 2;

    // for all of the configured tests print their status.
    //  This is intended for debugging only.
    void printTestStatus();

    // enumerate all of the refinements.
    $array SHNptr getWhyRefinements(searchHistoryNode *);
    $array SHNptr getWhenRefinements(searchHistoryNode *);
    $array SHNptr getWhereRefinements(searchHistoryNode *);
    $array SHNptr getAllRefinements(searchHistoryNode *);

    // return the current refinement node.
    searchHistoryNode *getCurrentRefinement();
    int getCurrentNodeId();

    // make PC available to user
    $async void startSHG();
    void printSHGNode(searchHistoryNode *);
    void printSHGList();

    // try to make automated refinements.  Up to limit number of steps.
    $async void search(Boolean, int limit);

    // stop the automated search process in its tracks.
    $async void pauseSearch();

    // change the current refinement node.
    //  NOTE:  The node must be true and an ancestral path to the root must
    //         also be true or set will fail.
    //         As a side effect, searching is paused!!
    int setCurrentSHGnode(int node);

    void createInterval(int, timeStamp, timeStamp, char*);

    // This doesn't work yet !!
    void doRefine($array int ids);

    // Really just setCurrentSHGnode(id of root node).
    $async void resetRefinement();
};
