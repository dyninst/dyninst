// whereAxisMisc.h
// Ariel Tamches

#ifndef _WHERE_AXIS_MISC_H_
#define _WHERE_AXIS_MISC_H_

#include <tclclean.h>
#include "where4tree.h"

void myTclEval(Tcl_Interp *interp, const char *buffer);
void getScrollBarValues(Tcl_Interp *interp, const string &sbname,
			float &first, float &last);
float moveScrollBar(Tcl_Interp *interp, const string &sbname,
		    float newFirst);
   // returns _actual_ newFirst (usually newFirst, as passed in, but may be pinned)

whereNodeRawPath graphical2RawPath(const whereNodeGraphicalPath &src);
void printPath(const whereNodeRawPath &thePath);

void nonSliderCallMeOnButtonRelease(ClientData cd, XEvent *theEvent);
void nonSliderButtonAutoRepeatCallback(ClientData cd);

void sliderCallMeOnMouseMotion(ClientData cd, XEvent *eventPtr);
void sliderCallMeOnButtonRelease(ClientData cd, XEvent *eventPtr);

string readUntilQuote(ifstream &is);
string readUntilSpace(ifstream &is);
string readItem(ifstream &is);

#endif
