// shgRootNode.h
// Ariel Tamches
// analagous to rootNode.h (for the where axis)

/* $Log: shgRootNode.h,v $
/* Revision 1.1  1995/10/17 22:08:52  tamches
/* initial version, for the new search history graph
/*
 */

#ifndef _SHG_ROOT_NODE_H_
#define _SHG_ROOT_NODE_H_

#ifndef PARADYN
#include "String.h"
#else
#include "util/h/String.h"
#endif

#include "where4treeConstants.h" // yuck

class shgRootNode {
 public:
   enum style {Uninstrumented, InstrumentedAndTesting,
		 TestedTentativelyTrue, TestedFalse};
 private:
   unsigned id;
   string label, fullInfo;
   bool highlighted;
   style theStyle;

   int pixWidthAsRoot, pixHeightAsRoot;
   int pixWidthAsListboxItem;

   static int borderPix;
   static int horizPad, vertPad;

 public:
   shgRootNode(unsigned iId, style iStyle,
	       const string &iLabel, const string &iFullInfo);
  ~shgRootNode() {}

   unsigned getId() const {return id;}

   bool operator<(const shgRootNode &other) const {return label < other.label;}
   bool operator>(const shgRootNode &other) const {return label > other.label;}

   const string &getName() const {return label;}
   const string &getLongName() const {return fullInfo;}

   bool getHighlighted() const {return highlighted;}
   void highlight() {highlighted = true;}
   void unhighlight() {highlighted = false;}
   void toggle_highlight() {highlighted = !highlighted;}

   int getHeightAsRoot() const {return pixHeightAsRoot;}
   int getWidthAsRoot()  const {return pixWidthAsRoot;}
   int getWidthAsListboxItem() const {return pixWidthAsListboxItem;}

   style getStyle() const {return theStyle;}
   bool configStyle(style newStyle);
      // returns true iff any changes.  Does not redraw.

   void drawAsRoot(Tk_Window, int theDrawable,
		   int centerx, int topy) const;

   static void prepareForDrawingListboxItems(Tk_Window, XRectangle &listboxBounds);
      // called by "where4tree<shgRootNode>" before it draws listbox items.
      // Gives us a chance to set up clipping bounds.
   static void doneDrawingListboxItems(Tk_Window);
      // called by "where4tree<shgRootNode>" after it draws listbox items.
   void drawAsListboxItem(Tk_Window, int theDrawable,
			  int boxLeft, int boxTop,
			  int boxWidth, int boxHeight,
			  int textLeft, int textBaseline) const;
   int pointWithinAsRoot(int x, int y, int root_centerx, int root_topy) const;
      // return values:
      // 1 -- yes
      // 2 -- no, point is above the root
      // 3 -- no, point is below root
      // 4 -- no, point is to the left of root
      // 5 -- no, point is to the right of root
};

#endif
