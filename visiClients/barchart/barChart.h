/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// barChart.h

/* $Log: barChart.h,v $
/* Revision 1.12  1996/08/16 21:35:21  tamches
/* updated copyright for release 1.1
/*
 * Revision 1.11  1996/05/15 18:01:22  tamches
 * added setMetricNewMaxLL
 *
 * Revision 1.10  1996/01/10 02:33:23  tamches
 * changed uses of dynamic1dArray/2d to the vector class
 * removed theWindowName
 * added a tkInstallIdle
 * int --> unsigned for many index variables
 * constructor now takes an array of color names
 * added getMetricColorName
 *
 * Revision 1.9  1995/09/22 19:24:03  tamches
 * removed warnings under g++ 2.7.0
 *
 * Revision 1.8  1994/11/06  10:26:20  tamches
 * removed fullResourceHeight as a member vrble
 *
 * Revision 1.7  1994/10/14  10:27:45  tamches
 * Swapped the x and y axes -- now resources print vertically and
 * metrics print horizontally.  Can fit many, many more resources
 * on screen at once with no label overlap.  Multiple metrics
 * are now shown in the metrics axis.  Metric names are shown in
 * a "key" in the lower-left.
 *
 */

#ifndef _BARCHART_H_
#define _BARCHART_H_

// Note: we should make an effort to keep this class as tk-independent as possible.

#include "Vector.h"
#include "String.h"
#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

class BarChart {
 private:
   Tk_Window theWindow;
   tkInstallIdle drawWhenIdle;

   bool HaveSeenFirstGoodWid;
      // set when wid becomes non-zero (usually 1st resize event)
   int     borderPix; // in pixels
   int     currScrollOffset; // in pixels (always <= 0?)
   int     width, height;
   Display *display; // low-level display structure used in Xlib drawing calls

   XColor *greyColor; // for the background
   vector<string> metricColorNames; // needed for call by tcl
   vector<XColor *> metricColors;
      // an arbitrary-sized array (not necessarily equal to # metrics)
      // (note: this is a new characteristic!!!)

   GC myGC; // we change colors here very frequently with XSetForeground/
            // XSetBackground before actual drawing; hence, we could not use
            // Tk_GetGC

  public:
   enum DataFormats {Current, Average, Total};
  private:
   DataFormats DataFormat;

   Pixmap doubleBufferPixmap;
      // set with XCreatePixmap; you should reset with XFreePixmap and another call
      // to XCreatePixmap whenever the window size changes.  Delete with
      // XFreePixmap when done.
   void changeDoubleBuffering();

   unsigned numMetrics, numResources;
   unsigned numValidMetrics, numValidResources;
      // how many are enabled by visi lib (as opposed to deleted)

   vector<unsigned> indirectResources;
   vector<bool> validMetrics, validResources;
      // which metrics and resources are valid and should be drawn?

   int totalResourceHeight; // same as tcl vrble "currResourceHeight"
   int individualResourceHeight; // fullResourceHeight / numMetrics, but pinned to a max value (maxIndividualColorHeight tcl vrble)
   int resourceBorderHeight; // vertical padding due to "90%" rule, above.

   vector< vector<double> > values;
      // array [metric][rsrc] of numerical (not pixel) bar values
      // the basis for barPixWidths[][]
   vector< vector<int> > barPixWidths;
      // array [metric][rsrc] of pixel widths for each bar.  This
      // changes quite often (every time new data arrives) and
      // needs to be saved for the case of expose events...
   vector<double> metricCurrMaxVals;
      // array [metric] of the current y-axis high value for each metric.
      // When new data comes in that is higher than this, I give the command
      // to rethink the metrics axis.

   bool TryFirstGoodWid();
      // if wid is still zero, try a fresh Tk_WindowId() to try and change that...

   void lowestLevelDrawBarsDoubleBuffer();
      // called by the below routine

   static void lowestLevelDrawBars(ClientData pthis);
      // assuming the barPixWidths[][] have changed, redraw bars
      // with a call to XFillRectanges() or Tk_Fill3DRectangle()'s.
      // the "static" ensures that "this" isn't required to
      // call this routine, which is absolutely necessary since
      // it gets installed as a Tk_DoWhenIdle() routine...

   void rethinkValidMetricsAndResources();

   void RethinkMetricColors();
      // assuming metrics have change, rethink "metricColors" array
      
   void RethinkBarLayouts();
      // assuming a resize (but not added/deleted metrics), fill in barXoffsets,
      // barPixWidths, resourceCurrMaxY, etc, 

   void rethinkBarPixWidths();
      // assuming given new bar values and/or new metric max values and/or change
      // in window width, recalculate barWidths[][].

   void rethinkValues();
      // reallocates values[][], assuming a major config change

   void rethinkMetricMaxValues();
   void setMetricNewMax(unsigned metricindex, double newmaxval);
   double nicelyRoundedMetricNewMaxValue(unsigned metricindex, double newmaxval);

  public:

   BarChart(char *tkWindowName, unsigned iNumMetrics, unsigned iNumResources,
	    const vector<string> &colorNames);
  ~BarChart();

   unsigned getNumMetrics() const {
      return numMetrics;
   }
   unsigned getNumFoci() const {
      return numResources;
   }

   void processFirstGoodWid();
   void processResizeWindow(int newWidth, int newHeight);
   void processExposeWindow();

   void RethinkMetricsAndResources();
      // erase and reallocate barXoffsets, barWidths, barWidths, resourceCurrMaxY;
      // set numMetrics, numResources -- all based on a complete re-reading from
      // dataGrid[][].  (If visi had provided more fine-grained callbacks than
      // ADDMETRICSRESOURCES, such a crude routine would not be necessary.)
      // When done, redraws.
   void rethinkIndirectResources(); // needed to implement sorting

   void processNewData(int newBucketIndex);
      // assuming new data has arrived at the given bucket index for all
      // metric/rsrc pairs, read the new information from dataGrid[][],
      // update barWidths[][] accordingly, and call lowLevelDrawBars()

   void processNewScrollPosition(int newPos);
   void rethinkDataFormat(DataFormats);

   const string &getMetricColorName(unsigned index) const {
      index %= metricColorNames.size();
      return metricColorNames[index];
   }

   void setMetricNewMaxLL(unsigned metricindex, double newmaxval); // ugly
};

extern BarChart *theBarChart;

#endif
