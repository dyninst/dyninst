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
/* Revision 1.4  1994/05/11 17:11:10  newhall
/* changed data values from double to float
/*
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
#include "mrlist.h"
#include "visiTypes.h"

#define FILETABLESIZE  64
#define EVENTSIZE      FOLD+1

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
extern visi_MRList    metricList; 
extern visi_MRList    resourceList;
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
extern void GetMetsRes(char *metrics,  // list of current metrics
		       char *resource, // list of current resources
		       int type);      // 0-histogram, 1-scalar

//
// stop data collection for a metric/resource pair
//
extern void StopMetRes(int metricId,    // id of metric to stop
		       int resourceId); // id of resource to stop

//
// define a new phase to paradyn
//
extern void NamePhase(timeType begin,  // start of phase in seconds
		      timeType end,    // end of phase in seconds
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
// registration callback routine for paradyn events
// sets eventCallbacks[event] to callback routine provided by user
//
extern int RegistrationCallback(msgTag event,int (*callBack)(int));

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
