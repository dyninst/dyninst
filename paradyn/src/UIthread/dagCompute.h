#ifndef _dagCompute_h
#define _dagCompute_h
/* 
 * dagCompute.h  
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>

extern "C" {
#include "tk.h"
}
class dag;

#ifndef TRUE
#define FALSE   0
#define TRUE    1
#endif TRUE
#ifndef __GNUC__
#ifndef inline
#define inline  /* nothing */
#endif inline
#endif __GNUC__
#define MAXINT		((int)((unsigned)-1 >> 1))
#define DONT_CARE	NULL	/* use NULL to avoid pointer casting */

typedef int Flag;

//extern void *malloc(size_t size);
// extern void free(void *p);

#define HASH_SIZE	11	/* large prime number for hash table */
#define DAGNODEMAXSTR   100     /* max node label length */
typedef enum {
    NO_LINE = 0, IN_ARROW, OUT_ARROW, INOUT_ARROW, NO_ARROW
} ArrowType;

typedef enum {
    ABBREVIATED = 0, VERBOSE, ELIDED, ELID_VERB,
	INVISIBLE, INVI_VERB, INVI_ELID, INVI_ELID_VERB,
	INVALID /* invalid used by graph widget */
} StringType;

typedef enum {
    APPL_NODE = 0, DUMMY_NODE, SAME_NODE
} NodeType;
 
typedef struct _graph *rGraph;
typedef struct _node *rNode;
typedef struct _row *Row;
typedef int ApplNode;			/* defined by application module */
struct eStyle;
struct nStyle;

typedef struct _edge {
  int EID;                /* unique identifier for this edge */
  ArrowType eType;	  /* type of edge */
  eStyle *style;          /* predefined set of options for edge */
  int x1, x2;		  /* x edge coordinates */
  rNode dest;		  /* endpoint of edge */
} *Edge;

struct _node {
  int selected;                 /* used to tag nodes during dag traversal */
  NodeType nType;		/* dummy type or application type */
  rNode hash;			/* linked-list collision resolution */
  rNode ancestor;		/* ancestor of APPL_NODE (not for dummies) */
  rNode forw, back;		/* row list */
  Edge upList;		        /* list of upward edges, sorted by column */
  int upSize;			/* number of up edges */
  Edge downList;		/* list of downward edges, sorted by column */
  int downSize;		        /* number of down edges */
  
  int x;			/* x display coordinates for node */
  int row;			/* row number of mapping */
  double placement;		/* column placement in row */
  int place1, place2;		/* range of non-dummy children to place */
  rNode leftNext;		/* leftmost relative child */
  
  ApplNode root;		/* root of application node */
  ApplNode aNode;		/* handle for application node */
  char string[DAGNODEMAXSTR];	/* string to be displayed */
  StringType strType;		/* default is ABBREVIATED */
  int sWidth, sHeight;	        /* string display parameters */
  nStyle *nodeStyle;            /* node style record */
};

struct _row {
    rNode first, last;		/* list endpoints */
    int rHeight;		/* row height */
    int y;			/* y display coordinates for node */
    int y0, y1, y2, y3;		/* y coordinates for vertical edges */
};

struct _graph {
    rNode hash[HASH_SIZE];	/* hash table for pointer lookup */
    Row row;			/* array of rows */
    int rSize;			/* number of rows */
    int row_spacing;            /* min. spacing between rows */
    int	col_spacing;            /* min. spacing between columns */
    float row_ratio;
    int spaceX;
    int spaceY;
};

 /* function prototypes */

rNode NewNode(rGraph g, int type, rNode ancestor, ApplNode aNode,
		     char *style, char label[]); 
void PrintNode(FILE *f, rGraph g, rNode me);
void AppendRow(rGraph g, int r, rNode me);
void RelativeSweep(rGraph g);
void AbsoluteSweep(rGraph g);
void EdgePositions(rGraph g);
void LayoutDummy(rGraph g, rNode src, rNode dst, eStyle *edge_style); 

#endif _dagLayout_h





