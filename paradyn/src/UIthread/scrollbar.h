// scrollbar.h
// Ariel Tamches
// The scrollbar class for where4tree.h/.C

/* $Log: scrollbar.h,v $
/* Revision 1.3  1995/10/17 22:06:39  tamches
/* Removed where4TreeConstants influences; now Tk_3DBorder is
/* passed directly to draw().
/*
 * Revision 1.2  1995/09/20 01:18:28  tamches
 * Some routines didn't need where4TreeConstants
 *
 * Revision 1.1  1995/07/17  04:58:58  tamches
 * First version of the new where axis
 *
 */

#ifndef _SCROLLBAR_H_
#define _SCROLLBAR_H_

#include <assert.h>
#include "tkclean.h"

class scrollbar {
 private:
   static int borderWidth; // 2
   static int arrowHeight; // 16
   static int arrowBorderWidth; // 2
   static int sliderBorderWidth; // 2
   static int totalWidth; // 16
  
   int pixFirst; // can't be unsigned; we use -1 to indicate invalid

 public:

   scrollbar() {pixFirst=0;}
  ~scrollbar() {}

   void draw(Tk_Window theTkWindow,
             Tk_3DBorder border,
             int theDrawable,
	     int leftpix,
	     int toppix, int botpix,
	     unsigned viewableDataPix,
	     unsigned totalFullPix // if no SB were needed
	     ) const;

   void invalidate() {pixFirst=-1;}
   void validate()   {if (pixFirst < 0) pixFirst=0;}
   bool isValid() const {return (pixFirst>=0);}

   int  getPixFirst() const {assert(isValid()); return pixFirst;}
   void setPixFirst(int newPixFirst) {assert(isValid()); pixFirst = newPixFirst;}

   void updateForNewBounds(unsigned actualAvailDataPix,
			   unsigned totalFullDataPix);
      // maintain the relation pixFirst+actualAvailDataPix-1 <= totalFullDataPix
      // (by ensuring pixFirst <= totalFullPix-actualAvailDataPix+1)

   void getSliderCoords(int scrollBarTop, int ScrollBarBottom,
			unsigned viewableDataPix,
			unsigned totalDataPix,
			int &sliderPixTop,
			int &sliderPixHeight) const;

   int pixFirstFromAbsoluteCoord(int scrollBarTop, int scrollBarBottom,
				 unsigned totalDataPix,
				 int tentativeNewSliderPixTop) const;

   int getArrowPixHeight() const {
      return totalWidth; // static member vrble
   }
};

#endif
