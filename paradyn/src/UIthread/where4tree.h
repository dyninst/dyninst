// where4tree.h
// Ariel Tamches

// Header file for subtree based on where4.fig [and where5.fig]

/* $Log: where4tree.h,v $
/* Revision 1.7  1995/09/20 01:24:11  tamches
/* Major cleanification; too many things to enumerate.  no path items
/* have negative values.  No more uses of graphical paths.
/*
 * Revision 1.6  1995/08/07  00:02:16  tamches
 * added selectUnSelectFromFullPathName
 *
 * Revision 1.5  1995/08/04  19:18:00  tamches
 * Added numChildrenAddedSinceLastSort field to every node.
 * Changes needed for using Vector::sort()
 *
 * Revision 1.4  1995/07/27  23:27:45  tamches
 * Crash upon sorting huge CMF application mysteriously
 * goes away when quicksort is altered slightly to remove
 * tail recursion.
 *
 * Revision 1.3  1995/07/24  21:34:58  tamches
 * Added sorting.
 * Removed member function addChildren()
 *
 * Revision 1.2  1995/07/18  03:41:20  tamches
 * Added ctrl-double-click feature for selecting/unselecting an entire
 * subtree (nonrecursive).  Added a "clear all selections" option.
 * Selecting the root node now selects the entire program.
 *
 * Revision 1.1  1995/07/17  04:59:01  tamches
 * First version of the new where axis
 *
 */


// Note: there are stray virtual functions and talk of inheritence...but I'm
// not sure that's how things will evolve.

#ifndef _WHERE4TREE_H_
#define _WHERE4TREE_H_

extern "C" {
   #include <X11/Xlib.h>
   #include <tcl.h>
   #include <tk.h>

   #include <stdlib.h>
}

#ifndef PARADYN
// The test program already has the correct -I paths set
#include "Vector.h"
#include "String.h"
#else
#include "util/h/Vector.h"
#include "util/h/String.h"
#endif

#include "simpSeq.h"

#include "where4treeConstants.h"
#include "whereAxisMisc.h"

#include "rootNode.h"
#include "scrollbar.h"

/* ********************************************************************* */

typedef simpSeq<unsigned> whereNodePosRawPath;


template <class USERNODEDATA>
class where4tree {
 private:
   rootNode theRootNode;
   USERNODEDATA theUserNodeData;
   unsigned numChildrenAddedSinceLastSort;
      // if << children.size(), then resort using selection sort; else, quicksort.
      // If 0, no sorting is needed.

   struct childstruct {
      where4tree *theTree; // a ptr since we may use inheritence
      bool isExplicitlyExpanded;
      int  nameTextWidthIfInListbox; // caches away an XTextWidth() call [lb width],
         // easing the pain of rethink_listbox_dimensions()

      static int cmpfunc(const void *ptr1, const void *ptr2) {
         // for passing to Vector::sort()
         const childstruct &child1 = *(const childstruct *)ptr1;
         const childstruct &child2 = *(const childstruct *)ptr2;

         if (child1 < child2)
            return -1;
         else if (child1 > child2)
            return 1;
         else
            return 0;
      }

      bool operator<(const childstruct &other) const {
         return theTree->getRootName() < other.theTree->getRootName();
      }
      bool operator>(const childstruct &other) const {
         return theTree->getRootName() > other.theTree->getRootName();
      }
   };

   vector<childstruct> theChildren;
      // Why a vector instead of a set?  Because the children must be ordered.
      // Why do we group these into a childstruct?  To make memory allocation faster.

// Sorry, "not yet implemented" in g++ 2.6.3: static members of a template type
//   static int masterlistboxBorderPix;
//   static int masterlistboxScrollBarWidth;

   int listboxPixWidth; // includes scrollbar, if applicable
   int listboxFullPixHeight; // total lb height (if no SB used)
   int listboxActualPixHeight; // we limit ourselves to a "window" this tall
 
   scrollbar theScrollbar;

   int allExpandedChildrenWidthAsDrawn, allExpandedChildrenHeightAsDrawn;
      // Both implicitly _and_ explicitly expanded children are included.
      // listbox (if any) is _not_ included.
      // If no children are expanded, then these are 0.
      // Non-leaf children can have their own listboxes and subtrees--all this
      // is counted.

 private:

   bool expandUnexpand1(const where4TreeConstants &tc, int childindex, bool expandFlag,
			bool rethinkFlag);

   // Mouse clicks and node expansion
   int point2ItemWithinListbox(const where4TreeConstants &tc,
			       int localx, int localy) const;
      // returns index of item clicked on (-2 if nothing), given point
      // local to the _data_ part of listbox.  0<=localy<listboxFullPixHeight

   int point2ItemOneStepScrollbar(int ypix,
				  int scrollbarTop,
				  int scrollbarHeight) const;
      // -3 for a point in listbox scrollbar up-arrow,
      // -4 for point in listbox scrollbar down-arrow,
      // -5 for point in listbox scrollbar pageup-region,
      // -6 for point in listbox scrollbar pagedown-region,
      // -7 for point in listbox scrollbar slider.

 public:
   int point2ItemOneStep(const where4TreeConstants &tc,
			 int xpix, int ypix,
			 int root_centerx, int root_topy) const;
      // returns -1 for a point in root, 0 thru n-1 for point w/in general
      // range of a child subtree [even if child is in a listbox],
      // -2 for a point on nothing,
      // -3 for a point in listbox scrollbar up-arrow,
      // -4 for point in listbox scrollbar down-arrow,
      // -5 for point in listbox scrollbar pageup-region,
      // -6 for point in listbox scrollbar pagedown-region,
      // -7 for point in listbox scrollbar slider.

 private:
   const where4tree *get_end_of_path0(const whereNodePosRawPath &thePath,
                                      unsigned index) const {
      if (index < thePath.getSize())
         return getChildTree(thePath[index])->get_end_of_path0(thePath, index+1);
      else
         return this;
   }

   where4tree *get_end_of_path0(const whereNodePosRawPath &thePath, unsigned index) {
      if (index < thePath.getSize())
         return getChildTree(thePath[index])->get_end_of_path0(thePath, index+1);
      else
         return this;
   }

   const where4tree *get_parent_of_end_of_path0(const whereNodePosRawPath &thePath,
                                                unsigned index) const {
      if (index < thePath.getSize()-1)
         return getChildTree(thePath[index])->get_parent_of_end_of_path0(thePath, index+1);
      else
         return this;
   }

   where4tree *get_parent_of_end_of_path0(const whereNodePosRawPath &thePath,
                                          unsigned index) {
      if (index < thePath.getSize()-1)
         return getChildTree(thePath[index])->get_parent_of_end_of_path0(thePath, index+1);
      else
         return this;
   }

 public:

   const where4tree *get_end_of_path(const whereNodePosRawPath &thePath) const {
      return get_end_of_path0(thePath, 0);
   }

   where4tree *get_end_of_path(const whereNodePosRawPath &thePath) {
      return get_end_of_path0(thePath, 0);
   }

   const where4tree *get_parent_of_end_of_path(const whereNodePosRawPath &thePath) const {
      return get_parent_of_end_of_path0(thePath, 0);
   }

   where4tree *get_parent_of_end_of_path(const whereNodePosRawPath &thePath) {
      return get_parent_of_end_of_path0(thePath, 0);
   }

   unsigned getNumChildren() const {return theChildren.size();}
   where4tree *getChildTree(unsigned index) {
      return theChildren[index].theTree;
   }
   const where4tree *getChildTree(unsigned index) const {
      return theChildren[index].theTree;
   }
   bool getChildIsExpandedFlag(unsigned index) const {
      return theChildren[index].isExplicitlyExpanded;
   }

   bool existsANonExplicitlyExpandedChild() const {
      // assert me when a listbox is up
      const unsigned numChildren = theChildren.size();
      for (unsigned childlcv = 0; childlcv < numChildren; childlcv++)
         if (!theChildren[childlcv].isExplicitlyExpanded)
           return true;
      return false;
   }

   int getListboxPixWidth() const {return listboxPixWidth;}
   int getListboxFullPixHeight() const {return listboxFullPixHeight;}
   int getListboxActualPixHeight() const {return listboxActualPixHeight;}
   scrollbar &getScrollbar() {return theScrollbar;}
   const scrollbar &getScrollbar() const {return theScrollbar;}

   void removeListbox();

   int scroll_listbox(int deltaYpix);
      // returns true scroll amt.  Doesn't redraw

   bool scroll_listbox(const where4TreeConstants &tc,
		       int listboxLeft,
		       int listboxTop,
		       int deltaYpix);
      // returns true iff any changes were made.  This version redraws.

   void rethink_listbox_dimensions(const where4TreeConstants &tc);
      // an expensive routine; time = O(numchildren).  Calculating just the
      // listbox height could be quicker.

   static void drawTriangle(const where4TreeConstants &tc, Drawable theDrawable,
		            int triangleEndX, int currBaseLine);
      // cost is O(XFillPolygon())

   void draw_listbox(const where4TreeConstants &tc, Drawable theDrawable,
		     int left, int top,
		     int datapart_relative_starty,
		     int datapart_relative_height) const;

   // Children Pixel Calculations (excl. those in a listbox)
   int horiz_offset_to_expanded_child(const where4TreeConstants &,
                                      unsigned childIndex) const;
      // Returns the horiz pix offset (from left side of leftmost item drawn
      // below root, which is usually the listbox) of a certain expanded
      // (implicitly or explicitly) child.
      // cost is O(childindex)

   void rethink_all_expanded_children_dimensions(const where4TreeConstants &tc) {
      allExpandedChildrenWidthAsDrawn =wouldbe_all_expanded_children_width(tc);
      allExpandedChildrenHeightAsDrawn=wouldbe_all_expanded_children_height(tc);
   }

   // The following 2 routines are expensive; call only to rethink
   // cache variables "allExpandedChildrenWidth(Height)AsDrawn".
   // Each assumes all descendants have updated layout variables.
   // Costs are O(numchildren)
   int wouldbe_all_expanded_children_width(const where4TreeConstants &tc) const;
   int wouldbe_all_expanded_children_height(const where4TreeConstants &tc) const;

   // The following routines are O(1):
   int horiz_pix_everything_below_root(const where4TreeConstants &tc) const;
   int vert_pix_everything_below_root() const;
   int entire_width (const where4TreeConstants &tc) const;
   int entire_height(const where4TreeConstants &tc) const;

   bool expandEntirePath(const where4TreeConstants &tc,
                         const whereNodePosRawPath &thePath,
                         unsigned pathIndex);
      // Given a path, forcibly expand out any and all listbox path elements.
      // Exception: won't expand a listbox item at the very end of the path.
      // Returns true iff any expansion(s) actually took place.

   bool explicitlyExpandSubchild(const where4TreeConstants &tc, unsigned childindex);
      // Expand a subtree out of a listbox.  Doesn't redraw.
      // Returns true iff any changes were made.
   bool explicitlyUnexpandSubchild(const where4TreeConstants &tc, unsigned childindex);
      // Unexpand a subchild (into a listbox).  Doesn't redraw.
      // Returns true iff any changes were made.
   
   bool expandAllChildren(const where4TreeConstants &tc);
   bool unExpandAllChildren(const where4TreeConstants &tc);

 private:   
   void manual_construct();

 public:

   where4tree(const USERNODEDATA &iUserNodeData,
              const string &init_str, const where4TreeConstants &tc) :
                 theRootNode(init_str, tc, false),
                 theUserNodeData(iUserNodeData) {
      manual_construct();
   }
   virtual ~where4tree();

   const USERNODEDATA &getUserNodeData() const { return theUserNodeData; }
   USERNODEDATA &getUserNodeData() { return theUserNodeData; }
   const string &getRootName() const { return theRootNode.getName(); }
   const rootNode &getRootNode() const { return theRootNode; }

   // Adding children:
   void addChild(where4tree *theNewChild,
		 bool explicitlyExpanded,
		 const where4TreeConstants &tc,
		 bool rethinkLayoutNow, bool resortNow);
      // add a child subtree **that has been allocated with new** (not negotiable)
      // NOTE: Current implementation puts child into listbox unless
      //       explicitlyExpanded.

   void doneAddingChildren(const where4TreeConstants &tc);
   void recursiveDoneAddingChildren(const where4TreeConstants &tc);
      // Needed after having called addChild() several times for a given root and
      // had passed false as the last parameter...

   void sortChildren();
      // does not redraw

   void draw(const where4TreeConstants &tc, Drawable theDrawable,
	     int middlex, int topy, bool rootOnly, bool listboxOnly) const;
      // Draws root node AND the lines connecting the subtrees; children are
      // drawn via recursion.

   // Resize:
   bool rethinkListboxAfterResize1(const where4TreeConstants &tc);
   bool rethinkListboxAfterResize(const where4TreeConstants &tc);
   
   void rethinkAfterResize(const where4TreeConstants &tc,
			   int horizSpaceToWorkWithThisSubtree);
      // Assuming a resize (change of tc.availableWidth, tc.availableHeight),
      // rethink whether or not there should be a listbox, and possibly
      // make internal changes.  Explicitly expanded items will stay expanded;
      // implicitly expanded items _may_ become un-expanded.

   int string2Path(whereNodePosRawPath &thePath, const where4TreeConstants &tc,
                   const string &str, where4tree *beginSearchFromPtr,
		   bool testRootNode);
      // If not found, thePath is left undefined, and 0 is returned.
      // Otherwise, modifies "thePath" and:
      //    -- returns 1 if no expansions are necessary (can scroll)
      //    -- returns 2 if expansions are necessary before scrolling

   // Subtree expansion/un-expansion
   int path2lbItemExpand(const where4TreeConstants &tc, 
			 const whereNodePosRawPath &thePath, unsigned pathIndex);
      // Given a path (ending in a listbox item), expand it.
      // returns 0 if you tried to expand an already-expanded item,
      // returns 1 if you tried to expand a leaf listbox item,
      // returns 2 normally; plus, does the following:
      // Updates thePath (if an expansion took place); the last item
      // is changed from a listbox item to a non-listbox one.  This adds 1
      // entry to the path.  The xpix/ypix parts of the path will be
      // undefined; we're only concerned with returning a correct sequence of 
      // childnums.  Why don't we update xpix/ypix?  Because, after an expansion,
      // the entire path's xpix/ypix becomes invalid due to changes in the
      // entire tree.  You can still use routines like
      // path2pixOffset, which ignore the path's xpix/ypix values.

   bool path2ExpandAllChildren(const where4TreeConstants &tc,
			       const whereNodePosRawPath &thePath,
			       unsigned index);

   bool path2lbItemUnexpand(const where4TreeConstants &tc,
			    const whereNodePosRawPath &thePath, unsigned pathIndex);
      // "thePath" ends in a non-listbox item; i.e., the item at the end of the
      // path is expanded.  This routine un-expands it, creating the listbox if needed.

   bool path2UnExpandAllChildren(const where4TreeConstants &tc,
				 const whereNodePosRawPath &thePath, unsigned index);

   bool rigListboxScrollbarSliderTopPix(const where4TreeConstants &tc,
					int scrollBarLeft,
					int scrollBarTop,
					int scrollBarBottom,
					int newScrollBarSliderTopPix,
					bool redrawNow);
      // returns true iff any changes were made

   // Highlighting

   bool isHighlighted() const {
      return theRootNode.getHighlighted();
   }
   void highlight() {
      // does not redraw
      theRootNode.highlight();
   }
   void unhighlight() {
      // does not redraw
      theRootNode.unhighlight();
   }
   void toggle_highlight() {
      theRootNode.toggle_highlight();
   }

//   void toggle_highlight(const where4TreeConstants &tc,
//			 int middlex, int topy);
//      // same as above, but also redraws now

   bool selectUnSelectFromFullPathName(const char *name, bool selectFlag);
      // returns true iff found.  char * is used instead of string because
      // we'll be using pointer arithmetic as we parse "name".

   vector<USERNODEDATA> getSelections() const;
   void recursiveClearSelections();
};

#endif
