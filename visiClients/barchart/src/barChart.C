// This is the C++ portion of barchart version 2.
// Much of the user interface mundane stuff (menus, etc.) is still
// programmed in tk/tcl in barChart.tcl.
// There is fairly extensive assertion checking, even in critical
// path routines.  May want to #define NDEBUG to turn off assertions
// for maximum speed.  Can be done from makefile, e.g. a "make optimized"
// option which does -O and -DNDEBUG

/* $Log: barChart.C,v $
/* Revision 1.13  1995/04/01 01:35:13  tamches
/* Implemented NaN checking (needed on HP) of incoming dataGrid values
/*
 * Revision 1.12  1994/11/11  06:41:30  tamches
 * Fixed bug that required all metrics to be valid or else would
 * crash with assertion error.  Just because we haven't implemented
 * deleting does not mean that metrics cannot become invalid; they
 * can become invalid when no more met/res pairs for the metric
 * are Enabled() in datagrid...
 *
 * Revision 1.11  1994/11/06  10:31:40  tamches
 * greatly improved commenting
 * Changed bar height algorithm to pin at a minimum individual bar
 * height.
 * Much better implementation of numValidResources, validResources[],
 * and indirectResources[], especially w.r.t assertion checking.
 * Many loops now use numValidResources instead of numResources
 * as their upper bounds, as they always should have (e.g. in
 * calculating the total needed height)
 *
 * Revision 1.10  1994/10/14  10:27:40  tamches
 * Swapped the x and y axes -- now resources print vertically and
 * metrics print horizontally.  Can fit many, many more resources
 * on screen at once with no label overlap.  Multiple metrics
 * are now shown in the metrics axis.  Metric names are shown in
 * a "key" in the lower-left.
 *
 * Revision 1.9  1994/10/13  00:49:57  tamches
 * Implemented sorting of resources.
 * Fixed deleting of resources.
 * Rearranged menus to be more standards-ish
 *
 * Revision 1.8  1994/10/11  22:05:11  tamches
 * Fixed resize bug whereupon a resize while paused would blank out the
 * bars until continue was pressed.
 *
 * Better support for deleted resources via variables
 * validMetrics and validResources
 *
 * Revision 1.7  1994/10/10  23:08:37  tamches
 * preliminary changes on the way to swapping the x and y axes
 *
 * Revision 1.6  1994/10/07  22:07:03  tamches
 * Fixed some resize bugs
 *
 * Revision 1.5  1994/10/04  22:11:31  tamches
 * moved color codes to barChart.tcl variable
 *
 * Revision 1.4  1994/10/04  19:01:05  tamches
 * Cleaned up the resource bar color choices
 *
 * Revision 1.3  1994/09/30  23:13:41  tamches
 * reads resource width from tcl as "currResourceWidth", to accomodate
 * new barChart.tcl code which adjusts this variable when resources
 * are added/deleted.  (previously it had been constant)
 *
 * Revision 1.2  1994/09/29  20:05:32  tamches
 * minor cvs fixes
 *
 * Revision 1.1  1994/09/29  19:48:27  tamches
 * initial implementation.  A to-do list is kept in barChart.tcl
 *
 */

// tk/tcl has a very nice interface for mixing C++ and tk/tcl
// scripts.  From within tcl, C++ code can be called via
// new commands; new commands are created with a call
// to Tcl_CreateCommand(...)  Unfortunately, you can see
// a catch-22 here; in order to even call Tcl_CreateCommand(),
// we need to be out of tcl and in C++!  The only way around
// this problem is to start the process in C++ and have it
// call tk/tcl.  This is done with Tcl_EvalFile(...).  Of
// course, just before this, we sneak in our Tcl_CreateCommand()
// call.

// Somewhere in the tk library, there is a main() that
// basically does this for us; you have to provide a -f tcl-file-name
// option so it knows what to send to Tcl_EvalFile(...)
// This main() assumes a function Tcl_InitApp(), which will
// be called before the Tcl_EvalFile() is performed...that's
// the time to sneak in our Tcl_CreateCommand() stuff...

#include <assert.h>
#include <stdlib.h> // exit()
#include <iostream.h>
#include <math.h>

#include <tcl.h>
#include <tk.h>

#include "visi/h/visualization.h"
#include "dg2.h"
#include "barChartTcl.h"

// g++ stuff for efficient template memory management:
#pragma implementation "array2d.h"

#include "barChart.h"

 // main data structure; holds bar information.  Does not
 // hold resources axis or metrics axis information (or contents), because
 // they are managed just fine from tcl (barChart.tcl) at present.
 // Created dynamically in DrawBarsInstallCommand.  Cannot be
 // created before then since necessary constructor arguments
 // such as tk window name are not yet known.

BarChart *theBarChart;

/* ********************************************************************
 * *********************** BarChart methods ***************************
 * ********************************************************************
*/

BarChart::BarChart(char *tkWindowName,
		   const bool dblBuffer, const bool noFlicker,
		   const int initNumMetrics, const int initNumResources,
		   const bool initFlushFlag) :
            metricColors(initNumMetrics),
	    indirectResources(initNumResources),
	    validMetrics(initNumMetrics),
	    validResources(initNumResources),
	    barWidths (initNumMetrics, initNumResources),
	    barValues  (initNumMetrics, initNumResources),
            metricCurrMaxVals(initNumMetrics)
             {

   theWindowName = new char[strlen(tkWindowName)+1];
   if (theWindowName == NULL)
      panic("BarChart constructor: out of memory!");
   strcpy(theWindowName, tkWindowName);

   theWindow = Tk_NameToWindow(MainInterp, tkWindowName, Tk_MainWindow(MainInterp));
   if (theWindow == NULL)
      panic("BarChart constructor: Tk_NameToWindow() failed!");

   HaveSeenFirstGoodWid = false;
   wid      = Tk_WindowId(theWindow);
   borderPix = 2;
   currScrollOffset = 0;
   width    = Tk_Width(theWindow);
   height   = Tk_Height(theWindow);
   display  = Tk_Display(theWindow);
   width  -= borderPix*2;
   height -= borderPix*2;
   
   greyColor = Tk_GetColor(MainInterp, theWindow, None, Tk_GetUid("grey"));

   DataFormat = Current;
   
   RethinkMetricsAndResources();
   // Rethinks numMetrics, numResources, numValidMetrics, numValidResources,
   //         validMetrics[], validResources[], barValues[][] from DataGrid.
   // Rethinks indirectResources[] from tcl.
   // Rethinks metric max values, colors, bar positioning
   // Clears screen; redraws.
}

BarChart::~BarChart() {
   Tk_CancelIdleCall(lowestLevelDrawBars, NULL);

   delete [] theWindowName;

   XFreeGC(display, myGC);   

   Tk_FreeColor(greyColor);
   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++)
      Tk_FreeColor(metricColors[metriclcv]);

   XFreePixmap(display, doubleBufferPixmap);
}

void BarChart::changeDoubleBuffering() {
   // note that we don't try to optimize and return immediately if
   // the mode hasn't changed.  Why? to accomodate possible change in size.

   if (!HaveSeenFirstGoodWid)
      return;

   // erase offscreen pixmap and reallocate with new size
   XFreePixmap(display, doubleBufferPixmap);

   doubleBufferPixmap = XCreatePixmap(display, wid,
				      Tk_Width(theWindow),
				      Tk_Height(theWindow),
				      Tk_Depth(theWindow));
         // note that we use Tk_Width and Tk_Height instead of width and height.
         // these values differ by (borderPix) in each dimension.
}

bool BarChart::TryFirstGoodWid() {
   // returns true if the wid is valid

   if (!HaveSeenFirstGoodWid) {
      wid = Tk_WindowId(theWindow);
      if (wid == 0)
         return false; // sigh; still invalid

      HaveSeenFirstGoodWid = true;

      // initialize gc
      XGCValues values;
      values.foreground = greyColor->pixel;
      values.background = greyColor->pixel;
      // values.graphics_exposures = False;

      myGC = XCreateGC(display, wid,
		       GCForeground | GCBackground,
		       &values);
      if (NULL==myGC)
         panic("BarChart constructor: XCreateGC() failed!");

      // initialize offscreen pixmap
      doubleBufferPixmap = XCreatePixmap(display, wid, 1, 1, 1); // temporary
      changeDoubleBuffering();
					 
      // simulate a resize
      processResizeWindow(Tk_Width(theWindow), Tk_Height(theWindow));
   }

   return true;   
}

void BarChart::processResizeWindow(const int newWidth, const int newHeight) {
   if (!TryFirstGoodWid())
      return; // our window is still invalid

   width  = newWidth - 2*borderPix; // subtract left+right border
   height = newHeight - 2*borderPix; // subract top+bottom border

   // update off-screen pixmap's dimensions to accomodate the resize
   changeDoubleBuffering();

   // rethink resource height and rethink widths of each bar
   RethinkBarLayouts();
}

void BarChart::processExposeWindow() {
   if (!TryFirstGoodWid())
      return; // our window is still invalid

   lowLevelDrawBars();
}

char *gimmeColorName(const int metriclcv) {
   // look in the tcl array "barColors"

   char *numColorsString = Tcl_GetVar(MainInterp, "numBarColors",
				      TCL_GLOBAL_ONLY);
   if (NULL==numColorsString) {
      cerr << "gimmeColorName: could not read tcl variable numBarColors." << endl;
      exit(5);
   }
   int numColors = atoi(numColorsString);

   const int index = metriclcv % numColors;
   char buffer[10];
   sprintf(buffer, "%d", index);

   char *resultString = Tcl_GetVar2(MainInterp, "barColors", buffer,
				    TCL_GLOBAL_ONLY);
   if (NULL==resultString)
      panic("gimmeColorName: could not read tcl variable barColors().");

   return resultString;
}

void BarChart::RethinkMetricColors() {
   metricColors.reallocate(numMetrics);

   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      XColor *theColor = Tk_GetColor(MainInterp, theWindow, None,
				     Tk_GetUid(gimmeColorName(metriclcv)));

      metricColors[metriclcv] = theColor;
   }
}

void BarChart::RethinkMetricsAndResources() {
   // Rethink numMetrics, numResources, numValidMetrics, numValidResources,
   //         validMetrics[], validResources[], barValues[][] from DataGrid.
   // Rethink indirectResources[] from tcl.
   // rethink metric max values, colors, bar positioning
   // Clears screen; redraws.

   numMetrics   = dataGrid.NumMetrics();
   numResources = dataGrid.NumResources();

   // reallocate and rethink validMetrics, validResources
   rethinkValidMetricsAndResources(); 

   // rethink bar values from dataGrid
   rethinkBarValues();

   if (TCL_ERROR == Tcl_Eval(MainInterp, "rethinkIndirectResources false"))
      panic("BarChart::RethinkMetricsAndResources() -- could not execute rethinkIndirectResources");

   rethinkIndirectResources();

   // rethink metric max values from tcl
   rethinkMetricMaxValues();
   RethinkMetricColors(); // reallocates and rethinks metricColors[]
   RethinkBarLayouts(); // as if there were a resize (no reallocations, but
                        // resets/rethinks barXoffsets[], barWidths[], barWidths[]

   if (HaveSeenFirstGoodWid)
      lowLevelDrawBars();
}

void BarChart::rethinkBarValues() {
   // Given: updated numMetrics, numResources, numValidMetrics,
   //        numValidResources, validMetrics[], validResources[]
   // Does:  reallocates and rethinks barValues[][] from dataGrid
   // Does not: do anything to the screen

   barValues.reallocate(numMetrics, numResources);

   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      if (!validMetrics[metriclcv]) continue;

      for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
         if (!validResources[resourcelcv]) continue;

         visi_GridCellHisto &theCell = dataGrid[metriclcv][resourcelcv];
         const sampleType theValue = theCell.Value(theCell.LastBucketFilled());
            // warning: "theValue" may be a NaN
         
         barValues[metriclcv][resourcelcv] = isnan(theValue) ? 0 : theValue;
      }
   }
}

void BarChart::rethinkMetricMaxValues() {
   // Given: updated numMetrics, validMetrics[]
   // Does:  reallocates and rethinks metricCurrMaxVals[] from tcl
   // Does not: draw anything

   metricCurrMaxVals.reallocate(numMetrics);
   
   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      if (!validMetrics[metriclcv]) continue;

      char buffer[64];
      sprintf(buffer, "%d", metriclcv);
      char *str = Tcl_GetVar2(MainInterp, "metricMaxValues", buffer,
			      TCL_GLOBAL_ONLY);
      if (str == NULL)
         panic("BarChart::RethinkMetricsAndResources() -- could not read 'metricMaxValues'");

      metricCurrMaxVals[metriclcv] = atof(str);
   }
}

void BarChart::processNewData(const int newBucketIndex) {
   // Given: new datagrid bucket data for all enabled metric/rsrc pairs.
   //        up-to-date numMetrics, numResources,
   //                   numValidMetrics, numValidResources,
   //                   validMetrics[], validResources[]
   // Does:  rethinks barValues[][] and (if necessary) metricCurrMaxVals[].
   //        Then calls rethinkBarWidths() which rethinks barWidths[][]
   //        and redraws.
   // Does not: care about sorting order in any way (a future, optimized
   //           version will)

   // Called on the critical path -- we must be quick!
   
   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      if (!validMetrics[metriclcv]) continue;

      visi_GridHistoArray &metricValues = dataGrid[metriclcv];

      for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
         if (!validResources[resourcelcv]) continue;

         visi_GridCellHisto &theCell = metricValues[resourcelcv];

         if (theCell.Valid) {
            // note that we check the .Valid flag, not the .enabled flag

            register double newVal;
            switch (DataFormat) {
               case Current:
                  newVal = theCell.Value(newBucketIndex);
                  break;
               case Average:
                  newVal = dataGrid.AggregateValue(metriclcv, resourcelcv);
                  break;
               case Total:
                  newVal = dataGrid.SumValue(metriclcv, resourcelcv);
                  break;
               default:
                  panic("BarChart::processNewData() -- unknown data format!");
	    }

            barValues[metriclcv][resourcelcv] = isnan(newVal) ? 0 : newVal;

            // the dreaded check for y-axis overflow (slows things down
            // greatly if there is indeed overflow)
            if (newVal > metricCurrMaxVals[metriclcv])
               setMetricNewMax(metriclcv, newVal); // nuts!
            else
               ; // yea!
	 }
         else
            barValues[metriclcv][resourcelcv]=0; // hmmm...
      }
   }

   RethinkBarWidths();
}

double BarChart::nicelyRoundedMetricNewMaxValue(int metricindex, double newmaxval) {
   assert(0<=metricindex && metricindex<numMetrics);

   char buffer[256];
   sprintf(buffer, "lindex [getMetricHints %s] 3", dataGrid.MetricName(metricindex));

   if (TCL_OK != Tcl_Eval(MainInterp, buffer))
      panic("BarChart::nicelyRoundedMetricNewMaxValue() -- could not eval");

   double hintedStep = atof(MainInterp->result);

   double result = newmaxval + (hintedStep - fmod(newmaxval, hintedStep));
      // if fmod() doesn't return a number greater than 0, this won't work quite right

   return result;
}

void BarChart::setMetricNewMax(int metricindex, double newmaxval) {
   // given an actual bar value (newmaxval) that overflows the current
   // y-axis maximum value for the given metric, rethink what the
   // y-axis maximum value for the given metric should be.
   // Note that it may not be exactly "newmaxval"; we usually want
   // to round to a relatively nice number.

   assert(0<=metricindex && metricindex<numMetrics);

   newmaxval = nicelyRoundedMetricNewMaxValue(metricindex, newmaxval);
   metricCurrMaxVals[metricindex] = newmaxval;

   char buffer[128];
   sprintf(buffer, "processNewMetricMax %d %g", metricindex, newmaxval);
   if (TCL_OK != Tcl_Eval(MainInterp, buffer))
      cerr << "warning -- BarChart::setMetricNewMax() could not inform barChart.tcl of new max-y-value (no script processNewMetricMax?)" << endl;
}

void BarChart::RethinkBarLayouts() {
   // note -- validResources[] **must** be set before calling this routine

   // Assuming a resize, change in sorting order, or added/deleted metric
   // (but not new data callbacks), reallocate and fill in bar heights and
   // calculate individualResourceHeight, etc.

   // does not touch metricCurrMaxVals or barValues[][]

   // Start off by reading some tcl vrbles (barChart.tcl)
   char *totalResourceHeightStr =
        Tcl_GetVar(MainInterp, "currResourceHeight", TCL_GLOBAL_ONLY);
   if (NULL == totalResourceHeightStr)
      panic("BarChart::RethinkBarLayouts(): couldn't read tcl 'currResourceHeight'");
   totalResourceHeight  = atoi(totalResourceHeightStr);

   char *maxIndividualColorHeightStr =
        Tcl_GetVar(MainInterp, "maxIndividualColorHeight", TCL_GLOBAL_ONLY);
   if (NULL == maxIndividualColorHeightStr)
      panic("BarChart::RethinkBarLayouts(): couldn't read tcl 'maxIndividualColorHeight'");
   const int maxIndividualColorHeight = atoi(maxIndividualColorHeightStr);

// minIndividualColorHeight [not yet implemented]
//   char *minIndividualColorHeightStr =
//        Tcl_GetVar(MainInterp, "minIndividualColorHeight", TCL_GLOBAL_ONLY);
//   if (NULL == minIndividualColorHeightStr)
//      panic("BarChart::RethinkBarLayouts(): couldn't read tcl 'minIndividualColorHeight'");
//   const int minIndividualColorHeight = atoi(minIndividualColorHeightStr);

   // Here we go:   

   // First of all, we want to use only 90% of the total resource height,
   // to avoid bars of 2 resources touching each other. (The extra space will
   // be padded evenly on both sides of the bars of this resource)
   int fullResourceHeight   = (totalResourceHeight * 90) / 100;

   // calculate height of each bar... [individualResourceHeight should be named
   //                                  individualColorHeight]
   individualResourceHeight = (numValidMetrics == 0) ? 0 :
                              fullResourceHeight / numValidMetrics;
   // ... but there is a maximum value (e.g. if we have just 1 metric)
   if (individualResourceHeight > maxIndividualColorHeight) {
//      cout << "Pinning individual color height from " << individualResourceHeight
//           << " to " << maxIndividualColorHeight << endl;
      individualResourceHeight = maxIndividualColorHeight;
   }   

   // rethink fullResourceHeight now...
   fullResourceHeight = individualResourceHeight * numValidMetrics;
   assert(fullResourceHeight * 100 <= 90 * totalResourceHeight);

   resourceBorderHeight = (totalResourceHeight - fullResourceHeight) / 2;

   barWidths.reallocate(numMetrics, numResources);
   RethinkBarWidths();
}

void BarChart::RethinkBarWidths() {
   // Given: udpated numValidMetrics, indirectResources[], validResources[],
   //                numValidResourcse, validMetrics[], barValues[][],
   //                metricCurrMaxVals[]
   // Does: rethinks barWidths[][], redraws

   // Set the height of each bar to the fraction of window height
   // that equals the fraction of the bar's current value to its
   // metric max value.

   // This routine is called on the "critical path" --- when "new data
   // callbacks" arrive.  We must be quick! (but keep the assert()s)

   // Move on screen in sorted order, but store changes in actual order
   for (int resourcelcv=0; resourcelcv<numValidResources; resourcelcv++) {
      // account for possible resources sorting:
      const int actualResource = indirectResources[resourcelcv];
      assert(0<=actualResource);
      assert(actualResource<numResources);
      assert(validResources[actualResource]);

      for (int metriclcv=0; metriclcv<numValidMetrics; metriclcv++) {
         // account for possible metrics sorting:
         const int actualMetric = metriclcv;
         assert(0<=actualMetric);
         assert(actualMetric<numMetrics);
         assert(validMetrics[actualMetric]);

         register double theWidth = barValues[actualMetric][actualResource] /
	                            metricCurrMaxVals[actualMetric];
         // scale by window width (excluding border pixels)
         theWidth *= this->width;
         // truncate and store
         barWidths[actualMetric][actualResource] = (int)theWidth;
      }
   }

   // Draw!
   if (HaveSeenFirstGoodWid)
      lowLevelDrawBars();
}

void BarChart::processNewScrollPosition(int newPos) {
   // Given: new scrollbar top position
   //        up to date bar heights
   // Does:  updates currScrollOffset and redraws the bars

   if (!HaveSeenFirstGoodWid)
      return;

   // cout << "BarChart::processNewScrollPosition -- new pos=" << newPos << endl;

   // adjust the current x-pixel scroll offset value and
   // simulate an expose event
   currScrollOffset = -newPos;
 
   lowLevelDrawBars();
}

void BarChart::rethinkDataFormat() {
   // assuming the data format has changed, read its
   // value (from tcl) and adjust our internal settings accordingly.

   char *dataFormatString = Tcl_GetVar(MainInterp, "DataFormat",
				       TCL_GLOBAL_ONLY);
   if (dataFormatString == NULL) {
      cerr << "warning: BarChart::rethinkDataFormat() -- could not read tcl vrble 'DataFormat'; ignoring" << endl;
      return;
   }

   if (0==strcmp(dataFormatString, "Instantaneous"))
      DataFormat = Current;
   else if (0==strcmp(dataFormatString, "Average"))
      DataFormat = Average;
   else if (0==strcmp(dataFormatString, "Sum"))
      DataFormat = Total;
   else {
      cerr << "BarChart::rethinkDataFormat() -- unrecognized format '"
           << dataFormatString << "'; ignoring" << endl;
      return;
   }

   // we have changed the data format.  The max y-values need
   // to be "re-thought" (else, what if we change from total to current;
   // the max y value will be so high and won't ever get lowered)
   RethinkMetricsAndResources();
      // a bit more than this is really needed!
}

void BarChart::rethinkValidMetricsAndResources() {
   // Given: updated numMetrics, numResources
   // Does:  reallocates and rethinks validMetrics[], validResources[],
   //        numValidMetrics, numValidResources based on dataGrid enabled
   //        flags

   validMetrics.reallocate(numMetrics);
   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++)
      validMetrics[metriclcv]=false;

   validResources.reallocate(numResources);
   for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) 
      validResources[resourcelcv]=false;

   for (metriclcv=0; metriclcv<numMetrics; metriclcv++)
      for (resourcelcv=0; resourcelcv<numResources; resourcelcv++)
         if (dataGrid[metriclcv][resourcelcv].Enabled()) {
            validMetrics[metriclcv] = true;
            validResources[resourcelcv] = true;
	 }

   numValidMetrics=0;
   for (metriclcv=0; metriclcv<numMetrics; metriclcv++)
      if (validMetrics[metriclcv])
         numValidMetrics++;

   numValidResources=0;
   for (resourcelcv=0; resourcelcv<numResources; resourcelcv++)
      if (validResources[resourcelcv])
         numValidResources++;
}

void BarChart::rethinkIndirectResources() {
   // Given: updated numValidResources, updated tcl array indirectResources()
   // Does:  reallocates and rethinks indirectResources[]
   //        extensive assertion checking

   indirectResources.reallocate(numValidResources);

   for (int resourcelcv=0; resourcelcv<numValidResources; resourcelcv++) {
      char buffer[20];
      sprintf(buffer, "%d", resourcelcv);

      char *string = Tcl_GetVar2(MainInterp, "indirectResources",
				 buffer, TCL_GLOBAL_ONLY);
      if (string == NULL)
         panic("BarChart::rethinkIndirectResources -- could not read indirectResources() from tcl");

      const int indirectNum = atoi(string);
      if (indirectNum<0)
         panic("BarChart::rethinkIndirectResources -- negative indirect value");
      else if (indirectNum>=numResources) {
         cerr << "BarChart::rethinkIndirectResources -- indirect value of "
              << indirectNum << ' ' << "is too high (numResources="
              << numResources << ')' << endl;
         panic("");
      }
      else if (!validResources[indirectNum]) {
         cerr << "BarChart::rethinkIndirectResources -- resource #"
              << indirectNum << "is not presently valid" << endl;
         panic("");
      }

      indirectResources[resourcelcv] = indirectNum;
   }
}

bool BarChart::currentlyInstalledLowLevelDrawBars=false;

void BarChart::lowLevelDrawBars() {
   // Called on the critical path (new-data callbacks).
   // We must be quick!

   // Perhaps we should check to see if the barchart program has
   // been shut down before attempting to do anything?  Nah,
   // just be sure to call Tk_CancelIdleCall() on lowestLevelDrawBars()
   // in the destructor.

   if (!Tk_IsMapped(theWindow))
      return;

   if (!HaveSeenFirstGoodWid)
      return;

   if (!currentlyInstalledLowLevelDrawBars) {
      currentlyInstalledLowLevelDrawBars = true;

      Tk_DoWhenIdle(lowestLevelDrawBars, NULL);
   }
}

void BarChart::lowestLevelDrawBarsDoubleBuffer() {
   // Called on the critical path -- we must be quick!

   unsigned long bgPixel = greyColor->pixel;

   // note that the double-buffer pixmap DOES include space for border
   // pixels, even though we don't make use of such pixels.  This was
   // done because keeping the offscreen pixmap the same size as the
   // window simplifies the drawing; e.g. barXoffsets[] already have
   // borderPix added in to each element and it would be clumsy to
   // compensate.

   // Our XCopyArea() (when done drawing) is rigged (check the args) to
   // not copy the border pixel area...

   // clear the offscreen buffer.  XClearArea() works only on windows,
   // so fill a rectangle with the background color.
   XSetForeground(display, myGC, bgPixel);
   XSetBackground(display, myGC, bgPixel);
   XFillRectangle(display, doubleBufferPixmap,
		  myGC,
		  borderPix, // x-offset, relative to drawable
		  borderPix, // y-offset, relative to drawable
		  width,     // does not include border pixels
		  height     // does not include border pixels
		  );

   // Do the drawing onto offscreen pixmap [yikes -- expensive!]

   // Loop through the resources in sorted order
   int resourceBoundary = borderPix + currScrollOffset;
   const int left = 0 + borderPix;
   
   for (int resourcelcv=0; resourcelcv<numValidResources; resourcelcv++) {
      // account for sorted resources:
      const int actualResource = indirectResources[resourcelcv];
      assert(actualResource>=0);
      assert(actualResource<numResources);
      assert(validResources[actualResource]);

      int top = resourceBoundary + resourceBorderHeight;

      // for robustness:
      int thisTimeNumValidMetrics=0;      

      for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
         // Eventually, we will account for sorted metrics here.
         // Until then, we test the validMetrics[] field.  If false,
         // then skip this metric.
         const int actualMetric = metriclcv;
         if (!validMetrics[actualMetric]) continue;

         assert(actualMetric>=0);
         assert(actualMetric<numMetrics);
         assert(validMetrics[actualMetric]);

         // for robustness:
         thisTimeNumValidMetrics++;

         XSetForeground(display, myGC, metricColors[actualMetric]->pixel);
   
         const int thisBarWidth = barWidths[actualMetric][actualResource];
         XFillRectangle(display,
			doubleBufferPixmap,
			myGC,
		        left,
			top,
			thisBarWidth,
			individualResourceHeight // height
			);

         top += individualResourceHeight;
      }

      resourceBoundary += totalResourceHeight;
 
      assert(thisTimeNumValidMetrics == numValidMetrics);
   }

   // copy offscreen pixmap onto screen
   
   XCopyArea(display,
	     doubleBufferPixmap, // source
	     wid,                // dest
	     myGC,
	     borderPix, borderPix, // source x, y (note how we exclude the border)
	     width, height, // these vrbles are preset to exclude the borders
	     borderPix, borderPix // dest x, y (note how we exclude the border)
	    );
}

void BarChart::lowestLevelDrawBars(ClientData ignore) {
   // NOTE: a --static-- member function --> no "this" exists!!

   currentlyInstalledLowLevelDrawBars = false;
      // now, new requests will be handled
      // It seems wrong to delay this line until the bottom of the
      // routine; it could cause new data to be ignored

   if (!theBarChart->HaveSeenFirstGoodWid) {
      // Perhaps a race condition; new data arrived before the barchart
      // was fully constructed?
      cout << "BarChart::lowestLevelDrawBars -- haven't yet mapped? (ignoring)" << endl;
      return;
   }

   // Finally, we actually draw:
   theBarChart->lowestLevelDrawBarsDoubleBuffer();
}
