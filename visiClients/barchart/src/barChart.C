// This is the C++ portion of barchart version 2.
// Much of the user interface mundane stuff (menus, etc.) is still
// programmed in tk/tcl in barChart.tcl.

/* $Log: barChart.C,v $
/* Revision 1.10  1994/10/14 10:27:40  tamches
/* Swapped the x and y axes -- now resources print vertically and
/* metrics print horizontally.  Can fit many, many more resources
/* on screen at once with no label overlap.  Multiple metrics
/* are now shown in the metrics axis.  Metric names are shown in
/* a "key" in the lower-left.
/*
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
	    barWidths (initNumMetrics, initNumResources),
	    barValues  (initNumMetrics, initNumResources),
	    validMetrics(initNumMetrics),
	    validResources(initNumResources),
	    indirectResources(initNumResources),
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
      // sets numMetrics, numResources, barXoffets, barWidths,
      // barWidths, barValues, metricCurrMaxVals
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

   RethinkBarLayouts();
}

void BarChart::processExposeWindow() {
   if (!TryFirstGoodWid())
      return; // our window is still invalid

   lowLevelDrawBars();
}

char *gimmeColorName(const int metriclcv) {
   // look in the tcl array "barColors"

   char *numColorsString = Tcl_GetVar(MainInterp, "numBarColors", TCL_GLOBAL_ONLY);
   if (NULL==numColorsString) {
      cerr << "gimmeColorName: could not read tcl variable numBarColors." << endl;
      exit(5);
   }
   int numColors = atoi(numColorsString);

   const int index = metriclcv % numColors;
   char buffer[10];
   sprintf(buffer, "%d", index);

   char *resultString = Tcl_GetVar2(MainInterp, "barColors", buffer, TCL_GLOBAL_ONLY);
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
   // Clear window, reallocate barXoffsets, barWidths, barWidths,
   // metricCurrMaxVals.

   // Re-read numMetrics, numResources, barValues[][] from dataGrid
   // When done, redraw.

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
   barValues.reallocate(numMetrics, numResources);

   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      if (!validMetrics[metriclcv]) continue;

      for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
         if (!validResources[resourcelcv]) continue;

         visi_GridCellHisto &theCell = dataGrid[metriclcv][resourcelcv];
         barValues[metriclcv][resourcelcv]=theCell.Value(theCell.LastBucketFilled());
      }
   }
}

void BarChart::rethinkMetricMaxValues() {
   // reallocate metricCurrMaxVals[] and read in their values from tcl
   // assumes numMetrics has been set

   metricCurrMaxVals.reallocate(numMetrics);
   
   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      if (!validMetrics[metriclcv]) continue;

      char buffer[64];
      sprintf(buffer, "%d", metriclcv);
      char *str = Tcl_GetVar2(MainInterp, "metricMaxValues", buffer, TCL_GLOBAL_ONLY);
      if (str == NULL)
         panic("BarChart::RethinkMetricsAndResources() -- could not read 'metricMaxValues'");

      metricCurrMaxVals[metriclcv] = atof(str);
   }
}

void BarChart::processNewData(const int newBucketIndex) {
   // assuming new data has arrived at the given bucket index for all
   // metric/rsrc pairs, read the new information from dataGrid[][],
   // update barWidths[][], barValues[][], and metricCurrMaxVals[] (if needed),
   // and call lowLevelDrawBars()
   
   // PASS 1: Update barValues[][] and check for y-axis overflow, calling
   //         setMetricNewMaxY() if overflow is detected.

   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      if (!validMetrics[metriclcv]) continue;

      visi_GridHistoArray &metricValues = dataGrid[metriclcv];

      for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
         if (!validResources[resourcelcv]) continue;

         visi_GridCellHisto &theCell = metricValues[resourcelcv];

         if (theCell.Valid) { // note that we check the .Valid flag, not the .enabled flag
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
                  assert(false);
	    }

            barValues[metriclcv][resourcelcv] = newVal;

            // the dreaded check for y-axis overflow (slows things down greatly if there
            // is indeed overflow)
            if (newVal > metricCurrMaxVals[metriclcv])
               setMetricNewMax(metriclcv, newVal);
	 }
         else
            barValues[metriclcv][resourcelcv]=0;
      }
   }

   // PASS 2: Now that the y-axis overflows, if any, have been processed, we can
   //         calculate barWidths with confidence that the scale isn't going to
   //         change.

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

   char buffer[256];
   sprintf(buffer, "processNewMetricMax %d %g", metricindex, newmaxval);
   if (TCL_OK != Tcl_Eval(MainInterp, buffer))
      cerr << "warning -- BarChart::setMetricNewMax() could not inform barChart.tcl of new-max-y-value (no script processNewMetricMax?)" << endl;
}

void BarChart::RethinkBarLayouts() {
   // note -- validResources[] **must** be set before calling this routine

   // Assuming a complete resize or a change in sorting order (but not an added or deleted metric
   // or resource or even new data values), reallocate and fill in bar widths and
   // calculate individualResourceHeight, etc.

   // does not touch metricCurrMaxVals or barValues

   // note: the following loop starts with resources, then does metrics.  should not
   // cause any big problems, and more intuitive in this case...
   char *fullResourceHeightStr = Tcl_GetVar(MainInterp,
					   "currResourceHeight", TCL_GLOBAL_ONLY);
   if (NULL == fullResourceHeightStr)
      panic("BarChart::RethinkBarLayouts() -- could not read 'currResourceHeight' from tcl");

   totalResourceHeight  = atoi(fullResourceHeightStr);
   fullResourceHeight   = (totalResourceHeight * 90) / 100;
   resourceBorderHeight = (totalResourceHeight - fullResourceHeight) / 2;
   individualResourceHeight = (numValidMetrics == 0) ? 0 : fullResourceHeight / numValidMetrics;

   barWidths.reallocate(numMetrics, numResources);
   RethinkBarWidths();
}

void BarChart::RethinkBarWidths() {
   // note -- validResources[] **must** be set before calling this routine

   // set the height of each bar to the fraction of window height
   // that equals the fraction of the bar's current value to its
   // metric max value.

   // move from left to right on the screen, in sorted order
   // (but store changes in original order)
   for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
      const int actualResource = indirectResources[resourcelcv];
      assert(0<=actualResource);
      assert(actualResource<numResources);
      if (!validResources[actualResource]) continue;

      for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
         const int actualMetric = metriclcv;
         if (!validMetrics[actualMetric]) continue;

         register double theWidth = barValues[actualMetric][actualResource] /
	                             metricCurrMaxVals[actualMetric];
         theWidth *= this->width; // scale by window height (excluding border pixels)
         barWidths[actualMetric][actualResource] = (int)theWidth;
      }
   }

   if (HaveSeenFirstGoodWid)
      lowLevelDrawBars();
}

void BarChart::processNewScrollPosition(int newPos) {
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

   char *dataFormatString = Tcl_GetVar(MainInterp, "DataFormat", TCL_GLOBAL_ONLY);
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
      // a bit more than is really needed!
}

void BarChart::rethinkValidMetricsAndResources() {
   // go to the datagrid in order to rethink which of
   // the metrics and resources are good ones and should
   // be drawn

   validMetrics.reallocate(numMetrics);
   validResources.reallocate(numResources);

   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++)
      validMetrics[metriclcv]=false;

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
   // The sorting order has changed; re-read from tcl variables
   // NOTE: If you delete a resource, be sure to update tcl vrble indirectResources()
   //       ** BEFORE ** calling this routine

   indirectResources.reallocate(numResources);

   for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
      char buffer[20];
      sprintf(buffer, "%d", resourcelcv);

      char *string = Tcl_GetVar2(MainInterp, "indirectResources", buffer, TCL_GLOBAL_ONLY);
      if (string == NULL)
         panic("BarChart::rethinkIndirectResources -- could not read indirectResources() from tcl");

      const int indirectNum = atoi(string);
      if (indirectNum<0)
         panic("BarChart::rethinkIndirectResources -- negative indirect value");
      else if (indirectNum>=numResources) {
         cerr << "BarChart::rethinkIndirectResources -- indirect value of " << indirectNum << ' ';
         cerr << "is too high (numResources=" << numResources << ')' << endl;
         panic("");
      }

      indirectResources[resourcelcv] = indirectNum;
   }

   //cout << "Leaving BarChart::rethinkIndirectResources() with {";
   //for (resourcelcv=0; resourcelcv<numResources; resourcelcv++)
   //   cout << ' ' << indirectResources[resourcelcv];
   //cout << "}" << endl;
}

bool BarChart::currentlyInstalledLowLevelDrawBars=false;

void BarChart::lowLevelDrawBars() {
   // perhaps we should check to see if the barchart program has
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
   unsigned long bgPixel = greyColor->pixel;

   // note that the double-buffer pixmap DOES include space for border
   // pixels, even though we don't make use of such pixels.  This was
   // done because keeping the offscreen pixmap the same size as the
   // window simplifies the drawing; e.g. barXoffsets[] already have
   // borderPix added in to each element and it would be clumsy to
   // compensate.

   // Our XCopyArea() (when done drawing) is rigged (check the args) to
   // not copy the border pixel area...

   // clear the offscreen buffer.  XClearArea() works only on windows, so fill a rectangle
   // with the background color.
   XSetForeground(display, myGC, bgPixel);
   XSetBackground(display, myGC, bgPixel);
   XFillRectangle(display, doubleBufferPixmap,
		  myGC,
		  borderPix, // x-offset, relative to drawable
		  borderPix, // y-offset, relative to drawable
		  width,     // does not include border pixels
		  height     // does not include border pixels
		  );

   // do the drawing onto offscreen pixmap

   // loop through the resources in sorted order

   int resourceBoundary = borderPix + currScrollOffset;
   const int left = 0 + borderPix;
   
   for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
      const int actualResource = indirectResources[resourcelcv];
      assert(actualResource>=0);
      assert(actualResource<numResources);
      if (!validResources[actualResource]) continue;

      int top = resourceBoundary + resourceBorderHeight;
      
      for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
         const int actualMetric = metriclcv; // allow for sorting metrics in the future
         assert(actualMetric>=0);
         assert(actualMetric<numMetrics);
         if (!validMetrics[actualMetric]) continue;

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
   }

   // copy offscreen pixmap onto screen
   // cout << '{'; cout.flush();
   
   XCopyArea(display,
	     doubleBufferPixmap, // source
	     wid,                // dest
	     myGC,
	     borderPix, borderPix, // source x, y (note how we exclude the border)
	     width, height, // these vrbles are preset to exclude the borders
	     borderPix, borderPix // dest x, y (note how we exclude the border)
	    );
//   cout << '}'; cout.flush();
}

void BarChart::lowestLevelDrawBars(ClientData ignore) {
   // cout << '['; cout.flush();

   // NOTE: a --static-- member function --> no "this" exists!!

   if (!theBarChart->HaveSeenFirstGoodWid) {
      cout << "BarChart::lowestLevelDrawBars -- haven't yet mapped? (ignoring)" << endl;
      return;
   }

   theBarChart->lowestLevelDrawBarsDoubleBuffer();

   currentlyInstalledLowLevelDrawBars = false; // now, new requests will be handled
      // It seems wrong to delay this line until the bottom of the
      // routine; it could cause new data to be ignored

   // cout << ']'; cout.flush();
}
