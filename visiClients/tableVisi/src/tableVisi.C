// tableVisi.C
// Ariel Tamches

/*
 * $Log: tableVisi.C,v $
 * Revision 1.2  1995/11/08 21:46:46  tamches
 * some ui bug fixes
 *
 * Revision 1.1  1995/11/04 00:45:20  tamches
 * First version of new table visi
 *
 */

#include "minmax.h"
#include "tkTools.h"
#include "tableVisi.h"

/* ************************************************************* */

extern "C" {bool isnan(double);}
void tableVisi::double2string(char *buffer, double val) const {
   // uses numSigFigs
   if (isnan(val)) {
      buffer[0] = '\0';
      return;
   }

   char conversionString[100];
   sprintf(conversionString, "%%.%dg", numSigFigs);
   sprintf(buffer, conversionString, val);

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

XFontStruct *tableVisi::myXLoadQueryFont(const string &fontName) const {
   XFontStruct *result = XLoadQueryFont(Tk_Display(theTkWindow),
					fontName.string_of());
   if (result == NULL) {
      cerr << "could not find font " << fontName << endl;
      exit(5);
   }

   return result;
}

XColor *tableVisi::myTkGetColor(Tcl_Interp *interp, const string &colorName) const {
   XColor *result = Tk_GetColor(interp, theTkWindow, Tk_GetUid(colorName.string_of()));
   if (result == NULL) {
      cerr << "could not allocate color " << colorName << endl;
      exit(5);
   }
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
		     unsigned iSigFigs
		     ) {
   // metrics[], foci[], indirectMetrics[], indirectFoci[], and cells[][]
   // are all initialized to zero-sized arrays.

   theTkWindow = iTkWindow;
   theDisplay = Tk_Display(theTkWindow);

   offscreenPixmap = (Pixmap)NULL;
      // sorry, can't XCreatePixmap() until the window becomes mapped.

   backgroundColor = myTkGetColor(interp, backgroundColorName);

   offset_x = offset_y = 0;
   all_cells_width = all_cells_height = 0;

   metricNameFont = myXLoadQueryFont(metricFontName);
   metricUnitsFont = myXLoadQueryFont(metricUnitsFontName);
   focusNameFont = myXLoadQueryFont(focusFontName);
   cellFont = myXLoadQueryFont(cellFontName);

   focusLongNameMode = true;
   numSigFigs = iSigFigs;
//   dataFormat = Current;

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

   Tk_FreeColor(cellColor);
   Tk_FreeColor(focusNameColor);
   Tk_FreeColor(metricUnitsColor);
   Tk_FreeColor(metricNameColor);
   Tk_FreeColor(lineColor);
   Tk_FreeColor(backgroundColor);

   XFreeFont(theDisplay, focusNameFont);
   XFreeFont(theDisplay, metricUnitsFont);
   XFreeFont(theDisplay, metricNameFont);

   if (!offscreenPixmap)
      // the offscreen pixmap was never allocated(!)...so, we never
      // got around to mapping the window!
      return;

   XFreeGC(theDisplay, cellGC);
   XFreeGC(theDisplay, focusNameGC);
   XFreeGC(theDisplay, metricUnitsGC);
   XFreeGC(theDisplay, metricNameGC);
   XFreeGC(theDisplay, lineColorGC);
   XFreeGC(theDisplay, backgroundGC);

   XFreePixmap(theDisplay, offscreenPixmap);
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
   offscreenPixmap = XCreatePixmap(Tk_Display(theTkWindow),
				   Tk_WindowId(theTkWindow),
				   1, 1, // dummy width, height
				   Tk_Depth(theTkWindow));
   XGCValues values;
   values.foreground = backgroundColor->pixel;
   backgroundGC = XCreateGC(Tk_Display(theTkWindow), Tk_WindowId(theTkWindow),
			    GCForeground, &values);

   values.foreground = lineColor->pixel;
   lineColorGC = XCreateGC(Tk_Display(theTkWindow), Tk_WindowId(theTkWindow),
			   GCForeground, &values);

   values.foreground = metricNameColor->pixel;
   values.font = metricNameFont->fid;
   metricNameGC = XCreateGC(Tk_Display(theTkWindow), Tk_WindowId(theTkWindow),
			    GCForeground | GCFont, &values);

   values.foreground = metricUnitsColor->pixel;
   values.font = metricUnitsFont->fid;
   metricUnitsGC = XCreateGC(Tk_Display(theTkWindow), Tk_WindowId(theTkWindow),
			     GCForeground | GCFont, &values);

   values.foreground = focusNameColor->pixel;
   values.font = focusNameFont->fid;
   focusNameGC = XCreateGC(Tk_Display(theTkWindow), Tk_WindowId(theTkWindow),
			   GCForeground | GCFont, &values);

   values.foreground = cellColor->pixel;
   values.font = cellFont->fid;
   cellGC = XCreateGC(Tk_Display(theTkWindow), Tk_WindowId(theTkWindow),
		      GCForeground | GCFont, &values);

   return true;
}

void tableVisi::resizeScrollbars(Tcl_Interp *interp) {
   // used to be a tcl routine (resize1Scrollbar):

   const int visible_cell_width = Tk_Width(theTkWindow) -
                                  getFocusAreaPixWidth();
   resizeScrollbar(interp, ".horizScrollbar",
		   all_cells_width, // total
		   visible_cell_width);

   const int visible_cell_height = Tk_Height(theTkWindow) -
                                   getMetricAreaPixHeight();
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
   // does not resize.  Does things like resize the offscreen pixmap
   if (tryFirst()) {
      if (offscreenPixmap) {
         XFreePixmap(Tk_Display(theTkWindow), offscreenPixmap);

         offscreenPixmap = XCreatePixmap(Tk_Display(theTkWindow),
					 Tk_WindowId(theTkWindow),
					 Tk_Width(theTkWindow), Tk_Height(theTkWindow),
					 Tk_Depth(theTkWindow));
      }
   }

   resizeScrollbars(interp);
   adjustHorizSBOffset(interp);
   adjustVertSBOffset(interp);
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

   drawFocusNames(theDrawable);
      // leftmost part of screen; unaffected by offset_x
   drawMetricNames(theDrawable);
      // topmost part of screen; unaffected byy offset_y
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

   XSetClipRectangles(Tk_Display(theTkWindow), metricNameGC,
		      0, 0, &clipRect, 1, YXBanded);
   XSetClipRectangles(Tk_Display(theTkWindow), metricUnitsGC,
		      0, 0, &clipRect, 1, YXBanded);

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
      XDrawString(Tk_Display(theTkWindow), theDrawable,
		  metricNameGC,
		  metric_name_left, metric_name_baseline,
		  metricNameStr.string_of(), metricNameStr.length());

      // draw the metric units:
      int metric_units_left = curr_middle_x - theMetric.getUnitsPixWidth() / 2;
      const string &metricUnitsNameStr = theMetric.getUnitsName();
      XDrawString(Tk_Display(theTkWindow), theDrawable,
		  metricUnitsGC,
		  metric_units_left, metric_units_baseline,
		  metricUnitsNameStr.string_of(), metricUnitsNameStr.length());

      curr_x = next_x;
   }

   if (curr_x >= minVisibleX) // clipping
      drawMetricVertLine(theDrawable, curr_x);

   XSetClipMask(Tk_Display(theTkWindow), metricNameGC, None);
   XSetClipMask(Tk_Display(theTkWindow), metricUnitsGC, None);
}

void tableVisi::drawMetricVertLine(Drawable theDrawable, int x) const {
   XDrawLine(Tk_Display(theTkWindow), theDrawable,
	     lineColorGC,
	     x, 0, x, Tk_Height(theTkWindow)-1);
}

unsigned tableVisi::getMetricAreaPixHeight() const {
   return 3 + metricNameFont->ascent + metricNameFont->descent + 3 +
              metricUnitsFont->ascent + metricUnitsFont->descent + 3;
}

unsigned tableVisi::getMetricNameBaseline() const {
   return 3 + metricNameFont->ascent - 1;
}

unsigned tableVisi::getMetricUnitsBaseline() const {
   return 3 + metricNameFont->ascent + metricNameFont->descent + 3 +
              metricUnitsFont->ascent - 1;
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

   XSetClipRectangles(Tk_Display(theTkWindow), focusNameGC,
		      0, 0, &clipRect, 1, YXBanded);
   
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

      XDrawString(Tk_Display(theTkWindow), theDrawable,
		  focusNameGC,
		  getHorizPixBeforeFocusName(),
		  curr_y_baseline,
		  theString.string_of(), theString.length());

      curr_y = next_y;
   }

   XSetClipMask(Tk_Display(theTkWindow), focusNameGC, None);

   if (curr_y >= minVisibleY)
      drawFocusHorizLine(theDrawable, curr_y);
}

void tableVisi::drawFocusHorizLine(Drawable theDrawable, int y) const {
   XDrawLine(Tk_Display(theTkWindow), theDrawable,
	     lineColorGC,
	     0, y, Tk_Width(theTkWindow)-1, y);
}

unsigned tableVisi::getFocusLinePixHeight() const {
   return 2 + focusNameFont->ascent + focusNameFont->descent + 2;
}

unsigned tableVisi::getVertPixFocusTop2Baseline() const {
   return 2 + focusNameFont->ascent;
}

unsigned tableVisi::getFocusAreaPixWidth() const {
   return getHorizPixBeforeFocusName() + maxFocusNamePixWidth +
          getHorizPixBeforeFocusName();
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
   clipRect.height = Tk_Width(theTkWindow) - clipRect.y + 1;

   XSetClipRectangles(Tk_Display(theTkWindow), cellGC,
		      0, 0, &clipRect, 1, YXBanded);
   
   for (unsigned metriclcv = 0; metriclcv < indirectMetrics.size(); metriclcv++) {
      if (curr_x > maxVisibleX)
         break;

      const tvMetric &theMetric = metrics[indirectMetrics[metriclcv]];
      const int next_x = curr_x + theMetric.getColPixWidth();

      if (next_x - 1 < minVisibleX) {
         curr_x = next_x;
         continue;
      }

      const vector<tvCell> &thisMetricCells = cells[indirectMetrics[metriclcv]];
      drawCells1Col(theDrawable,
		    (curr_x + next_x - 1) / 2, // middle x
		    offset_y + getMetricAreaPixHeight(), // start y
		    thisMetricCells);

      curr_x = next_x;
   }

   XSetClipMask(Tk_Display(theTkWindow), cellGC, None);
}

void tableVisi::drawCells1Col(Drawable theDrawable, int middle_x, int top_y,
			      const vector<tvCell> &thisMetricCells) const {
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

      // making a new "string" would be toop expensive (calls new):
      char buffer[200];
      double2string(buffer, theCell.getData());
//      sprintf(buffer, "%g", theCell.getData());

      int buffer_len = strlen(buffer);
      int string_pix_width = XTextWidth(cellFont, buffer, buffer_len);
 
      XDrawString(Tk_Display(theTkWindow), theDrawable,
		  cellGC,
		  middle_x - string_pix_width / 2,
		  curr_y + getVertPixCellTop2Baseline(),
		  buffer, buffer_len);

      curr_y = next_y;
   }
}

unsigned tableVisi::getVertPixCellTop2Baseline() const {
   return 2 + cellFont->ascent;
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
      vector<tvCell> &theVec = cells[i];
      theVec.resize(0);
   }

   all_cells_height = 0;
   maxFocusNamePixWidth = 0;

   if (offscreenPixmap)
      resize(interp);
}

void tableVisi::addMetric(const string &metricName, const string &metricUnits) {
   tvMetric newTvMetric(metricName, metricUnits, metricNameFont, metricUnitsFont);
   metrics += newTvMetric;
   indirectMetrics += (metrics.size()-1);
   cells += vector<tvCell>();

   all_cells_width += newTvMetric.getColPixWidth();

   assert(metrics.size() == indirectMetrics.size());
   assert(cells.size() == metrics.size());
}

void tableVisi::addFocus(const string &focusName) {
   tvFocus newTvFocus(focusName, focusNameFont);
   foci += newTvFocus;
   indirectFoci += (foci.size()-1);

   unsigned numMetrics = metrics.size();
   for (unsigned metriclcv=0; metriclcv < numMetrics; metriclcv++) {
      vector<tvCell> &metricCells = cells[metriclcv];
      metricCells += tvCell();
   }

   if (focusLongNameMode)
      ipmax(maxFocusNamePixWidth, newTvFocus.getLongNamePixWidth());
   else
      ipmax(maxFocusNamePixWidth, newTvFocus.getShortNamePixWidth());

   all_cells_height += getFocusLinePixHeight();

   assert(foci.size() == indirectFoci.size());
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
   sortMetrics(0, metrics.size()-1);
}

void tableVisi::unsortMetrics() {
   for (unsigned i=0; i < indirectMetrics.size(); i++)
      indirectMetrics[i] = i;
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

void tableVisi::sortFoci(int left, int right) {
   if (left < right) {
      int middle = partitionFoci(left, right);
      sortFoci(left, middle);
      sortFoci(middle+1, right);
   }
}

void tableVisi::sortFoci() {
   sortFoci(0, foci.size()-1);
}

void tableVisi::unsortFoci() {
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
