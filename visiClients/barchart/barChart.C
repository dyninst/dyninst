// This is the C++ portion of barchart version 2.
// Much of the user interface mundane stuff (menus, etc.) is still
// programmed in tk/tcl in barChart.tcl.

// $Log: barChart.C,v $
// Revision 1.1  1994/09/29 19:48:27  tamches
// initial implementation.  A to-do list is kept in barChart.tcl
//

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

#pragma implementation "array2d.h"

#include "barChart.h"

BarChart *theBarChart; // main data structure; holds bar information.  Does not
                       // hold x axis or y axis information (or contents), because
                       // they are managed just fine from tcl (barChart.tcl) at present.
                       // Created dynamically in DrawBarsInstallCommand.  Cannot be
                       // created before then since necessary constructor arguments
                       // such as tk window name are not yet known.

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
            barXoffsets(initNumMetrics, initNumResources),
            barWidths  (initNumMetrics, initNumResources),
	    barHeights (initNumMetrics, initNumResources),
	    barValues  (initNumMetrics, initNumResources),
            metricCurrMaxYs(initNumMetrics),
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
      // barHeights, prevBarHeights, barValues, metricCurrMaxYs
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
      processResizeWindow();
   }

   return true;   
}

void BarChart::processResizeWindow() {
   if (!TryFirstGoodWid())
      return; // our window is still invalid

   width  = Tk_Width(theWindow) - 2*borderPix; // subtract left+right border
   height = Tk_Height(theWindow) - 2*borderPix; // subract top+bottom border

   // update off-screen pixmap's dimensions to accomodate the resize
   changeDoubleBuffering(drawMode==DoubleBuffer, drawMode==NoFlicker);

   RethinkBarLayouts();
}

void BarChart::processExposeWindow() {
   if (!TryFirstGoodWid())
      return; // our window is still invalid

   cout << "Welcome to BarChart::processExposeWindow" << endl;
   if (drawMode == NoFlicker)
      ResetPrevBarHeights();

   lowLevelDrawBars();
}

char *gimmeColorName(const int metriclcv) {
   static char *theNames[] = {
      "blue",
      "red",
      "green",
      "orange"
   };

   return theNames[metriclcv % 4];
}

void BarChart::RethinkMetricColors() {
   metricColors.reallocate(numMetrics);

   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      XColor *theColor = Tk_GetColor(MainInterp, theWindow, None, Tk_GetUid(gimmeColorName(metriclcv)));

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
   // clear window, erase and reallocate barXoffsets, barWidths, prevBarHeights,
   // barHeights, metricCurrMaxYs; set numMetrics, numResources -- all based on a
   // complete re-reading from dataGrid[][].  (If visi had provided more fine-grained
   // callbacks than ADDMETRICSRESOURCES, such a crude routine would not be necessary.)

   // When done, redraw.

   // #include "visualization.h" from visi-lib to get global
   // variable "dataGrid"

   numMetrics   = dataGrid.NumMetrics();
   numResources = dataGrid.NumResources();

   cout << "Welcome to BarChart::RethinkMetricsAndResources; m=" << numMetrics << "; r=" << numResources << endl;

   // the following is very unfortunate for existing metric/resource pairs;
   // their values do not change and so should not be reset.  This is
   // particularly troublesome for metricCurrMaxYs[], where resetting their
   // values (as we do here) can be considered a bug.
   barXoffsets.reallocate(numMetrics, numResources);
   barWidths  .reallocate(numMetrics, numResources);
   barHeights .reallocate(numMetrics, numResources);
   prevBarHeights.reallocate(numMetrics, numResources);
   barValues  .reallocate(numMetrics, numResources);
   metricCurrMaxYs.reallocate(numMetrics);

   // reset bar values (very unfortunate for existing pairs)
   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++)
      for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++)
         barValues[metriclcv][resourcelcv]=0;

   // reset max y values (unfortunate for existing pairs)
   for (metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      char buffer[64];
      sprintf(buffer, "%d", metriclcv);
      char *str = Tcl_GetVar2(MainInterp, "metricMaxValues", buffer, TCL_GLOBAL_ONLY);
      if (str == NULL)
         panic("BarChart::RethinkMetricsAndResources() -- could not read 'metricMaxValues'");

      metricCurrMaxYs[metriclcv] = atof(str);
   }

   RethinkMetricColors(); // reallocates and rethinks metricColors[]

   RethinkBarLayouts(); // as if there were a resize (no reallocations, but
                        // resets/rethinks barXoffsets[], barWidths[], barHeights[],
                        // prevBarHeights[])

   if (HaveSeenFirstGoodWid)
      lowLevelDrawBars();
}

void BarChart::processNewData(int newBucketIndex) {
   // assuming new data has arrived at the given bucket index for all
   // metric/rsrc pairs, read the new information from dataGrid[][],
   // update barHeights[][], barValues[][], and metricCurrMaxYs[] (if needed),
   // and call lowLevelDrawBars()
   
   // PASS 1: Update barValues[][] and check for y-axis overflow, calling
   //         setMetricNewMaxY() if overflow is detected.

   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      visi_GridHistoArray &metricValues = dataGrid[metriclcv];

      for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
         visi_GridCellHisto &theCell = metricValues[resourcelcv];

         double newVal;
         switch (DataFormat) {
 	    case Current:
               newVal = theCell.Value(newBucketIndex);
               break;
            case Average:
               // newVal = theCell.AggregateValue(0); // ???
               newVal = dataGrid.AggregateValue(metriclcv, resourcelcv);
               break;
            case Total:
               // newVal = theCell.SumValue(1); // ???
               newVal = dataGrid.SumValue(metriclcv, resourcelcv);
               break;
            default:
               assert(false);
	 }

         barValues[metriclcv][resourcelcv] = newVal;

         // the dreaded check for y-axis overflow (slows things down greatly if there
         // is indeed overflow)
         double currMaxY = metricCurrMaxYs[metriclcv];
         if (newVal > currMaxY)
            setMetricNewMaxY(metriclcv, newVal);
      }
   }

   // PASS 2: Now that the y-axis overflows, if any, have been processed, we can
   //         calculate barHeights with confidence that the scale isn't going to
   //         change.

   RethinkBarHeights();
}

double BarChart::nicelyRoundedMetricNewYMaxValue(int metricindex, double newmaxval) {
   assert(0<=metricindex && metricindex<numMetrics);

   char buffer[256];
   sprintf(buffer, "lindex [getMetricHints %s] 3", dataGrid.MetricName(metricindex));

   if (TCL_OK != Tcl_Eval(MainInterp, buffer))
      panic("BarChart::nicelyRoundedMetricNewYMaxValue() -- could not eval");

   double hintedStep = atof(MainInterp->result);

   double result = newmaxval + (hintedStep - fmod(newmaxval, hintedStep));
      // if fmod() doesn't return a number greater than 0, this won't work quite right

   return result;
}

void BarChart::setMetricNewMaxY(int metricindex, double newmaxval) {
   // given an actual bar value (newmaxval) that overflows the current
   // y-axis maximum value for the given metric, rethink what the
   // y-axis maximum value for the given metric should be.
   // Note that it may not be exactly "newmaxval"; we usually want
   // to round to a relatively nice number.

   assert(0<=metricindex && metricindex<numMetrics);

   newmaxval = nicelyRoundedMetricNewYMaxValue(metricindex, newmaxval);
   metricCurrMaxYs[metricindex] = newmaxval;

   char buffer[256];
   sprintf(buffer, "processNewMetricMax %d %g", metricindex, newmaxval);
   if (TCL_OK != Tcl_Eval(MainInterp, buffer))
      cerr << "warning -- BarChart::setMetricNewMaxY() could not inform barChart.tcl of new-max-y-value (no script processNewMetricMax?)" << endl;
}

void BarChart::RethinkBarLayouts() {
   // assuming a complete resize (but not an added or deleted metric
   // or resource or even new data values), fill in barXoffsets, barWidths, etc.

   // does not touch metricCurrMaxYs or barValues

   // note: the following loop starts with resources, then does metrics.  should not
   // cause any big problems, and more intuitive in this case...
   char *fullResourceWidthStr = Tcl_GetVar(MainInterp, "resourceWidth", TCL_GLOBAL_ONLY);
   if (NULL == fullResourceWidthStr)
      panic("BarChart::RethinkBarLayouts() -- could not read 'resourceWidth' from tcl");

   int totalResourceWidth = atoi(fullResourceWidthStr);
   int fullResourceWidth = (totalResourceWidth * 90) / 100;

   int resourceBorderWidth = (totalResourceWidth - fullResourceWidth) / 2;
   int individualResourceWidth = (numMetrics == 0) ? 0 : fullResourceWidth / numMetrics;

   for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
      int left = borderPix + (resourcelcv * totalResourceWidth) + resourceBorderWidth;

      for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
         barXoffsets[metriclcv][resourcelcv] = left + metriclcv * individualResourceWidth;
         barWidths  [metriclcv][resourcelcv] = individualResourceWidth;
         barHeights [metriclcv][resourcelcv] = 0; // all bars start off flat
      }
   }

   if (drawMode == NoFlicker)
      ResetPrevBarHeights();
}

void BarChart::RethinkBarHeights() {
   // set the height of each bar to the fraction of window height
   // that equals the fraction of the bar's current value to its
   // metric max y value.

   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
         double theHeight = barValues[metriclcv][resourcelcv] /
                            metricCurrMaxYs[metriclcv];
         theHeight *= this->height; // scale by window height (excluding border pixels)
         barHeights[metriclcv][resourcelcv] = (int)theHeight;
      }
   }

   if (HaveSeenFirstGoodWid)
      lowLevelDrawBars();
}

bool currentlyInstalledLowLevelDrawBars=false;

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

   if (flushFlag)
      XSync(display, False);

   // do the drawing onto offscreen pixmap
   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      XSetForeground(display, myGC, metricColors[metriclcv]->pixel);
   
      for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
         XFillRectangle(display,
			doubleBufferPixmap,
			myGC,
			currScrollOffset + barXoffsets[metriclcv][resourcelcv], // left-x
			height - barHeights[metriclcv][resourcelcv] - borderPix, // top-y
			barWidths[metriclcv][resourcelcv], // width
			barHeights[metriclcv][resourcelcv] // height
		       );
      }
   }

   if (flushFlag)
      XSync(display, False);

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
   // cout << '}'; cout.flush();

   if (flushFlag)
      XSync(display, False);
}

void BarChart::lowestLevelDrawBarsNoFlicker() {
  // be smart about what gets drawn verses what gets erased; if a bar is lower than
  // last time, erase the appropriate chunk on top; if a bar is higher than
  // last time, draw the approrpriate chunk on top.
  
  unsigned long bgPixel = greyColor->pixel;

  for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
     unsigned long fgPixel = metricColors[metriclcv]->pixel;

     for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
        int newHeight = barHeights    [metriclcv][resourcelcv];
	int oldHeight = prevBarHeights[metriclcv][resourcelcv];
 
	if (newHeight > oldHeight) {
	   // the bar is higher than last time; draw the appropriate chunk on top
	   XSetForeground(display, myGC, fgPixel);
	   XSetBackground(display, myGC, bgPixel);

	   XFillRectangle(display, wid, myGC,
			  currScrollOffset + barXoffsets[metriclcv][resourcelcv], // left-x
			  height - newHeight - borderPix, // top-y
			  barWidths[metriclcv][resourcelcv], // width
			  newHeight - oldHeight // height
			 );
        }
	else if (newHeight < oldHeight) {
	   // the bar is lower than last time; erase appropriate chunk on top
	   XSetForeground(display, myGC, bgPixel);
	   XSetBackground(display, myGC, fgPixel);

	   XFillRectangle(display, wid, myGC,
			  currScrollOffset + barXoffsets[metriclcv][resourcelcv], // left-x
			  height - oldHeight - borderPix, // top-y
			  barWidths[metriclcv][resourcelcv], // width
			  oldHeight - newHeight // height
			  );
	}
        else
	    ; // no need to redraw bar at the same height as before

        // and finally...
        prevBarHeights[metriclcv][resourcelcv] = newHeight;
     }
  }

  if (flushFlag)
     XSync(display, False);
}

void BarChart::lowestLevelDrawBarsFlicker() {
   ClearScreen(); // clear window, leaving border pixels alone
   if (flushFlag)
      XSync(display, False);

   unsigned long bgPixel = greyColor->pixel;

   // do the drawing onto offscreen pixmap
   for (int metriclcv=0; metriclcv<numMetrics; metriclcv++) {
      XSetForeground(display, myGC, metricColors[metriclcv]->pixel);
      XSetBackground(display, myGC, bgPixel);
   
      for (int resourcelcv=0; resourcelcv<numResources; resourcelcv++) {
         XFillRectangle(display, wid,
			myGC,
			currScrollOffset + barXoffsets[metriclcv][resourcelcv], // left-x
			height - barHeights[metriclcv][resourcelcv] - borderPix, // top-y
			barWidths[metriclcv][resourcelcv], // width
			barHeights[metriclcv][resourcelcv] // height
		       );
   
      }
   }

   if (flushFlag)
      XSync(display, False);
}

void BarChart::lowestLevelDrawBars(ClientData ignore) {
   // cout << '['; cout.flush();

   // NOTE: a --static-- member function --> no "this" exists!!

   if (!theBarChart->HaveSeenFirstGoodWid) {
      cout << "BarChart::lowestLevelDrawBars -- haven't yet mapped? (ignoring)" << endl;
      return;
   }

   currentlyInstalledLowLevelDrawBars = false; // now, new requests will be handled
      // It seems wrong to delay this line until the bottom of the
      // routine; it could cause new data to be ignored

   switch (theBarChart->drawMode) {
      case DoubleBuffer:
         theBarChart->lowestLevelDrawBarsDoubleBuffer();
         break;
      case NoFlicker:
         theBarChart->lowestLevelDrawBarsNoFlicker();
         break;
      case Flicker:
         theBarChart->lowestLevelDrawBarsFlicker();
         break;
      default: assert(0);
   }

   // cout << ']'; cout.flush();
}

void BarChart::processNewScrollPosition(int newPos) {
   if (!HaveSeenFirstGoodWid)
      return;

   // cout << "BarChart::processNewScrollPosition -- new pos=" << newPos << endl;

   // adjust the current x-pixel scroll offset value and
   // simulate an expose event
   currScrollOffset = -newPos;
 
   // tk is clever with its canvas widgets to never start the leftmost
   // drawing portion at less than 0; we must be just as clever to keep
   // everything positioned correctly.
//   if (currScrollOffset < 0 && scrollbarisfullwidth)
//      currScrollOffset=0;

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
