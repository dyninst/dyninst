// dagCompute.C
//  new layout routines for dag class

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <assert.h>
#include "dagCompute.h"


#define MAX_SHORT	((short)((unsigned short)-1 >> 1))

/*****************************************************************************/

static void *Mal(size_t size) {
char *pointer = new char[size];
    if (pointer == NULL) {
	perror("malloc returned NULL");
	exit(-1);
    }
    memset(pointer, '\0', size);
    return(pointer);
}

inline static void FreeNull(void *p) {
    if (p != NULL)
	delete p;
}

static void *GrowArray(void *array, int *size, int element) {
void *temp = Mal((*size + 1) * element);
    memcpy(temp, array, *size * element);
    FreeNull(array);
    array = temp;
    ++*size;
    return(array);
}


/*****************************************************************************/
/*                                Layout                                     */
/*****************************************************************************/
inline static int Min(int a, int b) { return(a <= b ? a : b); }
inline static int Max(int a, int b) { return(a >= b ? a : b); }

inline static Flag IsDummy(rNode n) {
    return(n->nType != APPL_NODE);
}
inline static Flag IsStringType(StringType s, StringType t) {
    return((s & t) != 0);
}
inline static Flag IsVerbose(rNode n) {
    return(IsStringType(n->strType, VERBOSE));
}
inline static Flag IsElided(rNode n) {
    return(IsStringType(n->strType, ELIDED));
}
inline static Flag IsInvisible(rNode n) {
    return(IsStringType(n->strType, INVISIBLE));
}
inline static int PreSpacing(rNode me) {
    return(0);
}
inline static int PostSpacing(rNode me, rGraph g) {
    return(g->col_spacing);
}


void InsertRow (rGraph g, int r, rNode prev, rNode me) {
assert(r <= g->rSize);
    if (r == g->rSize) {  /* add a row */
	g->row = (Row) GrowArray(g->row, &g->rSize, sizeof(*g->row));
	me->forw = NULL;
	me->back = NULL;
	g->row[r].first = me;
	g->row[r].last = me;
	g->row[r].y3 = 0;
	/* rHeight and rWidth zeroed by Mal() in GrowArray() */
	me->placement = 1;
    } else if (prev == NULL ) {  /* prepend */
	assert(g->row[r].first != NULL);
	me->forw = g->row[r].first;
	me->back = NULL;
	g->row[r].first = me;
	me->forw->back = me;
	me->placement = me->forw->placement / 2;
    } else if (prev->forw == NULL) {  /* postpend */
	assert(g->row[r].first != NULL);
	me->forw = NULL;
	me->back = g->row[r].last;
	me->back->forw = me;
	g->row[r].last = me;
	me->placement = me->back->placement + 1;
    } else {  /* insert */
	rNode next;
	assert(g->row[r].first != NULL);
	me->forw = prev->forw;
	me->back = prev;
	prev->forw = me;
	me->forw->back = me;
	me->placement = (me->forw->placement + me->back->placement) / 2;
	if (me->placement >= me->forw->placement
		|| me->placement <= me->back->placement) {
	    /* recalibrate placements on this row */
	    g->row[r].first->placement = 1;
	    for (next = g->row[r].first->forw; next != NULL; next = next->forw)
		next->placement = next->back->placement + 1;
	}
    }
    me->row = r;
}

static void ComputeHeights(rGraph g, int r, int dx) {
int y;
int height = g->row[r].rHeight;
    /* node placement */
    if (r == 0)
	y = 15;    // leave a slight margin to the top of the enclosing frame
    else
	y = g->row[r-1].y + 1 + g->row[r-1].rHeight + g->spaceY
	  + Max(g->row_spacing, (int) (dx * g->row_ratio)) 
	    + g->spaceY + 1;
    g->row[r].y = y;

    /* vertical edges */
    if (r != 0) {
	g->row[r-1].y3 = y - 1;
	g->row[r-1].y2 = g->row[r-1].y3 - g->spaceY;
    }
    g->row[r].y0 = y + height + 1;
    g->row[r].y1 = g->row[r].y0 + g->spaceY;
}


void EdgePositions(rGraph g) {
int r, zs, zd;
int up_dx = 0;
    for (r = 0; r < g->rSize; r++) {
	rNode src;
	int dx = 0;
	for (src = g->row[r].first; src != NULL; src = src->forw) {
	    rNode last = NULL;
	    int dup = 0;  /* initialize to prevent gcc warning */
	    if (IsInvisible(src))
		continue;
	    for (zs = 0; zs < src->downSize; zs++) {
		Edge list = &src->downList[zs];
		rNode dst = list->dest;
		int count;

		if (IsInvisible(dst))
		    continue;
		if (dst != last)
		    dup = 0;
		else
		    ++dup;
		last = dst;

		/* find edge in dst->upList */
		count = dup;
		for (zd = 0; zd < dst->upSize; zd++) {
		    if (dst->upList[zd].dest == src) {
			if (count > 0) {
			    --count;
			    continue;
			}
			list->x1 = src->x + src->sWidth
				* (2 * zs + 1) / (2 * src->downSize);
			list->x2 = dst->x + dst->sWidth
				* (2 * zd + 1) / (2 * dst->upSize);
			if (list->x1 >= list->x2) {
			    if (dx < list->x1 - list->x2)
				dx = list->x1 - list->x2;
			} else {
			    if (dx < list->x2 - list->x1)
				dx = list->x2 - list->x1;
			}
			break;  /* next edge of src */
		    }
		}
		assert(zd < dst->upSize /* found edge in list */);
	    }
	}
	ComputeHeights(g, r, up_dx);
	up_dx = dx;
    }
}

/*****************************************************************************/


void RelativeSweep(rGraph g) {
int r;
rNode next = NULL;  /* first expected non-dummy non-relativized child */
    for (r = g->rSize - 1; r >= 0; r--) {
	int shift = 0;
	int column = 0;
	int position;
	rNode me;
	g->row[r].rHeight = 0;
	for (me = g->row[r].first; me != NULL; me = me->forw) {
	    int z, min2, max2;
	    Edge e;
	    if (IsInvisible(me) ) {
		me->x = column;
		me->leftNext = NULL;
		continue;  /* next me */
	    }
	    if (IsDummy(me)) {
		me->x = column;  /* needed for dummies with no left APPL_NODE */
		column = me->x + g->col_spacing;
		me->place1 = 0;
		me->place2 = -1;
		me->leftNext = NULL;
		continue;  /* next me */
	    }
	    if (g->row[r].rHeight < me->sHeight)
		g->row[r].rHeight = me->sHeight;

	    /* find first unplaced child */
	    if (! IsElided(me) && next != NULL) {
		for (z = 0; z < me->downSize; z++) {
		  if (IsInvisible(me->downList[z].dest))
			continue;
		    if (next->placement <= me->downList[z].dest->placement)
			break;
		}
		me->place1 = z;
	    } else {
		me->place1 = me->downSize;
	    }
	    if (me->downSize == me->place1) {
		me->place2 = -1;  /* no new children to place */
		me->x = column + PreSpacing(me);
		column = me->x + me->sWidth + PostSpacing(me, g);
		me->leftNext = NULL;
		continue;  /* next me */
	    }

	    /* find min child position */
	    for (z = me->place1 - 1; z >= 0; z--) {
		rNode n;
		for (n = next; n != NULL; n = n->back) {
		    if (! IsDummy(n) && ! IsInvisible(n))
			break;
		}
		if (n == NULL) {
		    z = -1;
		    break;
		}
		if (n != me->downList[z].dest)
		    break;
	    }
	    ++z;
	    if (z == me->place1) {
		min2 = 2 * me->downList[z].dest->x
			+ me->downList[z].dest->sWidth;
	    } else {
		for (e = &me->downList[z].dest->upList[0];
			IsDummy(e->dest); e++)
		    ;  /* find parent of preplaced child */
		assert(! IsInvisible(e->dest));
		min2 = 2 * (e->dest->x + me->downList[z].dest->x)
			+ me->downList[z].dest->sWidth;
	    }
	    me->leftNext = next;

	    /* find max child position */
	    for (z = me->downSize - 1; z > me->place1; z--) {
	        if(IsInvisible(me->downList[z].dest))
		    continue;
		if (! IsDummy(me->downList[z].dest))
		    break;
	    }
	    me->place2 = z;
	    max2 = 2 * me->downList[me->place2].dest->x
			+ me->downList[me->place2].dest->sWidth;

	    /* find position and new shift */
	    column = column + PreSpacing(me);
	    position = ((min2 + max2) / 2 - me->sWidth) / 2 + shift;
	    if (position < column) {
		me->x = column;
		shift = shift + column - position;
	    } else {
		me->x = position;
	    }
	    column = me->x + me->sWidth + PostSpacing(me, g);

	    /* shift children and make relative */
	    for ( ; next != NULL; next = next->forw) {
		if (! IsDummy(next) && ! IsInvisible(next))
		    next->x = next->x + shift - me->x;
		if (next == me->downList[me->place2].dest)
		    break;
	    }
	    /* update next */
	    if (next != NULL) {
		for (next = next->forw; next != NULL; next = next->forw) {
		    if (IsInvisible(next))
			continue;
		    if (! IsDummy(next) && ! IsInvisible(next))
			break;
		}
	    }
	}  /* next me */
	for (next = g->row[r].first; next != NULL; next = next->forw) {
	    if (! IsDummy(next) && ! IsInvisible(next))
		break;
	}
    }  /* next r */
}

/*****************************************************************************/
inline static int Odd(int x) { return((x & 1) == 1); }
inline static int Even(int x) { return((x & 1) == 0); }

void AbsoluteSweep(rGraph g) {
int r;
    for (r = 0; r < g->rSize; r++) {
	rNode me;
	rNode next = NULL;
	next = NULL;
	for (me = g->row[r].last; me != NULL; me = me->back) {
	    if (IsInvisible(me)) {
		if (me->forw == NULL)
		    me->x = MAXINT;
		else
		    me->x = me->forw->x;
		continue;  /* next me */
	    }

	    /* set absolute positions of children */
	    if (! IsDummy(me)) {
		rNode node = me->leftNext;
		if (node != NULL) {
		    for (node = me->leftNext; node != next; node = node->forw) {
			if (IsInvisible(node))
			    node->x = node->back->x;
			else if (IsDummy(node))
			    node->x = node->back->x + node->back->sWidth
					+ PostSpacing(node->back, g);
			else
			    node->x = node->x + me->x;
		    }
		    next = me->leftNext;
		}
		if (me->forw != NULL)
		    assert(me->x < me->forw->x);
	    }

	    /*
	     * move nodes with zero or one children,
	     * including dummies and elided nodes
	     */
	    if (me->place1 >= me->place2 && me->upSize > 0) {
		int left, right, position1;
		for (left = 0; left < me->upSize; left++) {
		    if (IsInvisible(me->upList[left].dest))
			continue;
		    if (IsElided(me->upList[left].dest))
			continue;
		    break;
		}
		for (right = me->upSize - 1; right >= 0; right--) {
		    if (IsInvisible(me->upList[right].dest))
			continue;
		    if (IsElided(me->upList[right].dest))
			continue;
		    break;
		}
		if (left > right) {
		    position1 = me->x;
		} else if (Even(right - left)) {
		    position1 = me->upList[left + (right - left) / 2].dest->x
			+ (me->upList[left + (right - left) / 2].dest->sWidth
					- me->sWidth) / 2;
		} else {  /* odd difference (even number in upList) */
		    int position2 = (2 * me->upList[left].dest->x
				+ me->upList[left].dest->sWidth
				+ 2 * me->upList[right].dest->x
				+ me->upList[right].dest->sWidth) / 2;
		    int min2 = 2 * me->upList[left + (right - left) / 2].dest->x
			+ me->upList[left + (right - left) / 2].dest->sWidth;
		    int max2 = 2 * me->upList[
				left + (right - left) / 2 + 1].dest->x
			+ me->upList[
				left + (right - left) / 2 + 1].dest->sWidth;
		    if (position2 <= min2)
			position1 = (min2 - me->sWidth) / 2;
		    else if (position2 >= max2)
			position1 = (max2 - me->sWidth) / 2;
		    else
			position1 = (position2 - me->sWidth) / 2;
		}
		if (me->x < position1) {
		    if (me->forw == NULL)
			me->x = position1;
		    else
			me->x = Min(position1,
				me->forw->x - me->sWidth - PostSpacing(me, g)
					- PreSpacing(me->forw));
		}
	    } else assert(!IsDummy(me));

	    if (me->forw != NULL) {
		if (!IsDummy(me))
		    assert(me->x < me->forw->x);
	    }
	}  /* next me */
    }  /* next r */
}

static void InsertHalfEdge(rGraph g,
	rNode src, rNode dst, ArrowType type, eStyle *edge_style) 
{
Edge *list, e;
int *size, s;
    if (src->row < dst->row) {
       assert(src->row == dst->row - 1);
       list = &src->downList;
       size = &src->downSize;
    } else {
       assert(src->row == dst->row + 1);
       list = &src->upList;
       size = &src->upSize;
    }
    for (e = *list, s = 0; s < *size; e++, s++)
	if (e->dest == dst)
	    break;
    if ((s < *size) && (e->eType == NO_LINE)) {
	/* edge already exists */
	assert(type != NO_LINE && type != NO_ARROW);
	e->eType = (ArrowType) ((e->eType & ~NO_ARROW) | type);
	e->style = edge_style;
	return;
      }
    *list = (Edge) GrowArray(*list, size, sizeof(**list));
    /* insertion sort, e is insertion point */
    for (s = *size - 1, e = &(*list)[s]; s > 0; s--, e--) {
	if ((e-1)->dest->placement <= dst->placement)
	    break;  /* found insertion point in middle of edge list */
	*e = *(e-1);  /* shift array element */
      }
    e->eType = type;
    e->style = edge_style;
    e->dest = dst;
}

void InsertEdge(rGraph g, rNode src, rNode dst, ArrowType type, 
		eStyle *edge_style) 
{
    switch(type) {
	case NO_LINE:
	case NO_ARROW:
	    InsertHalfEdge(g, src, dst, type, edge_style);
	    InsertHalfEdge(g, dst, src, type, edge_style);
	    break;
	case OUT_ARROW:
	    InsertHalfEdge(g, src, dst, OUT_ARROW, edge_style);
	    InsertHalfEdge(g, dst, src, IN_ARROW, edge_style);
	    break;
	case INOUT_ARROW:
	    InsertEdge(g, src, dst, OUT_ARROW, edge_style);
	    InsertEdge(g, dst, src, OUT_ARROW, edge_style);
	    break;
	case IN_ARROW:
	    InsertEdge(g, dst, src, OUT_ARROW, edge_style);
	    break;
	  }
  }

static rNode ImmediateAncestor(rGraph g, rNode node) 
{
  rNode back;

  /* returns immediate non-NULL ancestor */
  assert(node != NULL);
  if (!IsDummy(node) && (node->ancestor != NULL)) return(node->ancestor);
  if (node->upSize > 0) return(node->upList[0].dest);
#ifdef WARNING
  fprintf(stderr, "Warning: ImmediateAncestor() not found quickly\n");
#endif 
  if (node->row == 0) return(g->row[0].first);
  for (back = node->back; back != NULL; back = node->back) {
    if (!IsDummy(back) && (back->ancestor != NULL)) return(back->ancestor);
    node = back;
  }
  return(g->row[node->row - 1].first);
}

static rNode RowAncestor(rGraph g, rNode node, int row) 
{
  rNode ancestor;

  /* returns ancestor of given row */
  assert(node != NULL);
  assert(node->row >= row);
  for (ancestor = node->ancestor; ancestor != NULL;
	      node = ancestor, ancestor = node->ancestor) {
      if (ancestor->row < row || node->row == row) break;
  }
  while (node->row > row) node = ImmediateAncestor(g, node);
  return(node);
}

static int AncestorRow(rGraph g, rNode node) 
{
  /* returns row of first non-dummy ancestor */
  if ((node->ancestor != NULL) && !IsDummy(node->ancestor)) {
    return(node->ancestor->row);
  }
#ifdef WARNING
  fprintf(stderr, "Warning: AncestorRow() not found quickly\n");
#endif 
  for (node = ImmediateAncestor(g, node); IsDummy(node); 
    node = ImmediateAncestor(g, node))
    ;  /* find non-dummy ancestor */
  return(node->row);
}

static float AncestorPlacement(rGraph g, rNode node, int row) {
    /* returns placement of ancestor on given row */
    node = RowAncestor(g, node, row);
    return(node->placement);
}


/*
 * NewDummyNode
 * Create, initialize, and return a dummy node structure 
 */
static rNode NewDummyNode(NodeType type, rNode ancestor) 
{
    rNode  me = (rNode) Mal(sizeof(*me));

    me->nType = type;
    me->aNode = 0;
    me->ancestor = ancestor;
    me->nodeStyle = 0;

    /* everything else set to 0, NULL, APPL_NODE, or ABBREVIATED by Mal() */
    assert(NULL == 0);
    assert(APPL_NODE == 0);
    assert(ABBREVIATED == 0);
    return(me);
}

void AppendRow(rGraph g, int r, rNode me, rNode parent) {
  while (r > g->rSize) {
    rNode dummy = NewDummyNode(APPL_NODE, DONT_CARE);
    dummy->strType = INVISIBLE;
    InsertRow(g, g->rSize, DONT_CARE, dummy);
  }
  if (r == g->rSize)  /* add a row */
    InsertRow(g, r, DONT_CARE, me);
  else {  
    if (r == 0)
      // root node, just add to end of row 0
      InsertRow (g, r, g->row[r].last, me);
    else {
      // insert in row in appropriate order according to parent placement
      rNode curr = g->row[r].last;
      while (curr) {
	if (curr->upSize > 0) {
	  if (curr->upList[0].dest->placement < parent->placement)
	    break;
	}
	curr = curr->back;
      }
      InsertRow(g, r, curr, me);
    }
  }
}


static void DummySame(rGraph g, rNode src, rNode dst, eStyle *edge_style) {
rNode column, dummy;
    assert(src->row == dst->row);
    /* find insertion point: insert after column */
    if (src->row + 1 < g->rSize) {
	int z;
	column = g->row[src->row + 1].last;
	for (z = 0; z < src->downSize; z++) {
	    if (! IsDummy(src->downList[z].dest)) {
		/* place somewhere before first non-dummy child */
		column = src->downList[z].dest->back;
		break;
	    }
	    if (z == src->downSize - 1) {
		/* settle for near last dummy child */
		column = src->downList[z].dest;
		break;
	    }
	}
        for ( ; column != NULL; column = column->back) {
	    if (! IsDummy(column) || column->ancestor == dst)
		break;
        }
    } else {
	column = DONT_CARE;
    }
    dummy = NewDummyNode(SAME_NODE, dst);
    InsertRow(g, src->row + 1, column, dummy);
    InsertEdge(g, src, dummy, NO_ARROW, edge_style);
    InsertEdge(g, dummy, dst, OUT_ARROW, edge_style);
}

/*****************************************************************************/

static void DummyLeft(rGraph g, rNode src, rNode me, rNode tree, rNode dst,
		ArrowType arrow, eStyle *edge_style) {
rNode dummy;
    assert(arrow == IN_ARROW || arrow == OUT_ARROW || arrow == NO_ARROW);
    for (tree = ImmediateAncestor(g, tree); me->row > dst->row + 1;
		me = dummy, tree = ImmediateAncestor(g, tree)) {
	rNode column;
	/* find insertion point: insert after column */
	for (column = tree->back; column != NULL; column = column->back) {
	    if (! IsDummy(column))
		break;
	    assert(column->ancestor != NULL);
	    if (column->ancestor->row <= dst->row) {
		float ap = AncestorPlacement(g, column, dst->row);
		if (ap < dst->placement)
		    break;
		else if (dst->placement < ap)
		    continue;
	    } else {
		float aps = AncestorPlacement(g, src, column->ancestor->row);
		float apc = AncestorPlacement(g, column, column->ancestor->row);
		if (apc < aps)
		    break;
		else if (aps < apc)
		    continue;
	    }
	    {
		int row;
		row = AncestorRow(g, column);
		if (row < dst->row)
		    break;
		else if (dst->row < row)
		    continue;
	    }
	    if (column->nType == SAME_NODE)
		break;
	    assert(column->downSize == 1);
	    if (column->downList[0].dest->placement <= me->placement)
		break;
	}
	dummy = NewDummyNode(DUMMY_NODE, dst);
	InsertRow(g, me->row - 1, column, dummy);
	InsertEdge(g,
		me, dummy, (arrow == OUT_ARROW ? NO_ARROW : arrow), edge_style);
	if (arrow == IN_ARROW)
	    arrow = NO_ARROW;
    }
    InsertEdge(g, me, dst, arrow, edge_style);
}

/*****************************************************************************/

static void DummyRight(rGraph g,
	rNode src, rNode dst, ArrowType arrow, eStyle *edge_style) {
rNode me, tree, dummy;
    assert(arrow == IN_ARROW || arrow == OUT_ARROW || arrow == NO_ARROW);
    for (me = src, tree = ImmediateAncestor(g, me); me->row > dst->row + 1;
		me = dummy, tree = ImmediateAncestor(g, tree)) {
	rNode column = tree;
	if (column == NULL)
	    column = g->row[me->row - 1].first;
	else
	    column = column->forw;
	/* find insertion point: insert before column */
	for ( ; column != NULL; column = column->forw) {
	    if (! IsDummy(column))
		break;
	    assert(column->ancestor != NULL);
	    if (column->ancestor->row <= dst->row) {
		float ap = AncestorPlacement(g, column, dst->row);
		if (ap > dst->placement)
		    break;
		else if (dst->placement > ap)
		    continue;
	    } else {
		float aps = AncestorPlacement(g, src, column->ancestor->row);
		float apc = AncestorPlacement(g, column, column->ancestor->row);
		if (apc > aps)
		    break;
		else if (aps > apc)
		    continue;
	    }
	    {
		int row;
		row = AncestorRow(g, column);
		if (row > dst->row)
		    break;
		else if (dst->row > row)
		    continue;
	    }
	    if (column->nType == SAME_NODE)
		break;
	    assert(column->downSize == 1);
	    if (column->downList[0].dest->placement >= me->placement)
		break;
	}
	/* insert after column */
	if (column == NULL)
	    column = g->row[me->row - 1].last;
	else
	    column = column->back;
	dummy = NewDummyNode(DUMMY_NODE, dst);
	InsertRow(g, me->row - 1, column, dummy);
	InsertEdge(g,
		me, dummy, (arrow == OUT_ARROW ? NO_ARROW : arrow), edge_style);
	if (arrow == IN_ARROW)
	    arrow = NO_ARROW;
	if (column != NULL) {
	    if (! IsDummy(column)) {
		if (AncestorPlacement(g, column, dst->row) == dst->placement) {
		    DummyLeft(g, src, dummy, tree, dst, arrow, edge_style);
		    return;
		}
	    }
	}
    }
    InsertEdge(g, me, dst, arrow, edge_style);
}

/*****************************************************************************/

void LayoutDummy(rGraph g, rNode src, rNode dst, eStyle *edge_style) {
ArrowType arrow;
float placement;
int parent;
    assert(! IsDummy(src) && ! IsDummy(dst));
    if (src->row == dst->row) {
	DummySame(g, src, dst, edge_style);
	return;
    } else if (src->row < dst->row) {
	rNode temp = src;
	src = dst;
	dst = temp;
	arrow = IN_ARROW;
    } else {
	arrow = OUT_ARROW;
    }
    for (parent = 0; parent < src->upSize; parent++) {
	Edge e = &src->upList[parent];
	if (e->dest == dst || (IsDummy(e->dest) && e->dest->ancestor == dst)) {
	    if (arrow == IN_ARROW || e->dest == dst) {
		InsertEdge(g, src, e->dest, arrow, edge_style);
		return;
	      }
	    for (src = e->dest; src->upList[0].dest != dst;
			src = src->upList[0].dest)
		;  /* find child of dst */
	    InsertEdge(g, src, dst, arrow, edge_style);
	    return;
	}
    }
    placement = AncestorPlacement(g, src, dst->row);
    if (placement < dst->placement)
	DummyRight(g, src, dst, arrow, edge_style);
    else
	DummyLeft(g, src, src, src, dst, arrow, edge_style);
}

static Flag ElideUpDummy(rNode d, StringType type) {
    if (! IsDummy(d)) {
	return(type == INVISIBLE || (! IsInvisible(d) && ! IsElided(d)));
    } else {
	Flag change;
	assert(d->upSize == 1 || d->nType == SAME_NODE);
	change = ElideUpDummy(d->upList[0].dest, type);
	if (d->nType == SAME_NODE && type != INVISIBLE && change)
	    change = ElideUpDummy(d->upList[1].dest, type);
	if (change)
	    d->strType = (StringType) (type == INVISIBLE ?
		(d->strType | INVISIBLE) : (d->strType & ~INVISIBLE));
	return(change);
    }
}

static Flag ElideDownDummy(rNode d, StringType type) {
    if (! IsDummy(d)) {
	return(type == INVISIBLE || ! IsInvisible(d));
    } else {
	Flag change;
	assert(d->downSize == 1);
	change = ElideDownDummy(d->downList[0].dest, type);
	if (change)
	    d->strType = (StringType) (type == INVISIBLE ?
		(d->strType | INVISIBLE) : (d->strType & ~INVISIBLE));
	return(change);
    }
}




/*****************************************************************************/
/*                         Printing for Debugging                            */
/*****************************************************************************/
 
static void PrintString(FILE *f, rNode me) {
    if (f == NULL)
	f = stderr;
    if (me == NULL)
	fprintf(f, "me:NULL");
    else if (me->string == NULL)
	fprintf(f, "(s:null)");
    else
	fprintf(f, "\"%s\"", me->string);
}

static void PrintEdges(FILE *f, Edge e, int size) {
int z;
    if (f == NULL)
	f = stderr;
    for (z = 0; z < size; z++) {
      fprintf (f, "(EID %d: ", e[z].EID);
	switch(e[z].eType) {
	    case NO_LINE: fprintf(f, " NO_LINE, "); break;
	    case IN_ARROW: fprintf(f, " IN_ARROW, "); break;
	    case OUT_ARROW: fprintf(f, " OUT_ARROW, "); break;
	    case INOUT_ARROW: fprintf(f, " INOUT_ARROW, "); break;
	    case NO_ARROW: fprintf(f, " NO_ARROW, "); break;
	}
      fprintf (f, "style:%d, ", e[z].style);
      fprintf (f, "(x1 x2):(%d %d),", e[z].x1, e[z].x2);
      fprintf (f, "dest:");
	if (e[z].dest == NULL) fprintf(f, "NULL, ");
	else {
	  if (e[z].dest->nType == DUMMY_NODE) {
	    fprintf(f, "%X, ", (int)e[z].dest);
	  }
	  else {
	    fprintf(f, "%d, ", (int)e[z].dest->aNode);
	  }
	}
	PrintString(f, e[z].dest);
	fprintf(f, ")");
    }
}

void PrintNode(FILE *f, rGraph g, rNode me) {
    if (f == NULL)
	f = stderr;
    if (me->nType == DUMMY_NODE) {
      fprintf(f, "node = %X, nType = ", (int)me);
    } 
    else {
      fprintf(f, "node = %d, nType = ", (int)me->aNode);
    }
    switch(me->nType) {
	case APPL_NODE:	fprintf(f, "APPL_NODE, string = ");	break;
	case DUMMY_NODE:fprintf(f, "DUMMY_NODE, string = ");	break;
	case SAME_NODE:	fprintf(f, "SAME_NODE, string = ");	break;
    }
    PrintString(f, me);
    switch(me->strType) {
	case ABBREVIATED:	fprintf(f, " (ABBREVIATED)\n");	break;
	case VERBOSE:	fprintf(f, " (VERBOSE)\n");	break;
	case ELIDED:	fprintf(f, " (ELIDED)\n");	break;
	case ELID_VERB:	fprintf(f, " (ELID_VERB)\n");	break;
	case INVISIBLE:	fprintf(f, " (INVISIBLE)\n");	break;
	case INVI_VERB:	fprintf(f, " (INVI_VERB)\n");	break;
	case INVI_ELID:	fprintf(f, " (INVI_ELID)\n");	break;
	case INVI_ELID_VERB:fprintf(f, " (INVI_ELID_VERB)\n");	break;
	case INVALID:	fprintf(f, " (INVALID)\n");	break;
    }
    fprintf (f, "\tselected: %d\n", me->selected);

    fprintf(f, "\trow placement : x y w h = %d %.2f : %d %d %d %d\n",
		me->row, me->placement,
		me->x, g->row[me->row].y, me->sWidth, me->sHeight);

    fprintf(f, "\tancestor = ");  PrintString(f, me->ancestor);
    fprintf(f, ", forw = ");  PrintString(f, me->forw);
    fprintf(f, ", back = ");  PrintString(f, me->back);
    fprintf(f, "\n\tupList =");
    PrintEdges(f, me->upList, me->upSize);
    fprintf(f, "\n\tdownList =");
    PrintEdges(f, me->downList, me->downSize);
    fprintf(f, "\n");
}

/*****************************************************************************/
