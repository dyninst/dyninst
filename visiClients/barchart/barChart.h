// barChart.h

/* $Log: barChart.h,v $
/* Revision 1.7  1994/10/14 10:27:45  tamches
/* Swapped the x and y axes -- now resources print vertically and
/* metrics print horizontally.  Can fit many, many more resources
/* on screen at once with no label overlap.  Multiple metrics
/* are now shown in the metrics axis.  Metric names are shown in
/* a "key" in the lower-left.
/*
 * Revision 1.6  1994/10/13  00:51:03  tamches
 * Removed xoffsets and widths while implementing
 * sorting and bug-fixing deletion of resources.
 * double-buffer is now the only drawing option
 *
 * Revision 1.5  1994/10/11  22:02:48  tamches
 * added validMetrics and validResources arrays to avoid
 * drawing bars of deleted resources
 *
 * Revision 1.4  1994/10/10  23:08:39  tamches
 * preliminary changes on the way to swapping the x and y axes
 *
 * Revision 1.3  1994/10/10  14:36:14  tamches
 * fixed some resizing bugs
 *
 * Revision 1.2  1994/09/29  20:05:34  tamches
 * minor cvs fixes
 *
 * Revision 1.1  1994/09/29  19:48:42  tamches
 * initial implementation
 *
*/

#ifndef _BARCHART_H_
#define _BARCHART_H_

#include <array2d.h>

typedef dynamic1dArray<bool>   dynamic1dArrayBool;
typedef dynamic1dArray<int>    dynamic1dArrayInt;
typedef dynamic1dArray<double> dynamic1dArrayDouble;
typedef dynamic1dArray<XColor *> dynamic1dArrayXColor;

typedef dynamic2dArray<bool>   dynamic2dArrayBool;
typedef dynamic2dArray<int>    dynamic2dArrayInt;
typedef dynamic2dArray<double> dynamic2dArrayDouble;

class BarChart {
 private:
   static bool currentlyInstalledLowLevelDrawBars;

 private:
   char *theWindowName; // in tk-style; e.g. ".a.b.c"
   Tk_Window theWindow;

   bool HaveSeenFirstGoodWid;
      // set when wid becomes non-zero (which is unfortunately not
      // until the tk packer is run on theWindow)
   Window  wid; // low-level window id used in Xlib drawing calls
   int     borderPix; // in pixels
   int     currScrollOffset; // in pixels
   int     width, height;
   Display *display; // low-level display structure used in Xlib drawing calls

   XColor *greyColor; // for the background
   dynamic1dArrayXColor metricColors;
      // each metric has its own color-code in which its bars are drawn

   GC myGC; // we change colors here very frequently with XSetForeground/XSetBackground
            // before actual drawing; hence, we could not use Tk_GetGC

   enum DataFormats {Current, Average, Total};
   DataFormats DataFormat;

   Pixmap doubleBufferPixmap;
      // set with XCreatePixmap; you should reset with XFreePixmap and another call
      // to XCreatePixmap whenever the window size changes.  Delete with
      // XFreePixmap when done.

   int numMetrics, numResources;
   int numValidMetrics, numValidResources; // how many are enabled by visi lib (as opposed to deleted)

   dynamic1dArrayInt indirectResources;
   dynamic1dArrayBool validMetrics, validResources;
      // which metrics and resources are valid and should be drawn?

   int totalResourceHeight; // same as tcl vrble "currResourceHeight"
   int fullResourceHeight; // 90% of totalResourceHeight
   int resourceBorderHeight;
   int individualResourceHeight; // fullResourceHeight / numMetrics

   dynamic2dArrayInt barWidths;
      // array [metric][rsrc] of pixel widths for each bar.  This
      // changes quite often (every time new data arrives) and
      // needs to be saved for the case of expose events...
   dynamic2dArrayDouble barValues;
      // array [metric][rsrc] of numerical (not pixel) bar values
      // the basis for barWidths[][]
   dynamic1dArrayDouble metricCurrMaxVals;
      // array [metric] of the current y-axis high value for each metric.
      // When new data comes in that is higher than this, I give the command
      // to rethink the metrics axis.

   void changeDoubleBuffering();

   bool TryFirstGoodWid();
      // if wid is still zero, try a fresh Tk_WindowId() to try and change that...

   void lowLevelDrawBars();
      // sets up lowestLevelDrawBars() to be called the next time tk is idle.
   void lowestLevelDrawBarsDoubleBuffer();

   static void lowestLevelDrawBars(ClientData ignore);
      // assuming the barWidths[][] have changed, redraw bars
      // with a call to XFillRectanges() or Tk_Fill3DRectangle()'s.
      // the "static" ensures that "this" isn't required to
      // call this routine, which is absolutely necessary since
      // it gets installed as a Tk_DoWhenIdle() routine...

   void rethinkValidMetricsAndResources();

   void RethinkMetricColors();
      // assuming metrics have change, rethink "metricColors" array
      
   void RethinkBarLayouts();
      // assuming a resize (but not added/deleted metrics), fill in barXoffsets,
      // barWidths, resourceCurrMaxY, etc, 

   void RethinkBarWidths();
      // assuming given new bar values and/or new metric max values and/or change
      // in window width, recalculate barWidths[][].

   void rethinkBarValues();
   void rethinkMetricMaxValues();
   void setMetricNewMax(const int metricindex, const double newmaxval);
   double nicelyRoundedMetricNewMaxValue(const int metricindex, const double newmaxval);

  public:
   BarChart(char *tkWindowName, const bool dblBuffer, const bool noFlicker,
	    const int iNumMetrics, const int iNumResources,
	    const bool iFlushFlag);
  ~BarChart();

   void processFirstGoodWid();
   void processResizeWindow(const int newWidth, const int newHeight);
   void processExposeWindow();

   void RethinkMetricsAndResources();
      // erase and reallocate barXoffsets, barWidths, barWidths, resourceCurrMaxY;
      // set numMetrics, numResources -- all based on a complete re-reading from
      // dataGrid[][].  (If visi had provided more fine-grained callbacks than
      // ADDMETRICSRESOURCES, such a crude routine would not be necessary.)
      // When done, redraws.
   void rethinkIndirectResources(); // needed to implement sorting

   void processNewData(const int newBucketIndex);
      // assuming new data has arrived at the given bucket index for all
      // metric/rsrc pairs, read the new information from dataGrid[][],
      // update barWidths[][] accordingly, and call lowLevelDrawBars()

   void processNewScrollPosition(const int newPos);
   void rethinkDataFormat();
};

extern BarChart *theBarChart;

#endif
