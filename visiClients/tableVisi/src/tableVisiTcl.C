// tableVisiTcl.C
// Ariel Tamches

/*
 * $Log: tableVisiTcl.C,v $
 * Revision 1.1  1995/11/04 00:48:25  tamches
 * First version of new table visi
 *
 */

#include <iostream.h>
#include "tclclean.h"
#include "tkclean.h"
#include "tableVisi.h"
#include "visi/h/visualization.h"

#include "tableVisiTcl.h"

/* ************************************************************* */

extern tableVisi *theTableVisi; // our main data structure
extern bool xsynch_flag;
unsigned currFormat=0; // 0 --> current; 1 --> averages; 2 --> totals

/* ************************************************************* */

void tableWhenIdleDrawRoutine(ClientData) {
   theTableVisi->draw(xsynch_flag);
}
tkInstallIdle tableDrawWhenIdle(&tableWhenIdleDrawRoutine);

/* ************************************************************* */

int Dg2NewDataCallback(int lastBucket) {
//   cout << "Dg2NewDataCallback (c++)" << endl;
   if (theTableVisi == NULL) { 
      cout << "Dg2NewDataCallback tableVisi: missed an early sample since not yet initialized" << endl;
      return TCL_OK;
   }

   const unsigned numMetrics = theTableVisi->getNumMetrics();
   const unsigned numFoci = theTableVisi->getNumFoci();

   for (unsigned metriclcv=0; metriclcv < numMetrics; metriclcv++) {
      visi_GridHistoArray &metricValues = dataGrid[metriclcv];
    
      for (unsigned focuslcv=0; focuslcv < numFoci; focuslcv++) {
         visi_GridCellHisto &theCell = metricValues[focuslcv];

         if (!theCell.Valid)
            theTableVisi->invalidateCell(metriclcv, focuslcv);
         else if (currFormat==0) // current values
            theTableVisi->setCellValidData(metriclcv, focuslcv,
					   theCell.Value(lastBucket));
         else if (currFormat==1) // average values
            theTableVisi->setCellValidData(metriclcv, focuslcv,
					   dataGrid.AggregateValue(metriclcv,
								   focuslcv));
         else if (currFormat==2) // total values
            theTableVisi->setCellValidData(metriclcv, focuslcv,
					   dataGrid.SumValue(metriclcv, focuslcv));
      }
   }

   tableDrawWhenIdle.install(NULL);
   return TCL_OK;
}

int Dg2AddMetricsCallback(int) {
   // completely rethink metrics and resources
   // very ugly; necessary because the visilib interface
   // is rather crude in this area.
   // Nevertheless, an improved version of this routine can avoid
   // rethinking everything, by making a pass over the dataGrid to
   // _manually_ compute what has actually changed.

   extern Tcl_Interp *mainInterp;
   theTableVisi->clearMetrics(mainInterp);
   unsigned newNumMetrics = dataGrid.NumMetrics();
   for (unsigned metriclcv=0; metriclcv < newNumMetrics; metriclcv++)
      theTableVisi->addMetric(dataGrid.MetricName(metriclcv),
			      dataGrid.MetricUnits(metriclcv));

   theTableVisi->clearFoci(mainInterp);
   unsigned newNumFoci = dataGrid.NumResources();
   for (unsigned focuslcv=0; focuslcv < newNumFoci; focuslcv++)
      theTableVisi->addFocus(dataGrid.ResourceName(focuslcv));

   // now update the sorting, as applicable
   char *sortMetricsStr = Tcl_GetVar(mainInterp, "sortMetrics", TCL_GLOBAL_ONLY);
   bool sortMetricsFlag = atoi(sortMetricsStr);
   if (sortMetricsFlag)
      theTableVisi->sortMetrics();
   else
      theTableVisi->unsortMetrics();

   char *sortFociStr = Tcl_GetVar(mainInterp, "sortFoci", TCL_GLOBAL_ONLY);
   bool sortFociFlag = atoi(sortFociStr);
   if (sortFociFlag)
      theTableVisi->sortFoci();
   else
      theTableVisi->unsortFoci();

   theTableVisi->resize(mainInterp);
   tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int Dg2Fold(int) {
   cout << "welcome to Dg2Fold" << endl;
   tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int Dg2InvalidMetricsOrResources(int) {
   cout << "welcome to Dg2InvalidMetricsOrResources" << endl;
//   myTclEval(MainInterp, "DgInvalidCallback");
   tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int Dg2PhaseNameCallback(int) {
   cout << "welcome to Dg2PhaseNameCallback" << endl;
//   myTclEval(MainInterp, "DgPhaseCallback");
   tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

/* ************************************************************* */

void dummyDeleteProc(ClientData) {}

int tableVisiNewVertScrollPositionCommand(ClientData, Tcl_Interp *interp,
					  int argc, char **argv) {
   // The arguments will be one of:
   // 1) moveto [fraction]
   // 2) scroll [num-units] unit   (num-units is always either -1 or 1)
   // 3) scroll [num-pages] page   (num-pages is always either -1 or 1)
   // we let processScrollCallback handle 'em

   float newFirst;
   bool anyChanges = processScrollCallback(interp, argc, argv,
					   ".vertScrollbar",
					   theTableVisi->get_offset_y(),
					   theTableVisi->get_total_cell_y_pix(),
					   theTableVisi->get_visible_y_pix(),
					   newFirst);
   
   if (anyChanges)
      anyChanges = theTableVisi->adjustVertSBOffset(interp, newFirst);

   if (anyChanges)
      tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int tableVisiNewHorizScrollPositionCommand(ClientData, Tcl_Interp *interp,
					   int argc, char **argv) {
   // The arguments will be one of:
   // 1) moveto [fraction]
   // 2) scroll [num-units] unit   (num-units is always either -1 or 1)
   // 3) scroll [num-pages] page   (num-pages is always either -1 or 1)
   // we let processScrollCallback handle 'em

   float newFirst;
   bool anyChanges = processScrollCallback(interp, argc, argv,
					   ".horizScrollbar",
					   theTableVisi->get_offset_x(),
					   theTableVisi->get_total_cell_x_pix(),
					   theTableVisi->get_visible_x_pix(),
					   newFirst);
   
   if (anyChanges)
      anyChanges = theTableVisi->adjustHorizSBOffset(interp, newFirst);

   if (anyChanges)
      tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int tableVisiConfigureCommand(ClientData, Tcl_Interp *interp,
			      int, char **) {
   assert(theTableVisi->tryFirst());
   theTableVisi->resize(interp); // adjusts scrollbar
   tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int tableVisiExposeCommand(ClientData, Tcl_Interp *,
			   int, char **) {
   assert(theTableVisi->tryFirst());
   tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int updateNamesCommand(ClientData, Tcl_Interp *interp,
		       int argc, char **argv) {
   assert(argc==2);
//   cout << "welcome to updateNamesCommand: arg is " << argv[1] << endl;

   assert(theTableVisi->tryFirst());

   if (theTableVisi->setFocusNameMode(interp, (bool)atoi(argv[1])))
      tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int sigFigChangeCommand(ClientData, Tcl_Interp *interp,
			int argc, char **argv) {
   assert(argc == 2);
   int newNumSigFigs = atoi(argv[1]);

   assert(theTableVisi->tryFirst());

   if (theTableVisi->setSigFigs(newNumSigFigs))
      tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int sortMetricsCommand(ClientData, Tcl_Interp *interp,
		       int argc, char **argv) {
   theTableVisi->sortMetrics();
   if (theTableVisi->tryFirst())
      tableDrawWhenIdle.install(NULL);
   return TCL_OK;
}

int unsortMetricsCommand(ClientData, Tcl_Interp *interp,
		       int argc, char **argv) {
   theTableVisi->unsortMetrics();
   if (theTableVisi->tryFirst())
      tableDrawWhenIdle.install(NULL);
   return TCL_OK;
}

int sortFociCommand(ClientData, Tcl_Interp *interp,
		       int argc, char **argv) {
   theTableVisi->sortFoci();
   if (theTableVisi->tryFirst())
      tableDrawWhenIdle.install(NULL);
   return TCL_OK;
}

int unsortFociCommand(ClientData, Tcl_Interp *interp,
		       int argc, char **argv) {
   theTableVisi->unsortFoci();
   if (theTableVisi->tryFirst())
      tableDrawWhenIdle.install(NULL);
   return TCL_OK;
}

int formatChangedCommand(ClientData, Tcl_Interp *interp,
			 int argc, char **argv) {
   char *dataFormatStr = Tcl_GetVar(interp, "DataFormat", TCL_GLOBAL_ONLY);
   if (dataFormatStr == NULL)
      tclpanic(interp, "could not find DataFormat tcl vrble");

   int dataFormat = atoi(dataFormatStr);
   currFormat = dataFormat;

   if (theTableVisi->tryFirst())
      tableDrawWhenIdle.install(NULL);
   
   return TCL_OK;
}

void installTableVisiCommands(Tcl_Interp *interp) {
   Tcl_CreateCommand(interp, "horizScrollbarNewScrollPosition",
		     tableVisiNewHorizScrollPositionCommand,
		     NULL, dummyDeleteProc);
   Tcl_CreateCommand(interp, "vertScrollbarNewScrollPosition",
		     tableVisiNewVertScrollPositionCommand,
		     NULL, dummyDeleteProc);

   Tcl_CreateCommand(interp, "tableVisiConfigure",
		     tableVisiConfigureCommand,
		     NULL, dummyDeleteProc);
   Tcl_CreateCommand(interp, "tableVisiExpose",
		     tableVisiExposeCommand,
		     NULL, dummyDeleteProc);

   Tcl_CreateCommand(interp, "updateNames",
		     updateNamesCommand,
		     NULL, dummyDeleteProc);

   Tcl_CreateCommand(interp, "sigFigChange",
		     sigFigChangeCommand,
		     NULL, dummyDeleteProc);

   Tcl_CreateCommand(interp, "sortMetrics",
		     sortMetricsCommand,
		     NULL, dummyDeleteProc);
   Tcl_CreateCommand(interp, "unsortMetrics",
		     unsortMetricsCommand,
		     NULL, dummyDeleteProc);

   Tcl_CreateCommand(interp, "sortFoci",
		     sortFociCommand,
		     NULL, dummyDeleteProc);
   Tcl_CreateCommand(interp, "unsortFoci",
		     unsortFociCommand,
		     NULL, dummyDeleteProc);

   Tcl_CreateCommand(interp, "formatChanged",
		     formatChangedCommand,
		     NULL, dummyDeleteProc);
}

void unInstallTableVisiCommands(Tcl_Interp *interp) {
   Tcl_DeleteCommand(interp, "formatChanged");

   Tcl_DeleteCommand(interp, "sortMetrics");
   Tcl_DeleteCommand(interp, "unsortMetrics");
   Tcl_DeleteCommand(interp, "sortFoci");
   Tcl_DeleteCommand(interp, "unsortFoci");

   Tcl_DeleteCommand(interp, "sigFigChange");
   Tcl_DeleteCommand(interp, "tableVisiConfigure");
   Tcl_DeleteCommand(interp, "tableVisiExpose");
   Tcl_DeleteCommand(interp, "whereAxisNewHorizScrollPosition");
   Tcl_DeleteCommand(interp, "whereAxisNewVertScrollPosition");
}
