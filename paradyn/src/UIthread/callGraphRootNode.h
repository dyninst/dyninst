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

// $Id: callGraphRootNode.h,v 1.8 2005/01/28 18:12:04 legendre Exp $

#ifndef _CALLGRAPH_ROOTNODE_H_
#define _CALLGRAPH_ROOTNODE_H_

#ifndef PARADYN
// the where axis test program has the proper -I settings
#include "String.h"
#include "DMinclude.h" // resourceHandle
#else
#include "common/h/String.h"
#include "paradyn/src/DMthread/DMinclude.h" // resourceHandle
#endif

#include "tcl.h"
#include "tk.h"

// This class is intended to be used as the template type of where4tree<>
// for the call graph. It is based on code from shgRootNode, adapted for
//use with the call graph.

class callGraphRootNode {
 private:
  resourceHandle uniqueId;
  bool isRecursive;
  bool isShadowNode;
  bool shortNameIsDisplayed;
  pdstring shortName; // name of the root of this subtree
  pdstring fullName;
  pdstring *currentName; 
  //This variable points to either shortName of fullName, depending on the
  //setting from the "View" menu. By default, the shortName
  
  bool highlighted;
  
  int pixWidthAsRoot, pixHeightAsRoot;
  int pixWidthAsListboxItem;
  
  // note: we use the following static members of
  //       our "parent" class (whereAxis):
  //       getRootItemFontStruct(), getListboxItemFontStruct(),
  //       getRootItemTk3DBorder(), getRootItemTextGC()
  
  static int borderPix; 
  // both horiz & vertical [Tk_Fill3DRectangle forces these
  // to be the same]
  static int horizPad, vertPad;
  
  callGraphRootNode &operator=(const callGraphRootNode &src);
  
 public:
  
  callGraphRootNode(resourceHandle uniqueId, const pdstring &shortName, const pdstring &fullName, bool recursiveFlag, bool shadowNode);
  callGraphRootNode(const callGraphRootNode &src)  : shortNameIsDisplayed(src.shortNameIsDisplayed), shortName(src.shortName) , fullName(src.fullName), currentName(shortNameIsDisplayed? &shortName : &fullName) {
    uniqueId = src.uniqueId;
    highlighted = src.highlighted;
    pixWidthAsRoot = src.pixWidthAsRoot;
    pixHeightAsRoot = src.pixHeightAsRoot;
    pixWidthAsListboxItem = src.pixWidthAsListboxItem;
    isRecursive = src.isRecursive;
    isShadowNode = src.isShadowNode;
  }
  ~callGraphRootNode() {}

  bool anything2draw() const {return true;}
  // where axis items are never hidden
  
  bool operator<(const callGraphRootNode &other) {return fullName < other.fullName;}
  bool operator>(const callGraphRootNode &other) {return fullName > other.fullName;}
  
  resourceHandle getUniqueId() const {return uniqueId;}
  const pdstring &getName() const {return shortName;}
  const pdstring &getFullName() const {return fullName;}
  void showFullName();
  void showShortName();
  void updateName(pdstring & newname){
     fullName = newname; shortName = newname;
  }
  int getWidthAsRoot()  const {return pixWidthAsRoot;}
  int getHeightAsRoot() const {return pixHeightAsRoot;}
  int getWidthAsListboxItem() const {return pixWidthAsListboxItem;}
  bool isShadow() const {return isShadowNode;}
  bool getHighlighted() const {return highlighted;}
  
  static void prepareForDrawingListboxItems(Tk_Window, XRectangle &);
  static void doneDrawingListboxItems(Tk_Window);
  void drawAsRoot(Tk_Window theTkWindow,
		  int theDrawable, // could be offscren pixmap
		  int root_middlex, int topy) const;
  
  static GC getGCforListboxRay(const callGraphRootNode &parent,
				const callGraphRootNode &firstChild);
  // return GC to be used in an XDrawLine call from "parent" down to the
  // listbox of its children; "firstChild" is the node data for the first
  // such child.
  static GC getGCforNonListboxRay(const callGraphRootNode &parent,
				  const callGraphRootNode &child);
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

  bool shouldHide() const { return false; }
  
  // The following 3 routines don't redraw:
  void highlight() {highlighted=true;}
  void unhighlight() {highlighted=false;}
  void toggle_highlight() {highlighted = !highlighted;}
};

#endif
