// misc.C
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

#include "whereAxis.h"

// We define these variables on behalf of where4tree.C; necessary because
// G++ 2.6.3 doesn't support static member variables of template classes
int listboxBorderPix = 3;
int listboxScrollBarWidth = 16;

void myTclEval(Tcl_Interp *interp, const char *buffer) {
   if (TCL_OK != Tcl_Eval(interp, buffer)) {
      cerr << interp->result << endl;
      exit(5);
   }
}

void getScrollBarValues(Tcl_Interp *interp, const string &scrollBarName,
			float &first, float &last) {
   string commandStr = scrollBarName + " get";
   myTclEval(interp, commandStr.string_of());
   assert(2==sscanf(interp->result, "%f %f", &first, &last));
}

float moveScrollBar(Tcl_Interp *interp, const string &scrollBarName,
		   float newFirst) {
   // This routine adjusts a scrollbar to a new position.
   // The width of the scrollbar thumb stays unchanged.
   // If pinning is necessary, we make an effort to keep newFirst close to oldFirst

   float oldFirst, oldLast;
   getScrollBarValues(interp, scrollBarName, oldFirst, oldLast);
   const float oldDifference = oldLast-oldFirst;
   if (oldDifference > 1) {
      cerr << "moveScrollBar: I was handed a scrollbar with bad values [first=" << oldFirst << "; last=" << oldLast << "]...ignoring" << endl;
      return newFirst;
   }
   if (oldLast > 1.0) {
      cerr << "moveScrollBar: I was handed a scrollbar with bad last [first=" << oldFirst << "; last=" << oldLast << "]...ignoring" << endl;
      return newFirst;
   }

   newFirst = max(newFirst, 0.0f);

   assert(newFirst >= 0.0);

   float newLast = newFirst + oldDifference;
   if (newLast > 1.0) {
      newLast = 1.0;
      newFirst = 1.0 - oldDifference;
   }

   assert(newLast <= 1.0);
   assert(newFirst >= 0.0);

   char buffer[100];
   sprintf(buffer, "%f %f", newFirst, newLast);

   string commandStr = scrollBarName + " set " + buffer;
   myTclEval(interp, commandStr.string_of());

   return newFirst;
}

whereNodeRawPath graphical2RawPath(const whereNodeGraphicalPath &src) {
   whereNodeRawPath result;
   result.rigSize(src.getSize());

   for (int i=0; i < src.getSize(); i++)
      result[i].childnum = src[i].childnum;

   return result;
}

void printPath(const whereNodeRawPath &thePath) {
   cout << "Path: ";
   for (int pathlcv=0; pathlcv < thePath.getSize(); pathlcv++)
      cout << thePath[pathlcv].childnum << ' ';
   cout << endl;
}

bool nonSliderButtonCurrentlyPressed = false;
int  nonSliderButtonPressRegion;
Tk_TimerToken buttonAutoRepeatToken;

where4tree<resourceHandle> *nonSliderCurrentSubtree;
int nonSliderSubtreeCenter;
int nonSliderSubtreeTop;

void nonSliderCallMeOnButtonRelease(ClientData cd, XEvent *theEvent) {
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
   nonSliderCurrentSubtree->scrollBarClick(tc, nonSliderButtonPressRegion,
					   nonSliderSubtreeCenter,
					   nonSliderSubtreeTop,
					   true // redrawNow
					   );
      // 3 --> up-arrow; 4 --> down-arrow
      // 5 --> pageup;   6 --> pagedown

   // ...and reset the timer
   int repeatIntervalMillisecs;
   switch (nonSliderButtonPressRegion) {
      case 3: // up-arrow
      case 4: // down-arrow
         repeatIntervalMillisecs = 10;
         break;
      case 5: // pageup
      case 6: // pagedown
         repeatIntervalMillisecs = 250; // not so fast
         break;
      default:
         assert(false);
   }

   buttonAutoRepeatToken = Tk_CreateTimerHandler(repeatIntervalMillisecs, // millisecs
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

/* ********************************************************** */

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
