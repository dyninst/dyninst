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

// rootNode.h
// Ariel Tamches
// C++ class for the root node of subtrees declared in where4tree.h
// Basically, this file exists just to make where4tree.h that much shorter.

/* $Id: rootNode.h,v 1.12 2003/03/10 18:55:41 pcroth Exp $ */

#ifndef _ROOTNODE_H_
#define _ROOTNODE_H_

#include "tcl.h"
#include "tk.h"

#ifndef PARADYN
// the where axis test program has the proper -I settings
#include "String.h"
#include "DMinclude.h" // resourceHandle
#else
#include "common/h/String.h"
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
   string abbrevName;
   bool highlighted;
   bool retired;

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

      const char* mfl = getenv("PARADYN_MAX_FUNCTION_LENGTH");
      mfl = mfl ? mfl : "0";
      int abbrevLength = atoi(mfl);
      if(name.length() > abbrevLength && abbrevLength != 0) {
          abbrevName = name.substr(0, abbrevLength / 2);
          abbrevName += string("...");
          abbrevName += name.substr(name.length() - (abbrevLength / 2), name.length());
      } else {
          abbrevName = name;
      }
      
      uniqueId = src.uniqueId;
      highlighted = src.highlighted;
      retired = src.retired;
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

   // Can be used in where4tree to determine whether or not to draw item.
   // Needs to be done if tunable constant whereAxisHideRetiredRes is chosen.
   bool shouldHide() const { return is_retired(); }

   // The following 3 routines don't redraw:
   void highlight() {
      if(is_retired()) return;  // can't be selected if it no longer exists
      highlighted = true;
   }
   void unhighlight() {
      highlighted = false;
   }
   void toggle_highlight() {
      if(is_retired() && highlighted == false)
         return;

      highlighted = !highlighted;
   }

   void mark_as_retired()  { 
      unhighlight();  // can't be selected if it no longer exists
      retired = true;
   }
   bool is_retired() const { return retired; }
};

#endif
