// scrollbar.C
// Ariel Tamches
// The scrollbar class for where4tree.h/.C

/* $Log: scrollbar.C,v $
/* Revision 1.1  1995/07/17 04:58:59  tamches
/* First version of the new where axis
/*
 */

#include "minmax.h"
#include "scrollbar.h"

int scrollbar::borderWidth=2;
int scrollbar::arrowHeight=16;
int scrollbar::arrowBorderWidth=2;
int scrollbar::sliderBorderWidth=2;
int scrollbar::totalWidth=16;

void scrollbar::draw(const where4TreeConstants &tc,
		     int theDrawable,
                     const int leftpix,
		     const int toppix, const int botpix,
		     const unsigned viewableDataPix,
		     const unsigned totalFullDataPix // if no SB were needed
		     ) const {
   Tk_Fill3DRectangle(tc.theTkWindow, theDrawable,
		      tc.listboxScrollbarBorderNormal,
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
   Tk_Fill3DPolygon(tc.theTkWindow, theDrawable,
		    tc.listboxScrollbarBorderNormal,
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
   Tk_Fill3DPolygon(tc.theTkWindow, theDrawable,
		    tc.listboxScrollbarBorderNormal,
		    arrowPoints, 3,
		    arrowBorderWidth, // static member vrble
		    TK_RELIEF_RAISED);

   // slider:
   int sliderPixTop;
   int sliderPixHeight;
   getSliderCoords(tc, toppix, botpix,
		   viewableDataPix,
		   totalFullDataPix,
		   sliderPixTop, // filled in
                   sliderPixHeight // filled in
		   );

   Tk_Fill3DRectangle(tc.theTkWindow, theDrawable,
		      tc.listboxScrollbarBorderNormal,
		      leftpix+1,
		      sliderPixTop,
		      totalWidth-2,
		      sliderPixHeight,
		      sliderBorderWidth, // static member vrble (2)
                      TK_RELIEF_RAISED);
}

void scrollbar::updateForNewBounds(const unsigned actualAvailDataPix,
				   const unsigned totalFullDataPix) {
   // maintain the relation pixFirst+actualAvailDataPix-1 <= totalFullDataPix
   // (by ensuring pixFirst <= totalFullPix-actualAvailDataPix+1)

   assert(totalFullDataPix >= actualAvailDataPix);

   pixFirst=min(pixFirst, 
		(int)(totalFullDataPix - actualAvailDataPix + 1)
		);
}

void scrollbar::getSliderCoords(const where4TreeConstants &tc,
				const int scrollBarTop, const int scrollBarBottom,
				const unsigned viewableDataPix,
				const unsigned totalDataPix,
				int &sliderPixTop,
				int &sliderPixHeight) const {
   const int scrollBarHeight = scrollBarBottom - scrollBarTop + 1;
   const int totalSliderRealEstate = scrollBarHeight - 2*arrowHeight; // what about -2*borderWidth?

   const double fractionOfDataViewable = (double)viewableDataPix / 
                                         (double)totalDataPix;

   sliderPixHeight = max(10, (int)(fractionOfDataViewable * totalSliderRealEstate));
   sliderPixTop = scrollBarTop + arrowHeight + pixFirst*totalSliderRealEstate/totalDataPix; // what about +borderWidth?
}

int scrollbar::pixFirstFromAbsoluteCoord(const where4TreeConstants &tc,
					 const int scrollBarTop, const int scrollBarBottom,
					 const unsigned totalDataPix,
					 const int tentativeNewSliderPixTop) const {
   // This routine simply solves for "pixFirst" in the "sliderPixTop=....." equation
   // of "getSliderCoords".  The main input parameter is "tentativeNewSliderPixTop".

   const int scrollBarHeight = scrollBarBottom - scrollBarTop + 1;
   const int totalSliderRealEstate = scrollBarHeight - 2*arrowHeight;

   int result = tentativeNewSliderPixTop - scrollBarTop - arrowHeight; // could be < 0
   result = max(result, 0);
   result = result * totalDataPix / totalSliderRealEstate;
   return result;
}
