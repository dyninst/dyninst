// tableVisi.h
// Ariel Tamches

/*
 * $Log: tableVisi.h,v $
 * Revision 1.2  1995/11/08 21:47:04  tamches
 * removed some unused members
 *
 * Revision 1.1  1995/11/04 00:45:19  tamches
 * First version of new table visi
 *
 */

#ifndef _TABLE_VISI_H_
#define _TABLE_VISI_H_

#include "String.h"
#include "Vector.h"
#include "minmax.h"

#include "tclclean.h"
#include "tkclean.h"
#include "tkTools.h"

#include "tvMetric.h"
#include "tvFocus.h"
#include "tvCell.h"

class tableVisi {
 private:
   vector<tvMetric> metrics;
   vector<tvFocus> foci;
   vector<unsigned> indirectMetrics; // for sorting
   vector<unsigned> indirectFoci; // for sorting
   vector< vector<tvCell> > cells; // array[metrics] of array[foci]
   bool focusLongNameMode;
   unsigned numSigFigs;

   Tk_Window theTkWindow;
   Display *theDisplay; // needed only in the destructor
   Pixmap offscreenPixmap;
   XColor *backgroundColor; // for erasing the offscreen pixmap
   GC backgroundGC; // same...

   int offset_x, offset_y;
   int all_cells_width, all_cells_height; // # pixels needed in horiz, vert sb's

   XFontStruct *metricNameFont, *metricUnitsFont, *focusNameFont, *cellFont;
   unsigned maxFocusNamePixWidth; // max of foci[].getNamePixWidth()

   XColor *lineColor; GC lineColorGC;
   XColor *metricNameColor; GC metricNameGC;
   XColor *metricUnitsColor; GC metricUnitsGC;
   XColor *focusNameColor; GC focusNameGC;
   XColor *cellColor; GC cellGC;

 private:

   void double2string(char *, double) const;

   XFontStruct *myXLoadQueryFont(const string &fontName) const;
   XColor *myTkGetColor(Tcl_Interp *, const string &colorName) const;
   void resizeScrollbars(Tcl_Interp *);
   bool adjustHorizSBOffset(Tcl_Interp *interp);
   bool adjustVertSBOffset(Tcl_Interp *interp);

   // private metric helper functions
   void drawMetricNames(Drawable) const;
   void drawMetricVertLine(Drawable, int x) const;
   unsigned getMetricAreaPixHeight() const;
   unsigned getMetricNameBaseline() const; // assumes 0 is top y coord of metric area
   unsigned getMetricUnitsBaseline() const; // assumes 0 is top y coord of metric area

   // private focus helper functions
   void drawFocusNames(Drawable) const;
   void drawFocusHorizLine(Drawable, int y) const;
   unsigned getFocusLinePixHeight() const;
   unsigned getVertPixFocusTop2Baseline() const;
   unsigned getHorizPixBeforeFocusName() const {return 3;}
   unsigned getFocusAreaPixWidth() const;

   // private cell helper functions
   void drawCells(Drawable) const;
   void drawCells1Col(Drawable, int middle_x, int top_y,
                      const vector<tvCell> &thisMetricCells) const;
   unsigned getVertPixCellTop2Baseline() const;

   // helper functions for sorting
   void sortMetrics(int left, int right);
   int partitionMetrics(int left, int right);

   void sortFoci(int left, int right);
   int partitionFoci(int left, int right);

 public:
   tableVisi(Tcl_Interp *interp,
             Tk_Window iTkWindow,
             const string &metricNameFontName,
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
             );
  ~tableVisi();

   bool tryFirst();
   void resize(Tcl_Interp *); // does not redraw

   void draw(bool xsynch) const;

   unsigned getNumMetrics() const {return metrics.size();}
   unsigned getNumFoci()    const {return foci.size();}

   void clearMetrics(Tcl_Interp *interp);
   void clearFoci(Tcl_Interp *interp);

   void addMetric(const string &metricName, const string &metricUnits);
   void addFocus(const string &focusName);

   void sortMetrics();
   void unsortMetrics();

   void sortFoci();
   void unsortFoci();

   bool setFocusNameMode(Tcl_Interp *interp, bool longNameMode);
      // returns true iff any changes

   bool setSigFigs(unsigned);
      // returns true iff any changes

   void invalidateCell(unsigned theMetric, unsigned theFocus);
   void setCellValidData(unsigned theMetric, unsigned theFocus, double data);

   int get_offset_x() const {return offset_y;}
   int get_offset_y() const {return offset_y;}

   int get_total_cell_x_pix() const {return all_cells_width;}
   int get_total_cell_y_pix() const {return all_cells_height;}

   int get_visible_x_pix() const {return Tk_Width(theTkWindow);}
   int get_visible_y_pix() const {return Tk_Height(theTkWindow);}

   bool adjustHorizSBOffset(Tcl_Interp *, float newFirst);
   bool adjustVertSBOffset(Tcl_Interp *, float newFirst);
};

#endif
