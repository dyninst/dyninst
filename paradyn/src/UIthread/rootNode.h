// rootNode.h
// Ariel Tamches
// C++ class for the root node of subtrees declared in where4tree.h
// Basically, this file exists just to make where4tree.h that much shorter.

/* $Log: rootNode.h,v $
/* Revision 1.3  1995/09/20 01:18:03  tamches
/* minor cleanifications hardly worth mentioning
/*
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

extern "C" {
   #include <tcl.h>
   #include <tk.h>
   #include <X11/Xlib.h>
}

#ifndef PARADYN
// the where axis test program has the proper -I settings
#include "String.h"
#else
#include "util/h/String.h"
#endif

#include "where4treeConstants.h"

class rootNode {
 friend class whereAxis;

 private:
   string name; // name of the root of this subtree
   bool highlighted; // true iff the root of this subtree is highlighted.  Must set
                     // this flag recursively if you want entire subtree highlighted.
   int pixWidth, pixHeight;
      // keeping track of these values speeds up draw()

   static int borderPix; // both horiz & vertical [Tk_Fill3DRectangle forces these
                         // to be the same]
   static int horizPad, vertPad;

 private:

   int wouldbe_width(const where4TreeConstants &tc) const;
   int wouldbe_height(const where4TreeConstants &tc) const;

   void manual_construct(const char *init_str, const where4TreeConstants &tc,
			 const bool init_highlighted);

 public:

   rootNode(const string &init_str, const where4TreeConstants &tc, const bool hilited);
   rootNode(const char *init_str, const where4TreeConstants &tc, const bool hilited);
  ~rootNode() {}

   const string &getName() const;

   int getWidth() const;
   int getHeight() const;
   bool getHighlighted() const;
   
   void draw(const where4TreeConstants &tc, int theDrawable,
	     const int root_middlex, const int topy) const;

   // Mouse clicks and node expansion
   int pointWithin(const int xpix, const int ypix,
		   const int root_centerx, const int root_topy) const;
      // return values:
      // 1 -- yes
      // 2 -- no, point is north of root (or northwest or northeast)
      // 3 -- no, point is south of root (or southwest or southeast)
      // 4 -- no, point is west of root (but not north or south of root)
      // 5 -- no, point is east of root (but not north or south or root)

   // The following 3 routines don't redraw:
   void highlight();
   void unhighlight();
   void toggle_highlight();
};

#endif
