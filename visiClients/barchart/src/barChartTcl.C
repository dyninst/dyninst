/*
 * Copyright (c) 1996-1999 Barton P. Miller
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

// barChartTcl.C

/* $Id: barChartTcl.C,v 1.28 2003/07/18 15:45:24 schendel Exp $ */

#include <iostream>

#include "common/h/Vector.h"
#include "common/h/String.h"

#include "common/h/headers.h"
#include "pdutil/h/pdsocket.h"

#include "tcl.h"
#include "tk.h"
#include "dg2.h" // for dataGrid[][]
#include "visi/h/visualization.h"
#include "barChart.h"
#include "barChartUtil.h"

bool barChartIsValid = false;
   // set to true ** after ** barChart::barChart
   // until then, callbacks check this flag and do nothing

/* ************************************************************* */

void updatePhaseLabelIfFirstTime() {
   static bool firstTime = true;

   if (!firstTime)
      return;

   int phaseHandle = visi_GetMyPhaseHandle();
   if (phaseHandle < -1)
      return; // sorry, not yet defined

   extern Tcl_Interp *MainInterp;
   const char *phaseName = visi_GetMyPhaseName();
   if (phaseName == NULL) {
      // ugh; we have a current phase, but the name isn't yet known
      myTclEval(MainInterp, pdstring(".bargrph.phaseName config -text \"Phase: Current Phase\""));
      return; // return w/o setting firstTime to false
   }

   // success
   pdstring commandStr = pdstring(".bargrph.phaseName config -text \"Phase: ") + phaseName + "\"";
   myTclEval(MainInterp, commandStr);

   firstTime = false;
}

/* ************************************************************* */

int Dg2AddMetricsCallback(int) {
   updatePhaseLabelIfFirstTime();
   
   myTclEval(MainInterp, "DgConfigCallback");

   // if necessary, the tcl program will call xAxisHasChanged and/or
   // yAxisHasChanged, which are commands we implement in barChart.C.
   // We take action then.

   return TCL_OK;
}

int Dg2Fold(int) {
   myTclEval(MainInterp, "DgFoldCallback");
   return TCL_OK;
}

int Dg2InvalidMetricsOrResources(int) {
   myTclEval(MainInterp, "DgInvalidCallback");
   return TCL_OK;
}

int Dg2PhaseNameCallback(int) {
   myTclEval(MainInterp, "DgPhaseCallback");
   return TCL_OK;
}

int Dg2NewDataCallback(int lastBucket) {
   if (barChartIsValid) {
      updatePhaseLabelIfFirstTime();
      theBarChart->processNewData(lastBucket);
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}


// Dg2ParadynExitedCallback
// Function called when Paradyn has exited, or
// when we've failed to read off our Paradyn connection
int Dg2ParadynExitedCallback(int)
{
    // gracefully destroy the window
    // note that we don't use myTclEval here
    // because we want to avoid calling exit()
    // before destroying the Tk windows
    Tcl_Eval( MainInterp, "GracefulClose" );
    return TCL_OK;
}

int resizeCallbackCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char **argv) {
   // called from barChart.tcl when it detects a resize; gives our C++ code
   // a chance to process the resize, too.

   // params: new width, new height
   if (barChartIsValid && argc==3) {
      theBarChart->processResizeWindow(atoi(argv[1]), atoi(argv[2]));
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}

int exposeCallbackCommand(ClientData, Tcl_Interp *, int, TCLCONST char **) {
   // called from barChart.tcl when it detects an expose; gives our C++ code
   // a chance to process the expose, too.

   if (barChartIsValid) {
      theBarChart->processExposeWindow();
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}

int resourcesAxisHasChangedCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char **) {
   // called from barChart.tcl when the x-axis layout has changed due to resize,
   // insertion/deletion, etc; gives our C++ code a chance to update its
   // internal structures.

   // arg: new width

   if (barChartIsValid && argc==2) {
      theBarChart->RethinkMetricsAndResources();
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}

int metricsAxisHasChangedCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char **) {
   // called from barChart.tcl when the y-axis layout has changed due to resize,
   // insertion/deletion, etc; gives our C++ code a chance to update its
   // internal structures.

   // argument: new height (but currently unused)

   if (barChartIsValid && argc==2) {
      theBarChart->RethinkMetricsAndResources();
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}

int newScrollPositionCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char **argv) {
   // called by tcl code when it's time to scroll the bars to a given value.
   // argument: new first-visible-pixel.

   if (barChartIsValid && argc==2) {
      int newPos = atoi(argv[1]);
      theBarChart->processNewScrollPosition(newPos);
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}

int dataFormatHasChangedCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char **argv) {
   // rethink current vs. average vs. total
   assert(argc == 2);
   if (barChartIsValid) {
      const char *dataFormatString = argv[1];
      if (0==strcmp(dataFormatString, "Instantaneous"))
         theBarChart->rethinkDataFormat(BarChart::Current);
      else if (0==strcmp(dataFormatString, "Average"))
         theBarChart->rethinkDataFormat(BarChart::Average);
      else if (0==strcmp(dataFormatString, "Sum"))
         theBarChart->rethinkDataFormat(BarChart::Total);
      else
         Tcl_Panic("barChart dataFormatHasChangedCommand: unrecognized argument", NULL);
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}

int rethinkIndirectResourcesCommand(ClientData, Tcl_Interp *, int, TCLCONST char **) {
   // rethink how things are sorted
   if (barChartIsValid) {
      theBarChart->rethinkIndirectResources();
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}

int getMetricColorNameCommand(ClientData, Tcl_Interp *interp,
			      int argc, TCLCONST char **argv) {
   // argument: metric index
   assert(argc==2);
   unsigned index = atoi(argv[1]);

   const pdstring &result = theBarChart->getMetricColorName(index);
   Tcl_SetObjResult(interp, Tcl_NewStringObj(result.c_str(), -1));
   return TCL_OK;
}

int long2shortFocusNameCommand(ClientData, Tcl_Interp *interp, int argc, TCLCONST char **argv) {
	unsigned componentlcv;
   assert(argc==2);
   const char *longName = argv[1];

   // NOTE: most of this code is borrowed/stolen from tableVisi's tvFocus::tvFocus
   //       routine.

   if (0==strcmp(longName, "Whole Program")) {
      // no change
      Tcl_SetObjResult(interp, Tcl_NewStringObj(longName, -1));
      return TCL_OK;
   }

   // Step 1: split up into components; 1 per resource hierarchy
   pdvector<pdstring> components;
   const char *ptr = longName;

   while (*ptr != '\0') {
      // begin a new component; collect upto & including the first seen comma
      char buffer[200];
      char *bufferPtr = &buffer[0];
      do {
         *bufferPtr++ = *ptr++;
      } while (*ptr != ',' && *ptr != '\0');

      if (*ptr == ',')
         *bufferPtr++ = *ptr++;

      *bufferPtr = '\0';

      components += pdstring(buffer);
   }

   // Step 2: for each component, strip off all upto and including
   //         the last '/'
   for (componentlcv=0; componentlcv < components.size(); componentlcv++) {
      const pdstring &oldComponentString = components[componentlcv];

      char *ptr = strrchr(oldComponentString.c_str(), '/');
      if (ptr == NULL)
         cerr << "tableVisi: could not find / in component " << oldComponentString << endl;
      else if (ptr+1 == '\0')
         cerr << "tableVisi: there was nothing after / in component " << oldComponentString << endl;
      else
         components[componentlcv] = pdstring(ptr+1);
   }

   // Step 3: combine the components
   pdstring theShortName;
   for (componentlcv=0; componentlcv < components.size(); componentlcv++)
      theShortName += components[componentlcv];

   // Step 4: pull it all together:
   Tcl_SetObjResult(interp, Tcl_NewStringObj(theShortName.c_str(), -1));
   return TCL_OK;
}

int newMetricMaxValCallbackCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char **argv) {
   assert(theBarChart);
   assert(argc==3);
   theBarChart->setMetricNewMaxLL(atoi(argv[1]), atof(argv[2]));
   return TCL_OK;
}

int launchBarChartCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char **argv) {
   // called just once to fix some information needed by drawBarsCommand, especially
   // the (sub-)window in which to draw.

   // parameters:
   // 1) window name (tk-style; e.g. ".top.middle.bar") of the area in which the bars
   //    are drawn.
   // 2) do you want double-buffering? ("doublebuffer" or "nodoublebuffer")
   // 3) do you want no-flicker?       ("noflicker" or "flicker")
   //       [you automatically get noflicker with doublebuffer]
   // 4) initial numMetrics
   // 5) initial numResources
   // 6) flush flag (0 or 1); use 1 during debugging only

   // cout << "Welcome to launchBarChartCommand()" << endl;
   
   if (argc != 4)
      Tcl_Panic("launchBarChartCommand() -- cannot create barchart (incorrect #args)", NULL);

   TCLCONST char *wname = argv[1];
   const int iNumMetrics   = atoi(argv[2]);
   const int iNumResources = atoi(argv[3]);

   // bar colors: (see /usr/lib/X11/rgb.txt)
   pdvector<pdstring> barColorNames;
   barColorNames += "mediumslateblue";
   barColorNames += "hotpink";
//   barColorNames += "chartreuse"; // too bright
   barColorNames += "#3aa041"; // a type of green not far from mediumseagreen
//   barColorNames += "orange";
//   barColorNames += "lightsalmon"; // text part is too unreadable on grey
   barColorNames += "salmon";
   barColorNames += "chocolate";

   theBarChart = new BarChart(wname, iNumMetrics, iNumResources, barColorNames);
   assert(theBarChart);

   barChartIsValid = true;
   return TCL_OK;
}

void deleteLaunchBarChartCommand(ClientData) {
   // cout << "Gracefully closing down barchart..." << endl;

   barChartIsValid = false; // important!
   delete theBarChart;
}

void deleteDummyProc(ClientData) { }
   // do-nothing routine to be called when a command is deleted that
   // doesn't require closing down...
