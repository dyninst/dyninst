// whereAxis.h
// Ariel Tamches

// A where axis corresponds to _exactly_ one Paradyn abstraction.

/* $Log: whereAxis.h,v $
/* Revision 1.1  1995/07/17 04:59:07  tamches
/* First version of the new where axis
/*
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

   whereNodeRawPath lastClickPath;
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

   bool set_scrollbars(const int absolute_x, const int relative_x,
		       const int absolute_y, const int relative_y);
      // returns true iff any sb changes were made

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
	     const string &iNavigateMenuName) :
                        consts(in_interp, theTkWindow),
	                hash(hashFunc, 32),
	                horizSBName(iHorizSBName),
	                vertSBName(iVertSBName),
	                navigateMenuName(iNavigateMenuName) {
      USERNODEDATA rootUND = 0;

      rootPtr = new where4tree<USERNODEDATA>(rootUND, root_str, consts);
      assert(rootPtr);

      hash[rootUND] = rootPtr;

      beginSearchFromPtr = NULL;

      interp = in_interp;
      
      horizScrollBarOffset = vertScrollBarOffset = 0;
      
      rethink_nominal_centerx();
   }

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

//   where4tree<USERNODEDATA> *newSubtree(const char *rootName) {
//      where4tree<USERNODEDATA> *result =
//                                new where4tree<USERNODEDATA>(rootName, consts);
//      assert(result);
//      
//      return result;
//   }

   void addChildToRoot(where4tree<USERNODEDATA> *theChild,
		       const bool explicitlyExpanded);

   void addItem(const string &name,
		USERNODEDATA parentUniqueId,
		USERNODEDATA newNodeUniqueId,
		bool rethinkGraphicsNow);

   const where4TreeConstants &getConsts() const {
      return consts;
   }

   void draw(const bool doubleBuffer, const bool isXsynchOn) const;

   void resize(const int newWidth, const int newHeight);
   void resize(bool rethinkScrollbars);
      // should be true only if we are the currently displayed abstraction

   void recursiveDoneAddingChildren() {
      rootPtr->recursiveDoneAddingChildren(consts);
   }

   void processSingleClick(const int x, const int y,
			   const bool redrawNow);
   bool processDoubleClick(const int x, const int y, const bool redrawNow);
      // returns true iff a redraw of everything is still needed
   bool processShiftDoubleClick(const int x, const int y);

   void navigateTo(const int index);

   int find(const string &str);
      // uses and updates "beginSearchFromPtr"

   bool softScrollToPathItem(const whereNodeRawPath &thePath,
			     const int index);
      // scrolls s.t. the (centerx, topy) of the path item in
      // question is placed in the middle of the screen.  Returns true
      // iff the scrollbar settings changed

   bool softScrollToEndOfPath(const whereNodeRawPath &thePath);
      // Like the above routine, but always scrolls to the last item in the path.

   bool forciblyScrollToEndOfPath(const whereNodeRawPath &thePath);
      // Like the above routine, but explicitly expands any un-expanded children
      // along the path.
   bool forciblyScrollToPathItem(const whereNodeRawPath &thePath, const int index);

   // Noe of these scrollbar adjustment routines redraw anything
   void adjustHorizSBOffset(const float newFirstFrac);
   void adjustHorizSBOffsetFromDeltaPix(const int deltapix);
   void adjustHorizSBOffsetFromDeltaPages(const int deltapages);
   void adjustHorizSBOffset(); // Obtains FirstPix from actual tk scrollbar

   void adjustVertSBOffset (const float newFirstFrac);
   void adjustVertSBOffsetFromDeltaPix(const int deltapix);
   void adjustVertSBOffsetFromDeltaPages(const int deltapages);
   void adjustVertSBOffset(); // Obtains FirstPix from actual tk scrollbar

   void rethinkNavigateMenu();

   vector< vector<USERNODEDATA> > getSelections() const {
      // returns a vector[num-hierarchies] of vector of selections.
      // The number of hierarchies is defined as the number of children of the
      // root node.
      const unsigned numHierarchies = rootPtr->theChildren.size();

      vector < vector<USERNODEDATA> > result(numHierarchies);

      for (int i=0; i < numHierarchies; i++) {
         result[i] = rootPtr->theChildren[i].theTree->getSelections();
         if (result[i].size()==0) {
            // this hierarchy had no selections; therefore, choose the hierarchy's
            // root item...
            vector<USERNODEDATA> defaultHierarchy(1);
            defaultHierarchy[0] = rootPtr->theChildren[i].theTree->theUserNodeData;
            result[i] = defaultHierarchy;
	 }
      }

      return result;
   }
};

#endif
