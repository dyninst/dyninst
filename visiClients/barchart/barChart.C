// This is the C++ portion of barchart version 2.
// Much of the user interface mundane stuff (menus, etc.) is still
// programmed in tk/tcl in barChart.tcl.

/* $Log: barChart.C,v $
/* Revision 1.9  1994/10/13 00:49:57  tamches
/* Implemented sorting of resources.
/* Fixed deleting of resources.
/* Rearranged menus to be more standards-ish
/*
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
            prevBarHeights(initNumMetrics, initNumResources),
	    barHeights (initNumMetrics, initNumResources),
	    barValues  (initNumMetrics, initNumResources),
	    validMetrics(initNumMetrics),
	    validResources(initNumResources),
	    indirectResources(initNumResources),
            metricCurrMaxVals(initNumMetrics),
	    flushFlag (initFlushFlag)
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
   isMapped = Tk_IsMapped(theWindow);
   display  = Tk_Display(theWindow);
   width  -= borderPix*2;
   height -= borderPix*2;
   
   greyColor = Tk_GetColor(MainInterp, theWindow, None, Tk_GetUid("grey"));

   DataFormat = Current;
   
   drawMode = dblBuffer ? DoubleBuffer : (noFlicker ? NoFlicker : Flicker);

   RethinkMetricsAndResources();
      // sets numMetrics, numResources, barXoffets, barWidths,
      // barHeights, prevBarHeights, barValues, metricCurrMaxVals
}

BarChart::~BarChart() {
   Tk_CancelIdleCall(lowestLevelDrawBars, NULL);

   delete [] theWindowName;

   XFreeGC(display, myGC);   

   Tk_FreeColor(greyColor);
   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++)
      Tk_FreeColor(metricColors[metriclcv]);

   changeDoubleBuffering(false, false); // free offscreen pixmap, if necessary
}

void BarChart::changeDoubleBuffering(bool doubleBuffer, bool noFlicker) {
   // note that we don't try to optimize and return immediately if
   // the mode hasn't changed.  Why? to accomodate possible change in size.

   if (!HaveSeenFirstGoodWid)
      return;

   DrawModes newDrawMode;
   if (doubleBuffer)
      newDrawMode = DoubleBuffer;
   else if (noFlicker)
      newDrawMode = NoFlicker;
   else
      newDrawMode = Flicker;

   if (drawMode == DoubleBuffer)
      // erase offscreen pixmap and reallocate with new size
      XFreePixmap(display, doubleBufferPixmap);

   drawMode = newDrawMode;

   if (drawMode == DoubleBuffer)
      doubleBufferPixmap = XCreatePixmap(display, wid,
					 Tk_Width(theWindow),
					 Tk_Height(theWindow),
					 Tk_Depth(theWindow));
         // note that we use Tk_Width and Tk_Height instead of width and height.
         // these values differ by (borderPix) in each dimension.
   else if (drawMode == NoFlicker) 
      ResetPrevBarHeights();
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
      if (drawMode == DoubleBuffer)
         doubleBufferPixmap = XCreatePixmap(display,
					    wid, // used only to determine the screen
					    Tk_Width(theWindow),
					    Tk_Height(theWindow),
					    Tk_Depth(theWindow)
					    );
					 
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
   changeDoubleBuffering(drawMode==DoubleBuffer, drawMode==NoFlicker);

   RethinkBarLayouts();
}

void BarChart::processExposeWindow() {
   if (!TryFirstGoodWid())
      return; // our window is still invalid

   // cout << "Welcome to BarChart::processExposeWindow" << endl;
   if (drawMode == NoFlicker)
      ResetPrevBarHeights();

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
   if (NULL==resultString) {
      cerr << "gimmeColorName: could not read tcl variable barColors()." << endl;
      exit(5);
   }

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

void BarChart::ClearScreen() {
   XSetForeground(display, myGC, greyColor->pixel);
   XSetBackground(display, myGC, greyColor->pixel);

   XFillRectangle(display, wid, myGC,
		  borderPix, // start-x
		  borderPix, // start-y
		  width, // this vrble should have already been adjusted re: borderPix
		  height);
}

void BarChart::ResetPrevBarHeights() {
   prevBarHeights.reallocate(numMetrics, numResources);

   if (drawMode == NoFlicker) {
      for (int metriclcv=0; metriclcv<numMetrics; metriclcv++)
         for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++)
            prevBarHeights[metriclcv][resourcelcv]=0;
      if (HaveSeenFirstGoodWid)
         ClearScreen();
   }
}

void BarChart::RethinkMetricsAndResources() {
   // Clear window, reallocate barXoffsets, barWidths, barHeights,
   // metricCurrMaxVals.

   // Re-read numMetrics, numResources, barValues[][] from dataGrid
   // When done, redraw.

   numMetrics   = dataGrid.NumMetrics();
   numResources = dataGrid.NumResources();

   barHeights    .reallocate(numMetrics, numResources);
   prevBarHeights.reallocate(numMetrics, numResources);
   barValues     .reallocate(numMetrics, numResources);
   metricCurrMaxVals.reallocate(numMetrics);

   rethinkValidMetricsAndResources(); // reallocate and rethink validMetrics, validResources
   rethinkIndirectResources(); // reallocate and rethink indirectResources, used for sorting

   // rethink bar values from dataGrid
   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      if (!validMetrics[metriclcv]) continue;

      for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
         if (!validResources[resourcelcv]) continue;

         visi_GridCellHisto &theCell = dataGrid[metriclcv][resourcelcv];
         barValues[metriclcv][resourcelcv]=theCell.Value(theCell.LastBucketFilled());
      }
   }

   // rethink metric max values from tcl
   for (metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      if (!validMetrics[metriclcv]) continue;

      char buffer[64];
      sprintf(buffer, "%d", metriclcv);
      char *str = Tcl_GetVar2(MainInterp, "metricMaxValues", buffer, TCL_GLOBAL_ONLY);
      if (str == NULL)
         panic("BarChart::RethinkMetricsAndResources() -- could not read 'metricMaxValues'");

      metricCurrMaxVals[metriclcv] = atof(str);
   }

   RethinkMetricColors(); // reallocates and rethinks metricColors[]
   RethinkBarLayouts(); // as if there were a resize (no reallocations, but
                        // resets/rethinks barXoffsets[], barWidths[], barHeights[],
                        // prevBarHeights[])

   if (HaveSeenFirstGoodWid)
      lowLevelDrawBars();
}

void BarChart::processNewData(const int newBucketIndex) {
   // assuming new data has arrived at the given bucket index for all
   // metric/rsrc pairs, read the new information from dataGrid[][],
   // update barHeights[][], barValues[][], and metricCurrMaxVals[] (if needed),
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
   //         calculate barHeights with confidence that the scale isn't going to
   //         change.

   RethinkBarHeights();
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
   // Assuming a complete resize or a change in sorting order (but not an added or deleted metric
   // or resource or even new data values), fill in barXoffsets, barWidths, etc.

   // does not touch metricCurrMaxVals or barValues

   // note: the following loop starts with resources, then does metrics.  should not
   // cause any big problems, and more intuitive in this case...
   char *fullResourceWidthStr = Tcl_GetVar(MainInterp,
					   "currResourceWidth", TCL_GLOBAL_ONLY);
   if (NULL == fullResourceWidthStr)
      panic("BarChart::RethinkBarLayouts() -- could not read 'currResourceWidth' from tcl");

   totalResourceWidth = atoi(fullResourceWidthStr);
   fullResourceWidth = (totalResourceWidth * 90) / 100;
   resourceBorderWidth = (totalResourceWidth - fullResourceWidth) / 2;
   individualResourceWidth = (numValidMetrics == 0) ? 0 : fullResourceWidth / numValidMetrics;

   RethinkBarHeights();
}

void BarChart::RethinkBarHeights() {
   // set the height of each bar to the fraction of window height
   // that equals the fraction of the bar's current value to its
   // metric max y value.

   // move from left to right on the screen, in sorted order
   // (but store changes in original order)
   for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
      const int actualResource = indirectResources[resourcelcv];
      if (!validResources[actualResource]) continue;

      for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
         const int actualMetric = metriclcv;
         if (!validMetrics[actualMetric]) continue;

         register double theHeight = barValues[actualMetric][actualResource] /
	                             metricCurrMaxVals[actualMetric];
         theHeight *= this->height; // scale by window height (excluding border pixels)
         barHeights[actualMetric][actualResource] = (int)theHeight;
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
 
   if (drawMode == NoFlicker)
      ResetPrevBarHeights();

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
   indirectResources.reallocate(numResources);

   for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
      char buffer[20];
      sprintf(buffer, "%d", resourcelcv);

      char *string = Tcl_GetVar2(MainInterp, "indirectResources", buffer, TCL_GLOBAL_ONLY);
      if (string == NULL)
         panic("BarChart::rethinkIndirectResources -- could not read indirectResources() from tcl");

      const int indirectNum = atoi(string);
      if (indirectNum<0 || indirectNum>=numResources)
         panic("BarChart::rethinkIndirectResources -- bad indirect value");

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

   isMapped = Tk_IsMapped(theWindow);
   if (!isMapped)
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
   const int top = height + borderPix;
   
   for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
      const int actualResource = indirectResources[resourcelcv];
      assert(actualResource>=0);
      assert(actualResource<numResources);
      if (!validResources[actualResource]) continue;

      int left = resourceBoundary + resourceBorderWidth;
      
      for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
         const int actualMetric = metriclcv; // allow for sorting metrics in the future
         assert(actualMetric>=0);
         assert(actualMetric<numMetrics);
         if (!validMetrics[actualMetric]) continue;

         XSetForeground(display, myGC, metricColors[actualMetric]->pixel);
   
         const int thisBarHeight = barHeights[actualMetric][actualResource];
         XFillRectangle(display,
			doubleBufferPixmap,
			myGC,
		        left, // left
			top - thisBarHeight, // top-y
                        individualResourceWidth, // width
			thisBarHeight // height
			);

         left += individualResourceWidth;
      }

      resourceBoundary += totalResourceWidth;
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

   switch (theBarChart->drawMode) {
      case DoubleBuffer:
         theBarChart->lowestLevelDrawBarsDoubleBuffer();
         break;
//      case NoFlicker:
//         theBarChart->lowestLevelDrawBarsNoFlicker();
//         break;
//      case Flicker:
//         theBarChart->lowestLevelDrawBarsFlicker();
//         break;
      default: assert(false);
   }

   currentlyInstalledLowLevelDrawBars = false; // now, new requests will be handled
      // It seems wrong to delay this line until the bottom of the
      // routine; it could cause new data to be ignored

   // cout << ']'; cout.flush();
}

/* ******************************************************************************** */
/* ****************************** Ancient History ********************************* */
/* ******************************************************************************** */

//void BarChart::lowestLevelDrawBarsFlicker() {
//   ClearScreen(); // clear window, leaving border pixels alone
//   if (flushFlag)
//      XSync(display, False);
//
//   unsigned long bgPixel = greyColor->pixel;
//
//   // do the drawing onto offscreen pixmap
//   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
//      if (!validMetrics[metriclcv]) continue;
//
//      XSetForeground(display, myGC, metricColors[metriclcv]->pixel);
//      XSetBackground(display, myGC, bgPixel);
//   
//      for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
//         if (!validResources[resourcelcv]) continue;
//
//         XFillRectangle(display, wid,
//			myGC,
//			currScrollOffset + barXoffsets[metriclcv][resourcelcv], // left-x
//			height - barHeights[metriclcv][resourcelcv] - borderPix, // top-y
//			barWidths[metriclcv][resourcelcv], // width
//			barHeights[metriclcv][resourcelcv] // height
//		       );
//   
//      }
//   }
//
//   if (flushFlag)
//      XSync(display, False);
//}
//
//void BarChart::lowestLevelDrawBarsNoFlicker() {
//  // be smart about what gets drawn verses what gets erased; if a bar is lower than
//  // last time, erase the appropriate chunk on top; if a bar is higher than
//  // last time, draw the approrpriate chunk on top.
//  
//  unsigned long bgPixel = greyColor->pixel;
//
//  for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
//     if (!validMetrics[metriclcv]) continue;
//
//     unsigned long fgPixel = metricColors[metriclcv]->pixel;
//
//     for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
//        if (!validResources[resourcelcv]) continue;
//
//        int newHeight = barHeights    [metriclcv][resourcelcv];
//	int oldHeight = prevBarHeights[metriclcv][resourcelcv];
// 
//	if (newHeight > oldHeight) {
//	   // the bar is higher than last time; draw the appropriate chunk on top
//	   XSetForeground(display, myGC, fgPixel);
//	   XSetBackground(display, myGC, bgPixel);
//
//	   XFillRectangle(display, wid, myGC,
//			  currScrollOffset + barXoffsets[metriclcv][resourcelcv], // left-x
//			  height - newHeight - borderPix, // top-y
//			  barWidths[metriclcv][resourcelcv], // width
//			  newHeight - oldHeight // height
//			 );
//        }
//	else if (newHeight < oldHeight) {
//	   // the bar is lower than last time; erase appropriate chunk on top
//	   XSetForeground(display, myGC, bgPixel);
//	   XSetBackground(display, myGC, fgPixel);
//
//	   XFillRectangle(display, wid, myGC,
//			  currScrollOffset + barXoffsets[metriclcv][resourcelcv], // left-x
//			  height - oldHeight - borderPix, // top-y
//			  barWidths[metriclcv][resourcelcv], // width
//			  oldHeight - newHeight // height
//			  );
//	}
//        else
//	    ; // no need to redraw bar at the same height as before
//
//        // and finally...
//        prevBarHeights[metriclcv][resourcelcv] = newHeight;
//     }
//  }
//
//  if (flushFlag)
//     XSync(display, False);
//}
