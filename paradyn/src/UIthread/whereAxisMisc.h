// whereAxisMisc.h
// Ariel Tamches

#ifndef _WHERE_AXIS_MISC_H_
#define _WHERE_AXIS_MISC_H_

#include <fstream.h>
#include <tclclean.h>
#include "simpSeq.h"
#include "where4tree.h"
#include "tkTools.h"

void nonSliderCallMeOnButtonRelease(ClientData cd, XEvent *theEvent);
void nonSliderButtonAutoRepeatCallback(ClientData cd);

void sliderCallMeOnMouseMotion(ClientData cd, XEvent *eventPtr);
void sliderCallMeOnButtonRelease(ClientData cd, XEvent *eventPtr);

#ifndef PARADYN
// this is only for the where axis test program:
string readUntilQuote(ifstream &is);
string readUntilSpace(ifstream &is);
string readItem(ifstream &is);
#endif

#endif
