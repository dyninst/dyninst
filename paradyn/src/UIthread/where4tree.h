/*
 * Copyright (c) 1996 Barton P. Miller
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

// where4tree.h
// Ariel Tamches

// Header file for subtree based on where4.fig [and where5.fig]

/* $Id: where4tree.h,v 1.16 2001/02/12 14:53:06 wxd Exp $ */

// This class is sort of a placeholder.  It has variables to find out who
// is expanded and who isn't; it maintains the tree layout.
// But many details are left to the template <USERNODE> class, which
// is whereAxisRootNode for the where axis.  Such a class actually holds
// the node name, draws the node (both root and within-listbox), etc.
// In particular, this template class can maintain extra information
// allowing it to draw specially.  For example, the upcoming search-history-graph
// root node will maintain a state (instrumented, false, true, not instrumented)
// needed to do its drawing.

#ifndef _WHERE4TREE_H_
#define _WHERE4TREE_H_

#include <stdlib.h>

#ifndef PARADYN
// The test program already has the correct -I paths set
#include "Vector.h"
#include "String.h"
#else
#include "common/h/Vector.h"
#include "common/h/String.h"
#endif

#include "simpSeq.h"

#include "where4treeConstants.h"

#include "scrollbar.h"

/* ********************************************************************* */

typedef simpSeq<unsigned> whereNodePosRawPath;


template <class NODEDATA>
class where4tree {
 private:
   NODEDATA theNodeData;
      // must have several particular routines defined:
      // getName(), getPixWidthAsListboxItem(), getPixHeightAsListboxItem() [constant],
      // draw_as_listbox_item(), getPixHeightAsRoot(), drawAsRoot(), more...

   bool anything2Draw;
      // if false, drawing & measurement routines will ignore the entire
      // subtree rooted here.  This value is updated when outside code calls
      // updateAnything2Draw(), which in turn checks the value of
      // NODEDATA::anything2Draw() for each node.

   unsigned numChildrenAddedSinceLastSort;
      // if << children.size(), then resort using selection sort; else, quicksort.
      // If 0, no sorting is needed.

   struct childstruct {
      where4tree *theTree; // a ptr since we may use inheritence
      bool isExplicitlyExpanded;

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
         // up to the NODEDATA class:
         return theTree->getNodeData() < other.theTree->getNodeData();
      }
      bool operator>(const childstruct &other) const {
         // up to the NODEDATA class:
         return theTree->getNodeData() > other.theTree->getNodeData();
      }
   };

   vector<childstruct> theChildren;
      // Why a vector instead of a set?  Because the children are ordered.
      // Why do we group these into a childstruct?  To make memory allocation faster
      // and more efficient.

   int listboxPixWidth; // includes scrollbar, if applicable
   int listboxFullPixHeight; // total lb height (if no SB used)
   int listboxActualPixHeight; // we limit ourselves to a "window" this tall
 
   scrollbar theScrollbar;

   int allExpandedChildrenWidthAsDrawn, allExpandedChildrenHeightAsDrawn;
      // Both implicitly _and_ explicitly expanded children are included.
      // Listbox (if any) is _not_ included.
      // If no children are expanded, then these are 0.
      // Non-leaf children can be complex (have their own listboxes, subtrees, etc.).
      // This is counted.

 private:

   bool expandUnexpand1(const where4TreeConstants &tc, unsigned childindex,
                        bool expandFlag, bool rethinkFlag, bool force);
      // won't expand a leaf node.
      // NOTE: Expanding a child will require rethinking the all-expanded-children
      //       dimensions for ALL ancestors of the expandee.  This routine only takes
      //       care of updating those dimensions for the immediate ancestor (parent)
      //       of the expandee.  YOU MUST MANUALLY HANDLE THE REST OF THE PATH!

   // Mouse clicks and node expansion
   int point2ItemWithinListbox(const where4TreeConstants &tc,
			       int localx, int localy) const;
      // returns index of item clicked on (-2 if nothing), given point
      // local to the _data_ part of listbox.  0<=localy<listboxFullPixHeight

   int point2ItemOneStepScrollbar(const where4TreeConstants &tc,
				  int ypix,
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
      // should this routine return a reference instead?
      return theChildren[index].theTree;
   }
   const where4tree *getChildTree(unsigned index) const {
      // should this routine return a reference instead?
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

   int scroll_listbox(const where4TreeConstants &tc, int deltaYpix);
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

   void draw_listbox(Tk_Window, const where4TreeConstants &tc, Drawable theDrawable,
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

   bool explicitlyExpandSubchild(const where4TreeConstants &tc, unsigned childindex,
				 bool force);
      // Expand a subtree out of a listbox.  Doesn't redraw.
      // Returns true iff any changes were made.
      // NOTE: The same warning of expandUnexpand1() applies -- outside code must
      //       manually rethink_expanded_children_dimensions() for ALL ancestors of
      //       the expandee; this routine only handles the immeidate ancestor (parent).
   bool explicitlyUnexpandSubchild(const where4TreeConstants &tc, unsigned childindex);
      // Unexpand a subchild (into a listbox).  Doesn't redraw.
      // Returns true iff any changes were made.
      // NOTE: The same warning of expandUnexpand1() applies -- outside code must
      //       manually rethink_expanded_children_dimensions() for ALL ancestors of
      //       the expandee; this routine only handles the immeidate ancestor (parent).
   
   bool expandAllChildren(const where4TreeConstants &tc);
   bool unExpandAllChildren(const where4TreeConstants &tc);

   bool updateAnything2Draw1(const where4TreeConstants &tc);

 public:

   where4tree(const NODEDATA &iNodeData);
  ~where4tree();

   bool updateAnything2Draw(const where4TreeConstants &tc);

   const NODEDATA &getNodeData() const {return theNodeData;}
   NODEDATA &getNodeData() {return theNodeData;}

   const string &getRootName() const { return theNodeData.getName(); }

   // Adding children:
   void addChild(where4tree *theNewChild,
		 bool explicitlyExpanded,
		 const where4TreeConstants &tc,
		 bool rethinkLayoutNow, bool resortNow);
      // add a child subtree **that has been allocated with new** (not negotiable)
      // NOTE: Current implementation puts child into listbox unless
      //       explicitlyExpanded.
      // NOTE: Even if you pass rethinkLayoutNow as true, we only rethink the listbox
      //       dimensions and/or the all-expanded-children dimensions, as needed.
      //       In all likelihood, you'll also need to rethink all-expanded-children
      //       dimensions for all ancestors of this node; you must do this manually.

   void doneAddingChildren(const where4TreeConstants &tc, bool sortNow);
   void recursiveDoneAddingChildren(const where4TreeConstants &tc, bool sortNow);
      // Needed after having called addChild() several times for a given root and
      // had passed false as the last parameter...

   void sortChildren();
      // does not redraw

   void draw(Tk_Window, const where4TreeConstants &tc, Drawable theDrawable,
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
   bool path2lbItemExpand(const where4TreeConstants &tc, 
			 const whereNodePosRawPath &thePath, unsigned pathIndex);
      // Given a path (ending in a listbox item), expand it, as well as any
      // ancestors, as necessary.  Returns true iff any changes.

   bool path2lbItemUnexpand(const where4TreeConstants &tc,
			    const whereNodePosRawPath &thePath, unsigned pathIndex);
      // Un-expand the (currently expanded) item at the end of the path.
      // Creates a listbox if needed.  Returns true iff any changes.

   bool path2ExpandAllChildren(const where4TreeConstants &tc,
			       const whereNodePosRawPath &thePath,
			       unsigned index);

   bool path2UnExpandAllChildren(const where4TreeConstants &tc,
				 const whereNodePosRawPath &thePath, unsigned index);

   bool rigListboxScrollbarSliderTopPix(const where4TreeConstants &tc,
					int scrollBarLeft,
					int scrollBarTop,
					int scrollBarBottom,
					int newScrollBarSliderTopPix,
					bool redrawNow);
      // returns true iff any changes were made

   // The following highlight routines do not redraw:
   bool isHighlighted() const {
      return theNodeData.getHighlighted();
   }
   void highlight() {
      theNodeData.highlight();
   }
   void unhighlight() {
      theNodeData.unhighlight();
   }
   void toggle_highlight() {
      theNodeData.toggle_highlight();
   }
   void set_highlight(bool ishighlight)
   {
	if (ishighlight)
		highlight();
	else unhighlight();
   }

   bool selectUnSelectFromFullPathName(const char *name, bool selectFlag);
      // returns true iff found.  char * is used instead of string because
      // we'll be using pointer arithmetic as we parse "name".

   vector<const NODEDATA *> getSelections() const;

   void recursiveClearSelections();

private:
	vector<where4tree<NODEDATA> *>	shadow_nodes;
	whereNodePosRawPath	primary_path;
public:
	void addShadowNode(where4tree<NODEDATA> *shadow_node) {
		shadow_nodes += shadow_node;
		shadow_node->setPrimaryPath(getPrimaryPath());
	}
	void setPrimaryPath(whereNodePosRawPath _path,int child_index) {
		primary_path = _path;
		primary_path.append(child_index);
	}
	void setPrimaryPath(whereNodePosRawPath _path) {
		primary_path = _path;
	}
	whereNodePosRawPath	getPrimaryPath() { return primary_path;}
	bool	isPrimary()
	{
		return (shadow_nodes.size() > 0);
	}
	vector<where4tree<NODEDATA> *>  &getShadowNodes() { return shadow_nodes;}
};

#endif
