// methods for class dag

#include <stdio.h>

extern "C" {
#include "tk.h"
}
#include "dagCompute.h"
#include "dag.h"
#include "paradyn/src/DMthread/DMresource.h"

char *defaultNStyleArgs[] = 
{};

char tcommand[300];
inline static void ClearHash(rNode ht[HASH_SIZE]) {
    memset(ht, '\0', sizeof(ht[0]) * HASH_SIZE);
}

void RePaintDag (dag *dagInst);


dag::dag (Tcl_Interp *nterp) 
{
  int retval;

  flags = 0;
  mode = ALL;
  node_internal_width = 2;
  node_internal_height = 2;
  graph_x_extent = 100;
  graph_y_extent = 100;
  graph_internal_width = 10;
  graph_internal_height = 10;
  refresh_flag = 0;
  expose_enable = 0;
  displayActive = 0;
  interp = nterp;
  tkwin = Tk_MainWindow(interp);

  graph = new _graph;
  graph->rSize = 0;
  graph->spaceX = 10;
  graph->spaceY = 5;
  graph->col_spacing = 5;
  graph->row_spacing = 5;
  graph->row_ratio = 0;
  ClearHash (graph->hash);
  graph->row = NULL;
  abstr = NULL;

  // initialize hash tables for eStyle and nStyle 
    
  retval = AddEStyle (0, 0, 0, 0, 0, NULL, "black", 'b', 2.0);
  if (retval != AOK)
#if UIM_DAG_DEBUG
    printf ("Error with default EStyle\n");
#endif
  retval = AddNStyle (0, "red", "black", NULL, "*-times-medium-r-normal--*-100-*", 
		      "black", 'r', 1.0);
  if (retval != AOK)
    printf ("Error with default NStyle\n");
  defaultEdgeStyle = 0;
  defaultNodeStyle = 0;

  /* initialize hash table for node lookup by Tk item ID */
  Tcl_InitHashTable (&TkItemTbl, TCL_ONE_WORD_KEYS);

}

inline static void FreeNull(void *p) {
    if (p != NULL)
	delete p;
}

inline static int HashValue(unsigned node) {
    return((unsigned)node % HASH_SIZE);
}

static rNode FindHash(rNode ht[HASH_SIZE], unsigned node) {
rNode h;
    for (h = ht[HashValue(node)]; h != NULL; h = h->hash)
	if (h->aNode == node)
	    break;
    return(h);
}

static void AddHash(rNode ht[HASH_SIZE], rNode newNode) {
    assert(newNode != NULL);
    assert(newNode->aNode);
    assert (FindHash(ht, newNode->aNode) == NULL);
    newNode->hash = ht[HashValue(newNode->aNode)];
    ht[HashValue(newNode->aNode)] = newNode;
}

dag::~dag(void)
{
  int r;
  if (displayActive) 
    this->destroyDisplay();

  for (r = 0; r < graph->rSize; r++) {
    rNode me = graph->row[r].first;
    do {
      rNode next = me->forw;
      FreeNull(me->upList);
      FreeNull(me->downList);
      FreeNull(me);
      me = next;
    } while (me != NULL);
  }
  if (r != 0) {
    ClearHash(graph->hash);
    graph->rSize = 0;
  }
  FreeNull(graph->row);
  delete graph;
}

void 
dag::setRowSpacing (int newspace)
{
  graph->row_spacing = newspace;
}

int 
dag::createDisplay (char *parentWindow)
{
  int nameLength;
  nameLength = strlen(parentWindow);
  dframe = new char[nameLength + 1];
  dcanvas = new char[nameLength + 5];
  strcpy (dframe, parentWindow);
  strcpy (dcanvas, parentWindow);
  if (nameLength == 1) {
    strcpy (dcanvas+nameLength, "_c_");
  } else {
    strcpy (dcanvas+nameLength, "._c_");
  }
#if UIM_DAG_DEBUG
  printf ("canvas name: %s\n", dcanvas);
  printf ("frame name: %s\n", dframe);
#endif
  if (Tcl_VarEval (interp, "setupDAG ", dframe, 0) == TCL_ERROR)
    printf ("UIMcreateDisplay: %s\n", interp->result);

  displayActive = 1;

  RePaintDag(this);
  flags = 0;
  return 1;
}

char *
dag::getCanvasName ()
{
  return dcanvas;
}

void 
dag::destroyDisplay ()
{
  Tcl_VarEval (interp, "destroy ", dframe, 0);
  displayActive = 0;
  delete [] dframe;
  delete [] dcanvas;
  // flush redraw request from idle queue, if one exists
  if (flags & REDRAW_PENDING) {
      flags &= ~REDRAW_PENDING;
      Tk_CancelIdleCall (RePaintDag, (ClientData) this);
    }
}

void 
dag::PrintGraph(FILE *f) 
{
    int r;
    rNode me;

    if (f == NULL) f = stderr;
    fprintf(f, "rSize = %d\n", graph->rSize);
    for (r = 0; r < graph->rSize; r++) {
      fprintf (f, "*ROW %d:\n", r);
      fprintf (f, "\ty y0 y1 y2 y3: %d %d %d %d %d\n",
	       graph->row[r].y, graph->row[r].y0, 
	       graph->row[r].y1, graph->row[r].y2, graph->row[r].y3);
        for (me = graph->row[r].first; me != NULL; me = me->forw) {
            PrintNode(f, graph, me);
	}
    }
}

void
dag::scheduleRedraw () 
{
  if (displayActive) {
    flags |= REPAINT_NEEDED;
    if (!(flags & REDRAW_PENDING)) {
      flags |= REDRAW_PENDING;
      Tk_DoWhenIdle (RePaintDag, (ClientData) this);
    }
  }
}

int 
dag::addTkBinding (char *bindCmd)
{
  if (Tcl_VarEval (interp, dcanvas, " bind ", bindCmd, 0) != TCL_OK) {
    printf ("%s\n", interp->result);
    return 0;
  }
  else
    return 1;
}

inline static Flag IsDummy(rNode n) {
    return(n->nType != APPL_NODE);
}

/*****************************************************************************/
/*  clear "selected" field for every node in this graph                      */
void ClearAllTags(rGraph g) 
{
int r;
    for (r = 0; r < g->rSize; r++) {
	rNode me = g->row[r].first;
	while (me != NULL) {
	    me->selected = 0;
	    me = me->forw;
	} 
      }
}
/*****************************************************************************/
/*  set "selected" field for every non-dummy node in this graph              */
void SetAllTags(rGraph g) 
{
int r;
    for (r = 0; r < g->rSize; r++) {
	rNode me = g->row[r].first;
	while (me != NULL) {
	    me->selected = 1;
	    me = me->forw;
	} 
      }
}
/* 
 * Change strtype to invisible for all nodes with selected set 
 *                   ~invisible for all non-dummy nodes with selected clear
 */
void
MakeTaggedInvisible (rGraph g, int exclusive)
{
  int r;
  rNode me;
  for (r = 0; r < g->rSize; r++) {
    me = g->row[r].first;
    while (me != NULL) {
      if (me->selected)
	me->strType = (StringType) (me->strType | INVISIBLE);
      else if ((!IsDummy(me)) && (exclusive))
	me->strType = (StringType) (me->strType & ~INVISIBLE);
      me = me->forw;
    }
  }
}

/*
 * Change strtype to "invisible" for all nodes with "selected" clear 
 *                to ~invisible for all non-dummy nodes with selected set
 */
void 
MakeTaggedVisible (rGraph g, int exclusive)
{
  int r;
  rNode me;
  for (r = 0; r < g->rSize; r++) {
    me = g->row[r].first;
    while (me != NULL) {
      if (me->selected)
	me->strType = (StringType) (me->strType & ~INVISIBLE);
      else if (( !IsDummy (me)) && (exclusive))
	me->strType = (StringType) (me->strType | INVISIBLE);
      me = me->forw;
    }
  }
}

void 
MakeAllVisible (rGraph g)
{
  int r;
  rNode me;
  for (r = 0; r < g->rSize; r++) {
    me = g->row[r].first;
    while (me != NULL) {
      if (!IsDummy (me)) {
	me->strType =(StringType) (me->strType & ~INVISIBLE);
	me = me->forw;
      }
    }
  }
}
/**********************/
/* Given a tag, gets a tcl list of all canvas item id's with this tag.
 *  then, for each such id, look up its dag node and set the selected
 *  field.
 */  
/*
void
TagCertainNodes (rGraph g, dagTestFunc test)
{
  int r;
  rNode me;
  for (r = 0; r < g->rSize; r++) {
    rNode me = g->row[r].first;
    while (me != NULL) {
      if (!IsDummy (me)) {
	if (me->aNode->test()) 
	  me->selected = 1;
	else 
	  me->selected = 0;
	me = me->forw;
      }
    }
  }
}

void
UnTagCertainNodes (rGraph g, dagTestFunc *test)
{
  int r;
  rNode me;
  for (r = 0; r < g->rSize; r++) {
    rNode me = g->row[r].first;
    while (me != NULL) {
      if (!IsDummy (me)) {
	if (me->aNode->test()) 
	  me->selected = 0;
	else 
	  me->selected = 1;
	me = me->forw;
      }
    }
  }
}
*/
/*****************************************************************************/
/* set "selected" field for this node and all descendants */
void
tagSubgraph (rNode root)
{

  int i;
  if (root != NULL) {
    root->selected = 1;
    for (i = 0; i < root->downSize; i++)
      tagSubgraph (root->downList[i].dest);
  }
}

void
dag::tagExceptSubgraph (rNode root)
{
  int i;
  rNode me;

  for (i = 0; i < root->row; i++) {
    me = graph->row[i].first;
    while (me) {
      me->selected = 1;
      me = me->forw;
    }
  }
  me = graph->row[root->row].first;
  while (me) {
    if (me != root)
      tagSubgraph(me);
    me = me->forw;
  }
}

rNode 
dag::getNodePtr (unsigned nodeID) 
{
  return FindHash(graph->hash, nodeID);
}    


class displayOption;
  
/****************
 * addDisplayOptions
 */
int
dag::addDisplayOption (optionType opt, unsigned rootID)
{
  displayOption *currOpt;
  rNode root;

  if ((root = getNodePtr(rootID)) == NULL)
    return ERR_NCONFIG_BADNODE;
  
  ClearAllTags (graph);
  if ((opt == ONLY) || (opt == EXCEPT)) 
    delete constraints;    // delete all previously active constraints
  currOpt = new displaySubgraphOption(root, opt);
  currOpt->next = constraints;       // insert new option at start of list
  constraints = currOpt;

  if ((opt == ONLY) || (opt == ADD)) {
    tagSubgraph (root);
    MakeTaggedVisible (graph, (opt == ONLY));
  } else  {
    tagSubgraph (root);
    MakeTaggedInvisible (graph, (opt == EXCEPT));
  }
  scheduleRedraw();                
  return 1;
}

void
dag::clearAllDisplayOptions()
{

  //delete constraints;    // delete all previously active constraints
  constraints = NULL;
  MakeAllVisible (graph);

  scheduleRedraw();                
}

int 
displayHeightOption::nodeVisible (rNode node, rGraph g)
{
  if (node->row < numRows) {
    if ((otype == ADD) || (otype == ONLY) ) 
      return 1;
    else
      return 0;
  } else {
    if ((otype == ADD) || (otype == ONLY) ) 
      return 0;
    else
      return 1;
  }
}

int displaySubgraphOption::nodeVisible (rNode node, rGraph g)
{
  Edge parente;
  int i, retVal = 0;
  if (node->row <= (getRoot())->row) {
    if ((otype == ADD) || (otype == ONLY))
      return 0;
    else 
      return 1;
  }
  parente = node->upList;
  for (i = 0; i < node->upSize; i++) {
    if (node->upList[i].dest->selected) {
      retVal = 1;
      break;
    }
  }
  return retVal;
}

nStyle *
dag::getNStyle (int style)
{
  pRec *entryPtr;
  if (style == -1)
    style = defaultNodeStyle;
  entryPtr = nStyleTbl.find((void *) style);
  if (entryPtr)
    return (nStyle *)entryPtr->optr;
  else return NULL;
}

/*
 * CreateNode
 *  Check arguments and call NewNode
 *  Returns errorcode or AOK
 */
int
dag::CreateNode (unsigned nodeID, int root, char *nodeLabel, int style, 
		 void *appRecPtr)
{
  rNode me;
  nStyle *newStyle;

  if (FindHash(graph->hash, nodeID) != NULL) {
    printf ("ERROR in CreateNode: a node with that nodeID already exists\n");
    return 0;
  }

  newStyle = getNStyle(style);
  if (newStyle == NULL) {
    printf ("ERROR in CreateNode: invalid style id\n");
    return 0;
  }
  me = new _node;
  me->nType = APPL_NODE;
  me->aNode = nodeID;
  me->aObject = appRecPtr;
  AddHash(graph->hash, me);
  me->ancestor = NULL;
  me->nodeStyle = newStyle;
  me->selected = 0;
  me->downList = NULL;
  me->upList = NULL;
  me->downSize = 0;
  me->upSize = 0;
  me->strType = ABBREVIATED;
  if (nodeLabel != NULL) 
    strncpy (me->string, nodeLabel, DAGNODEMAXSTR);
  else 
    strcpy (me->string, "");
  
  if (root) 
    AppendRow (graph, 0, me, NULL);
  else 
    me->row = -1;
  calcLabelSize (me);

  if (root) 
    scheduleRedraw();

/*  if ((mode == SELECTED) || (mode == NOT_SELECTED))
    SetVisibility (dagPtr, interp->result, &me->selected, &me->strType);
*/
  return 1;
}

eStyle *
dag::getEStyle (int style)
{
  pRec *entryPtr;
  if (style == -1)
    style = defaultEdgeStyle;
  entryPtr = eStyleTbl.find((void *) style);
  if (entryPtr)
    return (eStyle *)entryPtr->optr;
  else return NULL;
}
  
/*****************************************************************************/
/* 
 * AddEdge
 * if node "src" or "dst" doesn't exist, returns error.
 *  else creates necessary number of edges
 *   returns : 
 *      AOK                  edge added
 *      ERR_ADDE_BADSTYLE    requested edge style not defined
 *      ERR_ADDE_BADSRC      requested source node not found
 *      ERR_ADDE_BADDST      requested dest'n node not found
 */
int 
dag::AddEdge(unsigned src, unsigned dst, int styleID)
{
  rNode srcNode, dstNode;
  eStyle *style;

  if (src == dst) 
    return ERR_ADDE_BADDST;
  if ((srcNode = getNodePtr(src)) == NULL)
    return ERR_ADDE_BADSRC;
  if ((dstNode = getNodePtr(dst)) == NULL) 
    return ERR_ADDE_BADDST;

  style = getEStyle (styleID);
  if (style == NULL) 
    return ERR_ADDE_BADSTYLE;

  if (dstNode->row == -1)  {
    dstNode->row = srcNode->row + 1;
    AppendRow (graph, dstNode->row, dstNode, srcNode);
  }
  LayoutDummy(graph, srcNode, dstNode, style);
  scheduleRedraw();
  
  return AOK;

}

/*
 * AddEStyle
 * Adds an entry to the EStyle hash table for this dag widget.
 * Returns ERR_EST_BADLBL - style label already in use
 *         ERR_EST_BADARG - illegal argument
 *         AOK - style added
 */
int 
dag::AddEStyle (int styleID, int arrow, int ashape1, int ashape2, int ashape3,
		char *stipple, char *fill, char capstyle, float width)
{
  eStyle *eStylePtr;
  pRec *newEntry;
  if (! eStyleTbl.find((void *)styleID)) {
    eStylePtr = new eStyle;
    // verify all options are valid and 
    // fill in specified or default values for new edge style record */

    eStylePtr->styleID = styleID;

    if (arrow)
      strcpy(eStylePtr->arrow, "last");
    else
      strcpy(eStylePtr->arrow, "none");
    if (ashape2 == 0)
      strcpy(eStylePtr->arrowshape, "");
    else 
      sprintf (eStylePtr->arrowshape, "%d %d %d", ashape1, ashape2, ashape3);
    if (width <= 0)
      strcpy(eStylePtr->width, "1.0");
    else 
      sprintf (eStylePtr->width, "%.1f", width);
    if (capstyle == 'b')
      strncpy (eStylePtr->capstyle, "butt", 5);
    else if (capstyle == 'r')
      strncpy (eStylePtr->capstyle, "round", 6);
    else if (capstyle == 'p')
      strncpy (eStylePtr->capstyle, "projecting", 11);
    else {
      delete eStylePtr;
      return 0;
    }
    if (stipple == NULL) 
      strcpy(eStylePtr->stipple, "");
    else {
      Pixmap stip;
      stip = Tk_GetBitmap (interp, tkwin, stipple);
      if (stip == None) {
	delete eStylePtr;
	return 0;
      } else {
	strcpy(eStylePtr->stipple, stipple);
      }
    }
    if (fill == NULL)
      strcpy(eStylePtr->fill, "black");
    else {
      XColor *col;
      col = Tk_GetColor (interp, tkwin, fill);
      if (col == NULL) {
	delete eStylePtr;
	return 0;
      } else {
	strcpy (eStylePtr->fill, fill);
      }
    }
  newEntry = new pRec;
  newEntry->id = styleID;
  newEntry->optr = eStylePtr;
  eStyleTbl.add(newEntry, (void *)newEntry->id);

#ifdef DEBUG
    fprintf (stderr, "Estyle %d created w/: %s %s %s %s %s %s\n",
	     eStylePtr->styleID, eStylePtr->arrow, eStylePtr->arrowshape, 
	     eStylePtr->fill, eStylePtr->stipple, eStylePtr->width, 
	     eStylePtr->capstyle);
#endif
    return AOK;
  }
  return ERR_EST_BADLBL;      // styleID already in use
}

/*
 * AddNStyle
 * Adds an entry to the NStyle hash table for this dag widget.
 * Returns ERR_NST_BADLBL - style label already in use
 *         ERR_NST_BADARG - illegal argument
 *         ERR_NST_BADFONT - illegal X font specification
 *         AOK - style added
 */
int 
dag::AddNStyle (int styleID, const char *bg, const char *outline, char *stipple,
		const char *font, const char *text, char shape, float width)
{
  nStyle *nStylePtr, *hnStylePtr;
  pRec *newEntry;

  if (! nStyleTbl.find((void *)styleID)) {
    nStylePtr = new nStyle;
    
    if (width <= 0)
      strcpy(nStylePtr->width, "1.0");
    else 
      sprintf (nStylePtr->width, "%.1f", width);

    if (bg == NULL)
      strcpy(nStylePtr->bg, "yellow");
    else {
      XColor *col;
      col = Tk_GetColor (interp, tkwin, bg);
      if (col == NULL) {
	delete nStylePtr;
	return 0;
      } else {
	strcpy (nStylePtr->bg, bg);
      }
    }
    if (outline == NULL)
      strcpy(nStylePtr->outline, "black");
    else {
      XColor *col;
      col = Tk_GetColor (interp, tkwin, outline);
      if (col == NULL) {
	delete nStylePtr;
	return 0;
      } else {
	strcpy (nStylePtr->outline, outline);
      }
    }
    if (stipple == NULL) 
      strcpy(nStylePtr->stipple, "");
    else {
      Pixmap stip;
      stip = Tk_GetBitmap (interp, tkwin, stipple);
      if (stip == None) {
	delete nStylePtr;
	return 0;
      } else {
	strcpy(nStylePtr->stipple, stipple);
      }
    }
    if (text == NULL)
      strcpy(nStylePtr->text, "black");
    else {
      XColor *col;
      col = Tk_GetColor (interp, tkwin, text);
      if (col == NULL) {
	delete nStylePtr;
	return 0;
      } else {
	strcpy (nStylePtr->text, text);
      }
    }
    if (shape == 'r')
      strcpy(nStylePtr->shape, "rectangle");
    else {
      delete nStylePtr;
      return ERR_NST_BADSHAPE;
    }
    if (font == NULL)
      strcpy(nStylePtr->font, "-Adobe-times-medium-r-normal--*-100*");
    else
      strcpy (nStylePtr->font, font);
    nStylePtr->fontstruct = Tk_GetFontStruct (interp, tkwin, 
					      nStylePtr->font);
    if (nStylePtr->fontstruct == NULL)   {  /* not a valid font */
      delete nStylePtr;
      return ERR_NST_BADFONT;
    }
    nStylePtr->styleID = styleID;

    newEntry = new pRec;
    newEntry->id = styleID;
    newEntry->optr = nStylePtr;
    nStyleTbl.add(newEntry, (void *)newEntry->id);

#ifdef DEBUG
    fprintf (stderr, "Nstyle %d created w/: %s %s %s %s %s %s %s\n",
	     nStylePtr->styleID,
	     nStylePtr->bg, nStylePtr->stipple, nStylePtr->width, 
	     nStylePtr->outline, nStylePtr->font, nStylePtr->text,
	     nStylePtr->shape);
#endif
    // create style for highlighted nodes
    hnStylePtr = new nStyle;
    strcpy (hnStylePtr->bg, nStylePtr->text);
    strcpy (hnStylePtr->outline, nStylePtr->outline);
    strcpy(hnStylePtr->stipple, nStylePtr->stipple);
    strcpy(hnStylePtr->width, nStylePtr->width);
    strcpy(hnStylePtr->font, nStylePtr->font);
    strcpy(hnStylePtr->text, nStylePtr->bg);
    hnStylePtr->styleID = nStylePtr->styleID + 1000;
    hnStylePtr->fontstruct = nStylePtr->fontstruct;
    strcpy(hnStylePtr->shape, nStylePtr->shape);
    
    newEntry = new pRec;
    newEntry->id = styleID + 1000;
    newEntry->optr = hnStylePtr;
    nStyleTbl.add(newEntry, (void *)newEntry->id);

    return AOK;
  }
  return ERR_NST_BADLBL;
}


/*****************************************************************************/
/* calcLabelSize
   uses Xfontstruct info to calculate the width and height for the node based 
    on its label
 */
void 
dag::calcLabelSize (_node *node)
{

  int width = 0;
  int height = 0;
  int h;
  char *label = node->string;

  h = node->nodeStyle->fontstruct->ascent 
    + node->nodeStyle->fontstruct->descent;
  while (label[0] != '\0') {
    int wid, z;
    for (z = 0; label[z] != '\n' && label[z] != '\0'; z++)
      ;       /* find newline */
    wid = XTextWidth (node->nodeStyle->fontstruct, label, z);
    if (wid > width) 
      width = wid;
    height = height + h;
    label = label + z + (label[z] == '\0' ? 0 : 1);
  }
  if (width == 0) {
    width = 1;
    if (height == 0) 
      height = h;
  }
       /** note:  add highlight thickness here? **/
  node->sWidth = width + 1 + 2 * (node_internal_width);
  node->sHeight = height + 1 + 2 * (node_internal_height);

  if (node->upSize > 1 || node->downSize > 1) {
    int x1 = graph->spaceY * node->upSize;
    int x2 = graph->spaceY * node->downSize;
    if (x1 > x2) 
      x2 = x1;
    if (x2 > node->sWidth)
      node->sWidth = x2;
  }
}
 

/********
 * configureNode
 * Cange the label and/or style for an existing node.  Schedules redraw if 
 *  change succeeds.
 * Returns:
 *      ERR_NCONFIG_BADNODE   nodeID not found
 *      ERR_NCONFIG_BADSTYLE  styleID not found
 *      AOK                   change made; redraw scheduled
 * 
 * flag set to 1 if full recalc/redraw needs to be scheduled; else 0
 */

int
dag::configureNode (unsigned nodeID, char *label, int styleID)
{
  rNode me;
  nStyle *newStyle;

     /* verify valid nodeID */
  me = FindHash(graph->hash, nodeID);
  if (me == NULL)
    return ERR_NCONFIG_BADNODE;

  newStyle = getNStyle (styleID);
  if (newStyle == NULL)
    return ERR_NCONFIG_BADSTYLE;
  me->nodeStyle = newStyle;
  if (label != NULL) {
    strncpy (me->string, label, DAGNODEMAXSTR);
  }    
  calcLabelSize (me);         // record new label size
   // schedule redraw
  if (displayActive && (me->row != -1)) {
    flags |= REPAINT_NEEDED;
    if (!(flags & REDRAW_PENDING)) {
      flags |= REDRAW_PENDING;
      Tk_DoWhenIdle (RePaintDag, (ClientData) this);
    }
  }
  return AOK;
}

/* 
 * NOTE: this version with existing node pointer and NO REDRAW SCHEDULED
 * label recalculated only if new label string provided
 */
int
dag::configureNode (rNode me, char *label, int styleID)
{
  nStyle *newStyle;

  newStyle = getNStyle (styleID);
  if (newStyle == NULL)
    return ERR_NCONFIG_BADSTYLE;
  me->nodeStyle = newStyle;
  if (label != NULL) {
    strncpy (me->string, label, DAGNODEMAXSTR);
    calcLabelSize (me);         // record new label size
  }    
  return AOK;
}

int
dag::highlightAllRootNodes ()
{
  rNode me;

  for (me = graph->row[0].first; me; me = me->forw) {
    if (me->nodeStyle->styleID < 1000) 
      highlightNode (me);
  }

  return AOK;
}
  
int
dag::clearAllHighlighting ()
{
  rNode me;
  int r;
  for (r = 0; r < graph->rSize; r++) {
    me = graph->row[r].first;
    while (me != NULL) {
      if (me->nodeStyle->styleID >= 1000) 
	unhighlightNode (me);
      me = me->forw;
    }
  }
  return AOK;
}

int
dag::isHighlighted (rNode curr)
{
  if (curr->nodeStyle->styleID > 1000)
    return 1;
  else
    return 0;
}

void
dag::listHighlightedNodes (rNode curr, vector<unsigned> *tokenlist)
{
  if (isHighlighted (curr)) {
    *tokenlist += curr->aNode;
  }
  for (int i = 0; i < curr->downSize; i++)
    listHighlightedNodes (curr->downList[i].dest, tokenlist);
}

vector<numlist> *
dag::listAllHighlightedNodes ()
{
  vector<numlist> *result;
  vector<unsigned> *buildup;
  result = new vector<numlist>;
  rNode me = graph->row[0].first;

  while (me != NULL) {
      buildup = new vector<unsigned>;
      // get all highlighted child nodes of current parent node 
      listHighlightedNodes (me, buildup); 

      // add the curr parent node if no nodes have been highlighted 
      if (buildup->size() == 0){
          *buildup += me->aNode;   
	  *result += *buildup;
      }
      // add all hightlighted nodes
      else {  
          *result += *buildup;
      }
      /*
      for(unsigned i=0; i < buildup->size(); i++){
           printf("resource %d: %d %s\n",i,(*buildup)[i],
			resource::getName((*buildup)[i]));
      }
      */
      buildup = 0;
      me = me->forw;
  }
  buildup = 0;
  return result;
}
    
int
dag::highlightNode (unsigned nodeID)
{
  rNode me;
  nStyle *newStyle;
  unsigned newStyleID;

     /* verify valid nodeID */
  me = FindHash(graph->hash, nodeID);
  if (me == NULL) 
    return ERR_NCONFIG_BADNODE;

  /* store new style in node record */
  if (me->nodeStyle->styleID < 1000)  
    newStyleID = me->nodeStyle->styleID + 1000;
  else
    newStyleID = me->nodeStyle->styleID - 1000;
  newStyle = getNStyle (newStyleID);
  me->nodeStyle = newStyle;

  /* directly change display using canvas configure */
  sprintf (tcommand, "%s itemconfigure n%d -fill %s",
	   dcanvas, nodeID, newStyle->bg);
  Tcl_VarEval (interp, tcommand, 0);
  sprintf (tcommand, "%s itemconfigure t%d -fill %s",
	   dcanvas, nodeID, newStyle->text);
  Tcl_VarEval (interp, tcommand, 0);
  return AOK;
}
 
int
dag::highlightNode (rNode me)
{
  nStyle *newStyle;
  unsigned newStyleID;

  /* store new style in node record */
  if (me->nodeStyle->styleID < 1000)  
    newStyleID = me->nodeStyle->styleID + 1000;
  else
    newStyleID = me->nodeStyle->styleID - 1000;
  newStyle = getNStyle (newStyleID);
  me->nodeStyle = newStyle;

  /* directly change display using canvas configure */
  sprintf (tcommand, "%s itemconfigure n%d -fill %s",
	   dcanvas, me->aNode, newStyle->bg);
  Tcl_VarEval (interp, tcommand, 0);
  sprintf (tcommand, "%s itemconfigure t%d -fill %s",
	   dcanvas, me->aNode, newStyle->text);
  Tcl_VarEval (interp, tcommand, 0);
  return AOK;
}
 
int
dag::unhighlightNode (unsigned nodeID)
{
  rNode me;
  nStyle *newStyle;

     /* verify valid nodeID */
  me = FindHash(graph->hash, nodeID);
  if (me == NULL)
    return ERR_NCONFIG_BADNODE;

  if (me->nodeStyle->styleID >= 1000) {

    newStyle = getNStyle (me->nodeStyle->styleID - 1000);
    me->nodeStyle = newStyle;

    /* directly change display using canvas configure */
    sprintf (tcommand, "%s itemconfigure n%d -fill %s",
	     dcanvas, nodeID, newStyle->bg);
    Tcl_VarEval (interp, tcommand, 0);
    sprintf (tcommand, "%s itemconfigure t%d -fill %s",
	     dcanvas, nodeID, newStyle->text);
    Tcl_VarEval (interp, tcommand, 0);
  }
  return AOK;
}
 
int
dag::unhighlightNode (rNode me)
{
  nStyle *newStyle;

  if (me->nodeStyle->styleID >= 1000) {

    newStyle = getNStyle (me->nodeStyle->styleID - 1000);
    me->nodeStyle = newStyle;

    /* directly change display using canvas configure */
    sprintf (tcommand, "%s itemconfigure n%d -fill %s",
	     dcanvas, me->aNode, newStyle->bg);
    Tcl_VarEval (interp, tcommand, 0);
    sprintf (tcommand, "%s itemconfigure t%d -fill %s",
	     dcanvas, me->aNode, newStyle->text);
    Tcl_VarEval (interp, tcommand, 0);
  }
  return AOK;
}
 
/* don't forget this is a dag; all this routine does is grab one ancestor
 * from row 0 without being particular.  Used in resource selection.
 */
rNode
findaroot (rNode x)
{
  rNode tptr;
  for (tptr = x; tptr->row > 0; tptr = tptr->upList->dest) 
    ;
  return tptr;
}
    
void 
dag::unHighlightchildren (rNode curr) 
{
  if (isHighlighted (curr))
    unhighlightNode (curr->aNode);
  for (int i = 0; i < curr->downSize; i++)
    unHighlightchildren (curr->downList[i].dest);
}

int
dag::constrHighlightNode (unsigned nodeID)
{
  rNode hnode, rnode;
  if ( (hnode = getNodePtr(nodeID)) == NULL)
    return 0;
  if ( (rnode = findaroot (hnode)) == NULL)
    return 0;
  unHighlightchildren (rnode);
  if (highlightNode (nodeID))
    return 1;
  else
    return 0;
}

int
dag::configureEdge (unsigned srcID, unsigned dstID, int styleID)
{
  rNode me;
  eStyle *newStyle;
  Edge list;
  int e;

     /* verify valid source node ID */
  me = FindHash(graph->hash, srcID);
  if (me == NULL)
    return ERR_ECONFIG_BADNODE;

  newStyle = getEStyle (styleID);
  if (newStyle == NULL)
    return ERR_ECONFIG_BADSTYLE;

  // find edge 
  for (e = 0, list = &me->downList[e]; e < me->downSize; e++, list++) {
    if (list->dest->aNode == dstID)
      break;
  }
  if ((e ==  me->downSize) && (list->dest->aNode != dstID))
    return ERR_ECONFIG_BADNODE;
  list->style = newStyle;

  // schedule redraw
  if (displayActive) {
    flags |= REPAINT_NEEDED;
    if (!(flags & REDRAW_PENDING)) {
      flags |= REDRAW_PENDING;
      Tk_DoWhenIdle (RePaintDag, (ClientData) this);
    }
  }
  return AOK;
}

/******************************************************************************/
/*   Paint Functions */
/******************************************************************************/
inline static int Max(int a, int b) { return(a >= b ? a : b); }

inline static Flag IsStringType(StringType s, StringType t) {
    return((s & t) != 0);
}
inline static Flag IsElided(rNode n) {
    return(IsStringType(n->strType, ELIDED));
}
inline static Flag IsInvisible(rNode n) {
    return(IsStringType(n->strType, INVISIBLE));
}
inline static Flag IsAncestor(rNode src, rNode dst) {
    while ((src && dst && (src->row > dst->row)))
	src = src->ancestor;
    return(src == dst);
}

/*****************************************************************************/
/*  PaintEdge
 *  adds line items to widget's canvas given x and y coordinates.
 */

/* ARGSUSED */
void PaintEdge(Tcl_Interp *interp, char *dcanvas, 
	       int x1, int y1,        /* raw "from" coordinates */ 
	       int x2, int y2,        /* raw "to" coordinates  */
	       ArrowType arrow,       /* none, first, last, or both */ 
	       eStyle *styleRec)        /* display characteristics */
	      
{
       /* create line on canvas */
if (arrow == OUT_ARROW)

  sprintf (tcommand, 
	   "%s create line %d %d %d %d -arrow %s -fill %s -width %s -tags edge",
	   dcanvas, 
	   x1, y1, x2, y2, 
	   styleRec->arrow, 
	   styleRec->fill,
	   styleRec->width);
else
  sprintf (tcommand, 
	   "%s create line %d %d %d %d -fill %s -width %s -tags edge",
	   dcanvas, 
	   x1, y1, x2, y2, 
	   styleRec->fill,
	   styleRec->width);
#if UIM_DAG_DEBUG
	   fprintf (stderr, tcommand);
#endif
if (Tcl_VarEval (interp, tcommand, 0) != TCL_OK) 
     printf ("Error painting Edge: %s\n", interp->result);

}


void 
dag::AdjustSize(int width, int height) 
{ 
    width = width + graph_internal_width;
    height += graph_internal_height;
    if (graph_x_extent < width) {
        graph_x_extent = width;
	sprintf(tcommand, "%s config -scrollregion \"0 0 %d %d\"",
	    dcanvas, graph_x_extent, graph_y_extent);
	Tcl_VarEval (interp, tcommand, 0);
      }
    if (graph_y_extent < height)  {
        graph_y_extent = height;
	sprintf(tcommand, "%s config -scrollregion \"0 0 %d %d\"",
	    dcanvas, graph_x_extent, graph_y_extent);
	Tcl_VarEval (interp, tcommand, 0);
      }
}


/*****************************************************************************/
/* PaintNode
 * creates a shape item and a text item for "node" at given coordinates
 * returns standard TCL result 
 */
int 
dag::PaintNode(nStyle *styleRec, 
	      rNode node,  
	      int x, int y, 
	      int width, int height) 
{
#if UIM_DAG_DEBUG
  fprintf (stderr, "%d: style: %d bg: %s outline: %s stipple: %s width: %s\n",
	   node->aNode, styleRec->styleID, styleRec->bg, 
	   styleRec->outline, styleRec->stipple,
	   styleRec->width);
#endif
  sprintf (tcommand, 
	   "%s create rectangle %d %d %d %d -fill %s -outline %s -width %s -tags \"n%d node\"",
	   dcanvas,
	   x, y, x+width, y+height,
	   styleRec->bg,
	   styleRec->outline,
	   styleRec->width,
	   node->aNode);
#if UIM_DAG_DEBUG
  fprintf (stderr, tcommand);
#endif

  if (Tcl_VarEval (interp, tcommand, 0) != TCL_OK)
    return 0;

#if UIM_DAG_DEBUG
  fprintf (stderr, "%s: x:%d y:%d text: %s\n", 
                    dcanvas, x, y, styleRec->text);
  fprintf (stderr, "    font: %s nodeID: %d\n", styleRec->font, node->aNode);
#endif
  sprintf (tcommand,
    "%s create text %d %d -text {%s} -anchor n -fill %s -font %s -tags \"t%d text\"",
	   dcanvas,
	   x + width/2, y + node_internal_height, 
	   node->string,
	   styleRec->text,
	   styleRec->font,
	   node->aNode);

  if (Tcl_VarEval (interp, tcommand, 0) != TCL_OK) 
    return 0;

  this->AdjustSize((x + width), (y + height));
  return(1);
}

/*****************************************************************************/
/*
 * PaintEdges
 * Calls PaintEdge for all of a node's edges
 */
void 
dag::PaintEdges(rNode me, int offset) 
{
  int e;
  Edge list;
  Row r = graph->row + (me->row);
  assert( !IsInvisible(me) && me->strType != INVALID);
  if (! IsElided(me)) {
      for (e = 0, list = &me->downList[e]; e < me->downSize; e++, list++) {
	  ArrowType arrow;
	  if (IsInvisible(list->dest))
	      continue;
	  if (IsElided(list->dest)) {
	      if (! IsAncestor(list->dest, me))
		  continue;
	      if (e > 0)
		  if (list->dest == (list-1)->dest)
		      continue;
	    }
	  arrow = (list->eType == OUT_ARROW ? OUT_ARROW : NO_ARROW);
	  PaintEdge(interp, dcanvas, list->x1 + offset, 
		    r->y + me->sHeight, 
		    list->x2 + offset, r->y3,
		    arrow, list->style);
	}
    }
}


/* 
 *  calculate an offset to add to x coordinates for centering effect.
 */
int 
dag::centeringOffset ()
{
  int r;
  int nodecnt = 0;
  rNode me;
  rNode firstnode = NULL;

     /* search for first visible node, from root.  need x value and 
         nodecount for the row. */

  for (r = 0; r < graph->rSize; r++) {
    me = graph->row[r].first;
    while (me != NULL) {
      if (!IsDummy (me) && !IsInvisible (me)) {
	nodecnt++;
	if (firstnode == NULL)
	  firstnode = me;
      }
      me = me->forw;
    }
    if (nodecnt)
      break;
  }
  if (firstnode == NULL)     /* no nodes or no visible nodes */
    return 0;
#if UIM_DAG_DEBUG
	fprintf (stderr, "window width = %d; x coord = %d.\n", 
	   Tk_ReqWidth (Tk_NameToWindow (interp, dcanvas, tkwin)), 
		     firstnode->x);
#endif
	return Max (0, (Tk_ReqWidth (tkwin) / (nodecnt + 1)) 
    - firstnode->x);
}

/*
 * PaintDag
 *  Calls PaintNode and PaintEdge as needed to draw the graph onto 
 *   a blank canvas.
 *    (replaces RefreshGraph, DisplayNode)
 */

void 
dag::PaintDag() 
{
  int r;
  rNode me;
  rGraph g = graph;
  Row row = g->row;
  int retval;
  int offset;      // number of pixels to add to x coordinate for centering

  if (g->rSize != 0) {
    offset = this->centeringOffset(); 
    for (r = g->rSize - 1; r >= 0; r--) {
      for (me = g->row[r].last; me != NULL; me = me->back) {

	// if this is dummy node, only draw edge
	if (IsDummy(me)) {
	  
	  if (! IsInvisible(me->downList[0].dest)) {
	    PaintEdge(interp, dcanvas,   
		      me->x + offset, row->y, me->x + offset, 
		      row->y + row->rHeight,
		      me->downList[0].eType, 
		      me->downList[0].style);
	  }
	  // otherwise, draw node and edges 
	} else if (!IsInvisible(me)) { 
	  retval = this->PaintNode(me->nodeStyle, me, me->x + offset, 
				   g->row[me->row].y, me->sWidth, 
				   me->sHeight);
	  this->PaintEdges(me, offset);
	}
      }
    }
  }
}

void RePaintDag (dag *dagInst)
{
  Tcl_VarEval (dagInst->interp, dagInst->dcanvas, " delete all", 0);

  RelativeSweep(dagInst->graph);     /* recompute all node positions */
  AbsoluteSweep(dagInst->graph);
  EdgePositions(dagInst->graph);  /* also computes heights */

  dagInst->PaintDag();
  dagInst->flags = 0;
}
