// whereAxisMisc.C
// non-member functions otherwise related to whereAxis.C & where4tree.C

#include <ctype.h>
#include <tclclean.h>
#include <tk.h>

#include "String.h"

#include "minmax.h"

#ifndef PARADYN
// The where axis test program has the proper -I settings
#include "DMinclude.h"
#else
#include "paradyn/src/DMthread/DMinclude.h"
#endif

#include "graphicalPath.h"
#include "whereAxis.h"

// We define these variables on behalf of where4tree.C; necessary because
// G++ 2.6.3 doesn't support static member variables of template classes
int listboxBorderPix = 3;
int listboxScrollBarWidth = 16;

bool nonSliderButtonCurrentlyPressed = false;
whereNodeGraphicalPath<unsigned>::pathEndsIn nonSliderButtonPressRegion;
Tk_TimerToken buttonAutoRepeatToken;

where4tree<resourceHandle> *nonSliderCurrentSubtree;
int nonSliderSubtreeCenter;
int nonSliderSubtreeTop;

void nonSliderCallMeOnButtonRelease(ClientData, XEvent *) {
//   cout << "welcome to non-slider button release...un-installing timer handler" << endl;

   nonSliderButtonCurrentlyPressed = false;
   Tk_DeleteTimerHandler(buttonAutoRepeatToken);
}

void nonSliderButtonAutoRepeatCallback(ClientData cd) {
   // If the mouse button has been released, do NOT re-invoke
   // the timer.
   if (!nonSliderButtonCurrentlyPressed)
      return;

   where4TreeConstants &tc = *(where4TreeConstants *)cd;

   // take desired action now...
   const int listboxLeft = nonSliderSubtreeCenter -
                           nonSliderCurrentSubtree->
                              horiz_pix_everything_below_root(tc) / 2;
   const int listboxTop = nonSliderSubtreeTop +
                          nonSliderCurrentSubtree->getRootNode().getHeight() +
			  tc.vertPixParent2ChildTop;

   extern int listboxBorderPix;
   const int listboxActualDataPix = nonSliderCurrentSubtree->getListboxActualPixHeight() - 2*listboxBorderPix;
   int deltaYpix = 0;
   int repeatIntervalMillisecs = 100;

   switch (nonSliderButtonPressRegion) {
      case whereNodeGraphicalPath<unsigned>::ListboxScrollbarUpArrow:
         deltaYpix = -4;
         repeatIntervalMillisecs = 10;
         break;
      case whereNodeGraphicalPath<unsigned>::ListboxScrollbarDownArrow:
         deltaYpix = 4;
         repeatIntervalMillisecs = 10;
         break;
      case whereNodeGraphicalPath<unsigned>::ListboxScrollbarPageup:
         deltaYpix = -listboxActualDataPix;
         repeatIntervalMillisecs = 250; // not so fast
         break;
      case whereNodeGraphicalPath<unsigned>::ListboxScrollbarPagedown:
         deltaYpix = listboxActualDataPix;
         repeatIntervalMillisecs = 250; // not so fast
         break;
      default:
         assert(false);
   }

   (void)nonSliderCurrentSubtree->scroll_listbox(tc, listboxLeft, listboxTop, deltaYpix);
   buttonAutoRepeatToken = Tk_CreateTimerHandler(repeatIntervalMillisecs,
						 nonSliderButtonAutoRepeatCallback,
						 &tc);
}

int slider_scrollbar_left;
int slider_scrollbar_top;
int slider_scrollbar_bottom;
int slider_initial_yclick;
int slider_initial_scrollbar_slider_top;
where4tree<resourceHandle> *slider_currently_dragging_subtree;

void sliderCallMeOnMouseMotion(ClientData cd, XEvent *eventPtr) {
   assert(eventPtr->type == MotionNotify);

   where4TreeConstants &tc = *(where4TreeConstants *)cd;

   const int y = eventPtr->xmotion.y;

   const int amount_moved = y - slider_initial_yclick; // may be negative, of course.

   const int newScrollBarSliderTopPix = slider_initial_scrollbar_slider_top +
                                        amount_moved;

   assert(slider_currently_dragging_subtree != NULL);
   (void)slider_currently_dragging_subtree->rigListboxScrollbarSliderTopPix
                         (tc, slider_scrollbar_left,
			  slider_scrollbar_top,
			  slider_scrollbar_bottom,
			  newScrollBarSliderTopPix,
			  true // redraw now
			  );
}

void sliderCallMeOnButtonRelease(ClientData cd, XEvent *eventPtr) {
   assert(eventPtr->type == ButtonRelease);

   where4TreeConstants &tc = *(where4TreeConstants *)cd;

   Tk_DeleteEventHandler(tc.theTkWindow,
			 PointerMotionMask,
			 sliderCallMeOnMouseMotion,
			 cd);
   Tk_DeleteEventHandler(tc.theTkWindow,
			 ButtonReleaseMask,
			 sliderCallMeOnButtonRelease,
			 cd);
   slider_currently_dragging_subtree=NULL;
}

// Stuff only for the where axis test program;
#ifndef PARADYN
string readUntilQuote(ifstream &is) {
   char buffer[256];
   char *ptr = &buffer[0];
   
   while (true) {
      char c;
      is.get(c);
      if (!is || is.eof())
         return string(); // empty
      if (c=='"')
         break;
      if (c==')') {
         cerr << "readUntilQuote: found ) before \"" << endl;
         exit(5);
      }
      *ptr++ = c;
   }

   *ptr = '\0'; // finish off the string
   return string(buffer);
}

string readUntilSpace(ifstream &is) {
   char buffer[256];
   char *ptr = &buffer[0];
   
   while (true) {
      char c;
      is.get(c);
      if (!is || is.eof())
         return string(); // empty
      if (isspace(c))
         break;
      if (c==')') {
         is.putback(c);
         break;
      }
      *ptr++ = c;
   }

   *ptr = '\0'; // finish off the string
   return string(buffer);
}

string readItem(ifstream &is) {
   char c;
   is >> c;
   if (!is || is.eof())
      return (char *)NULL;

   if (c== ')') {
      is.putback(c);
      return string();
   }

   if (c=='"')
      return readUntilQuote(is);
   else {
      is.putback(c);
      return readUntilSpace(is);
   }
}
#endif
