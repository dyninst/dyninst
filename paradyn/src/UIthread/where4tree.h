// where4tree.h
// Ariel Tamches

// Header file for subtree based on where4.fig [and where5.fig]

/* $Log: where4tree.h,v $
/* Revision 1.3  1995/07/24 21:34:58  tamches
/* Added sorting.
/* Removed member function addChildren()
/*
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
#include "rootNode.h"
#include "scrollbar.h"

/* ********************************************************************* */

struct whereNodeGraphicalPathItem {
   int root_centerxpix, root_topypix;
   int childnum;
      // children.size() refers to end-of-path (stopping on a non-listbox-item)
      // 0 thru children.size()-1 refers to a child (not end of path)
      // < 0 refers to end-of-path (stopping on a listbox item whose index
      //            equals the absolute value of this number - 1)

   whereNodeGraphicalPathItem() {}
   whereNodeGraphicalPathItem(const int cn) {
      childnum = cn; root_centerxpix=-1; root_topypix=-1;
   }
   whereNodeGraphicalPathItem(const int x, const int y, const int cn) {
      root_centerxpix = x;
      root_topypix = y;
      childnum = cn;
   }
   whereNodeGraphicalPathItem(const whereNodeGraphicalPathItem &src) {
      this->root_centerxpix = src.root_centerxpix;
      this->root_topypix    = src.root_topypix;
      this->childnum        = src.childnum;
   }
   whereNodeGraphicalPathItem &operator=(const whereNodeGraphicalPathItem &src) {
      this->root_centerxpix = src.root_centerxpix;
      this->root_topypix    = src.root_topypix;
      this->childnum  = src.childnum;
      return *this;
   }
};

struct whereNodeRawPathItem {
   int childnum;
   whereNodeRawPathItem() {}
   whereNodeRawPathItem(const int cn) {childnum=cn;}
   whereNodeRawPathItem(const whereNodeRawPathItem &src) {childnum=src.childnum;}
   whereNodeRawPathItem &operator=(const whereNodeRawPathItem &src) {
      childnum = src.childnum;
      return *this;
   }

   operator int() const {return childnum;}
};

typedef simpSeq<whereNodeGraphicalPathItem> whereNodeGraphicalPath;
typedef simpSeq<whereNodeRawPathItem> whereNodeRawPath;


#include "whereAxis.h"
template <class USERNODEDATA>
class where4tree {
 friend class whereAxis<USERNODEDATA>;
 private:
   rootNode theRootNode;
   USERNODEDATA theUserNodeData;

   struct childstruct {
      where4tree *theTree;
      bool isExplicitlyExpanded;
      int  nameTextWidthIfInListbox; // caches away an XTextWidth() call [lb width],
         // easing the pain of rethink_listbox_dimensions()

      bool operator<(const childstruct &other) {
         return theTree->getRootName() < other.theTree->getRootName();
      }
      bool operator>(const childstruct &other) {
         return theTree->getRootName() > other.theTree->getRootName();
      }
   };

   vector<childstruct> theChildren;
      // Why a vector instead of a set?  Because the children must be ordered.
      // Why is "theTree" a pointer?  Because we may use inheritance
      // isExplicitlyExpanded is true iff the subtree has been _explicitly_
      //    expanded at least one level.  Other subtrees (all other or none other)
      //    will be implicitly (automatically) expanded if there is enough screen
      //    real estate for them all.
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

   // Mouse clicks and node expansion
   int point2ItemWithinListbox(const where4TreeConstants &tc,
			       int localx, int localy) const;
      // returns index of item clicked on (-2 if nothing), given point
      // local to the _data_ part of listbox.  0<=localy<=listboxFullPixHeight

   int point2ItemOneStepScrollbar(const where4TreeConstants &tc,
				  const int ypix,
				  const int scrollbarTop,
				  const int scrollbarHeight) const;
      // -3 for a point in listbox scrollbar up-arrow,
      // -4 for point in listbox scrollbar down-arrow,
      // -5 for point in listbox scrollbar pageup-region,
      // -6 for point in listbox scrollbar pagedown-region,
      // -7 for point in listbox scrollbar slider.

   int point2ItemOneStep(const where4TreeConstants &tc,
			 const int xpix, const int ypix,
			 const int root_centerx, const int root_topy) const;
      // returns -1 for a point in root, 0 thru n-1 for point w/in general
      // range of a child subtree [even if child is in a listbox],
      // -2 for a point on nothing,
      // -3 for a point in listbox scrollbar up-arrow,
      // -4 for point in listbox scrollbar down-arrow,
      // -5 for point in listbox scrollbar pageup-region,
      // -6 for point in listbox scrollbar pagedown-region,
      // -7 for point in listbox scrollbar slider.

 public: // these things had to be public to whereAxis.h could access 'em.

   void makeGraphicalPathFromRawPath(const where4TreeConstants &tc,
                                     int index,
                                     whereNodeGraphicalPath &destPath,
                                     whereNodeRawPath &srcPath,
                                     int rootNodeCenterX, int rootNodeTopY);
   void noNegativeChildNums(whereNodeRawPath &thePath, int index) {
      if (index == thePath.getSize()-1) {
         if (thePath[index].childnum < 0) {
            thePath[index].childnum = -thePath[index].childnum - 1;
            thePath.append(theChildren[thePath[index].childnum].theTree->theChildren.size());
         }
      }
      else {
         if (thePath[index].childnum < 0)
            thePath[index].childnum = -thePath[index].childnum - 1;

         theChildren[thePath[index].childnum].theTree->
                                              noNegativeChildNums(thePath, index+1);
      }
   }

   void removeListbox();

   bool existsANonExplicitlyExpandedChild() const {
      // assert me when a listbox is up
      const int numChildren = theChildren.size();
      for (int childlcv = 0; childlcv < numChildren; childlcv++)
         if (!theChildren[childlcv].isExplicitlyExpanded)
           return true;
      return false;
   }

   // listbox:
   void rethink_listbox_dimensions(const where4TreeConstants &tc);
      // an expensive routine; time = O(numchildren).  Calculating just the
      // listbox height could be quicker.

   void drawTriangle(const where4TreeConstants &tc,
		     int theDrawable,
		     const int triangleEndX, const int currBaseLine) const;
      // cost is O(XFillPolygon())

   void draw_listbox(const where4TreeConstants &tc, int theDrawable,
                            const int left, const int top,
			    const int datapart_relative_starty,
			    const int datapart_relative_height
			    ) const;
      // crude clipping at no extra charge

   // Children Pixel Calculations (excl. those in a listbox)
   int horiz_offset_to_expanded_child(const where4TreeConstants &,
                                      const int childIndex) const;
      // Returns the horiz pix offset (from left point of leftmost item drawn
      // below root, which is usually the listbox) of a certain
      // expanded (implicitly or explicitly) child.
      // Simply adds up listbox width plus all the expanded children widths
      // from 0 to childIndex-1 (plus padding where appropriate).
      // cost is O(childindex)

   void rethink_all_expanded_children_dimensions(const where4TreeConstants &tc) {
      allExpandedChildrenWidthAsDrawn =wouldbe_all_expanded_children_width(tc);
      allExpandedChildrenHeightAsDrawn=wouldbe_all_expanded_children_height(tc);
   }

   int wouldbe_all_expanded_children_width(const where4TreeConstants &tc) const;
      // expensive; call only to rethink "allExpandedChildrenWidthAsDrawn".
      // MAJOR ASSUMPTION: All descendants have up-to-date values for sizes of
      // everything, and this node has up-to-date listbox sizes
      // (so we can know whether or not one is up).
      // cost is O(numchildren)

   int wouldbe_all_expanded_children_height(const where4TreeConstants &tc) const;
      // expensive; call only to rethink "allExpandedChildrenHeightAsDrawn".
      // MAJOR ASSUMPTION: all descendants have up-to-date values for sizes of
      // everything, and this node has up-to-date listbox sizes
      // (so we can know whether or not one is up).
      // cost is O(numchildren)

   int horiz_pix_everything_below_root(const where4TreeConstants &tc) const;
      // cost is O(1)
   int vert_pix_everything_below_root() const;
      // cost is O(1)

   // Entire Tree Calculations:
   int myEntireWidthAsDrawn(const where4TreeConstants &tc) const;
      // cost is O(1)
   int myEntireHeightAsDrawn(const where4TreeConstants &tc) const;
      // cost is O(1)

   bool expandEntirePath(const where4TreeConstants &tc,
                         const whereNodeRawPath &thePath,
                         const int pathIndex);
      // Given a path with no negative childnum's [i.e., one in which we
      // determine manually whether or not an element is w/in a listbox],
      // forcibly expand out any and all listbox path elements.
      // Exception: won't expand a listbox item at the very end of the path.
      // Returns true iff any expansion(s) actually took place.

   bool explicitlyExpandSubchild1Level(const where4TreeConstants &tc,
			  	       const int childindex);
      // Expand a subtree out of a listbox, and update internal structures.  Doesn't
      // redraw.  Implements explicit expansion (via mouse-double-click within
      // the listbox).  Returns true iff any changes were made.
   
   bool explicitlyUnexpandSubchild(const where4TreeConstants &tc,
				   const int childindex);
      // Unexpand a subchild (into a listbox), and update internal structures; but,
      // don't redraw.  Returns true iff any changes were made.

   bool expandAllChildren(const where4TreeConstants &tc);
   bool unExpandAllChildren(const where4TreeConstants &tc);
   
   void manual_construct(const where4TreeConstants &tc);
      // the constructor

   int partitionChildren(int left, int right); // for quicksort

 public:

   where4tree(const USERNODEDATA &iUserNodeData,
              const string &init_str, const where4TreeConstants &tc) :
                 theRootNode(init_str, tc, false),
                 theUserNodeData(iUserNodeData) {
      manual_construct(tc);
   }
   virtual ~where4tree();

   const string &getRootName() const { return theRootNode.getName(); }

   // Adding children
   void addChild(where4tree *theNewChild,
		 const bool explicitlyExpanded,
		 const where4TreeConstants &tc,
		 const bool rethinkGraphicsNow,
                 const bool resortNow);
      // add a child subtree **that has been allocated with new** (not negotiable)
      // NOTE: In the current implementation, we always put the child into the listbox
      //       unless explicitlyExpanded is true.

   void doneAddingChildren(const where4TreeConstants &tc);
   void recursiveDoneAddingChildren(const where4TreeConstants &tc);
      // Needed after having called addChild() several times for a given root and
      // had passed false as the last parameter...(Of course, calling addChildren()
      // just once would have been even better, but it's not always convenient to use.)

   void sortChildren(unsigned left, unsigned right);
   void sortChildren();
      // does no redrawing.

   void draw(const where4TreeConstants &tc,
	     Drawable theDrawable,
	     const int middlex, const int topy,
	     const bool rootOnly, const bool listboxOnly) const;
      // Note: draw() for a given subree is responsible for drawing the root
      //       node AND the lines connecting the subtrees; it indirectly draws
      //       the child subtrees via recursion.

   // Resize:
   bool rethinkListboxAfterResize1(const where4TreeConstants &tc);
   bool rethinkListboxAfterResize(const where4TreeConstants &tc);
   
   void rethinkAfterResize(const where4TreeConstants &tc,
			   int horizSpaceToWorkWithThisSubtree);
      // Assuming a resize (change of tc.availableWidth, tc.availableHeight),
      // rethink whether or not there should be a listbox, and possibly
      // make internal changes.  Explicitly expanded items will stay expanded;
      // implicitly expanded items _may_ become un-expanded.

   int point2GraphicalPath(whereNodeGraphicalPath &thePath,
			   const where4TreeConstants &tc,
			   const int xpix, const int yix,
			   const int root_centerxpix, const int root_topypix);
      // Returns 1 for point in an expanded node, 2 for point in listbox data item,
      // 3 for point in listbox scrollbar up-arrow,
      // 4 for point in listbox scrollbar down-arrow,
      // 5 for point in listbox scrollbar pageup-region,
      // 6 for point in listbox scrollbar pagedown-region,
      // 7 for point in listbox scrollbar slider,
      // -1 for point in nothing.
      // The actual path is "returned" by modifying "thePath".
      // Uses point2ItemOneStep() for its main work...

   int string2Path(whereNodeRawPath &thePath, const where4TreeConstants &tc,
                   const string &str, where4tree *beginSearchFromPtr,
		   bool testRootNode);
      // returns 0 if not found.  returns 1 (and modifies "thePath") iff found,
      // and no expansions are necessary (we can scroll directly there); returns
      // 2 (and modifies "thePath") iff found but expansions of some intermediate
      // path items would be needed.
      // NOTE: The path returned will not contain ANY listbox childnums, except perhaps
      //       at the end.  You must check for this manually.

   void path2pixOffset(const where4TreeConstants &tc,
		       const whereNodeRawPath &thePath, const int currPathIndex,
		       const int root_centerx, const int root_topy,
                       int &result_offset_centerx, int &result_offset_topy) const;
      // Finds the [centerx, topy] pixels (relative to leftx, topy of the root) of
      // the item at the end of the path.
      // WARNING: All elements along the path must be expanded.
      // WARNING: For a listbox item, we simply return the (centerx, topy) of the
      //          listbox itself, which is a mediocre solution.  (We also want the
      //          vertical offset w/in the listbox)

   // Subtree expansion/un-expansion
   int path2lbItemExpand(const where4TreeConstants &tc,
			 whereNodeRawPath &thePath,
			 const int pathIndex);
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
      // path2pixOffset, which specifically ignore the path's xpix/ypix values.

   bool path2lbItemScrollSoVisible(const where4TreeConstants &tc,
				   const whereNodeGraphicalPath &thePath,
				   const int pathIndex);
      // Assuming thePath ends in a listbox item, scroll within the listbox
      // so that the item at the end of the path is visible.
      // returns true iff any changes were made

   bool path2ExpandAllChildren(const where4TreeConstants &tc,
			       const whereNodeRawPath &thePath,
			       const int index);

   bool path2lbItemUnexpand(const where4TreeConstants &tc,
			    const whereNodeRawPath &thePath,
			    const int pathIndex);
      // Given a path (ending in a non-listbox item), un-expand it into a listbox.
      // (Creating the listbox if necessary)

   bool path2UnExpandAllChildren(const where4TreeConstants &tc,
				 const whereNodeRawPath &thePath,
				 const int index);

   // Scrollbar clicks & scrolling:
   bool scroll_listbox(const where4TreeConstants &tc,
		       const int listboxLeft,
		       const int listboxTop,
		       const int deltaYpix,
		       const bool redrawNow);

   void scrollBarClick(const where4TreeConstants &tc,
		       const int clicklocation,
		          // 3 --> up arrow
		          // 4 --> down arrow
		          // 5 --> pageup   region
		          // 6 --> pagedown region
		       const int root_centerx,
		       const int root_topy,
		       const bool redrawNow);

   void path2lbScrollBarClick(const where4TreeConstants &tc,
			      const whereNodeGraphicalPath &thePath,
			      const int pathlcv,
			      const int clickLocation,
			         // 3 --> up arrow
			         // 4 --> down arrow
			         // 5 --> pageup   region
			         // 6 --> pagedown region
			      const bool redrawNow);
   int  path2lbScrollBarSliderTopPix(const where4TreeConstants &tc,
				     const whereNodeGraphicalPath &thePath,
				     const int pathlcv);
   bool rigListboxScrollbarSliderTopPix(const where4TreeConstants &tc,
					const int scrollBarLeft,
					const int scrollBarTop,
					const int scrollBarBottom,
					const int newScrollBarSliderTopPix,
					const bool redrawNow
					);
      // returns true iff any changes were made

   // Highlighting
   void toggleHighlightFromPath(const where4TreeConstants &tc,
				const whereNodeGraphicalPath &thePath,
				const bool redrawNow);
      // "thePath" was presumably initialized with a call to point2NodePath

   void toggle_highlight_root_only(const bool redrawNow,
				   const where4TreeConstants &tc,
				   const int middlex, const int topy);
      // when called, the next redraw will (presumably) look different (although
      // the derived class could be written to ignore highlightedness, I suppose).

   bool isHighlighted() const {
      return theRootNode.getHighlighted();
   }
   void highlight() {
      theRootNode.highlight();
   }
   void unhighlight() {
      theRootNode.unhighlight();
   }

   vector<USERNODEDATA> getSelections() const;
   void recursiveClearSelections();
};

#endif
