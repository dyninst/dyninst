// whereAxis.h
// Ariel Tamches

// A where axis corresponds to _exactly_ one Paradyn abstraction.

/* $Log: whereAxis.h,v $
/* Revision 1.5  1995/09/20 01:27:10  tamches
/* constness removed from many prototypes; other changes to correspond
/* with whereAxis.C
/*
 * Revision 1.4  1995/08/07  00:02:52  tamches
 * Added selectUnSelectFromFullPathName
 *
 * Revision 1.3  1995/07/24  21:36:03  tamches
 * removed addChildToRoot() member function.
 * Some changes related to newly implemented where4tree sorting.
 *
 * Revision 1.2  1995/07/18  03:41:24  tamches
 * Added ctrl-double-click feature for selecting/unselecting an entire
 * subtree (nonrecursive).  Added a "clear all selections" option.
 * Selecting the root node now selects the entire program.
 *
 * Revision 1.1  1995/07/17  04:59:07  tamches
 * First version of the new where axis
 *
 */

#ifndef _WHEREAXIS_H_
#define _WHEREAXIS_H_

#include <fstream.h>
#include "where4treeConstants.h"
#include "where4tree.h"

#ifndef PARADYN
#include "Dictionary.h"
#else
#include "util/h/Dictionary.h"
#endif

template <class USERNODEDATA>
class whereAxis {
 private:
   where4TreeConstants consts;
      // Each where axis has its own set of constants, so different axis may,
      // for example, have different color configurations.
   where4tree<USERNODEDATA> *rootPtr;

   dictionary_hash<USERNODEDATA, where4tree<USERNODEDATA> *> hash;
      // associative array: USERNODEDATA-->node

   string horizSBName; // e.g. ".nontop.main.bottsb"
   string vertSBName;  // e.g. ".nontop.main.leftsb"
   string navigateMenuName;

   whereNodePosRawPath lastClickPath;
      // used in the navigate menu
   where4tree<USERNODEDATA> *beginSearchFromPtr;
      // if NULL, then begin from the top.  Otherwise,
      // find() treats ignores found items until this one is reached.

   Tcl_Interp *interp;

   int nominal_centerx; // actual centerx = nominal_centerx + horizScrollBarOffset
   int horizScrollBarOffset; // note: always <= 0

   // why isn't there a nominal_topy?  Because it's always at the top of the window
   // (0 or 3)
   int vertScrollBarOffset; // note: always <= 0

   void resizeScrollbars();

   bool set_scrollbars(int absolute_x, int relative_x,
		       int absolute_y, int relative_y,
		       bool warpPointer);
      // returns true iff any sb changes were made
      // Moves the cursor if warpPointer is true.

 protected:
   void rethink_nominal_centerx();

   static unsigned hashFunc(const USERNODEDATA &und) {return (unsigned)und;}

#ifndef PARADYN
   // only the where axis test program reads from a file
   int readTree(ifstream &, USERNODEDATA parentUniqueId,
		USERNODEDATA nextAvailChildId);
      // returns # of nodes read in
#endif
		 
 public:
   whereAxis(Tcl_Interp *in_interp, Tk_Window theTkWindow,
	     const string &root_str,
	     const string &iHorizSBName, const string &iVertSBName,
	     const string &iNavigateMenuName);

#ifndef PARADYN
   // only the where axis test program reads from a file
   whereAxis(ifstream &infile, Tcl_Interp *interp, Tk_Window theWindow,
	     const char *iHorizSBName, const char *iVertSBName,
	     const char *iNavigateMenuName);
#endif

   virtual ~whereAxis() {delete rootPtr;}

   const string &getRootName() const {
      return rootPtr->getRootName();
   }

   // the return values of the next 2 routines will be <= 0
   int getVertSBOffset() const {return vertScrollBarOffset;}
   int getHorizSBOffset() const {return horizScrollBarOffset;}

   int getTotalVertPixUsed() const {return rootPtr->entire_height(consts);}
   int getTotalHorizPixUsed() const {return rootPtr->entire_width(consts);}

   int getVisibleVertPix() const {return Tk_Height(consts.theTkWindow);}
   int getVisibleHorizPix() const {return Tk_Width(consts.theTkWindow);}

   void addItem(const string &name,
		USERNODEDATA parentUniqueId,
		USERNODEDATA newNodeUniqueId,
		bool rethinkGraphicsNow,
		bool resortNow);

   const where4TreeConstants &getConsts() const {
      return consts;
   }

   void draw(bool doubleBuffer, bool isXsynchOn) const;

   void resize(int newWidth, int newHeight);
   void resize(bool rethinkScrollbars);
      // should be true only if we are the currently displayed abstraction

   void recursiveDoneAddingChildren() {
      rootPtr->recursiveDoneAddingChildren(consts);
   }

   void processSingleClick(int x, int y, bool redrawNow);
   bool processDoubleClick(int x, int y);
      // returns true iff a redraw of everything is still needed
   bool processShiftDoubleClick(int x, int y);
   bool processCtrlDoubleClick (int x, int y);

   void navigateTo(unsigned pathLen);
      // forcibly scrolls to item #pathLen of "lastClickPath"

   int find(const string &str);
      // uses and updates "beginSearchFromPtr"
      // returns 0 if not found; 1 if found & no expansion needed;
      // 2 if found & some expansion is needed

   bool softScrollToPathItem(const whereNodePosRawPath &thePath,
			     unsigned index);
      // scrolls s.t. the (centerx, topy) of the path item in question is placed in the
      // middle of the screen.  Returns true iff the scrollbar settings changed.

   bool softScrollToEndOfPath(const whereNodePosRawPath &thePath);
      // Like the above routine, but always scrolls to the last item in the path.

   bool forciblyScrollToEndOfPath(const whereNodePosRawPath &thePath);
      // Like the above routine, but explicitly expands any un-expanded children
      // along the path.
   bool forciblyScrollToPathItem(const whereNodePosRawPath &thePath, unsigned pathLen);

   // Noe of these scrollbar adjustment routines redraw anything
   bool adjustHorizSBOffset(float newFirstFrac);
   bool adjustHorizSBOffsetFromDeltaPix(int deltapix);
      // needed for alt-mousemove
//   bool adjustHorizSBOffsetFromDeltaPages(int deltapages);
   bool adjustHorizSBOffset(); // Obtains FirstPix from actual tk scrollbar

   bool adjustVertSBOffset (float newFirstFrac);
   bool adjustVertSBOffsetFromDeltaPix(int deltapix);
      // needed for alt-mousemove
//   bool adjustVertSBOffsetFromDeltaPages(int deltapages);
   bool adjustVertSBOffset(); // Obtains FirstPix from actual tk scrollbar

   void rethinkNavigateMenu();

   bool selectUnSelectFromFullPathName(const string &name, bool select);
      // returns true iff the item was found
      // pass true for the 2nd param iff you want to select it; false
      // if you want to unselect it.
   
   vector< vector<USERNODEDATA> > getSelections() const;
   void clearSelections();
};

#endif
