/*
 * Copyright (c) 1996-1999 Barton P. Miller
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

// tableVisi.C
// Ariel Tamches

/*
 * $Log: tableVisi.C,v $
 * Revision 1.17  2002/12/20 07:50:09  jaw
 * This commit fully changes the class name of "vector" to "pdvector".
 *
 * A nice upshot is the removal of a bunch of code previously under the flag
 * USE_STL_VECTOR, which is no longer necessary in many cases where a
 * functional difference between common/h/Vector.h and stl::vector was
 * causing a crash.
 *
 * Generally speaking, Dyninst and Paradyn now use pdvector exclusively.
 * This commit DOES NOT cover the USE_STL_VECTOR flag, which will now
 * substitute stl::vector for BPatch_Vector only.  This is currently, to
 * the best of my knowledge, only used by DPCL.  This will be updated and
 * tested in a future commit.
 *
 * The purpose of this, again, is to create a further semantic difference
 * between two functionally different classes (which both have the same
 * [nearly] interface).
 *
 * Revision 1.16  2002/05/13 19:54:07  mjbrim
 * update string class to eliminate implicit number conversions
 * and replace all use of string_of with c_str  - - - - - - - - - - - - - -
 * change implicit number conversions to explicit conversions,
 * change all use of string_of to c_str
 *
 * Revision 1.15  2001/11/07 05:03:27  darnold
 * Bug fix: gcvt() => sprintf() so that values in table are of fixed
 *          length regardless of "zeroes after the decimal point"
 *
 * Revision 1.14  2000/07/28 17:23:01  pcroth
 * Updated #includes to reflect util library split
 *
 * Revision 1.13  2000/03/21 23:58:39  pcroth
 * Added guard to protect against division by zero when resizing scroll bars.
 *
 * Revision 1.12  1999/11/09 15:55:11  pcroth
 * Updated uses of XCreateGC and XFreeGC to Tk_GetGC and Tk_FreeGC.
 *
 * Revision 1.11  1999/07/13 17:16:11  pcroth
 * Fixed ordering problem of destroying GUI and destructing static variable
 * pdLogo::all_logos.  On NT, the static variable is destroyed before the
 * GUI, but a callback for the GUI ends up referencing the variable causing
 * an access violation error.
 *
 * Revision 1.10  1999/03/13 15:24:04  pcroth
 * Added support for building under Windows NT
 *
 * Revision 1.9  1996/08/16 21:36:56  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.8  1996/04/30 20:17:37  tamches
 * double2string optimized for 0
 * gcvt called instead of sprintf on the off-chance that
 * it's a bit faster.
 *
 * Revision 1.7  1995/12/22 22:43:10  tamches
 * selection
 * deletion
 * sort foci by value
 *
 * Revision 1.6  1995/12/19 00:46:19  tamches
 * calls to tvMetric constructor use new args
 * changeNumSigFigs now recognizes possibility of column pix width change
 *
 * Revision 1.5  1995/12/03 21:09:19  newhall
 * changed units labeling to match type of data being displayed
 *
 * Revision 1.4  1995/11/20  20:20:20  tamches
 * horizontal & vertical grid lines no longer expand past the
 * last cells.
 *
 */

#include <iostream.h>

#include "common/h/headers.h"
#include "minmax.h"
#include "tkTools.h"
#include "tableVisi.h"

/* ************************************************************* */

extern "C" {int isnan(double);}

void tableVisi::updateConversionString() {
   sprintf(conversionString, "%%.%df", numSigFigs);
}

void tableVisi::double2string(char *buffer, double val) const {
   if (isnan(val)) {
      buffer[0] = '\0';
      return;
   }
   
   // Optimize for a common case:
   if (val == 0) {
      buffer[0] = '0';
      buffer[1] = '\0';
      return;
   }

   sprintf(buffer, conversionString, val);
//   gcvt(val, numSigFigs, buffer);

//   cout << "from " << buffer << " to " << flush;

   // add commas
   if (strlen(buffer)==0)
      return;

   const char *decPtr = strchr(buffer, '.');

   if (decPtr && (decPtr - buffer <= 3))
      return; // no commas will be added since there aren't at least 4 integer digits

   if (decPtr == NULL) {
      // try for exponential notation
      decPtr = strchr(buffer, 'e');

      if (decPtr==NULL)
         decPtr = &buffer[strlen(buffer)]; // the '\0'
   }

   // invariant: decPtr now points to the character AFTER the
   // last integer digit (i.e., the decimal point), or the '\0'
   // if none exists.

   // Now let's walk backwards, looking for places to insert commas
   char integerPartBuffer[200]; // will include commas
   char *integerPart = integerPartBuffer;
   const char *walkPtr = decPtr-1;
   unsigned copyCount=0;
   while (walkPtr >= buffer) {
      if (copyCount > 0 && copyCount % 3 == 0)
         *integerPart++ = ',';

      copyCount++;
      *integerPart++ = *walkPtr--;
   }

   char decimalPart[200];
      // will incl dec point, if applicable.  May be
      // the empty-string.
   strcpy(decimalPart, decPtr);

   // note: integerPartBuffer is backwards...we must reverse it
   char *bufferPtr = buffer;
   while (--integerPart >= integerPartBuffer)
      *bufferPtr++ = *integerPart;
   strcpy(bufferPtr, decimalPart);

//   cout << buffer << endl;
}

/* ************************************************************* */

Tk_Font tableVisi::myTkGetFont(Tcl_Interp* interp, const string &fontName) const {
   Tk_Font result = Tk_GetFont(interp,
					theTkWindow,
					fontName.c_str());
   if (result == NULL) {
      cerr << "could not find font " << fontName << endl;
      exit(5);
   }

   return result;
}

XColor *tableVisi::myTkGetColor(Tcl_Interp *interp, const string &colorName) const {
   XColor *result = Tk_GetColor(interp, theTkWindow, Tk_GetUid(colorName.c_str()));
   if (result == NULL)
      tclpanic(interp, "table visi: could not allocate color");

   return result;
}

/* ************************************************************* */

tableVisi::tableVisi(Tcl_Interp *interp,
		     Tk_Window iTkWindow,
		     const string &metricFontName,
		     const string &metricUnitsFontName,
		     const string &focusFontName,
		     const string &cellFontName,
		     const string &iLineColorName,
		     const string &iMetricColorName,
		     const string &iMetricUnitsColorName,
		     const string &iFocusColorName,
		     const string &cellColorName,
		     const string &backgroundColorName,
		     const string &highlightedBackgroundColorName,
		     unsigned iSigFigs
		     ) {
   // metrics[], foci[], indirectMetrics[], indirectFoci[], and cells[][]
   // are all initialized to zero-sized arrays.

   theTkWindow = iTkWindow;
   theDisplay = Tk_Display(theTkWindow);

   offscreenPixmap = (Pixmap)NULL;
      // sorry, can't create a pixmap until the window becomes mapped.

   backgroundColor = myTkGetColor(interp, backgroundColorName);
   highlightedBackgroundColor = myTkGetColor(interp, highlightedBackgroundColorName);

   offset_x = offset_y = 0;
   all_cells_width = all_cells_height = 0;

   // access fonts and font metrics
   metricNameFont = myTkGetFont(interp, metricFontName);
   Tk_GetFontMetrics( metricNameFont, &metricNameFontMetrics );

   metricUnitsFont = myTkGetFont(interp, metricUnitsFontName);
   Tk_GetFontMetrics( metricUnitsFont, &metricUnitsFontMetrics );

   focusNameFont = myTkGetFont(interp, focusFontName);
   Tk_GetFontMetrics( focusNameFont, &focusNameFontMetrics );

   cellFont = myTkGetFont(interp, cellFontName);
   Tk_GetFontMetrics( cellFont, &cellFontMetrics );

   focusLongNameMode = true;
   numSigFigs = iSigFigs;
   updateConversionString();

   theSelection = none;
   // we leave selectedRow, selectedCol undefined

   maxFocusNamePixWidth = 0;

   // The GC's, like offscreenPixmap, can't be created until the
   // window becomes mapped.
   lineColor = myTkGetColor(interp, iLineColorName);
   lineColorGC = NULL;

   metricNameColor = myTkGetColor(interp, iMetricColorName);
   metricNameGC = NULL;

   metricUnitsColor = myTkGetColor(interp, iMetricUnitsColorName);
   metricUnitsGC = NULL;
  
   focusNameColor = myTkGetColor(interp, iFocusColorName);
   focusNameGC = NULL;

   cellColor = myTkGetColor(interp, cellColorName);
   cellGC = NULL;
}


tableVisi::~tableVisi() {
   // arrays metrics[], foci[], indirectMetrics[], indirectFoci[], cells[][] will
   // delete themselves.

   if (!offscreenPixmap)
      // the offscreen pixmap was never allocated(!)...so, we never
      // got around to mapping the window!
      return;

   Tk_FreeGC(theDisplay, cellGC);
   Tk_FreeGC(theDisplay, focusNameGC);
   Tk_FreeGC(theDisplay, metricUnitsGC);
   Tk_FreeGC(theDisplay, metricNameGC);
   Tk_FreeGC(theDisplay, lineColorGC);
   Tk_FreeGC(theDisplay, backgroundGC);
   Tk_FreeGC(theDisplay, highlightedBackgroundGC);

   Tk_FreePixmap(theDisplay, offscreenPixmap);
}

bool tableVisi::tryFirst() {
   if (offscreenPixmap) {
      // the offscreen pixmap has been allocated, so the window
      // has presumably been mapped already
      assert(Tk_WindowId(theTkWindow) != 0);
      return true;
   }

   // the offscreen pixmap hasn't been allocated, so it's now time
   // to check to see if it should be.
   if (Tk_WindowId(theTkWindow) == 0)
      return false; // nuts; not ready yet

   // Ready to allocate graphical structures now!
   offscreenPixmap = Tk_GetPixmap(Tk_Display(theTkWindow),
				   Tk_WindowId(theTkWindow),
				   1, 1, // dummy width, height
				   Tk_Depth(theTkWindow));
   XGCValues values;
   values.foreground = backgroundColor->pixel;
   backgroundGC = Tk_GetGC(theTkWindow,
			    GCForeground, &values);

   values.foreground = highlightedBackgroundColor->pixel;
   highlightedBackgroundGC = Tk_GetGC(theTkWindow,
				       GCForeground, &values);

   values.foreground = lineColor->pixel;
   lineColorGC = Tk_GetGC(theTkWindow,
			   GCForeground, &values);

   values.foreground = metricNameColor->pixel;
   values.font = Tk_FontId( metricNameFont );
   metricNameGC = Tk_GetGC(theTkWindow,
			    GCForeground | GCFont, &values);

   values.foreground = metricUnitsColor->pixel;
   values.font = Tk_FontId( metricUnitsFont );
   metricUnitsGC = Tk_GetGC(theTkWindow,
			     GCForeground | GCFont, &values);

   values.foreground = focusNameColor->pixel;
   values.font = Tk_FontId( focusNameFont );
   focusNameGC = Tk_GetGC(theTkWindow,
			   GCForeground | GCFont, &values);

   values.foreground = cellColor->pixel;
   values.font = Tk_FontId( cellFont );
   cellGC = Tk_GetGC(theTkWindow,
		      GCForeground | GCFont, &values);

   return true;
}

void tableVisi::resizeScrollbars(Tcl_Interp *interp) {
   // used to be a tcl routine (resize1Scrollbar):

   int visible_cell_width = Tk_Width(theTkWindow) -
                                  getFocusAreaPixWidth();
	if( visible_cell_width <= 0 )
	{
		visible_cell_width = 1;
	}
   resizeScrollbar(interp, ".horizScrollbar",
		   all_cells_width, // total
		   visible_cell_width);

   int visible_cell_height = Tk_Height(theTkWindow) -
                                   getMetricAreaPixHeight();
	if( visible_cell_height <= 0 )
	{
		visible_cell_height = 1;
	}
   resizeScrollbar(interp, ".vertScrollbar",
		   all_cells_height, // total
		   visible_cell_height);
}

bool tableVisi::adjustHorizSBOffset(Tcl_Interp *interp) {
   float first, last;
   getScrollBarValues(interp, ".horizScrollbar", first, last);
   return adjustHorizSBOffset(interp, first);
}

bool tableVisi::adjustVertSBOffset(Tcl_Interp *interp) {
   float first, last;
   getScrollBarValues(interp, ".vertScrollbar", first, last);
   return adjustVertSBOffset(interp, first);
}

void tableVisi::resize(Tcl_Interp *interp) {
   // does not redraw.  Does things like resize the offscreen pixmap
   if (tryFirst()) {
      if (offscreenPixmap) {
         Tk_FreePixmap(Tk_Display(theTkWindow), offscreenPixmap);

         offscreenPixmap = Tk_GetPixmap(Tk_Display(theTkWindow),
					 Tk_WindowId(theTkWindow),
					 Tk_Width(theTkWindow), Tk_Height(theTkWindow),
					 Tk_Depth(theTkWindow));
      }
   }

   resizeScrollbars(interp);
   adjustHorizSBOffset(interp);
   adjustVertSBOffset(interp);
}

void tableVisi::drawHighlightBackground(Drawable theDrawable) const {
   if (theSelection == none)
      return;

   int left, right;
   if (theSelection == rowOnly) {
      left = 0;
      right = min(Tk_Width(theTkWindow)-1,
		  (int)getFocusAreaPixWidth() + all_cells_width-1);
   }
   else {
      // theSelection == colOnly || theSelection == cell
      left = metric2xpix(selectedCol);
      right = left + metrics[indirectMetrics[selectedCol]].getColPixWidth() - 1;

      left = max(left, (int)getFocusAreaPixWidth()+1);
      right = max(right, (int)getFocusAreaPixWidth()+1);
   }

   int top, bottom;
   if (theSelection == rowOnly || theSelection == cell) {
      top = focus2ypix(selectedRow);
      bottom = top + getFocusLinePixHeight() - 1;

      top = max(top, (int)getMetricAreaPixHeight()+1);
      bottom = max(bottom, (int)getMetricAreaPixHeight()+1);
   }
   else {
      // theSelection == colOnly
      top = 0;
      bottom = min((int)getMetricAreaPixHeight() + all_cells_height - 1,
		   Tk_Height(theTkWindow)-1);
   }

   int width = right - left + 1;
   int height = bottom - top + 1;

   XFillRectangle(Tk_Display(theTkWindow), theDrawable,
		  highlightedBackgroundGC,
		  left, top, width, height);
}

void tableVisi::draw(bool xsynch) const {
   if (!offscreenPixmap)
      return; // we haven't done a tryFirst() yet

   bool doubleBuffer = !xsynch;

   Drawable theDrawable = doubleBuffer ? offscreenPixmap : Tk_WindowId(theTkWindow);

   // XClearArea() works only on windows; the following works on pixmaps, too:
   XFillRectangle(Tk_Display(theTkWindow), theDrawable,
		  backgroundGC,
		  0, 0, // x, y offset
		  Tk_Width(theTkWindow), Tk_Height(theTkWindow));

   // Is anything (a cell, an entire row, an entire column) highlighted?
   // If so, let's do that now
   drawHighlightBackground(theDrawable);

   drawFocusNames(theDrawable);
      // leftmost part of screen; unaffected by offset_x
   drawMetricNames(theDrawable);
      // topmost part of screen; unaffected by offset_y
   drawCells(theDrawable);

   if (doubleBuffer)
      XCopyArea(Tk_Display(theTkWindow),
		offscreenPixmap, // src drawable
		Tk_WindowId(theTkWindow), // dest drawable
		backgroundGC, // only a dummy GC is needed here (well, sort of)
		0, 0, // src x, y offsets
		Tk_Width(theTkWindow), Tk_Height(theTkWindow),
		0, 0 // dest x, y offsets
		);
}

/*
 * private metric helper functions
 *
 */
void tableVisi::drawMetricNames(Drawable theDrawable) const {
   int curr_x = offset_x + getFocusAreaPixWidth();

   const int minVisibleX = getFocusAreaPixWidth();
   const int maxVisibleX = Tk_Width(theTkWindow) - 1;

   const int metric_name_baseline = getMetricNameBaseline();
   const int metric_units_baseline = getMetricUnitsBaseline();

   // we need to clip 2 GC's: metricNameGC and metricUnitsGC.
   // we don't need to clip lineGC, since our manual clipping is effective
   XRectangle clipRect;
   clipRect.x = getFocusAreaPixWidth();
   clipRect.y = 0;
   clipRect.width = Tk_Width(theTkWindow) - clipRect.x + 1;
   clipRect.height = Tk_Height(theTkWindow);

#if !defined(i386_unknown_nt4_0)
   XSetClipRectangles(Tk_Display(theTkWindow), metricNameGC,
		      0, 0, &clipRect, 1, YXBanded);
   XSetClipRectangles(Tk_Display(theTkWindow), metricUnitsGC,
		      0, 0, &clipRect, 1, YXBanded);
#endif // !defined(i386_unknown_nt4_0)

   for (unsigned metriclcv=0; metriclcv < indirectMetrics.size(); metriclcv++) {
      if (curr_x > maxVisibleX)
         break; // everthing else will be too far right

      const tvMetric &theMetric = metrics[indirectMetrics[metriclcv]];
      const int next_x = curr_x + theMetric.getColPixWidth();

      if (next_x - 1 < minVisibleX) {
         curr_x = next_x;
         continue;
      }

      if (curr_x >= minVisibleX) // clipping
         drawMetricVertLine(theDrawable, curr_x);

      int curr_middle_x = (curr_x + next_x - 1) / 2;

      // draw the metric name:
      int metric_name_left = curr_middle_x - theMetric.getNamePixWidth() / 2;
      const string &metricNameStr = theMetric.getName();
      Tk_DrawChars(Tk_Display(theTkWindow), theDrawable,
		  metricNameGC,
		  metricNameFont,
		  metricNameStr.c_str(), metricNameStr.length(),
		  metric_name_left, metric_name_baseline);

      // draw the metric units:
      int metric_units_left = curr_middle_x - theMetric.getUnitsPixWidth() / 2;
      const string &metricUnitsNameStr = theMetric.getUnitsName();
      Tk_DrawChars(Tk_Display(theTkWindow), theDrawable,
		  metricUnitsGC,
		  metricUnitsFont,
		  metricUnitsNameStr.c_str(), metricUnitsNameStr.length(),
		  metric_units_left, metric_units_baseline);

      curr_x = next_x;
   }

   if (curr_x >= minVisibleX) // clipping
      drawMetricVertLine(theDrawable, curr_x);

   XSetClipMask(Tk_Display(theTkWindow), metricNameGC, None);
   XSetClipMask(Tk_Display(theTkWindow), metricUnitsGC, None);
}

void tableVisi::drawMetricVertLine(Drawable theDrawable, int x) const {
   int line_height = getMetricAreaPixHeight() + get_total_cell_y_pix();
   ipmin(line_height, Tk_Height(theTkWindow));

   XDrawLine(Tk_Display(theTkWindow), theDrawable,
	     lineColorGC,
	     x, 0, x, 0+line_height-1);
}

unsigned tableVisi::getMetricAreaPixHeight() const {
   return 3 + metricNameFontMetrics.linespace + 3 +
              metricUnitsFontMetrics.linespace + 3;
}

unsigned tableVisi::getMetricNameBaseline() const {
   return 3 + metricNameFontMetrics.ascent - 1;
}

unsigned tableVisi::getMetricUnitsBaseline() const {
   return 3 + metricNameFontMetrics.linespace + 3 +
              metricUnitsFontMetrics.ascent - 1;
}

bool tableVisi::xpix2col(int x, unsigned &theCol) const {
   if (x < (int)getFocusAreaPixWidth())
      return false; // too far left

   x -= getFocusAreaPixWidth();

   // Adjust for horiz scrollbar:
   x -= offset_x;

   if (x >= all_cells_width)
      return false; // too far right

   for (unsigned metriclcv = 0; metriclcv < indirectMetrics.size(); metriclcv++) {
      const tvMetric &theMetric = metrics[indirectMetrics[metriclcv]];
      if (x < (int)theMetric.getColPixWidth()) {
         theCol = metriclcv;
         return true;
      }
      x -= theMetric.getColPixWidth();
   }

   assert(false); // this case should have been caught in the "x >= all_cells_width"
   return false; // placate compiler
}

int tableVisi::metric2xpix(unsigned theColumn) const {
   int result = getFocusAreaPixWidth();

   // Adjust for scrollbar:
   result += offset_x; // subtracts from 'result' since offset_x <= 0

   for (unsigned collcv=0; collcv < theColumn; collcv++)
      result += metrics[indirectMetrics[collcv]].getColPixWidth();

   return result;
}

/*
 * private focus helper functions
 *
 */
void tableVisi::drawFocusNames(Drawable theDrawable) const {
   int curr_y = offset_y + getMetricAreaPixHeight();

   const int minVisibleY = getMetricAreaPixHeight();
   const int maxVisibleY = Tk_Height(theTkWindow) - 1;

   XRectangle clipRect;
   clipRect.x = 0;
   clipRect.y = getMetricAreaPixHeight();
   clipRect.width = Tk_Width(theTkWindow);
   clipRect.height = Tk_Height(theTkWindow) - clipRect.y + 1;

#if !defined(i386_unknown_nt4_0)
   XSetClipRectangles(Tk_Display(theTkWindow), focusNameGC,
		      0, 0, &clipRect, 1, YXBanded);
#endif // !defined(i386_unknown_nt4_0)

   for (unsigned focuslcv = 0; focuslcv < indirectFoci.size(); focuslcv++) {
      if (curr_y > maxVisibleY)
         break;

      const tvFocus &theFocus = foci[indirectFoci[focuslcv]];
      const int next_y = curr_y + getFocusLinePixHeight();
      if (next_y - 1 < minVisibleY) {
         curr_y = next_y;
         continue;
      }

      if (curr_y >= minVisibleY)
         drawFocusHorizLine(theDrawable, curr_y);

      int curr_y_baseline = curr_y + getVertPixFocusTop2Baseline();

      const string &theString = focusLongNameMode ? theFocus.getLongName() :
                                                    theFocus.getShortName();

      Tk_DrawChars(Tk_Display(theTkWindow), theDrawable,
		  focusNameGC,
		  focusNameFont,
		  theString.c_str(), theString.length(),
		  getHorizPixBeforeFocusName(), curr_y_baseline);

      curr_y = next_y;
   }

   XSetClipMask(Tk_Display(theTkWindow), focusNameGC, None);

   if (curr_y >= minVisibleY)
      drawFocusHorizLine(theDrawable, curr_y);
}

void tableVisi::drawFocusHorizLine(Drawable theDrawable, int y) const {
   int line_width = getFocusAreaPixWidth() + get_total_cell_x_pix();
   ipmin(line_width, get_visible_x_pix());
   
   XDrawLine(Tk_Display(theTkWindow), theDrawable,
	     lineColorGC,
	     0, y, 0+line_width-1, y);
}

unsigned tableVisi::getFocusLinePixHeight() const {
   return 2 + focusNameFontMetrics.linespace + 2;
}

unsigned tableVisi::getVertPixFocusTop2Baseline() const {
   return 2 + focusNameFontMetrics.ascent;
}

unsigned tableVisi::getFocusAreaPixWidth() const {
   return getHorizPixBeforeFocusName() + maxFocusNamePixWidth +
          getHorizPixBeforeFocusName();
}

bool tableVisi::ypix2row(int y, unsigned &theRow) const {
   if (y < (int)getMetricAreaPixHeight())
      return false; // too far up

   y -= getMetricAreaPixHeight();

   // Adjust for scrollbar:
   y -= offset_y; // adds since offset_y is <= 0

   unsigned result = y / getFocusLinePixHeight();
   if (result >= indirectFoci.size())
      return false; // too far down

   theRow = result;
   return true;
}

int tableVisi::focus2ypix(unsigned theRow) const {
   int result = getMetricAreaPixHeight();
   
   // Adjust for scrollbar:
   result += offset_y; // subtracts since offset_y is <= 0

   result += theRow * getFocusLinePixHeight();

   return result;
}

/*
 * private cell helper functions
 *
 */
void tableVisi::drawCells(Drawable theDrawable) const {
   int curr_x = offset_x + getFocusAreaPixWidth();

   const int minVisibleX = 0;
   const int maxVisibleX = Tk_Width(theTkWindow)-1;

   // we need to clip the GCs used for drawing cells s.t. neither the
   // metrics nor foci are overwritten.
   XRectangle clipRect;
   clipRect.x = getFocusAreaPixWidth();
   clipRect.y = getMetricAreaPixHeight();
   clipRect.width = Tk_Width(theTkWindow) - clipRect.x + 1;
   clipRect.height = Tk_Height(theTkWindow) - clipRect.y + 1;

#if !defined(i386_unknown_nt4_0)
   XSetClipRectangles(Tk_Display(theTkWindow), cellGC,
		      0, 0, &clipRect, 1, YXBanded);
#endif // !defined(i386_unknown_nt4_0)

   for (unsigned metriclcv = 0; metriclcv < indirectMetrics.size(); metriclcv++) {
      if (curr_x > maxVisibleX)
         break;

      const tvMetric &theMetric = metrics[indirectMetrics[metriclcv]];
      const int next_x = curr_x + theMetric.getColPixWidth();

      if (next_x - 1 < minVisibleX) {
         curr_x = next_x;
         continue;
      }

      const pdvector<tvCell> &thisMetricCells = cells[indirectMetrics[metriclcv]];
      drawCells1Col(theDrawable,
		    (curr_x + next_x - 1) / 2, // middle x
		    offset_y + getMetricAreaPixHeight(), // start y
		    thisMetricCells);

      curr_x = next_x;
   }

   XSetClipMask(Tk_Display(theTkWindow), cellGC, None);
}

void tableVisi::drawCells1Col(Drawable theDrawable, int middle_x, int top_y,
			      const pdvector<tvCell> &thisMetricCells) const {
   // uses getVertPixFocusTop2Baseline() and getFocusLinePixHeight()
   int curr_y = top_y;

   int minVisibleY = 0;
   int maxVisibleY = Tk_Height(theTkWindow)-1;

   for (unsigned focuslcv=0; focuslcv < indirectFoci.size(); focuslcv++) {
      if (curr_y > maxVisibleY)
         break;

      const int next_y = curr_y + getFocusLinePixHeight();
      if (next_y - 1 < minVisibleY) {
         curr_y = next_y;
         continue;
      }

      const tvCell &theCell = thisMetricCells[indirectFoci[focuslcv]];
      if (!theCell.isValid()) {
         curr_y = next_y;
         continue;
      }

      // making a new "string" would be too expensive (calls new):
      char buffer[200];
      double2string(buffer, theCell.getData());

      int buffer_len = strlen(buffer);
      int string_pix_width = Tk_TextWidth(cellFont, buffer, buffer_len);
 
      Tk_DrawChars(Tk_Display(theTkWindow), theDrawable,
		  cellGC,
		  cellFont,
		  buffer, buffer_len,
		  middle_x - string_pix_width / 2,
		  curr_y + getVertPixCellTop2Baseline());

      curr_y = next_y;
   }
}

unsigned tableVisi::getVertPixCellTop2Baseline() const {
   return 2 + cellFontMetrics.ascent;
}


void
tableVisi::ReleaseResources( void )
{
    // release Tk resources that should be
    // gone by the time we destroy the GUI
    Tk_FreeColor(cellColor);
    Tk_FreeColor(focusNameColor);
    Tk_FreeColor(metricUnitsColor);
    Tk_FreeColor(metricNameColor);
    Tk_FreeColor(lineColor);
    Tk_FreeColor(backgroundColor);
    Tk_FreeColor(highlightedBackgroundColor);

    Tk_FreeFont(cellFont);
    Tk_FreeFont(focusNameFont);
    Tk_FreeFont(metricUnitsFont);
    Tk_FreeFont(metricNameFont);
}



/* *************************************************************** */

void tableVisi::clearMetrics(Tcl_Interp *interp) {
   metrics.resize(0);
   indirectMetrics.resize(0);
   cells.resize(0);
   all_cells_width = 0;

   if (offscreenPixmap)
      resize(interp);
}

void tableVisi::clearFoci(Tcl_Interp *interp) {
   foci.resize(0);
   indirectFoci.resize(0);

   unsigned numMetrics = getNumMetrics();
   for (unsigned i=0; i < numMetrics; i++) {
      pdvector<tvCell> &theVec = cells[i];
      theVec.resize(0);
   }

   all_cells_height = 0;
   maxFocusNamePixWidth = 0;

   if (offscreenPixmap)
      resize(interp);
}

void tableVisi::addMetric(unsigned iVisiLibMetId,
			  const string &metricName, const string &metricUnits) {
   tvMetric newTvMetric(iVisiLibMetId,
			metricName, metricUnits,
			metricNameFont,
			metricUnitsFont,
			cellFont,
			numSigFigs);
   metrics += newTvMetric;
   indirectMetrics += (metrics.size()-1);
   cells += pdvector<tvCell>();

   all_cells_width += newTvMetric.getColPixWidth();

   assert(metrics.size() == indirectMetrics.size());
   assert(cells.size() == metrics.size());
}

void tableVisi::changeUnitsLabel (unsigned which, const string &new_name) {
   if (which < indirectMetrics.size()) {
       tvMetric &theMetric = metrics[indirectMetrics[which]];
       theMetric.changeUnitsName(new_name);
   }
}

void tableVisi::addFocus(unsigned iVisiLibFocusId, const string &focusName) {
   tvFocus newTvFocus(iVisiLibFocusId, focusName, focusNameFont);
   foci += newTvFocus;
   indirectFoci += (foci.size()-1);

   unsigned numMetrics = metrics.size();
   for (unsigned metriclcv=0; metriclcv < numMetrics; metriclcv++) {
      pdvector<tvCell> &metricCells = cells[metriclcv];
      metricCells += tvCell();
   }

   if (focusLongNameMode)
      ipmax(maxFocusNamePixWidth, newTvFocus.getLongNamePixWidth());
   else
      ipmax(maxFocusNamePixWidth, newTvFocus.getShortNamePixWidth());

   all_cells_height += getFocusLinePixHeight();

   assert(foci.size() == indirectFoci.size());
}

void tableVisi::deleteMetric(unsigned theColumn) {
	unsigned metriclcv;

   // fry selection, if necessary
   if (theSelection == cell || theSelection == colOnly)
      if (selectedCol == theColumn)
         theSelection = none;

   unsigned actualMetricIndex = indirectMetrics[theColumn];

   // Update some internal gfx vrbles:
   all_cells_width -= metrics[actualMetricIndex].getColPixWidth();

   unsigned newNumMetrics = indirectMetrics.size()-1;
   for (metriclcv=theColumn; metriclcv < newNumMetrics; metriclcv++)
      indirectMetrics[metriclcv] = indirectMetrics[metriclcv+1];
   indirectMetrics.resize(newNumMetrics);

   for (metriclcv=actualMetricIndex; metriclcv < newNumMetrics; metriclcv++)
      metrics[metriclcv] = metrics[metriclcv+1];
   metrics.resize(newNumMetrics);

   for (metriclcv=actualMetricIndex; metriclcv < newNumMetrics; metriclcv++)
      cells[metriclcv] = cells[metriclcv+1];
   cells.resize(newNumMetrics);

   // now look for items whose index need to be 1 lower
   for (metriclcv=0; metriclcv < newNumMetrics; metriclcv++)
      if (indirectMetrics[metriclcv] > actualMetricIndex)
         indirectMetrics[metriclcv]--;
      else if (indirectMetrics[metriclcv] == actualMetricIndex)
         assert(false); // we should have deleted this guy

   // now see if the selected column needs to made 1 lower
   if (theSelection == cell || theSelection == colOnly)
      if (selectedCol > theColumn)
         selectedCol--;

   // a little sanity checking
   assert(indirectMetrics.size() == metrics.size());
   assert(cells.size() == metrics.size());
   for (metriclcv=0; metriclcv < newNumMetrics; metriclcv++)
      assert(indirectMetrics[metriclcv] < newNumMetrics);
   if (theSelection == cell || theSelection == colOnly)
      assert(selectedCol < newNumMetrics);
}

void tableVisi::deleteFocus(unsigned theRow) {
	unsigned focuslcv;
	unsigned metriclcv;

   // A shameless carbon copy of the routine "deleteMetric"

   // fry selection, if necessary
   if (theSelection == cell || theSelection == rowOnly)
      if (selectedRow == theRow)
         theSelection = none;

   unsigned actualFocusIndex = indirectFoci[theRow];
   
   // update some internal gfx vrbles:
   all_cells_height -= getFocusLinePixHeight();
   
   unsigned newNumFoci = indirectFoci.size()-1;
   for (focuslcv=theRow; focuslcv < newNumFoci; focuslcv++)
      indirectFoci[focuslcv] = indirectFoci[focuslcv+1];
   indirectFoci.resize(newNumFoci);

   for (focuslcv=actualFocusIndex; focuslcv < newNumFoci; focuslcv++)
      foci[focuslcv] = foci[focuslcv+1];
   foci.resize(newNumFoci);

   for (metriclcv=0; metriclcv < metrics.size(); metriclcv++) {
      pdvector<tvCell> &theColumn = cells[metriclcv];

      for (focuslcv=actualFocusIndex; focuslcv < newNumFoci; focuslcv++)
         theColumn[focuslcv] = theColumn[focuslcv+1];

      theColumn.resize(newNumFoci);
   }

   // now look for items whose index need to be 1 lower
   for (focuslcv=0; focuslcv < newNumFoci; focuslcv++)
      if (indirectFoci[focuslcv] > actualFocusIndex)
         indirectFoci[focuslcv]--;
      else if (indirectFoci[focuslcv] == actualFocusIndex)
         assert(false); // we should have deleted this guy

   // now see if the selection needs to be made 1 lower
   if (theSelection == cell || theSelection == rowOnly)
      if (selectedRow > theRow)
         selectedRow --;

   // a little sanity checking
   assert(indirectFoci.size() == foci.size());
   for (focuslcv = 0; focuslcv < newNumFoci; focuslcv++)
      assert(indirectFoci[focuslcv] < newNumFoci);
   if (theSelection == cell || theSelection == rowOnly)
      assert(selectedRow < newNumFoci);

   // Finish updating some internal gfx vrbles:
   maxFocusNamePixWidth = 0;
   for (focuslcv=0; focuslcv < newNumFoci; focuslcv++)
      if (focusLongNameMode)
         ipmax(maxFocusNamePixWidth, foci[focuslcv].getLongNamePixWidth());
      else
         ipmax(maxFocusNamePixWidth, foci[focuslcv].getShortNamePixWidth());
}

int tableVisi::partitionMetrics(int left, int right) {
   const tvMetric &pivot = metrics[indirectMetrics[left]];

   int l = left-1;
   int r = right+1;

   while (true) {
      while (metrics[indirectMetrics[--r]] > pivot)
         ;

      while (metrics[indirectMetrics[++l]] < pivot)
         ;

      if (l < r) {
         unsigned temp = indirectMetrics[l];
         indirectMetrics[l] = indirectMetrics[r];
         indirectMetrics[r] = temp;
      }
      else
         return r;
   }
}

void tableVisi::sortMetrics(int left, int right) {
   if (left < right) {
      int middle = partitionMetrics(left, right);
      sortMetrics(left, middle);
      sortMetrics(middle+1, right);
   }
}

void tableVisi::sortMetrics() {
   unsigned realSelectedMetric;
   if (theSelection == cell || theSelection == colOnly)
      realSelectedMetric = indirectMetrics[selectedCol];

   sortMetrics(0, metrics.size()-1);

   if (theSelection == cell || theSelection == colOnly) {
      for (unsigned metriclcv=0; metriclcv < indirectMetrics.size(); metriclcv++) {
         if (indirectMetrics[metriclcv] == realSelectedMetric) {
            selectedCol = metriclcv;
            return;
         }
      }
      assert(false);
   }
}

void tableVisi::unsortMetrics() {
   // After this call, the metric which used to be in sorted order X will now
   // be in sorted order indirectMetrics[X] (this array lookup must be done first tho)
   unsigned newSelectedMetric;
   if (theSelection == cell || theSelection == colOnly)
      newSelectedMetric = indirectMetrics[selectedCol];

   for (unsigned i=0; i < indirectMetrics.size(); i++)
      indirectMetrics[i] = i;

   if (theSelection == cell || theSelection == colOnly)
      selectedCol = newSelectedMetric;
}

int tableVisi::partitionFoci(int left, int right) {
   const tvFocus &pivot = foci[indirectFoci[left]];

   int l = left-1;
   int r = right+1;

   while (true) {
      do {
         r--;
      } while (foci[indirectFoci[r]].greater_than(pivot, focusLongNameMode)); 

      do {
         l++;
      } while (foci[indirectFoci[l]].less_than(pivot, focusLongNameMode));

      if (l < r) {
         unsigned temp = indirectFoci[l];
         indirectFoci[l] = indirectFoci[r];
         indirectFoci[r] = temp;
      }
      else
         return r;
   }
}

int tableVisi::partitionFociByValues(const pdvector<tvCell> &theMetricColumn,
				     int left, int right) {
   // note: theMetricColumn comes straight from "cells"; index it via focus numbers
   //       (not by the sorted indexes but rather by the real indexes) to get the
   //       cell value, needed for doing the comparisoin.

   const tvCell &pivot = theMetricColumn[indirectFoci[left]];

   int l = left-1;
   int r = right+1;

   while (true) {
       do {r--;} while (theMetricColumn[indirectFoci[r]] > pivot);
       do {l++;} while (theMetricColumn[indirectFoci[l]] < pivot);

       if (l < r) {
          unsigned temp = indirectFoci[l];
	  indirectFoci[l] = indirectFoci[r];
	  indirectFoci[r] = temp;
       }
       else
          return r;
   }
}

void tableVisi::sortFoci(int left, int right) {
   if (left < right) {
      int middle = partitionFoci(left, right);
      sortFoci(left, middle);
      sortFoci(middle+1, right);
   }
}

void tableVisi::sortFoci() {
   unsigned selectedRealFocus;
   if (theSelection == rowOnly || theSelection == cell)
      selectedRealFocus = indirectFoci[selectedRow];

   sortFoci(0, foci.size()-1);

   if (theSelection == rowOnly || theSelection == cell) {
      // we need to keep selectedRow consistent...
      for (unsigned focuslcv=0; focuslcv < indirectFoci.size(); focuslcv++) {
         if (indirectFoci[focuslcv] == selectedRealFocus) {
            selectedRow = focuslcv;
            return;
	 }
      }
      assert(false);
   }
}

void tableVisi::sortFociByValues(const pdvector<tvCell> &theMetricColumn,
				 int left, int right) {
   if (left < right) {
      int middle = partitionFociByValues(theMetricColumn, left, right);
      sortFociByValues(theMetricColumn, left, middle);
      sortFociByValues(theMetricColumn, middle+1, right);
   }
}

bool tableVisi::sortFociByValues() {
   // returns true iff successful (if there was a selected col which to sort by)
   if (theSelection != colOnly && theSelection != cell)
      return false;

   assert(theSelection == colOnly || theSelection == cell);
   unsigned selectedRealFocus;
   if (theSelection == cell) {
      // in this case, we must keep "selectedRow" consistent.
      // Let's say that before this call, selectedRow was 0 (the first row).
      // Where is it after this call?  Unfortunately, it seems that we'll have to
      // go looking through the new, sorted, indirectFoci[], looking for the
      // original "real" focus.  (Actually, another solution is to intrude into the
      // actual sort routine, updating "selectedRow" whenever it moves)
      selectedRealFocus = indirectFoci[selectedRow];
   }

   sortFociByValues(cells[indirectMetrics[selectedCol]], 0, foci.size()-1);

   if (theSelection == cell) {
      // as described above...
      for (unsigned focuslcv=0; focuslcv < indirectFoci.size(); focuslcv++) {
         if (indirectFoci[focuslcv] == selectedRealFocus) {
            selectedRow = focuslcv;
            return true;
         }
      }
      assert(false);
   }

   return true;
}

void tableVisi::unsortFoci() {
   // keep theSelection consistent.  Let's say the first focus (in sorted order) was
   // selected before calling this routine.  What sorted position will this focus be
   // after this routine?  Simple.  It will be located at the "true" position before
   // the sorting is done.
   if (theSelection == rowOnly || theSelection == cell)
      selectedRow = indirectFoci[selectedRow];

   for (unsigned i=0; i < indirectFoci.size(); i++)
      indirectFoci[i] = i;
}

bool tableVisi::setFocusNameMode(Tcl_Interp *interp, bool longNameMode) {
   // returns true iff any changes
   if (focusLongNameMode == longNameMode)
      return false;

   focusLongNameMode = longNameMode;

   // recalculate maxFocusNamePixWidth:
   maxFocusNamePixWidth=0;
   for (unsigned focuslcv=0; focuslcv < foci.size(); focuslcv++) {
      const tvFocus &theFocus = foci[focuslcv];

      if (focusLongNameMode)
         ipmax(maxFocusNamePixWidth, theFocus.getLongNamePixWidth());
      else
         ipmax(maxFocusNamePixWidth, theFocus.getShortNamePixWidth());
   }

   resize(interp);

   return true;
}

bool tableVisi::setSigFigs(unsigned newNumSigFigs) {
   if (newNumSigFigs == numSigFigs)
      return false;

   numSigFigs = newNumSigFigs;
   updateConversionString();
   all_cells_width = 0;
      // we'll be recalcing this from scratch, since col widths can change

   for (unsigned met=0; met < metrics.size(); met++) {
      // sorted order is not important here...
      tvMetric &theMetric = metrics[met];

      theMetric.changeNumSigFigs(newNumSigFigs, cellFont);

      all_cells_width += theMetric.getColPixWidth();
   }

   return true;
}

void tableVisi::invalidateCell(unsigned theMetric, unsigned theFocus) {
   cells[theMetric][theFocus].invalidate();
}

void tableVisi::setCellValidData(unsigned theMetric, unsigned theFocus, double data) {
   cells[theMetric][theFocus].setValidData(data);
}

/* *************************************************************** */

bool tableVisi::adjustHorizSBOffset(Tcl_Interp *interp, float newFirst) {
   // doesn't redraw; returns true iff any changes.
   newFirst = moveScrollBar(interp, ".horizScrollbar", newFirst);

   int total_cell_width = get_total_cell_x_pix();
   int old_offset_x = offset_x;
   offset_x = -(int)(newFirst * total_cell_width); // yes, always <= 0

   return (offset_x != old_offset_x);
}

bool tableVisi::adjustVertSBOffset(Tcl_Interp *interp, float newFirst) {
   // doesn't redraw; returns true iff any changes.
   newFirst = moveScrollBar(interp, ".vertScrollbar", newFirst);

   int total_cell_height = get_total_cell_y_pix();
   int old_offset_y = offset_y;
   offset_y = -(int)(newFirst * total_cell_height); // yes, always <= 0

   return (offset_y != old_offset_y);
}

bool tableVisi::processClick(int x, int y) {
   if (x < 0)
      return false; // click was too far left
   if (y < 0)
      return false; // click was too far up

   int focusAreaPixWidth = getFocusAreaPixWidth();
   int metricAreaPixHeight = getMetricAreaPixHeight();
   int focusLinePixHeight = getFocusLinePixHeight();

   if (x < focusAreaPixWidth) {
      // Judging by the x coords, it looks like a click on a focus name...
      if (y < metricAreaPixHeight)
         return false; // ...but it fell in that blank space, upper-left corner

      // adjust for scrollbar setting & metric area:
      y -= offset_y; // will _add_ to y
      y -= metricAreaPixHeight;

      assert(y >= 0);
      unsigned rowClickedOn = y / focusLinePixHeight;
      if (rowClickedOn >= getNumFoci()) {
         // clicked below all rows.  This implies visible-y-pix  > total-y-pix, which
         // implies that the vertical sb was turned off hence offset_y will be zero.
         assert(offset_y == 0);
         return false;
      }

      if (theSelection == rowOnly && selectedRow == rowClickedOn) {
         // cout << "unselected focus row " << rowClickedOn << endl;
         theSelection = none;
         return true;
      }
      else {
         // cout << "Selected focus row " << rowClickedOn << endl;
         theSelection = rowOnly;
         selectedRow = rowClickedOn;
         return true;
      }
   }
   else if (y < metricAreaPixHeight) {
      // Judging by the y coords, it looks like a click on a metric name.
      // We already know that the x coord excludes the possibility of it being a click
      // on a focus name.

      unsigned clickCol;
      if (!xpix2col(x, clickCol))
         return false; // too far right or left
      
      if (theSelection == colOnly && selectedCol == clickCol) {
         // unselect the col
         theSelection = none;
         // cout << "Unselected metric col " << selectedCol << endl;
      }
      else {
         theSelection = colOnly;
         selectedCol = clickCol;
         // cout << "selected metric col " << selectedCol << endl;
      }

      return true;
   }
   else {
      // We did not click on a focus or on a metric.  The best guess now is for
      // a click on some cell.

      unsigned clickCol;
      if (!xpix2col(x, clickCol))
         return false; // too far right or left

      unsigned clickRow;
      if (!ypix2row(y, clickRow))
         return false; // too far up or down
 
      if (theSelection==cell && selectedRow == clickRow && selectedCol == clickCol) {
         // unselect this cell
         theSelection = none;
         // cout << "unselected cell (" << selectedRow << "," << selectedCol << ")" << endl;
      }
      else {
         theSelection = cell;
         selectedRow = clickRow;
         selectedCol = clickCol;
         // cout << "selected cell (" << selectedRow << "," << selectedCol << ")" << endl;
      }

      return true;
   }
}
