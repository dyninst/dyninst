#ifndef _visualization_h
#define _visualization_h
/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

/* $Log: visualization.h,v $
/* Revision 1.16  1996/01/05 20:02:28  newhall
/* changed parameters to showErrorVisiCallback, so that visilib users are
/* not forced into using our string class
/*
 * Revision 1.15  1995/12/20 20:19:31  newhall
 * removed matherr.h
 *
 * Revision 1.14  1995/12/20 18:35:02  newhall
 * including matherr.h so that it does not need to be included by visis
 *
 * Revision 1.13  1995/12/18 17:22:02  naim
 * Adding function showErrorVisiCallback to display error messages from
 * visis - naim
 *
 * Revision 1.12  1995/12/15  20:15:12  naim
 * Adding call back function to display error messages from visis - naim
 *
 * Revision 1.11  1995/09/18  18:26:00  newhall
 * updated test subdirectory, added visilib routine GetMetRes()
 *
 * Revision 1.10  1995/08/01  01:58:46  newhall
 * changes relating to phase interface stuff
 *
 * Revision 1.9  1995/02/26  01:59:31  newhall
 * added phase interface functions
 *
 * Revision 1.8  1994/10/13  15:38:52  newhall
 * QuitVisi added
 *
 * Revision 1.7  1994/09/25  01:58:16  newhall
 * changed interface definitions to work for new version of igen
 * changed AddMetricsResources def. to take array of metric/focus pairs
 *
 * Revision 1.6  1994/08/13  20:34:07  newhall
 * removed all code associated with class visi_MRList
 *
 * Revision 1.5  1994/05/23  20:55:21  newhall
 * To visi_GridCellHisto class: added deleted flag, SumValue
 * method function, and fixed AggregateValue method function
 *
 * Revision 1.4  1994/05/11  17:11:10  newhall
 * changed data values from double to float
 *
 * Revision 1.3  1994/04/13  21:23:15  newhall
 * added routines: GetMetsRes, StopMetRes, NamePhase
 *
 * Revision 1.2  1994/03/17  05:13:41  newhall
 * change callback type
 *
 * Revision 1.1  1994/03/14  20:27:33  newhall
 * changed visi subdirectory structure
 *  */ 
/////////////////////////////////////////////////////////
//  This file should be included in all visualizations.
//  It contains definitions for all the Paradyn visi
//  interface routines.  
/////////////////////////////////////////////////////////

#include "datagrid.h"
#include "visiTypes.h"
#include "util/h/makenan.h"

#define FILETABLESIZE  64
#define EVENTSIZE      FOLD+1
#define MAXSTRINGSIZE  16*1024

//
// global variables assoc. with histo datagrid
//   dataGrid: use visi_DataGrid method functions to access current
//             data values and information about metrics and resources 
//   metricList, resourceList: use visi_MRList method functions to 
//             access current metric and resource lists (changes to this
//	       list do not affect the datagrid) primarly, these are used
//             to obtain and modify the parameters to GetMetsRes()
//   
extern visi_DataGrid  dataGrid;   
extern int            lastBucketSent;

//
// file descriptor array: 1st file desc. is assoc. w/ Paradyn
// remaining file descriptors are assigned by user when user
// uses main routine provided by Paradyn
//
extern int fileDesc[FILETABLESIZE];

//
// array of callback routines assoc. with file descriptors when
// user uses main routine provided by Paradyn
//
extern int (*fileDescCallbacks[FILETABLESIZE])();

//
// array of procedure pointers for callback routines assoc.
// with paradyn events  (ex. DATAVALUES,INVALIDMETRICSRESOURCES...)
// events types are defined in visiTypes.h
//
extern int (*eventCallbacks[EVENTSIZE])(int);

//
// callback associated with paradyn-visualization interface routines
//
extern int visi_callback();

/////////////////////////////////////////////////////////////
// these functions invoke upcalls to a visi interface client
// (call from visualization process to paradyn)
/////////////////////////////////////////////////////////////
//
// get a new set of metrics and resources from Paradyn
//
extern void GetMetsRes(char *metres,  // predefined list met-res pairs 
		       int numElements,  
		       int type);      // 0-histogram, 1-scalar

//
// equivalent to a call to GetMetRes(0,0,0)
//
extern void GetMetsRes();

//
// stop data collection for a metric/resource pair
// arguments are the datagrid indicies associated with the
// metric and resource to stop
//
extern void StopMetRes(int metricIndex,    // datagrid index of metric
		       int resourceIndex); // datagrid index of resource

//
// define a new phase to paradyn, can specify some time in the future
// for the phase to begin or a begin value of -1 means now
//
extern void DefinePhase(timeType begin,  // in seconds 
		        char *name);     // name of phase 

////////////////////////////////////////////////////////////////
//   initialization routines 
////////////////////////////////////////////////////////////////
//
// connects to parent socket and registers the mainLoop routine
// as a callback on events on fileDesc[0]
// This routine should be called before entering the visualization's
// main loop, and before calling any other visi-interface routines
//
extern int  VisiInit();
extern int  initDone;


//
//  Makes initial call to get Metrics and Resources.
//  For visualizations that do not provide an event that
//  invokes the GetMetsRes upcall this routine should be
//  called by the visualization before entrering the mainloop.
//  (this will be the only chance to get metrics and resources
//  for the visualization).
//  For other visualizaitons, calling this routine before
//  entering the mainloop is optional. 
//
extern int  StartVisi(int argc,char *argv[]);

//
// cleans up visi interface data structs
// Visualizations should call this routine before exiting 
extern void QuitVisi();


//
// registration callback routine for paradyn events
// sets eventCallbacks[event] to callback routine provided by user
//
extern int RegistrationCallback(msgTag event,int (*callBack)(int));

//
// request to Paradyn to display error message
//
extern void showErrorVisiCallback(const char *msg);

//
// main loop provided by paradyn (not currently supported)
//
extern void ParadynMain();

//
// fd registration and callback routine registration for user
// to register callback routines when they use the provided main routine
//
extern int RegFileDescriptors(int *fd, int (*callBack)()); 

#endif
