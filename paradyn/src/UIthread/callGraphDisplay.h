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

// $Id: callGraphDisplay.h,v 1.8 2002/12/20 07:50:04 jaw Exp $

#ifndef _CALLGRAPH_DISPLAY_H_
#define _CALLGRAPH_DISPLAY_H_

#include <fstream.h>
#include "callGraphConsts.h"
#include "where4treeConstants.h"
#include "where4tree.h"
#include "callGraphRootNode.h"
#include "graphicalPath.h"

#ifndef PARADYN
#include "Dictionary.h"
#else
#include "common/h/Dictionary.h"
#endif

//callGraphDisplay:: this class is adapted from the shg class, 
//It handles the display of one call graph. Multiple call graphs are kept 
//track of by the the callGraphs class.

class callGraphDisplay {
 private:
   // these static members are needed by whereAxisRootNode (rootNode.h)
   static Tk_Font theRootItemFontStruct, theRootItemShadowFontStruct;
   static Tk_Font theListboxItemFontStruct, theListboxItemShadowFontStruct;
   static pdvector<Tk_3DBorder> rootItemTk3DBordersByStyle;
   static pdvector<Tk_3DBorder> listboxItemTk3DBordersByStyle;
   static GC rootItemTextGC, rootItemShadowTextGC;
   static GC listboxItemGC, listboxItemShadowTextGC;
   static GC listboxRayGC;
   static GC nonListboxRayGC;

   // this appears to be the WRONG class for the following vrbles:
   static int listboxBorderPix; // 3
   static int listboxScrollBarWidth; // 16

   // These ugly variables keep track of a button press (and possible hold-down)
   // in scrollbar-up/down or pageup/pagedown region.  They are not used for
   // press (and possible hold-down) on the scrollbar slider
   bool nonSliderButtonCurrentlyPressed; // init to false
   whereNodeGraphicalPath<callGraphRootNode>::pathEndsIn nonSliderButtonPressRegion;
   Tk_TimerToken buttonAutoRepeatToken;
   where4tree<callGraphRootNode> *nonSliderCurrentSubtree;
   int nonSliderSubtreeCenter;
   int nonSliderSubtreeTop;

   // Analagous to above; used only for scrollbar slider
   int slider_scrollbar_left, slider_scrollbar_top, slider_scrollbar_bottom;
   int slider_initial_yclick, slider_initial_scrollbar_slider_top;
   where4tree<callGraphRootNode> *slider_currently_dragging_subtree;

   void initializeStaticsIfNeeded();
   int programId;
   
   where4TreeConstants consts;
   callGraphConsts theCallGraphConsts;
   // Each where axis has its own set of constants, so different axis may,
   // for example, have different color configurations.
   where4tree<callGraphRootNode> *rootPtr;

   dictionary_hash<resourceHandle, where4tree<callGraphRootNode> *> hash;
   
   string horizSBName; // e.g. ".nontop.main.bottsb"
   string vertSBName;  // e.g. ".nontop.main.leftsb"
   
   const string executable_name;
   
   whereNodePosRawPath lastClickPath;
   // used in the navigate menu
   where4tree<callGraphRootNode> *beginSearchFromPtr;
   // if NULL, then begin from the top.  Otherwise,
   // find() treats ignores found items until this one is reached.

   Tcl_Interp *interp;

   bool obscured;
      // true if the underlying window is partially or fully obscured.
      // Currently, used to know when to properly redraw after scrolling
      // a listbox, since I'm having trouble getting tk to recognize
      // GraphicsExpose events...sigh

   int nominal_centerx; // actual centerx = nominal_centerx + horizScrollBarOffset
   int horizScrollBarOffset; // always <= 0

   // why isn't there a nominal_topy?  Because it's always at the top of the window
   // (0 or 3)
   int vertScrollBarOffset; // always <= 0

   void resizeScrollbars();

   bool set_scrollbars(int absolute_x, int relative_x,
		       int absolute_y, int relative_y,
		       bool warpPointer);
      // returns true iff any sb changes were made
      // Moves the cursor if warpPointer is true.

   whereNodeGraphicalPath<callGraphRootNode> point2path(int x, int y) const;

   static void nonSliderButtonRelease(ClientData cd, XEvent *);
   static void nonSliderButtonAutoRepeatCallback(ClientData cd);

   void processNonSliderButtonPress(whereNodeGraphicalPath<callGraphRootNode> &thePath);

   static void sliderMouseMotion(ClientData cd, XEvent *eventPtr);
   static void sliderButtonRelease(ClientData cd, XEvent *eventPtr);
   void recursiveChangeNameStyle(where4tree<callGraphRootNode> *ptr, 
				 bool fullName);
 protected:
   void rethink_nominal_centerx();
   
   static unsigned hashFunc(const resourceHandle &uniqueId) {return uniqueId;}
   // needed for hash table class...
   
 public:
   callGraphDisplay(int pid, resourceHandle rootId,  Tcl_Interp *in_interp, 
		    Tk_Window theTkWindow, const string &exe_name,
		    const string &shortName, const string &fullName,
		    const string &iHorizSBName, const string &iVertSBName);


  ~callGraphDisplay() {delete rootPtr;}

  void changeNameStyle(bool fullName);
  int getProgramId() {return programId;}
  const string &getName(){return executable_name;}

  // the return values of the next 2 routines will be <= 0
  int getVertSBOffset() const {return vertScrollBarOffset;}
  int getHorizSBOffset() const {return horizScrollBarOffset;}
  
  int getTotalVertPixUsed() const {return rootPtr->entire_height(consts);}
  int getTotalHorizPixUsed() const {return rootPtr->entire_width(consts);}
  
  int getVisibleVertPix() const {return Tk_Height(consts.theTkWindow);}
  int getVisibleHorizPix() const {return Tk_Width(consts.theTkWindow);}
  
  void addItem(const string &shortName, const string &fullName,
	       resourceHandle parentUniqueId,
	       resourceHandle newNodeUniqueId,
	       bool recursiveFlag,
	       bool isShadowNode,
	       bool rethinkGraphicsNow,
		bool resortNow);
  void rethinkEntireLayout();
  
  void recursiveDoneAddingChildren(bool resortNow) {
    rootPtr->recursiveDoneAddingChildren(consts, resortNow);
  }
  
   static Tk_Font &getRootItemFontStruct(bool shadow) {
     assert(theRootItemFontStruct); // a static member vrble
     if(shadow)
       return theRootItemShadowFontStruct;
      else 
	return theRootItemFontStruct;
   }
   static Tk_Font &getListboxItemFontStruct(bool shadow) {
     assert(theListboxItemFontStruct); // a static member vrble
     if(shadow)
       return theListboxItemShadowFontStruct;
     else 
       return theListboxItemFontStruct;
   }
   static Tk_3DBorder getRootItemTk3DBorder(bool isRecursive) {
     unsigned index = (int) isRecursive;
     return rootItemTk3DBordersByStyle[index];
   }
   static GC getRootItemTextGC(bool shadow) {
     if(shadow)
       return rootItemShadowTextGC;
     else 
       return rootItemTextGC;
   }
   static Tk_3DBorder getListboxItemTk3DBorder(bool isRecursive) {
     unsigned index = (int) isRecursive;
     return rootItemTk3DBordersByStyle[index];
   }
   static GC getListboxItemGC(bool shadow) {
     if(shadow)
       return listboxItemShadowTextGC;
     else
       return listboxItemGC;
   }
   static GC getGCforListboxRay() {
     return listboxRayGC;
   }
   static GC getGCforNonListboxRay() {
     return nonListboxRayGC;
   }
   
   void draw(bool doubleBuffer, bool isXsynchOn) const;
   
   void resize(bool rethinkScrollbars);
   // should be true only if we are the currently displayed abstraction
   
   void makeVisibilityUnobscured() {consts.makeVisibilityUnobscured();}
   void makeVisibilityPartiallyObscured() {
     consts.makeVisibilityPartiallyObscured();}
   void makeVisibilityFullyObscured() {consts.makeVisibilityFullyObscured();}
   
   void processSingleClick(int x, int y);
   bool processDoubleClick(int x, int y);
   // returns true iff a redraw of everything is still needed
   bool processShiftDoubleClick(int x, int y);
   bool processCtrlDoubleClick (int x, int y);
   
   int find(const string &str);
   // uses and updates "beginSearchFromPtr"
   // returns 0 if not found; 1 if found & no expansion needed;
   // 2 if found & some expansion is needed
   
   bool softScrollToPathItem(const whereNodePosRawPath &thePath, 
			     unsigned index);
   // scrolls s.t. the (centerx, topy) of the path item in question 
   // is placed in the middle of the screen.  Returns true iff the 
   //scrollbar settings changed.
   
   bool softScrollToEndOfPath(const whereNodePosRawPath &thePath);
   // Like the above routine, but always scrolls to the last item in the path.
   
   bool forciblyScrollToEndOfPath(const whereNodePosRawPath &thePath);
   // Like the above routine, but explicitly expands any un-expanded children
   // along the path.
   bool forciblyScrollToPathItem(const whereNodePosRawPath &thePath, 
				 unsigned pathLen);
   
   // None of these scrollbar adjustment routines redraw anything
   bool adjustHorizSBOffset(float newFirstFrac);
   bool adjustHorizSBOffsetFromDeltaPix(int deltapix);
   
   // needed for alt-mousemove
   // bool adjustHorizSBOffsetFromDeltaPages(int deltapages);
   bool adjustHorizSBOffset(); // Obtains FirstPix from actual tk scrollbar

   bool adjustVertSBOffset (float newFirstFrac);
   bool adjustVertSBOffsetFromDeltaPix(int deltapix);
   // needed for alt-mousemove
   // bool adjustVertSBOffsetFromDeltaPages(int deltapages);
   bool adjustVertSBOffset(); // Obtains FirstPix from actual tk scrollbar

   void navigateTo(unsigned pathLen);
   // forcibly scrolls to item #pathLen of "lastClickPath"
   
   bool selectUnSelectFromFullPathName(const string &name, bool select);
   // returns true iff the item was found
   // pass true for the 2nd param iff you want to select it; false
   // if you want to unselect it.
   
   pdvector< pdvector <resourceHandle> >getSelections(bool &wholeProgram, 
				   pdvector<unsigned> &wholeProgramFocus) const;
   void clearSelections();

   void map_to_WhereAxis(resourceHandle,bool);
   void map_from_WhereAxis(resourceHandle,bool);
   void notify_shadow(resourceHandle,bool);
};

#endif











