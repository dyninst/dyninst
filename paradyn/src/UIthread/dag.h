#ifndef _dag_h
#define _dag_h

#define UIM_DAG_DEBUG 0

#include <stdio.h>
#include "util/h/list.h"
#include "util/h/Vector.h"
#include "util/h/String.h"
#include "dagCompute.h"
#include "../pdMain/paradyn.h"

class resourceList;

#define MAXSELECTTAGLEN 20
typedef enum {
    ALL = 0, SUBGRAPH, SELECTED, SEL, NOT_SELECTED
} DisplayMode;

class dagNode;
typedef int (dagNode::*dagTestFunc) ();

class dag;         // forward declaration for displaySubgraphOption

typedef unsigned nodeIdType;
typedef vector<nodeIdType> numlist;

typedef enum {
  ONLY, EXCEPT, ADD, SUBTRACT
} optionType;

typedef enum {
  SUBGRPH, TEST, HEIGHT
} selType;

class displayOption {
 protected:
  optionType otype;
  displayOption *next;
 public:
  friend class dag;
  displayOption (optionType type) {next = NULL; otype = type;}    
  ~displayOption() {delete next;}
  virtual int nodeVisible (rNode node, rGraph g) {return 1;}
  displayOption *getNextOption () {return next;}
};
class displayTestOption: public displayOption {
  dagTestFunc test;
 public:
  displayTestOption (dagTestFunc testfunc, optionType type) 
    :displayOption (type) {
      test = testfunc;
    }
//  int nodeVisible (rNode node);
};
class displayHeightOption: public displayOption {
  int numRows;
 public:
  displayHeightOption (int rows, optionType type) 
    :displayOption (type) {
     if ((type == EXCEPT) || (type == SUBTRACT))
       return;
      numRows = rows;
    }
  int nodeVisible (rNode node, rGraph g);
  int getHeight () {return numRows;}
};
class displaySubgraphOption: public displayOption {
  rNode root;
 public:
  displaySubgraphOption (rNode rootNode, optionType type) 
    :displayOption (type) {
      root = rootNode;
    }
  int nodeVisible (rNode node, rGraph g);
  rNode getRoot() {return root;}
};

class pRec {
public:
  int id;
  void *optr;
  void print() {printf ("%d\n", id);}
};

class dag {
    // from dagLayout
    rGraph graph;

    // from dag display
    int flags;			/* Various flags;  see below for */

    char *dcanvas;	        /* canvas used for drawing the dag. */
    char *dframe;               /* frame which contains the dag */

    HTable<pRec *> nStyleTbl;      /* lookup symbolic node names */
    HTable<pRec *> eStyleTbl;      /* lookup symbolic edge names */

    Tcl_HashTable TkItemTbl;      /* lookup node by tkitemID */
    int defaultNodeStyle;
    int defaultEdgeStyle;

    int		node_internal_width;
    int		node_internal_height;
    int		graph_internal_width;
    int		graph_internal_height;
    int		graph_x_extent;
    int		graph_y_extent;

    displayOption *constraints;
    DisplayMode mode;                  /* special display states */
    dagTestFunc selectFunc;   /* tag for current display mode */
    int			str_type;
    StringType		string_type;	/* non-INVALID calls ToggleStringType*/
    int		super_graph;
    int		refresh_flag;	/* True activates RefreshEdges() */
    int		expose_enable;	/* True allows GraphExpose */
    int		radioGraph;	/* Can only one node be selected */
    int		row_arg;
    string      abstr;

    int displayActive;      // set if dag currently displayed onscreen 
    Tcl_Interp *interp;			      
    Tk_Window tkwin;

    friend void RePaintDag (dag *dagInst);
    friend int processVisiSelectionCmd (ClientData clientData, 
					Tcl_Interp *interp, 
					int argc, 
					char *argv[]);
    friend void getSubtreeSelections (rNode curr, resourceList *selection);
    int centeringOffset (); 
    void PaintEdges(rNode me, int offset); 
    int PaintNode(nStyle *styleRec, 
		  rNode node,  
		  int x, int y, 
		  int width, int height); 
    void AdjustSize(int width, int height); 
    void calcLabelSize (rNode node);
    void scheduleRedraw();
    void tagExceptSubgraph (rNode root);
    void PaintDag();
    rNode getNodePtr (unsigned nodeID);
    // returns list of nodeID's for all highlighted nodes within a 
    // subtree; uses dfs
    void listHighlightedNodes (rNode curr, vector<unsigned> *tokenlist);


  public:
    dag(Tcl_Interp *nterp);
    ~dag();
    void setRowSpacing (int newspace); 
    int createDisplay (char *parentWindow);
    void destroyDisplay ();
    void PrintGraph(FILE *f);
    int AddEStyle (int styleID, int arrow, int ashape1, int ashape2, 
		   int ashape3, char *stipple, char *fill, 
		   char capstyle, float width);
    int AddNStyle (int styleID, const char *bg, const char *outline, char *stipple,
		   const char *font, const char *text, char shape, float width);
    int CreateNode (unsigned nodeID, int root, char *nodeLabel, int style,
		    void *appRecPtr);
    int AddEdge (unsigned src, unsigned dst, int edge_style);
    eStyle *getEStyle (int style);
    nStyle *getNStyle (int style);
    int configureNode (unsigned nodeID, char *label, int styleID);
    int configureNode (rNode me, char *label, int styleID);
    int configureEdge (unsigned srcID, unsigned dstID, int styleID);
    int highlightNode (unsigned nodeID);
    int unhighlightNode (unsigned nodeID);
    int highlightNode (rNode me);
    int unhighlightNode (rNode me);
    int clearAllHighlighting();
    int highlightAllRootNodes();
    int addTkBinding (char *bindCmd);
    int addDisplayOption (optionType opt, unsigned rootID);
    void clearAllDisplayOptions ();
    const char *getAbstraction(){ return(abstr.string_of()); }
    void setAbstraction(const char *name){ abstr = name; }
    char *getCanvasName ();
    int isHighlighted (rNode curr);
    int constrHighlightNode (unsigned nodeID);
    void unHighlightchildren (rNode curr); 
    vector<numlist> *listAllHighlightedNodes ();
  };

/*
 * Flag bits for Dags:
 *
 * REDRAW_PENDING:		Non-zero means a DoWhenIdle handler
 *				has already been queued to redraw
 *				this window.
 * REPAINT_NEEDED;		Need to call layout routine when redrawing.
 */

#define REDRAW_PENDING		1
#define REPAINT_NEEDED		2

/** Error Codes **/
#define AOK 1
#define ERR_EST_BADARG 2
#define ERR_EST_BADLBL 3
#define ERR_ADDE_BADSRC 4
#define ERR_ADDE_BADDST 5
#define ERR_ADDE_BADSTYLE 6
#define ERR_NCONFIG_BADNODE 7
#define ERR_NCONFIG_BADSTYLE 8
#define ERR_REDRAW_BADARG 9
#define ERR_REDRAW_BADNODE 10
#define ERR_NST_BADLBL 11
#define ERR_NST_BADARG 12
#define ERR_NST_BADFONT 13
#define ERR_NST_BADSHAPE 14
#define ERR_ECONFIG_BADNODE 15
#define ERR_ECONFIG_BADSTYLE 16


typedef struct eStyle {
  int styleID;      /* unique id for this style */
  char arrow[6];          /* none, first, last, or both -tk line semantics */
  char arrowshape[12];   
  char fill[25];
  char stipple[25];
  char width[6];
  char capstyle[11];
} eStyle;

typedef struct nStyle {
  char bg[80];        /* color to fill node background */
  char outline[80];
  char stipple[25];   
  char width[6];      
  char font[80];      /* font for label text */
  char text[80];      /* color for label text */
  int styleID;
  XFontStruct *fontstruct;      /* font record */
  char shape[10];
} nStyle;

#endif




