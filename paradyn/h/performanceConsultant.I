
/*
 * Interface between the Performance Consultant and the rest of paradyn.
 *
 * $Log: performanceConsultant.I,v $
 * Revision 1.2  1994/02/02 00:43:20  hollings
 * Added Auto Refinement options.
 *
# Revision 1.1  1994/02/01  18:47:35  hollings
# Upgrades for performanceConsultant thread.
#
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
    $version 1;
    void printTestStatus();
    $array SHNptr getWhyRefinements(searchHistoryNode *);
    $array SHNptr getWhenRefinements(searchHistoryNode *);
    $array SHNptr getWhereRefinements(searchHistoryNode *);
    $array SHNptr getAllRefinements(searchHistoryNode *);
    searchHistoryNode *getCurrentRefinement();
    void printSHGNode(searchHistoryNode *);
    void changeCurrentRefinement(int);
    void printSHGList();
    void search(Boolean);
    void autoRefine(int limit);
    void createInterval(int, timeStamp, timeStamp, char*);
    void doRefine($array int ids);
    void resetRefinement();
};
