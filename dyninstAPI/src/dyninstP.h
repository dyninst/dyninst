/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef _DYNINSTP_H
#define _DYNINSTP_H

/*
 * private structures used by the implementation of the instrumentation 
 *   interface.  modules that use the instrumentation interface should not
 *   include this file.
 *  
 * This file will be empty during the restructuring of the paradyn daemon
 *
 * $Log: dyninstP.h,v $
 * Revision 1.6  1995/02/16 08:33:10  markc
 * Changed igen interfaces to use strings/vectors rather than char*/igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.5  1994/11/02  11:04:16  markc
 * Changed casts to remove compiler warnings.
 *
 * Revision 1.4  1994/09/22  01:51:40  markc
 * Added most of dyninst.h, temporary
 *
 * Revision 1.3  1994/08/08  20:13:35  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.2  1994/02/01  18:46:51  hollings
 * Changes for adding perfConsult thread.
 *
 * Revision 1.1  1994/01/27  20:31:18  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.6  1993/08/23  23:14:58  hollings
 * removed unused pid field.
 *
 * Revision 1.5  1993/07/13  18:27:00  hollings
 * new include file syntax.
 *
 * Revision 1.4  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.3  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.2  1993/04/27  14:39:21  hollings
 * signal forwarding and args tramp.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */

#include "dyninst.h"
#include "process.h"

#include <stdio.h>

class metric;
class metricDefinitionNode;
class metricListRec;

typedef enum { selfTermination, controlTermination } executableType;

bool isApplicationPaused();

/* descriptive information about a resource */
class resourceInfo {
 public:
    string name;			/* name of actual resource */
    string fullName;		/* full path name of resource */
    string abstraction;          /* abstraction name */
    timeStamp creation;		/* when did it get created */
};		

class resourceListRec {
    public:
        resourceListRec() {
	  elements = NULL; count=0; maxItems=0;
	}
	resource **elements;		/* actual data in list */
	int count;			/* number of items in the list */
	int maxItems;		/* limit of current array */
};

/* something that data can be collected for */
class resource {
    public:
	string getName() const { return(info.name); }
	resource(bool suppress=false)
	  : suppressed(suppress), parent(NULL), handle(NULL), children(NULL) {
	    info.creation = 0.0;
	  }

	bool suppressed;		/* don't collect data about this */
	resource *parent;		/* parent of this resource */
	void *handle;		/* handle to resource specific data */
	resourceListRec *children;	/* children of this resource */
	resourceInfo info;
};

typedef enum { Trace, Sample } dataType;

/*
 * error handler call back.
 *
 */
typedef int (*errorHandler)(int errno, char *message);

/*
 * Define a program to run (this is very tentative!)
 *
 *   argv - arguments to command
 *   envp - environment args, for pvm
 */
int addProcess(vector<string> argv, vector<string> nenv);

/*
 * Find out if an application has been.defines yet.
 *
 */
bool applicationDefined();

/*
 * Start an application running (This starts the actual execution).
 *  app - an application context from createPerformanceConext.
 */
bool startApplication();

/*
 *   Stop all processes associted with the application.
 *	app - an application context from createPerformanceConext.
 *
 * Pause an application (I am not sure about this but I think we want it).
 *      - Does this force buffered data to be delivered?
 *	- Does a paused application respond to enable/disable commands?
 */
bool pauseAllProcesses();

/*
 * Continue a paused application.
 *    app - an application context from createPerformanceConext.
 */
bool continueAllProcesses();


/*
 * Disconnect the tool from the process.
 *    pause - leave the process in a stopped state.
 *
 */
bool detachProcess(int pid, bool pause);

/*
 * Routines to control data collection.
 *
 * resourceList		- a list of resources
 * metric		- what metric to collect data for
 *
 */
int startCollecting(resourceListRec*, metric*);


/*
 * Return the expected cost of collecting performance data for a single
 *    metric at a given focus.  The value returned is the fraction of
 *    perturbation expected (i.e. 0.10 == 10% slow down expected).
 */
float guessCost(resourceListRec*, metric*);

/*
 * Control information arriving about a resource Classes
 *
 * resource		- enable notification of children of this resource
 */
bool enableResourceCreationNotification(resource*);

/*
 * Resource utility functions.
 *
 */
resourceListRec *getRootResources();

extern resource *rootResource;

string getResourceName(resource*);

resource *getResourceParent(resource*);

resourceListRec *getResourceChildren(resource*);

bool isResourceDescendent(resource *parent, resource *child);

resource *findChildResource(resource *parent, const string name);

int getResourceCount(resourceListRec*);

resource *getNthResource(resourceListRec*, int n);

resourceInfo *getResourceInfo(resource*);

resourceListRec *createResourceList();

bool addResourceList(resourceListRec*, resource*);

resource *newResource(resource *parent,
		      void *handle,
		      const string abstraction,
		      const string name,
		      timeStamp creation,
		      const string unique);

/*
 * manipulate user handle (a single void * to permit mapping between low level
 *   resource's and the resource consumer.
 *
 */
void *getResourceHandle(resource*);

void setResourceHandle(resource*, void*);

resourceListRec *findFocus(const vector<string> &focusString);

/*
 * Get the static configuration information.
 *
 */
metricListRec *getMetricList();

/*
 * looks for a specifc metric instance in an application context.
 *
 */
metric *findMetric(const string name);

/*
 * Metric utility functions.
 *
 */
string getMetricName(metric*);

/*
 * Get metric out of a metric instance.
 *
 */
metric *getMetric(metricDefinitionNode*);

extern resource *findResource(const string &name);

const string nullString((char*) NULL);

#endif


