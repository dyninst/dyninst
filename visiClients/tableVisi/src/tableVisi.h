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

// tableVisi.h
// Ariel Tamches

/*
 * $Log: tableVisi.h,v $
 * Revision 1.10  2002/12/20 07:50:09  jaw
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
 * Revision 1.9  2000/07/28 17:23:01  pcroth
 * Updated #includes to reflect util library split
 *
 * Revision 1.8  1999/07/13 17:16:11  pcroth
 * Fixed ordering problem of destroying GUI and destructing static variable
 * pdLogo::all_logos.  On NT, the static variable is destroyed before the
 * GUI, but a callback for the GUI ends up referencing the variable causing
 * an access violation error.
 *
 * Revision 1.7  1999/03/13 15:24:05  pcroth
 * Added support for building under Windows NT
 *
 * Revision 1.6  1996/08/16 21:36:58  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1995/12/22 22:43:08  tamches
 * selection
 * deletion
 * sort foci by value
 *
 * Revision 1.4  1995/12/19 00:44:39  tamches
 * changeUnitsLabel now takes in a string &, not char *
 *
 * Revision 1.3  1995/12/03 21:09:27  newhall
 * changed units labeling to match type of data being displayed
 *
 * Revision 1.2  1995/11/08  21:47:04  tamches
 * removed some unused members
 *
 * Revision 1.1  1995/11/04 00:45:19  tamches
 * First version of new table visi
 *
 */

#ifndef _TABLE_VISI_H_
#define _TABLE_VISI_H_

#include "common/h/String.h"
#include "common/h/Vector.h"
#include "minmax.h"

#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

#include "tvMetric.h"
#include "tvFocus.h"
#include "tvCell.h"

class tableVisi {
 private:
   pdvector<tvMetric> metrics;
   pdvector<tvFocus> foci;
   pdvector<unsigned> indirectMetrics; // for sorting
   pdvector<unsigned> indirectFoci;    // for sorting
   pdvector< pdvector<tvCell> > cells;   // array[metrics] of array[foci]
   bool focusLongNameMode;
   unsigned numSigFigs;
   char conversionString[100]; // e.g. "%.5g" if numSigFigs is currently 5

 public:
   enum selection {rowOnly, colOnly, cell, none};

 private:
   selection theSelection;
   // The value of "theSelection" guides how to interpret the following 2 vrbles;
   // in particular, one or both may be undefined.  Question: should these be
   // in sorted or "real" order.  Currently, it's sorted.
   unsigned selectedRow;
   unsigned selectedCol;

   Tk_Window theTkWindow;
   Display *theDisplay; // needed only in the destructor
   Pixmap offscreenPixmap;
   XColor *backgroundColor; // for erasing the offscreen pixmap...
   GC backgroundGC;         // ...same
   XColor *highlightedBackgroundColor; // for drawing selected cells...
   GC highlightedBackgroundGC;         // ...same

   int offset_x, offset_y; // <= 0
   int all_cells_width, all_cells_height; // # pixels needed in horiz, vert sb's

   Tk_Font	metricNameFont;
   Tk_Font	metricUnitsFont;
   Tk_Font	focusNameFont;
   Tk_Font	cellFont;
   Tk_FontMetrics	metricNameFontMetrics;
   Tk_FontMetrics	metricUnitsFontMetrics;
   Tk_FontMetrics	focusNameFontMetrics;
   Tk_FontMetrics	cellFontMetrics;
   unsigned maxFocusNamePixWidth; // max of foci[].getNamePixWidth()

   XColor *lineColor; GC lineColorGC;
   XColor *metricNameColor; GC metricNameGC;
   XColor *metricUnitsColor; GC metricUnitsGC;
   XColor *focusNameColor; GC focusNameGC;
   XColor *cellColor; GC cellGC;

 private:

   void updateConversionString();
   void double2string(char *, double) const;

   Tk_Font myTkGetFont(Tcl_Interp*, const string &fontName) const;
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
   bool xpix2col(int x, unsigned &theColumn) const;
      // sets theColumn and returns true iff the pixel is within some column.
      // note that x should _not_ be adjusted for scrollbar; we take care of that
   int metric2xpix(unsigned theColumn) const;
      // returns the x coord where this column starts.  Do not adjust for the
      // scrollbar; we do that for you.

   // private focus helper functions
   void drawFocusNames(Drawable) const;
   void drawFocusHorizLine(Drawable, int y) const;
   unsigned getFocusLinePixHeight() const;
   unsigned getVertPixFocusTop2Baseline() const;
   unsigned getHorizPixBeforeFocusName() const {return 3;}
   unsigned getFocusAreaPixWidth() const;
   bool ypix2row(int y, unsigned &theRow) const;
      // sets therow and returns true iff the y coord is w/in some row.
      // note that y should _not_ be adjusted for scrollbar; we take care of that.
   int focus2ypix(unsigned theRow) const;
      // returns the y coord where this row starts.  Do not adjust for the scrollbar;
      // we do that for you.

   // private cell helper functions
   void drawCells(Drawable) const;
   void drawCells1Col(Drawable, int middle_x, int top_y,
                      const pdvector<tvCell> &thisMetricCells) const;
   unsigned getVertPixCellTop2Baseline() const;

   // helper function for drawing
   void drawHighlightBackground(Drawable) const;

   // helper functions for sorting
   void sortMetrics(int left, int right);
   int partitionMetrics(int left, int right);

   void sortFoci(int left, int right);
   void sortFociByValues(const pdvector<tvCell> &theMetricColumn, int left, int right);
   int partitionFoci(int left, int right);
   int partitionFociByValues(const pdvector<tvCell> &, int left, int right);

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
             const string &highlightedBackgroundColorName,
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

   void addMetric(unsigned iVisiLibMetId,
		  const string &metricName, const string &metricUnits);
   void addFocus(unsigned iVisiLibFocusId, const string &focusName);
   void changeUnitsLabel(unsigned which, const string &new_name);

   // The following routines should be followed by a call to resize(), in order
   // to make the scrollbars, etc. coherent again:
   void deleteMetric(unsigned theColumn); // theColumn is in sorted order
   void deleteFocus(unsigned theRow); // theRow is in sorted order

   void sortMetrics(); // may change selectedCol, as appropriate.
   void unsortMetrics(); // may change selectedCol, as appropriate.

   void sortFoci(); // may change selectedRow, as appropriate
   bool sortFociByValues(); // may change selectedRow, as appropriate
      // If a column (metric) is selected, sort all foci according to that col.
      // Returns true iff successful (if exactly 1 metric col was selected)
   void unsortFoci(); // may change selectedRow, as appopriate

   bool setFocusNameMode(Tcl_Interp *interp, bool longNameMode);
      // returns true iff any changes

   bool setSigFigs(unsigned);
      // returns true iff any changes.  Can change column widths.

   void invalidateCell(unsigned theMetric, unsigned theFocus);
      // theMetric and theFocus are _not_ in sorted order; they're the real thing
   void setCellValidData(unsigned theMetric, unsigned theFocus, double data);
      // theMetric and theFocus are _not_ in sorted order; they're the real thing

   int get_offset_x() const {return offset_y;}
   int get_offset_y() const {return offset_y;}

   int get_total_cell_x_pix() const {return all_cells_width;}
   int get_total_cell_y_pix() const {return all_cells_height;}

   int get_visible_x_pix() const {return Tk_Width(theTkWindow);}
   int get_visible_y_pix() const {return Tk_Height(theTkWindow);}

   bool adjustHorizSBOffset(Tcl_Interp *, float newFirst);
   bool adjustVertSBOffset(Tcl_Interp *, float newFirst);

   bool processClick(int x, int y);
      // make a row, col, or cell selection or unselection, based on coords relative
      // to the upper-left pixel of the table (i.e. _not_ adjusted for scrollbar
      // settings; we do that for you)
      // returns true iff any changes were made, in which case outside code should
      // probably launch a redraw operation...
   selection getSelection() const {return theSelection;}
   unsigned getSelectedMetId() const {
      assert(theSelection == cell || theSelection == colOnly);
      return col2MetId(selectedCol);
   }
   unsigned getSelectedCol() const {
      assert(theSelection == cell || theSelection == colOnly);
      return selectedCol;
   }

   unsigned getSelectedResId() const {
      assert(theSelection == cell || theSelection == rowOnly);
      return row2ResId(selectedRow);
   }
   unsigned getSelectedRow() const {
      assert(theSelection == cell || theSelection == rowOnly);
      return selectedRow;
   }

   unsigned col2MetId(unsigned col) const {
      return metrics[indirectMetrics[col]].getVisiLibId();
   }
   unsigned row2ResId(unsigned row) const {
      return foci[indirectFoci[row]].getVisiLibId();
   }

   unsigned metric2MetId(unsigned theMetric) const {
      return metrics[theMetric].getVisiLibId();
   }
   unsigned focus2ResId(unsigned theFocus) const {
      return foci[theFocus].getVisiLibId();
   }

   void deleteSelection();

   void ReleaseResources( void );
};

#endif
