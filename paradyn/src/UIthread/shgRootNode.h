// shgRootNode.h
// Ariel Tamches
// analagous to rootNode.h (for the where axis)

/* $Log: shgRootNode.h,v $
/* Revision 1.4  1996/02/15 23:13:19  tamches
/* added code to properly support why vs. where axis refinement
/*
 * Revision 1.3  1996/01/23 07:09:04  tamches
 * style split up into evaluationState & active flag
 * added shadow node features
 *
 * Revision 1.2  1996/01/11 23:42:21  tamches
 * there are now 6 node styles instead of 4
 *
 * Revision 1.1  1995/10/17 22:08:52  tamches
 * initial version, for the new search history graph
 *
 */

#ifndef _SHG_ROOT_NODE_H_
#define _SHG_ROOT_NODE_H_

#ifndef PARADYN
#include "String.h"
#else
#include "util/h/String.h"
#endif

#include "tcl.h"
#include "tk.h"

class shgRootNode {
 public:
   enum evaluationState {es_never, es_unknown, es_true, es_false};
   enum refinement {ref_why, ref_where, ref_undefined};

 private:
   unsigned id;
   string label;
   string fullInfo;
      // note: now that we have a perf cons igen call getNodeInfo(), perhaps
      // keeping "fullInfo" here is a mistake?

   bool highlighted;

   // the combination of the following 2 vrbles defines the 7 possible states
   // (active/es_never is undefined)
   bool active;
   evaluationState evalState;

   // the following vrble tells whether, in the shg, this node is a why or a where
   // refinement of its parent node.  Presumably, outside code will use the value
   // to draw different edge styles between nodes.  Probably undefined for root.
   refinement theRefinement;

   bool shadowNode;

   int pixWidthAsRoot, pixHeightAsRoot;
   int pixWidthAsListboxItem;

   static int borderPix;
   static int horizPad, vertPad;

   void initialize(unsigned iId, bool iActive, evaluationState iEvalState,
		   refinement iRefinement,
		   bool iShadowNode,
		   const string &iLabel, const string &iFullInfo);

 public:

   shgRootNode(unsigned iId, bool iActive, evaluationState iEvalState,
	       refinement iRefinement,
	       bool iShadowNode,
	       const string &iLabel, const string &iFullInfo);
   shgRootNode(unsigned iId, bool iActive, evaluationState iEvalState,
	       bool iShadowNode,
	       const string &iLabel, const string &iFullInfo);
   shgRootNode(const shgRootNode &src);
  ~shgRootNode() {}

   shgRootNode shadowify(const char *newlabel) const {
      return shgRootNode(id, active, evalState,
			 theRefinement,
			 true, // shadow node
			 newlabel, fullInfo);
   }

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

   bool isActive() const {return active;}
   evaluationState getEvalState() const {return evalState;}

   refinement getRefinement() const {return theRefinement;}
   void setRefinement(refinement newRefinement) {theRefinement = newRefinement;}

   bool configStyle(bool newActive, evaluationState newEvalState);
      // returns true iff any changes.  Does not redraw.

   void drawAsRoot(Tk_Window, int theDrawable,
		   int centerx, int topy) const;

   static GC getGCforListboxRay(const shgRootNode &parent,
				const shgRootNode &firstChild);
      // return GC to be used in an XDrawLine call from "parent" down to the
      // listbox of its children; "firstChild" is the node data for the first
      // such child.
   static GC getGCforNonListboxRay(const shgRootNode &parent,
				   const shgRootNode &child);
      // assuming that "parent" is an expanded (explicitly or not) node, return the GC
      // to be used in an XDrawLine call from it down to "child".

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
