// rootNode.h
// Ariel Tamches
// C++ class for the root node of subtrees declared in where4tree.h
// Basically, this file exists just to make where4tree.h that much shorter.

/* $Log: rootNode.h,v $
/* Revision 1.6  1996/03/08 00:21:05  tamches
/* added anything2draw()
/*
 * Revision 1.5  1996/02/15 23:09:03  tamches
 * added getGCforListboxRay and getGCforNonListboxRay (to better support
 * why vs. where refinement in shg)
 *
 * Revision 1.4  1995/10/17 20:56:44  tamches
 * Changed class name from "rootNode" to "whereAxisRootNode".
 * More versatile -- now holds pixWidthAsRoot and pixWidthAsListboxItem.
 * Added static members prepareForDrawingListboxItems and
 * doneDrawingListboxItems.  Added drawAsRoot() and drawAsListboxItem().
 *
 * Revision 1.3  1995/09/20 01:18:03  tamches
 * minor cleanifications hardly worth mentioning
 *
 * Revision 1.2  1995/07/18  03:41:18  tamches
 * Added ctrl-double-click feature for selecting/unselecting an entire
 * subtree (nonrecursive).  Added a "clear all selections" option.
 * Selecting the root node now selects the entire program.
 *
 * Revision 1.1  1995/07/17  04:58:56  tamches
 * First version of the new where axis
 *
 */

#ifndef _ROOTNODE_H_
#define _ROOTNODE_H_

#include "tcl.h"
#include "tk.h"

#ifndef PARADYN
// the where axis test program has the proper -I settings
#include "String.h"
#include "DMinclude.h" // resourceHandle
#else
#include "util/h/String.h"
#include "paradyn/src/DMthread/DMinclude.h" // resourceHandle
#endif

// This class is intended to be used as the template type of where4tree<>
// for the where axis.  For the search history graph, try some other type.
// In particular, this class contains a resourceHandle, while is not applicable
// in the searc history graph.

class whereAxisRootNode {
 private:
   resourceHandle uniqueId;

   string name; // name of the root of this subtree
   bool highlighted;

   int pixWidthAsRoot, pixHeightAsRoot;
   int pixWidthAsListboxItem;

   // note: we use the following static members of
   //       our "parent" class (whereAxis):
   //       getRootItemFontStruct(), getListboxItemFontStruct(),
   //       getRootItemTk3DBorder(), getRootItemTextGC()

   static int borderPix; // both horiz & vertical [Tk_Fill3DRectangle forces these
                         // to be the same]
   static int horizPad, vertPad;

 public:

   whereAxisRootNode(resourceHandle uniqueId, const string &init_str);
   whereAxisRootNode(const whereAxisRootNode &src)  : name(src.name) {
      uniqueId = src.uniqueId;
      highlighted = src.highlighted;
      pixWidthAsRoot = src.pixWidthAsRoot;
      pixHeightAsRoot = src.pixHeightAsRoot;
      pixWidthAsListboxItem = src.pixWidthAsListboxItem;
   }
  ~whereAxisRootNode() {}

   bool anything2draw() const {return true;}
      // where axis items are never hidden

   bool operator<(const whereAxisRootNode &other) {return name < other.name;}
   bool operator>(const whereAxisRootNode &other) {return name > other.name;}

   resourceHandle getUniqueId() const {return uniqueId;}
   const string &getName() const {return name;}

   int getWidthAsRoot()  const {return pixWidthAsRoot;}
   int getHeightAsRoot() const {return pixHeightAsRoot;}
   int getWidthAsListboxItem() const {return pixWidthAsListboxItem;}
   
   bool getHighlighted() const {return highlighted;}

   static void prepareForDrawingListboxItems(Tk_Window, XRectangle &);
   static void doneDrawingListboxItems(Tk_Window);
   void drawAsRoot(Tk_Window theTkWindow,
		   int theDrawable, // could be offscren pixmap
		   int root_middlex, int topy) const;

   static GC getGCforListboxRay(const whereAxisRootNode &parent,
				const whereAxisRootNode &firstChild);
      // return GC to be used in an XDrawLine call from "parent" down to the
      // listbox of its children; "firstChild" is the node data for the first
      // such child.
   static GC getGCforNonListboxRay(const whereAxisRootNode &parent,
				   const whereAxisRootNode &child);
      // assuming that "parent" is an expanded (explicitly or not) node, return the GC
      // to be used in an XDrawLine call from it down to "child".
   
   void drawAsListboxItem(Tk_Window theTkWindow,
			  int theDrawable, // could be offscreen pixmap
			  int boxLeft, int boxTop,
			  int boxWidth, int boxHeight,
			  int textLeft, int textBaseline) const;

   // Mouse clicks and node expansion
   int pointWithinAsRoot(int xpix, int ypix,
			 int root_centerx, int root_topy) const;
      // return values:
      // 1 -- yes
      // 2 -- no, point is north of root (or northwest or northeast)
      // 3 -- no, point is south of root (or southwest or southeast)
      // 4 -- no, point is west of root (but not north or south of root)
      // 5 -- no, point is east of root (but not north or south or root)

   // The following 3 routines don't redraw:
   void highlight() {highlighted=true;}
   void unhighlight() {highlighted=false;}
   void toggle_highlight() {highlighted = !highlighted;}
};

#endif
