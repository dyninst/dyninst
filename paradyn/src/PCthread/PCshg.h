/*
 *
 * $Log: PCshg.h,v $
 * Revision 1.7  1994/07/25 04:47:11  hollings
 * Added histogram to PCmetric so we only use data for minimum interval
 * that all metrics for a current batch of requests has been enabled.
 *
 * added hypothsis to deal with the procedure level data correctly in
 * CPU bound programs.
 *
 * changed inst hypothesis to use observed cost metric not old procedure
 * call based one.
 *
 * Revision 1.6  1994/07/14  23:47:01  hollings
 * added flag to record if a SHG node has ever been true.
 *
 * Revision 1.5  1994/06/27  18:55:12  hollings
 * Added compiler flag to add SHG nodes to dag only on first evaluation.
 *
 * Revision 1.4  1994/05/19  00:00:31  hollings
 * Added tempaltes.
 * Fixed limited number of nodes being evaluated on once.
 * Fixed color coding of nodes.
 *
 * Revision 1.3  1994/04/21  04:58:08  karavan
 * Added changeStatus and changeActive member functions to searchHistoryNode.
 *
 * Revision 1.2  1994/02/03  23:27:03  hollings
 * Changes to work with g++ version 2.5.2.
 *
 * Revision 1.1  1994/02/02  00:38:20  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.6  1993/08/11  18:53:16  hollings
 * added cost function.
 *
 * Revision 1.5  1993/08/05  19:01:11  hollings
 * new includes
 *
 * Revision 1.4  1993/05/07  20:19:17  hollings
 * Upgrade to use dyninst interface.
 *
 * Revision 1.3  1993/01/28  19:32:14  hollings
 * make SearchHistoryGraph a pointer to a SHG node.
 *
 * Revision 1.2  1992/10/23  20:13:37  hollings
 * Working version right after prelim.
 *
 * Revision 1.1  1992/08/24  15:12:20  hollings
 * Initial revision
 *
 *
 * Interface to the search history graph.
 *
 */
#include <stdio.h>

#include "PCwhy.h"
#include "PCwhere.h"
#include "PCwhen.h"


class searchHistoryNode;
class hint;

typedef List<searchHistoryNode*> searchHistoryNodeList;

class hintList: public List<hint*> {
    public:
	void add(focus *f, char* message);
	void add(hypothesis *w, char* message);
	void add(timeInterval *ti, char* message);
};

class searchHistoryNode {
    public:
	searchHistoryNode(hypothesis*, focus*, timeInterval*);
	float cost();
	void print(int);
	Boolean print(int parent, FILE *fp);
	hypothesis *why;
	focus *where;
	timeInterval *when;
	Boolean ableToEnable;		// refine has no data.
	Boolean resetActive();
	Boolean marked;			// for print.
	searchHistoryNodeList *children;
	searchHistoryNodeList parents;
	void resetStatus();
	int nodeId;			// unique id for this node.
	char *name;			// name of this node.
	char *shortName;		// name of this node.
	hintList *hints;
	static stringPool shgNames;
	void changeStatus(Boolean newstat);
	void changeActive(Boolean newact);
	void changeTested(Boolean newtested);
	void changeSuppressed(Boolean nv) { suppressed = nv; }
	Boolean getActive() { return(active); }
	Boolean getStatus() { return(status); }
	Boolean getBeenTrue() { return(beenTrue); }
	Boolean getSuppressed() { return(suppressed); }
    private:
	void changeColor();		// update node color.
	Boolean beenActive;		// ever been tried
	Boolean active;			// currently at or along path to 	
					// root of item(s) under test.
	Boolean beenTested;	
	Boolean status;			// true not not true.
	Boolean beenTrue;		// has it ever been true??
	int style;			// current color
	Boolean suppressed;		// don't search here
};

searchHistoryNode *findAndAddSHG(searchHistoryNode *parent,
				 hypothesis *why, 
				 focus *where, 
				 timeInterval *when);


class hint {
    public:
        char *message;
        void print();
	// component to refine on.
	hypothesis *why;
	focus *where;
	timeInterval *when;
};

extern searchHistoryNode *SearchHistoryGraph;
extern searchHistoryNode *currentSHGNode;
extern searchHistoryNodeList allSHGNodes;
