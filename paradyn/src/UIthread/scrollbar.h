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

// scrollbar.h
// Ariel Tamches
// The scrollbar class for where4tree.h/.C

/* $Id: scrollbar.h,v 1.7 2004/03/23 01:12:30 eli Exp $ */

#ifndef _SCROLLBAR_H_
#define _SCROLLBAR_H_

#include <assert.h>
#include "tk.h"

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
