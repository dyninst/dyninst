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

// shgRootNode.h
// Ariel Tamches
// analagous to rootNode.h (for the where axis)

/* $Id: shgRootNode.h,v 1.11 2003/05/23 07:27:57 pcroth Exp $ */

#ifndef _SHG_ROOT_NODE_H_
#define _SHG_ROOT_NODE_H_

#ifndef PARADYN
#include "String.h"
#else
#include "common/h/String.h"
#endif

#include "tcl.h"
#include "tk.h"

class shgRootNode {
 public:
   enum evaluationState {es_never, es_unknown, es_true, es_false};
   enum refinement {ref_why, ref_where, ref_undefined};

 private:
   bool hidden;

   unsigned id;
   string label;
   string abbrevLabel;
   string fullInfo;
      // note: now that we have a perf cons igen call getNodeInfo(), perhaps
      // keeping "fullInfo" here is a mistake?

   bool highlighted;

   // the combination of the following 2 vrbles defines the 7 possible states
   // (active/es_never is undefined)
   bool active;
   evaluationState evalState;
   bool deferred;

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
		   const string &iLabel, const string &iFullInfo,
		   bool hidden);

 public:

   shgRootNode(unsigned iId, bool iActive, evaluationState iEvalState,
	       refinement iRefinement,
	       bool iShadowNode,
	       const string &iLabel, const string &iFullInfo,
	       bool iHidden);
   shgRootNode(unsigned iId, bool iActive, evaluationState iEvalState,
	       bool iShadowNode,
	       const string &iLabel, const string &iFullInfo,
	       bool iHidden);
   shgRootNode(const shgRootNode &src);
  ~shgRootNode() {}

   shgRootNode &operator=(const shgRootNode &src);

   bool anything2draw() const {return !hidden;}

   void hidify() {
      hidden = true;
   }
   void unhide() {
      hidden = false;
   }

   bool isShadowNode() const {return shadowNode;}
   shgRootNode shadowify(const char *newlabel) const {
      return shgRootNode(id, active, evalState,
			 theRefinement,
			 true, // shadow node
			 newlabel, fullInfo, hidden);
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

   bool shouldHide() const { return false; }

   int getHeightAsRoot() const {return pixHeightAsRoot;}
   int getWidthAsRoot()  const {return pixWidthAsRoot;}
   int getWidthAsListboxItem() const {return pixWidthAsListboxItem;}

   bool isActive() const {return active;}
   evaluationState getEvalState() const {return evalState;}
   bool isDeferred( void ) const { return deferred; }

   refinement getRefinement() const {return theRefinement;}
   void setRefinement(refinement newRefinement) {theRefinement = newRefinement;}

   bool configStyle(bool newActive,
                    evaluationState newEvalState,
                    bool newDeferred );
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
