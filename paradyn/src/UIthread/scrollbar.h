// scrollbar.h
// Ariel Tamches
// The scrollbar class for where4tree.h/.C

/* $Log: scrollbar.h,v $
/* Revision 1.1  1995/07/17 04:58:58  tamches
/* First version of the new where axis
/*
 */

#ifndef _SCROLLBAR_H_
#define _SCROLLBAR_H_

#include <assert.h>
#include "where4treeConstants.h"

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

   void draw(const where4TreeConstants &tc,
             int theDrawable,
	     const int leftpix,
	     const int toppix, const int botpix,
	     const unsigned viewableDataPix,
	     const unsigned totalFullPix // if no SB were needed
	     ) const;

   void invalidate() {pixFirst=-1;}
   void validate()   {if (pixFirst < 0) pixFirst=0;}
   bool isValid() const {return (pixFirst>=0);}
   int getPixFirst() const {assert(isValid()); return pixFirst;}
   void setPixFirst(const int newPixFirst) {assert(isValid()); pixFirst = newPixFirst;}
   void updateForNewBounds(const unsigned actualAvailDataPix,
			   const unsigned totalFullDataPix);

   void getSliderCoords(const where4TreeConstants &tc,
			const int scrollBarTop, const int ScrollBarBottom,
			const unsigned viewableDataPix,
			const unsigned totalDataPix,
			int &sliderPixTop,
			int &sliderPixHeight) const;

   int pixFirstFromAbsoluteCoord(const where4TreeConstants &tc,
				 const int scrollBarTop, const int scrollBarBottom,
				 const unsigned totalDataPix,
				 const int tentativeNewSliderPixTop) const;

   int getArrowPixHeight() const {
      return totalWidth; // static member vrble
   }
};

#endif
