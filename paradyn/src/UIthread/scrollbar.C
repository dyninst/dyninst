/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// scrollbar.C
// Ariel Tamches
// The scrollbar class for where4tree.h/.C

/* $Id: scrollbar.C,v 1.7 2004/03/23 01:12:30 eli Exp $ */

#include "paradyn/src/UIthread/minmax.h"
#include "scrollbar.h"

int scrollbar::borderWidth=2;
int scrollbar::arrowHeight=16;
int scrollbar::arrowBorderWidth=2;
int scrollbar::sliderBorderWidth=2;
int scrollbar::totalWidth=16;

void scrollbar::draw(Tk_Window theTkWindow,
		     Tk_3DBorder border,
		     int theDrawable,
                     int leftpix,
		     int toppix, int botpix,
		     unsigned viewableDataPix,
		     unsigned totalFullDataPix // if no SB were needed
		     ) const {
   Tk_Fill3DRectangle(theTkWindow, theDrawable,
		      border,
		      leftpix, toppix,
		      totalWidth, // static member vrble
		      botpix-toppix+1,
		      borderWidth,
		      TK_RELIEF_GROOVE);
   
   // top arrow
   XPoint arrowPoints[4];
   arrowPoints[0].x = leftpix;
   arrowPoints[0].y = toppix + arrowHeight - 1;
   arrowPoints[1].x = leftpix + totalWidth - 1;
   arrowPoints[1].y = arrowPoints[0].y;
   arrowPoints[2].x = leftpix + totalWidth/2;
   arrowPoints[2].y = toppix;
   Tk_Fill3DPolygon(theTkWindow, theDrawable,
		    border,
		    arrowPoints, 3,
		    arrowBorderWidth, // static member vrble
		    TK_RELIEF_RAISED);

   // bottom arrow
   arrowPoints[0].x = leftpix;
   arrowPoints[0].y = botpix - arrowHeight + 1;
   arrowPoints[1].x = leftpix + totalWidth/2;
   arrowPoints[1].y = botpix;
   arrowPoints[2].x = leftpix + totalWidth - 1;
   arrowPoints[2].y = arrowPoints[0].y;
   Tk_Fill3DPolygon(theTkWindow, theDrawable,
		    border,
		    arrowPoints, 3,
		    arrowBorderWidth, // static member vrble
		    TK_RELIEF_RAISED);

   // slider:
   int sliderPixTop;
   int sliderPixHeight;
   getSliderCoords(toppix, botpix,
		   viewableDataPix,
		   totalFullDataPix,
		   sliderPixTop, // filled in
                   sliderPixHeight // filled in
		   );

   Tk_Fill3DRectangle(theTkWindow, theDrawable,
		      border,
		      leftpix+1,
		      sliderPixTop,
		      totalWidth-2,
		      sliderPixHeight,
		      sliderBorderWidth, // static member vrble (2)
                      TK_RELIEF_RAISED);
}

void scrollbar::updateForNewBounds(unsigned actualAvailDataPix,
				   unsigned totalFullDataPix) {
   // maintain the relation pixFirst+actualAvailDataPix-1 <= totalFullDataPix
   // (by ensuring pixFirst <= totalFullPix-actualAvailDataPix+1)

   assert(totalFullDataPix >= actualAvailDataPix);
   ipmin(pixFirst, (int)(totalFullDataPix - actualAvailDataPix + 1));
}

void scrollbar::getSliderCoords(int scrollBarTop, int scrollBarBottom,
				unsigned viewableDataPix,
				unsigned totalDataPix,
				int &sliderPixTop,
				int &sliderPixHeight) const {
   const int minSliderPixHeight = 10;
   const int totalRawSliderHeight = scrollBarBottom - scrollBarTop + 1 - 2*arrowHeight;
      // what about -2*borderWidth?

   const double fractionOfDataViewable = (double)viewableDataPix / 
                                         (double)totalDataPix;

   sliderPixHeight = max(minSliderPixHeight,
			 (int)(fractionOfDataViewable * totalRawSliderHeight));

   sliderPixTop = scrollBarTop + arrowHeight +
                  pixFirst * totalRawSliderHeight / totalDataPix;
      // what about +borderWidth?

//   // Now, we need to pin sliderPixTop; this necessary when the slider is at
//   // the bottom of the scrollbar & the viewable data area is so huge that
//   // sliderPixHeight would have been less than 10, had we not max'd it up to 10, above.
//   const int maxSliderPixBottom = scrollBarBottom - arrowHeight;
//   const int maxSliderPixTop = maxSliderPixBottom - sliderPixHeight + 1;
//   ipmin(sliderPixTop, maxSliderPixTop);
}

int scrollbar::pixFirstFromAbsoluteCoord(int scrollBarTop,
					 int scrollBarBottom,
					 unsigned totalDataPix,
					 int tentativeNewSliderPixTop) const {
   // This routine simply solves for "pixFirst" in the "sliderPixTop=....." equation
   // of "getSliderCoords()".  The main input parameter is "tentativeNewSliderPixTop".

   const int scrollBarHeight = scrollBarBottom - scrollBarTop + 1;
   const int totalSliderRealEstate = scrollBarHeight - 2*arrowHeight;

   int result = tentativeNewSliderPixTop - scrollBarTop - arrowHeight; // could be < 0
   ipmax(result, 0);
   result = result * totalDataPix / totalSliderRealEstate;
   return result;
}
