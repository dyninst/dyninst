// barChartTcl.C

/* $Log: barChartTcl.C,v $
/* Revision 1.9  1996/01/10 02:24:25  tamches
/* dataFormatHasChangedCommand now takes in an arg
/* added getMetricColorNameCommand
/* hardcoded barColorNames here
/*
 * Revision 1.8  1995/09/22 19:25:29  tamches
 * removed warnings under g++ 2.7.0
 *
 * Revision 1.7  1994/11/06  10:24:59  tamches
 * minor cleanups (especially comments)
 *
 * Revision 1.6  1994/10/14  10:29:33  tamches
 * commented out diagnosted message when gracefully closing
 *
 * Revision 1.5  1994/10/13  00:52:38  tamches
 * Minor additions to support a new command related to sorting
 * of resources
 *
 * Revision 1.4  1994/10/10  23:08:44  tamches
 * preliminary changes on the way to swapping the x and y axes
 *
 * Revision 1.3  1994/10/10  14:36:17  tamches
 * fixed some resizing bugs
 *
 * Revision 1.2  1994/09/29  20:05:37  tamches
 * minor cvs fixes
 *
 * Revision 1.1  1994/09/29  19:51:38  tamches
 * initial implementation.
 * Receiving point for visi lib callback routines.  Pretty much
 * just calls the appropriate class member function in barChart.C
 *
*/

#include <iostream.h>

#include <tcl.h>
#include <tk.h>
#include "dg2.h" // for dataGrid[][]
#include "visi/h/visualization.h"
#include "barChart.h"

bool barChartIsValid = false;
   // set to true ** after ** barChart::barChart
   // until then, callbacks check this flag and do nothing

int Dg2NewDataCallback(int lastBucket) {
   if (barChartIsValid) {
      theBarChart->processNewData(lastBucket);
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}

int resizeCallbackCommand(ClientData, Tcl_Interp *, int argc, char **argv) {
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

int exposeCallbackCommand(ClientData, Tcl_Interp *, int, char **) {
   // called from barChart.tcl when it detects an expose; gives our C++ code
   // a chance to process the expose, too.

   if (barChartIsValid) {
      theBarChart->processExposeWindow();
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}

int resourcesAxisHasChangedCommand(ClientData, Tcl_Interp *, int argc, char **) {
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

int metricsAxisHasChangedCommand(ClientData, Tcl_Interp *, int argc, char **) {
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

int newScrollPositionCommand(ClientData, Tcl_Interp *, int, char **argv) {
   // called by tcl code when it's time to scroll the bars to a given value.
   // argument: new scrollbar position, in pixels.  Note: this is a bit old;
   // shouldn't we change to tk4.0's floating point representation???

   if (barChartIsValid) {
      int newPos = atoi(argv[1]);
      theBarChart->processNewScrollPosition(newPos);
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}

int dataFormatHasChangedCommand(ClientData, Tcl_Interp *, int argc, char **argv) {
   // rethink current vs. average vs. total
   assert(argc == 2);
   if (barChartIsValid) {
      char *dataFormatString = argv[1];
      if (0==strcmp(dataFormatString, "Instantaneous"))
         theBarChart->rethinkDataFormat(BarChart::Current);
      else if (0==strcmp(dataFormatString, "Average"))
         theBarChart->rethinkDataFormat(BarChart::Average);
      else if (0==strcmp(dataFormatString, "Sum"))
         theBarChart->rethinkDataFormat(BarChart::Total);
      else
         panic("barChart dataFormatHasChangedCommand: unrecognized argument");
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}

int rethinkIndirectResourcesCommand(ClientData, Tcl_Interp *, int, char **) {
   // rethink how things are sorted
   if (barChartIsValid) {
      theBarChart->rethinkIndirectResources();
      return TCL_OK;
   }
   else
      return TCL_ERROR;
}

int getMetricColorNameCommand(ClientData, Tcl_Interp *interp,
			      int argc, char **argv) {
   // argument: metric index
   assert(argc==2);
   unsigned index = atoi(argv[1]);

   const string &result = theBarChart->getMetricColorName(index);
   strcpy(interp->result, result.string_of());
   return TCL_OK;
}

int launchBarChartCommand(ClientData, Tcl_Interp *, int argc, char **argv) {
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
   
   if (argc != 7)
      panic("launchBarChartCommand() -- cannot create barchart (incorrect #args)");

   char *wname = argv[1];
   const int iNumMetrics   = atoi(argv[4]);
   const int iNumResources = atoi(argv[5]);
//   const bool iFlushFlag = (0==strcmp("1", argv[6]));

   // bar colors: (see /usr/lib/X11/rgb.txt)
   vector<string> barColorNames;
   barColorNames += "cornflower blue";
   barColorNames += "medium sea green";
   barColorNames += "hotpink";
   barColorNames += "chocolate";
   barColorNames += "orange";

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
