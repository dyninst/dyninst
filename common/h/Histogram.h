/* 
 * Copyright (c) 1993 Barton P. Miller, Morgan Clark, Timothy Torzewski,
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
 * $Log: Histogram.h,v $
 * Revision 1.1  1993/08/26 18:23:09  hollings
 * Initial revision
 *
 * Revision 1.2  1993/01/26  21:10:30  hollings
 * Changed to use IPS style selection of ymax and increments.
 *
 * Revision 1.1  1993/01/08  21:20:23  hollings
 * Initial revision
 *
 */

/*****************************************************************
		Histogram Widget 
******************************************************************/

#ifndef _Histogram_h
#define _Histogram_h

#include <X11/Xaw/Form.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG	1

/******************************************************************/

/* Parameters:

Name		     Class		RepType		Default Value
----		     -----		-------		-------------
background	     Background		Pixel		XtDefaultBackground
border		     BorderColor	Pixel		XtDefaultForeground
borderWidth	     BorderWidth	Dimension	1
foreground	     Foreground		Pixel		XtDefaultForeground
graphHeight	     GraphHeight	Dimension	200
graphWidth	     GraphWidth		Dimension	600
font		     Font		Font		"6x13"
maxCurve	     MaxCurve		Int		10
maxGroup	     MaxGroup		Int		10
colorLineStyle	     ColorLineStyle	String		
bwLineStyle	     BwLineStyle	String		
numXMarks	     NumXMarks		Int		10
yInc		     YInc		Float		0.2
yMin	 	     YMin		Float		5.0
smoothNpts           SmoothNpts  	Int 		8
compact		     Compact		Blloean		Flase
*/

#define XtCGraphForeground "GraphForeground"
#define XtCGraphBackground "GraphBackground"
#define XtCGraphWidth "GraphWidth"
#define XtCGraphHeight "GraphHeight"
#define XtCMaxCurve "MaxCurve"
#define XtCMaxGroup  "MaxGroup"
#define XtCNumXMarks 	"NumXMarks"
#define XtCColorLineStyles  "ColorLineStyles"
#define XtCBwLineStyles  "BwLineStyles"
#define XtCYInc	"YInc"
#define XtCYMin	"YMin"
#define XtCSmoothNpts  "SmoothNpts"
#define XtCCompact "Compact"

#define XtNgraphForeground "graphForeground"
#define XtNgraphBackground "graphBackground"
#define XtNgraphWidth "graphWidth"
#define XtNgraphHeight "graphHeight"
#define XtNmaxCurve "maxCurve"
#define XtNmaxGroup  "maxGroup"
#define XtNnumXMarks	"numXMarks"
#define XtNcolorLineStyles  "colorLineStyles"
#define XtNbwLineStyles  "bwLineStyles"
#define XtNyInc	"yInc"
#define XtNyMin	"yMin"
#define XtNsmoothNpts  "smoothNpts"
#define XtNcompact "compact"


/* define mask for each bit in a mask */
#define MASK_1	01
#define MASK_2  02
#define MASK_3	04
#define MASK_4  010
#define MASK_5 	020
#define MASK_6	040
#define MASK_7 	0100
#define MASK_8 	0200
#define MASK_9 	0400
#define MASK_10 01000


/* used to indicate what fields in CurveInfo are specified, 
   can be OR'ed */
enum CurveInfoMask {
	/* LINEPROPERTY to mask the property of a line : linestyle,
	 * foreground and width. 
	 * once used, all three properties have to be supplied
	 */
	LINEPROPERTY = MASK_3,
	YTITLE = MASK_6,
	YINC = MASK_7,
};

/* property of a curve, use CurveInfoMASK to indicate what are defined*/
typedef struct _CurveInfo {
	Pixel 	foreground;	/* foreground color */
	int	lineWidth;		/* width of line */
	int	lineStyle;	
	int	yInterval;	/* interval in y-axis */
	int 	yInc;		/* yInc more when out of yMax limit*/
	int 	yMax;		/* max value of y-axis */
	char 	*yTitle;	/* title for y-axis */
} CurveInfo;


/* each curve */
typedef struct _CurveRec {
	int	curveID;	/* filled after return, will be used  
				   as a reference thereafter */
	int	groupID; 	/* used to find its group */
	char	*name;	/* curve label shown at the bottom */
	CurveInfo curveInfo;
} CurveRec;


/**************************************************************/

typedef struct _HistogramClassRec	*HistogramWidgetClass;
typedef struct _HistogramRec		*HistogramWidget;

extern WidgetClass histogramWidgetClass;

/******************************************************/

extern void HistAddNewCurve(Widget, unsigned long, CurveRec *,Boolean);
extern void HistSampleInfo(Widget, double, int, double, Boolean);
extern void HistSetCurveData(Widget, int, int, int, float*);
extern void HistDeleteCurve(Widget, int **, int *);
extern void HistRestoreCurve(Widget, int *, int);
extern void HistClose(Widget);
extern void HistSmoothCurve(Widget, int **, int *);
extern void HistUnsmoothCurve(Widget, int **, int *);

#ifdef __cplusplus
}
#endif

#endif /* _Histogram_h */
