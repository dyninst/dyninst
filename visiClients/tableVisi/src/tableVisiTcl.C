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

// $Id: tableVisiTcl.C,v 1.20 2003/06/20 02:23:24 pcroth Exp $

#include <iostream.h>

#include "common/h/headers.h"
#include "pdutil/h/pdsocket.h"
#include "visi/h/visualization.h"

#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

#include "tableVisi.h"

#include "tableVisiTcl.h"
#include "visiClients/auxiliary/h/Export.h"

/* ************************************************************* */

extern tableVisi *theTableVisi; // our main data structure
extern Tcl_Interp* mainInterp;
extern bool xsynch_flag;
unsigned currFormat=0; // 0 --> current; 1 --> averages; 2 --> totals
bool profileMode = false;

/* ************************************************************* */

void tableWhenIdleDrawRoutine(ClientData) {
   if (theTableVisi->tryFirst())
      theTableVisi->draw(xsynch_flag);
}
tkInstallIdle tableDrawWhenIdle(&tableWhenIdleDrawRoutine);

/* ************************************************************* */

void updatePhaseLabelIfFirstTime() {
   static bool firstTime = true;

   if (!firstTime)
      return;

   int phaseHandle = visi_GetMyPhaseHandle();
   if (phaseHandle < -1)
      return; // sorry, not yet defined

   const char *phaseName = visi_GetMyPhaseName();
   if (phaseName == NULL) {
      // ugh; we have a current phase, but the name isn't yet known
      myTclEval(mainInterp, string(".phasename config -text \"Phase: Current Phase\""));
      return; // return w/o setting firstTime to false
   }

   // success
   string commandStr = string(".phasename config -text \"Phase: ") + phaseName + "\"";
   myTclEval(mainInterp, commandStr);

   firstTime = false;
}

/* ************************************************************* */

int Dg2NewDataCallback(int) {
   // This callback implies that new data has arrived for each valid metric/focus
   // pair; hence, it's time to update the screen!

   if (theTableVisi == NULL) { 
#ifdef TABLE_DEBUG
      cout << "Dg2NewDataCallback tableVisi: missed an early sample since not yet initialized" << endl;
#endif
      return TCL_OK;
   }

   updatePhaseLabelIfFirstTime();

   // This should give the number of _enabled_ metrics & foci:
   const unsigned numMetrics = theTableVisi->getNumMetrics();
   const unsigned numFoci    = theTableVisi->getNumFoci();

   // Loop through the enabled metrics:
   for (unsigned metriclcv=0; metriclcv < numMetrics; metriclcv++) {
      const unsigned metId = theTableVisi->metric2MetId(metriclcv);
    
      // Loop through the enabled foci
      for (unsigned focuslcv=0; focuslcv < numFoci; focuslcv++) {
         const unsigned resId = theTableVisi->focus2ResId(focuslcv);
          
         // const unsigned bucketToUse = lastBucket;
         const unsigned bucketToUse = visi_LastBucketFilled(metId,resId);

         if (!visi_Valid(metId,resId))
            theTableVisi->invalidateCell(metriclcv, focuslcv);
         else if (currFormat==0) // current values
            theTableVisi->setCellValidData(metriclcv, focuslcv,
				visi_DataValue(metId,resId,bucketToUse));
         else if (currFormat==1) // average values
            theTableVisi->setCellValidData(metriclcv, focuslcv,
					   visi_AverageValue(metId, resId));
         else if (currFormat==2) // total values
            theTableVisi->setCellValidData(metriclcv, focuslcv,
					   visi_SumValue(metId, resId));
      }
   }

   // Now, if we are sorting foci by values (profile-table-mode), then it's
   // time to resort!
   if (profileMode)
      if (!theTableVisi->sortFociByValues()) {
         // could not sort because not exactly 1 column/metric was selected
#ifdef TABLE_DEBUG
         cout << "table visi; cannot sort foci by value (profile mode) when not exactly 1 column is selected" << endl;
#endif
      }

   tableDrawWhenIdle.install(NULL);
   return TCL_OK;
}

int Dg2PhaseDataCallback(int) {
   myTclEval(mainInterp, "DgPhaseDataCallback");

   return TCL_OK;
}

int Dg2AddMetricsCallback(int) {
   // completely rethink metrics and resources
   // very ugly; necessary because the visilib interface
   // is rather crude in this area.
   // Nevertheless, an improved version of this routine can avoid
   // rethinking everything, by making a pass over the dataGrid to
   // _manually_ compute what has actually changed.

   updatePhaseLabelIfFirstTime();

   theTableVisi->clearMetrics(mainInterp);

   unsigned newNumMetrics = visi_NumMetrics();
      // not necessarily the correct value; some metric(s) may be disabled

   for (unsigned metriclcv=0; metriclcv < newNumMetrics; metriclcv++) {
      // Is this metric enabled?  Boils down to "is this metric enabled for
      // at least one focus?"
      bool enabled = false;
      for (int focuslcv=0; focuslcv < visi_NumResources(); focuslcv++) {
         if (visi_Enabled(metriclcv,focuslcv)) {
            enabled = true;
	    break;
	 }
      }
      if (!enabled)
         continue;

      if (currFormat==0) // current values
         theTableVisi->addMetric(metriclcv,
				 visi_MetricName(metriclcv),
				 visi_MetricLabel(metriclcv));
      else if (currFormat==1) // average values
         theTableVisi->addMetric(metriclcv,
				 visi_MetricName(metriclcv),
				 visi_MetricAveLabel(metriclcv));
      else if (currFormat==2) // total values
         theTableVisi->addMetric(metriclcv,
				 visi_MetricName(metriclcv),
				 visi_MetricSumLabel(metriclcv));
   }

   theTableVisi->clearFoci(mainInterp);

   unsigned newNumFoci = visi_NumResources();
      // not necessarily the correct value; foci may be disabled

   for (unsigned focuslcv=0; focuslcv < newNumFoci; focuslcv++) {
      // is this focus enabled?
      bool enabled = false;
      for (unsigned metriclcv=0; metriclcv < newNumMetrics; metriclcv++)
         if (visi_Enabled(metriclcv,focuslcv)) {
	    enabled = true;
	    break;
	 }

      if (!enabled) {
#ifdef TABLE_DEBUG
         cout << "table: the focus " << visi_ResourceName(focuslcv) << " isn't enabled...not adding" << endl;
#endif
         continue;
      }

      theTableVisi->addFocus(focuslcv, visi_ResourceName(focuslcv));
   }

   // now update the sorting, as applicable
   const char *sortMetricsStr = Tcl_GetVar(mainInterp, "sortMetrics", TCL_GLOBAL_ONLY);
   int sortMetricsFlag = atoi(sortMetricsStr);
   if (sortMetricsFlag==0)
      theTableVisi->sortMetrics();
   else
      theTableVisi->unsortMetrics();

   const char *sortFociStr = Tcl_GetVar(mainInterp, "sortFoci", TCL_GLOBAL_ONLY);
   int sortFociFlag = atoi(sortFociStr);
   if (sortFociFlag==0)
      theTableVisi->sortFoci();
   else if (sortFociFlag==1)
      theTableVisi->unsortFoci();
   else
      theTableVisi->sortFociByValues();

   theTableVisi->resize(mainInterp);
   tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}


//
// Dg2ParadynExitedCallback
//
// Called when connection between Paradyn and visi is broken.
// We use it to shut down the visi gracefully.
//
int Dg2ParadynExitedCallback(int)
{
    // shut the GUI down gracefully
    //
    // We don't use myTclEval here because we want to
    // avoid calling exit().  There is a problem with
    // the ordering of Tk destroying its windows and
    // the destruction of Paradyn's static objects
    //
    Tcl_Eval( mainInterp, "GracefulShutdown" );

    return TCL_OK;
}


/* ************************************************************* */

void dummyDeleteProc(ClientData) {}

int tableVisiNewVertScrollPositionCommand(ClientData, Tcl_Interp *interp,
					  int argc, TCLCONST char **argv) {
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
					   int argc, TCLCONST char **argv) {
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
			      int, TCLCONST char **) {
   assert(theTableVisi->tryFirst());
   theTableVisi->resize(interp); // adjusts scrollbar
   tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int tableVisiExposeCommand(ClientData, Tcl_Interp *,
			   int, TCLCONST char **) {
   assert(theTableVisi->tryFirst());
   tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

void tableVisiUpdateDeleteMenu(Tcl_Interp *interp) {
   const string &menuPrefix = ".top.left.menubar.acts.m entryconfigure 2 ";
   if (theTableVisi->getSelection()==tableVisi::none)
      myTclEval(interp, menuPrefix + "-label \"Delete Selected Entry\" -state disabled");
   else if (theTableVisi->getSelection()==tableVisi::cell)
      myTclEval(interp, menuPrefix + "-label \"Delete Selected Cell\" -state normal -command DeleteSelection");
   else if (theTableVisi->getSelection()==tableVisi::rowOnly)
      myTclEval(interp, menuPrefix + "-label \"Delete Selected Focus (entire row)\" -state normal -command DeleteSelection");
   else if (theTableVisi->getSelection()==tableVisi::colOnly)
      myTclEval(interp, menuPrefix + "-label \"Delete Selected Metric (entire column)\" -state normal -command DeleteSelection ");
   else
      assert(false);
}

int tableVisiClickCommand(ClientData, Tcl_Interp *interp,
			  int argc, TCLCONST char **argv) {
   assert(theTableVisi->tryFirst());
   
   assert(argc == 3);
   int x = atoi(argv[1]);
   int y = atoi(argv[2]);

   if (!theTableVisi->processClick(x, y))
      return TCL_OK;

   // Now we need to update the delete menu item of the actions menu.
   tableVisiUpdateDeleteMenu(interp);

   tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int tableVisiDeleteSelectionCommand(ClientData, Tcl_Interp *interp,
				    int, TCLCONST char **) {
   assert(theTableVisi->tryFirst());

   switch(theTableVisi->getSelection()) {
      case tableVisi::none:
         assert(false);
      case tableVisi::cell: {
         unsigned theMetId = theTableVisi->getSelectedMetId();
         unsigned theResId = theTableVisi->getSelectedResId();
#ifdef TABLE_DEBUG
         cout << "tableVisiDeleteSelectionCommand: about to delete a cell..." << endl;
         cout << "...whose resource name is " << visi_ResourceName(theResId) << endl;
         cout << "...and whose metric name is " << visi_MetricName(theMetId) << endl;
#endif
         visi_StopMetRes(theMetId, theResId);

         unsigned theRow = theTableVisi->getSelectedRow();
         unsigned theCol = theTableVisi->getSelectedCol();
         theTableVisi->invalidateCell(theCol, theRow);
         break;
      }
      case tableVisi::rowOnly: {
         unsigned theResId = theTableVisi->getSelectedResId();
#ifdef TABLE_DEBUG
         cout << "tableVisiDeleteSelectionCommand: about to delete a row..." << endl;
         cout << "...whose resource name is " << visi_ResourceName(theResId) << endl;
#endif
         for (unsigned collcv=0; collcv < theTableVisi->getNumMetrics(); collcv++) {
            unsigned theMetId = theTableVisi->col2MetId(collcv);
            visi_StopMetRes(theMetId, theResId);
	 }

         unsigned theRow = theTableVisi->getSelectedRow();
         theTableVisi->deleteFocus(theRow);
         break;
      }
      case tableVisi::colOnly: {
         unsigned theMetId = theTableVisi->getSelectedMetId();
#ifdef TABLE_DEBUG
         cout << "tableVisiDeleteSelectionCommand: about to delete a column..." << endl;
         cout << "...whose metric name is " << MetricName(theMetId) << endl;
#endif
         for (unsigned rowlcv=0; rowlcv < theTableVisi->getNumFoci(); rowlcv++) {
            unsigned theResId = theTableVisi->row2ResId(rowlcv);
            visi_StopMetRes(theMetId, theResId);
	 }

         unsigned theCol = theTableVisi->getSelectedCol();
         theTableVisi->deleteMetric(theCol);
         break;
      }
      default:
         break;
   }

   tableVisiUpdateDeleteMenu(interp);

   theTableVisi->resize(interp);

   tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int updateNamesCommand(ClientData, Tcl_Interp *interp,
		       int argc, TCLCONST char **argv) {
   assert(argc==2);

   assert(theTableVisi->tryFirst());

   if (theTableVisi->setFocusNameMode(interp, (atoi(argv[1])!=0)))
      tableDrawWhenIdle.install(NULL);

   return TCL_OK;
}

int sigFigChangeCommand(ClientData, Tcl_Interp *interp,
			int argc, TCLCONST char **) {
   assert(argc == 1);

   const char *sigFigStr = Tcl_GetVar(interp, "SignificantDigits", TCL_GLOBAL_ONLY);
   if (sigFigStr == NULL)
      tclpanic(interp, "could not find vrble SignificantDigits");

   int newNumSigFigs = atoi(sigFigStr);

   assert(theTableVisi->tryFirst());

   if (theTableVisi->setSigFigs(newNumSigFigs)) {
      // resizing may have occurred
      theTableVisi->resize(interp);
      tableDrawWhenIdle.install(NULL);
   }

   return TCL_OK;
}

int sortMetricsCommand(ClientData, Tcl_Interp *,
		       int, TCLCONST char **) {
   theTableVisi->sortMetrics();
   if (theTableVisi->tryFirst())
      tableDrawWhenIdle.install(NULL);
   return TCL_OK;
}

int unsortMetricsCommand(ClientData, Tcl_Interp *,
		       int, TCLCONST char **) {
   theTableVisi->unsortMetrics();
   if (theTableVisi->tryFirst())
      tableDrawWhenIdle.install(NULL);
   return TCL_OK;
}

int sortFociCommand(ClientData, Tcl_Interp *,
		    int, TCLCONST char **) {
   profileMode = false;
   theTableVisi->sortFoci();
   if (theTableVisi->tryFirst())
      tableDrawWhenIdle.install(NULL);
   return TCL_OK;
}

int sortFociByValuesCommand(ClientData, Tcl_Interp *,
			    int, TCLCONST char **) {
   profileMode = true;
   theTableVisi->sortFociByValues();
   if (theTableVisi->tryFirst())
      tableDrawWhenIdle.install(NULL);
   return TCL_OK;
}

int unsortFociCommand(ClientData, Tcl_Interp *,
		       int, TCLCONST char **) {
   profileMode = false;
   theTableVisi->unsortFoci();
   if (theTableVisi->tryFirst())
      tableDrawWhenIdle.install(NULL);
   return TCL_OK;
}

int formatChangedCommand(ClientData, Tcl_Interp *interp,
			 int, TCLCONST char **) {
   const char *dataFormatStr = Tcl_GetVar(interp, "DataFormat", TCL_GLOBAL_ONLY);
   if (dataFormatStr == NULL)
      tclpanic(interp, "could not find DataFormat tcl vrble");

   int dataFormat = atoi(dataFormatStr);
   currFormat = dataFormat;

   for (int i =0; i < visi_NumMetrics(); i++){
       if (currFormat==0) // current values
            theTableVisi->changeUnitsLabel(i, visi_MetricLabel(i));
       else if (currFormat==1) // average values
            theTableVisi->changeUnitsLabel(i, visi_MetricAveLabel(i));
       else // total values
            theTableVisi->changeUnitsLabel(i, visi_MetricSumLabel(i));
   }

   Dg2AddMetricsCallback(0);
   Dg2NewDataCallback(0);

   return TCL_OK;
}



int tableVisiDestroyCommand( ClientData,
                             Tcl_Interp* /*interp*/,
			                 int /*argc*/,
                             TCLCONST char* /*argv*/[])
{
    // release resources that must be
    // gone before we destroy the GUI
    theTableVisi->ReleaseResources();
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
   Tcl_CreateCommand(interp, "tableVisiClick",
		     tableVisiClickCommand,
		     NULL, dummyDeleteProc);

   Tcl_CreateCommand(interp, "DeleteSelection",
		     tableVisiDeleteSelectionCommand,
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
   Tcl_CreateCommand(interp, "sortFociByValues",
		     sortFociByValuesCommand,
		     NULL, dummyDeleteProc);
   Tcl_CreateCommand(interp, "unsortFoci",
		     unsortFociCommand,
		     NULL, dummyDeleteProc);

   Tcl_CreateCommand(interp, "formatChanged",
		     formatChangedCommand,
		     NULL, dummyDeleteProc);

   Tcl_CreateCommand(interp,
                     "tableVisiDestroyHook",
		             tableVisiDestroyCommand,
		             NULL,
                     dummyDeleteProc);

   //Create commands for export functionality
   Tcl_CreateObjCommand(interp, "get_subscribed_mrpairs",
		       get_subscribed_mrpairs, NULL, NULL);
   Tcl_CreateObjCommand(interp, "DoExport", DoExport, NULL, NULL);
}
