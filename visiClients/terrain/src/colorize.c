/*
 * Copyright (c) 1989, 1990 Barton P. Miller, Morgan Clark, Timothy Torzewski
 *     Jeff Hollingsworth, and Bruce Irvin. All rights reserved.
 *
 * This software is furnished under the condition that it may not
 * be provided or otherwise made available to, or used by, any
 * other person.  No title to or ownership of the software is
 * hereby transferred.  The name of the principals
 * may not be used in any advertising or publicity related to this
 * software without specific, written prior authorization.
 * Any use of this software must include the above copyright notice.
 *
 */

/*
 * colorize.c - colorize surfaces according to their height.
 *
 * $Id: colorize.c,v 1.8 2002/02/11 22:08:01 tlmiller Exp $
 */

#ifdef i386_unknown_linux2_0 || defined(ia64_unknown_linux2_4)
#define _HAVE_STRING_ARCH_strcpy  /* gets rid of warnings */
#define _HAVE_STRING_ARCH_strsep
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "terrain.h"   
#include "misc.h"
#include "graph3d.h"
#include "colorize.h"
   
#define TRUE	1
#define FALSE	0

/* Internal representation of polygon -- a linked list of points */

struct ptLstNode_t {
   float x,y,z;
   struct ptLstNode_t *next;
};

typedef struct ptLstNode_t *ptLst_t;

/* The z-tic list -- map height (z coord) to colors */

struct ticNode_t {
   float z;
   int color;
   struct ticNode_t *next;
};

typedef struct ticNode_t *ticLst_t;

void draw_colorPoly();
void colorPoly(ptLst_t, int);
void colorize(ptLst_t, ticLst_t);
void colorizeLayers(ptLst_t, ticLst_t);
 
ptLst_t findGoUnder(/*ptLst_t, ptLst_t, float*/);
ptLst_t findGoAbove(/*ptLst_t, ptLst_t, float*/);
ptLst_t makeMidPt(/*ptLst_t, float*/);
ptLst_t copyPt(/*ptLst_t*/);
void freePtLst(/*ptLst_t*/);

extern float zmin, zmax;	/* Max & min points of the graph */
extern int Ntcolors;		/* Number of colors available */

static int pix = 0;

/*******************************************************************
* draw_colorPoly - Draw a colorized polygon.
*
* draw_colorPoly takes 4 points and draw it with colors.  The colors
* used are determined by the z dimension of the polygon the the z-tics.
* If the polygon run accross more than one z-tics, the polygon will
* be cut up and assigned with diffrernt colors.
*
*******************************************************************/

void draw_colorPoly(x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4, remap, pix_map)
float x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4;
int remap;
int pix_map;
{
   static ticLst_t tics=NULL;
   ticLst_t curTic;
   ptLst_t drawLst, curPt;
   ticLst_t next;

   int i;
   float curZ, ticSize;

   /* Initialize the tic list (mapping between z and colors if that */
   /* has not been done yet or there is change in maxz.             */

   if (remap == -1 && tics != NULL)
   {
      while(tics->next != NULL)
      {
           next = tics->next;
	   free(tics);
           tics = next;
      }

      free(tics);
      tics = NULL;
   }
      
   if (tics==NULL) {
      curTic = tics = (ticLst_t) terrain_alloc(sizeof(struct ticNode_t));
       
      curZ = zmin;
      ticSize = (zmax - zmin)/Ntcolors;
      for (i=0; i<Ntcolors - 1; i++) {
	 curTic->z = curZ+=ticSize;
	 curTic->color = i;
	 curTic->next = (ticLst_t) terrain_alloc(sizeof(struct ticNode_t));
	 curTic = curTic->next;
      }
      curTic->z = curZ+=ticSize;
      curTic->color = i;
      curTic->next = NULL;
   }
   
#ifdef COLORIZE_DEBUG
   
   printf("(%1.2lf %1.2lf %1.1lf)-(%1.2lf %1.2lf %1.1lf)-(%1.2lf %1.2lf %1.1lf)-(%1.2lf %1.2lf %1.1lf)\n", x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4);
   
#endif

   /* Convert the points into internation representation */

   drawLst = (ptLst_t) terrain_alloc(sizeof(struct ptLstNode_t));
   drawLst->x = x1;
   drawLst->y = y1;
   drawLst->z = z1;
   drawLst->next = (ptLst_t) terrain_alloc(sizeof(struct ptLstNode_t));
   
   curPt = drawLst->next;

   curPt->x = x2;
   curPt->y = y2;
   curPt->z = z2;
   curPt->next = (ptLst_t) terrain_alloc(sizeof(struct ptLstNode_t));

   curPt = curPt->next;

   curPt->x = x3;
   curPt->y = y3;
   curPt->z = z3;
   curPt->next = (ptLst_t) terrain_alloc(sizeof(struct ptLstNode_t));

   curPt = curPt->next;

   curPt->x = x4;
   curPt->y = y4;
   curPt->z = z4;
   curPt->next = NULL;

   /* set whether draw on pixmap or on actual window */
   pix = pix_map;

   /* Do the real job */

   colorize(drawLst, tics);
   
#ifdef COLORIZE_DEBUG
   
   printf("\n");
   
#endif
   
}



/*********************************************************************
*
* colorPoly - draw the polygon defined by "pts" with "color"
*             This routine calls the X driver.
*
*********************************************************************/


void colorPoly(pts, color)
ptLst_t pts;
int color;
{
   ptLst_t curPt;
   int i, size=0;
   unsigned *dpts;

   /* Check how many points the polygon has */

   for(curPt = pts; curPt->next != pts; curPt = curPt->next) {
      
#ifdef COLORIZE_DEBUG
      
      printf("	%10.6lf %10.6lf %10.6lf -- %d\n",
	     curPt->x, curPt->y, curPt->z, color);
      
#endif
      
      size++;
   }
   
#ifdef COLORIZE_DEBUG
   
   printf("	%10.6lf %10.6lf %10.6lf -- %d\n",
	  curPt->x, curPt->y, curPt->z, color);
   
#endif
   
   size++;

   dpts = (unsigned *)terrain_alloc(sizeof(unsigned)*size*2);
   
   for (i=0, curPt = pts; i<size; i++, curPt = curPt->next)
      map3d_xy(curPt->x, curPt->y, curPt->z, 
	       &(dpts[i<<1]), &(dpts[(i<<1)+1]));
   

   if (pix > 0)
      X11_colorPoly_window(dpts, size, color);

      X11_colorPoly(dpts, size, color);

   free(dpts);
}



/*********************************************************************
*
* colorize - find out the lowerest point of the polygon and start
*            the colorization rolling from there.
*
* 
*********************************************************************/


void colorize(pts, tics)
ptLst_t pts;			/* Points of the polygon */
ticLst_t tics;			/* List of all tics  */
{
   ticLst_t curTic;
   ptLst_t curPt;
   float minZ;
   
   /* Search for the lowerest point */
   curPt = pts;
   minZ = curPt->z;
   for (curPt = pts; curPt->next != NULL; curPt = curPt->next)
      minZ = (curPt->next->z < minZ)? curPt->next->z : minZ;
   
   /* Make the list circular */
   curPt->next = pts;
   
   /* Skip irrevalent tics */
   curTic = tics;
   while ((curTic->next != NULL) && (curTic->z <= minZ))
      curTic = curTic->next;
   
   if (curTic->next == NULL)
      colorPoly(pts, curTic->color);
   else
      colorizeLayers(pts, curTic);
   
   return;
}





/*********************************************************************
*
* colorizeLayers - cut the polygon into levels accroding to the z-tics,
*                  and draw them with colors.
*
*********************************************************************/


void colorizeLayers(pts, tic)
ptLst_t pts;
ticLst_t tic;
{
   ptLst_t curPt, startPt, goUnderPt, goAbovePt, drawPt, newPt;
   ticLst_t curTic;
   int done = FALSE;
   
   curTic = tic;
   startPt = pts;		/* Keep the while loop happy */
   
   /* Start filling the surface now */
   while((curTic->next != NULL) && (startPt != NULL)) {
      
      /* Find a start point */
      if ((curPt = findGoUnder(startPt, startPt, curTic->z)) == NULL) {
	 /* All points are under the current tic */
	 colorPoly(startPt, curTic->color);
	 freePtLst(startPt);
	 startPt = NULL;
      } else {
	 /* Calculate the real start point and add that to the list */
	 startPt = curPt;
	 if (startPt->z != curTic->z) {
	    newPt = makeMidPt(startPt, curTic->z);
	    newPt->next = startPt->next;
	    startPt->next = newPt;
	    curPt = startPt = goUnderPt = newPt;
	 } else {
	    curPt = goUnderPt = startPt;
	 }
	 
	 /* Walk through all points in that level and form sub-polygons */
	 /* Note that there can be more than one sub-polygon in one level */

	 done = FALSE;
	 while(!done) {
	    if ((goAbovePt = findGoAbove(curPt, startPt, curTic->z)) == NULL) {

	       /* Done if we run out of points */

	       colorPoly(startPt, curTic->color);
	       freePtLst(startPt);
	       return;
	    } else {

	       /* Find the end point of the sub-polygon */

	       if (goAbovePt->z != curTic->z) {
		  newPt = makeMidPt(goAbovePt, curTic->z);
		  newPt->next = goAbovePt->next;
		  drawPt = copyPt(goUnderPt);
		  curPt = goUnderPt->next = newPt;
		  goAbovePt->next = copyPt(newPt);
		  goAbovePt->next->next = drawPt;
	       } else {
		  newPt = copyPt(goAbovePt);
		  curPt = goAbovePt->next = newPt;
		  drawPt = copyPt(goUnderPt);
		  goUnderPt->next = newPt;
		  goAbovePt->next = drawPt;
	       }
	       
	       /* Draw it ! */

	       colorPoly(drawPt, curTic->color);
	       freePtLst(drawPt);
	    }
	    
	    /* Search for next point under the tic and start over */
	    if ((goUnderPt = findGoUnder(curPt, startPt, curTic->z)) == NULL)
	       done = TRUE;
	    else {
	       if (goUnderPt->z != curTic->z) {
		  newPt = makeMidPt(goUnderPt, curTic->z);
		  newPt->next = goUnderPt->next;
		  goUnderPt->next = newPt;
		  curPt = goUnderPt = newPt;
	       } else {
		  curPt = goUnderPt;
	       }
	    }
	 }
	 
#ifdef COLORIZE_DEBUG
	 
	 for (curPt=startPt; curPt->next!=startPt; curPt=curPt->next)
	    printf("   (%1.1lf,%1.1lf,%1.1lf)-", curPt->x, curPt->y, curPt->z);
	 
	 printf("   (%1.1lf,%1.1lf,%1.1lf)\n", curPt->x, curPt->y, curPt->z);
	 
#endif
	 
      }
      
      curTic = curTic->next;
   }
   
   /* If there are remaining points, they must be above all the tics */
   if (startPt != NULL) {
      colorPoly(startPt, curTic->color);
      freePtLst(startPt);
   }
   
   return;
}




/*********************************************************************
*
* findGoUnder - walk along the current sub-polygon and find a coordinate
*               which has the z coordinate value of "z" and its last point
*               is above the surface z and its next point below z.
*               Note that the point may not be a existing point.  It can
*               be somewhere between the existing points.
*
* Returns:
*    ptLst_t : The new point found.
*              NULL means all points of the current sun-polygon are
*              above z.
*
*********************************************************************/


ptLst_t
findGoUnder(startPt, endPt, z)
ptLst_t startPt, endPt;		/* Range of the search */
float z;			/* the z coordinate of the new point */
{
   ptLst_t curPt;
   
#ifdef COLORIZE_DEBUG
   printf("V          ");
   
   for (curPt=startPt; curPt->next!=startPt; curPt=curPt->next) {
      if (curPt==endPt)
	 printf("[%1.1lf,%1.1lf,%1.1lf]-", curPt->x, curPt->y, curPt->z);
      else
	 printf("(%1.1lf,%1.1lf,%1.1lf)-", curPt->x, curPt->y, curPt->z);
   }
   
   if (curPt==endPt)
      printf("[%1.1lf,%1.1lf,%1.1lf]\n", curPt->x, curPt->y, curPt->z);
   else
      printf("(%1.1lf,%1.1lf,%1.1lf)\n", curPt->x, curPt->y, curPt->z);
   
#endif
   
   if (startPt->z >= z && startPt->next->z < z) {
      
#ifdef COLORIZE_DEBUG
      
      printf("V            %1.1lf >= %1.1lf, %1.1lf < %1.1lf\n",
	     startPt->z, z, startPt->next->z, z);
      
#endif
      
      return startPt;
   }
   
   for (curPt=startPt->next;
	(curPt != endPt) && !(curPt->z >= z && curPt->next->z < z);
	curPt = curPt->next)
      
#ifdef COLORIZE_DEBUG
      
      printf("V            %1.1lf >= %1.1lf, %1.1lf < %1.1lf\n",
	     curPt->z, z, curPt->next->z, z);
   
   printf("V            The anser is : %c\n", (curPt != endPt)?'T':'F');
   
#else
   ;
#endif
   
   return (curPt != endPt)? curPt : NULL;
}




/*********************************************************************
*
* findGoAbove - walk along the current sub-polygon and find a coordinate
*               which has the z coordinate value of "z" and its last point
*               is below the surface z and its next point is above z.
*               Note that the point may not be a existing point.  It can
*               be somewhere between the existing points.
*
* Returns:
*    ptLst_t : The new point found.
*              NULL means all points of the current sun-polygon are
*              above z.
*
*********************************************************************/


ptLst_t
findGoAbove(startPt, endPt, z)
ptLst_t startPt, endPt;		/* Start & end pts of the current polygon */
float z;			/* The z coordinate value to be tested */
{
   ptLst_t curPt;
   
#ifdef COLORIZE_DEBUG
   printf("^          ");
   
   for (curPt=startPt; curPt->next!=startPt; curPt=curPt->next) {
      if (curPt==endPt)
	 printf("[%1.1lf,%1.1lf,%1.1lf]-", curPt->x, curPt->y, curPt->z);
      else
	 printf("(%1.1lf,%1.1lf,%1.1lf)-", curPt->x, curPt->y, curPt->z);
   }
   
   if (curPt==endPt)
      printf("[%1.1lf,%1.1lf,%1.1lf]\n", curPt->x, curPt->y, curPt->z);
   else
      printf("(%1.1lf,%1.1lf,%1.1lf)\n", curPt->x, curPt->y, curPt->z);
   
#endif
   
   if (startPt->z < z && startPt->next->z >= z)
      return startPt;
   
#ifdef COLORIZE_DEBUG
   
   printf("^            %1.1lf <= %1.1lf, %1.1lf > %1.1lf\n",
	  startPt->z, z, startPt->next->z, z);
   
#endif
   
   for (curPt=startPt->next;
	(curPt != endPt) && !(curPt->z < z && curPt->next->z >= z);
	curPt = curPt->next)
      
#ifdef COLORIZE_DEBUG
      
      printf("^            %1.1lf <= %1.1lf, %1.1lf > %1.1lf\n",
	     curPt->z, z, curPt->next->z, z);
   
   printf("^            The anser is : %c\n", (curPt != endPt)?'T':'F');
   
#else
   ;
#endif
   
   return (curPt != endPt)? curPt : NULL;
   
}





/*********************************************************************
*
* makeMidPt - produce a new point which is between two given points and
*             with it has the value "z" as its z coordinate.
*             One can think of the point as the intersection between
*             the line pt, pts->next and a horizontal line z.
*
* Returns:
*    ptLst_t : The new point.
*
*********************************************************************/


ptLst_t
makeMidPt(pts, z)
ptLst_t pts;
float z;
{
   ptLst_t newPt;		/* The new 'middle' point */
   float zIndex;
   
   if ((newPt = (ptLst_t) terrain_alloc(sizeof(struct ptLstNode_t))) == NULL) {
      printf("Memory allocation error\n");
      exit(-1);
   }

   /* x & y coordinate are between pts and pts->next and weighted by z */
   
   zIndex = (pts->z - z)/(pts->z -  pts->next->z);
   newPt->z = z;
   newPt->x = pts->x + ((pts->next->x - pts->x) * zIndex);
   newPt->y = pts->y + ((pts->next->y - pts->y) * zIndex);
   newPt->next = NULL;
   
   return newPt;
}




/*********************************************************************
*
* freePtLst - Free a list of points.
*
*********************************************************************/


void freePtLst(pts)
ptLst_t pts;
{
   ptLst_t lastPt, curPt;
   
   curPt = pts;
   while(curPt->next != pts) {
      lastPt = curPt;
      curPt = curPt->next;
      free(lastPt);
   }
   
   free(curPt);
   
   return;
}




/*********************************************************************
*
* copyPt - duplicate a point.
*
* Returns:
*  ptLst_t  : The new copy of the point given
*
*********************************************************************/

ptLst_t
copyPt(source)
ptLst_t source;
{
   ptLst_t target;
   
   if ((target = (ptLst_t) terrain_alloc(sizeof(struct ptLstNode_t))) == NULL) {
      printf("Memory allocation error.\n");
      exit(-1);
   }
   
#ifdef BCOPY
   bcopy(source, target, sizeof(struct ptLstNode_t));
#else
   memcpy(target, source, sizeof(struct ptLstNode_t));
#endif
   
   return target;
}
