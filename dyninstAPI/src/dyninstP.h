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
 * Revision 1.4  1994/09/22 01:51:40  markc
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

extern "C" {
#include <stdio.h>
}

class metric;
class metricDefinitionNode;
class metricListRec;

typedef enum { selfTermination, controlTermination } executableType;

Boolean isApplicationPaused();

/* descriptive information about a resource */
class resourceInfo {
 public:
    stringHandle name;			/* name of actual resource */
    stringHandle fullName;		/* full path name of resource */
    stringHandle abstraction;          /* abstraction name */
    timeStamp creation;		/* when did it get created */
};		

class executableRec {
    public:
        executableRec() {
	  name = NULL; machine = NULL; user = NULL;
	  argc = 0; argv = NULL; type = selfTermination; state = neonatal;
	  next = NULL; controlPath = NULL; proc = NULL;
	}
	char *name;
	char *machine;
	char *user;
	int argc;
	char **argv;
	executableType type;
	processState state;
	executableRec *next;
	FILE *controlPath;
	process *proc;	/* for directly connected processes */
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
	char *getName()	{ return((char*)info.name); }
	resource(Boolean full = True) {
	    if (full) {
		parent = NULL;
		handle = NULL;
		children = NULL;
		info.name = pool.findAndAdd("");
		info.fullName = pool.findAndAdd("");
		info.creation = 0.0;
		suppressed = False;
	    }
	};
	Boolean suppressed;		/* don't collect data about this */
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
int addProcess(int argc, char*argv[], int nenv=0, char *envp[]=0);

/*
 * Find out if an application has been defined yet.
 *
 */
Boolean applicationDefined();

/*
 * Start an application running (This starts the actual execution).
 *  app - an application context from createPerformanceConext.
 */
Boolean startApplication();

/*
 *   Stop all processes associted with the application.
 *	app - an application context from createPerformanceConext.
 *
 * Pause an application (I am not sure about this but I think we want it).
 *      - Does this force buffered data to be delivered?
 *	- Does a paused application respond to enable/disable commands?
 */
Boolean pauseAllProcesses();

/*
 * Continue a paused application.
 *    app - an application context from createPerformanceConext.
 */
Boolean continueAllProcesses();


/*
 * Disconnect the tool from the process.
 *    pause - leave the process in a stopped state.
 *
 */
Boolean detachProcess(int pid, Boolean pause);

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
Boolean enableResourceCreationNotification(resource*);

/*
 * Resource utility functions.
 *
 */
resourceListRec *getRootResources();

extern resource *rootResource;

stringHandle getResourceName(resource*);

resource *getResourceParent(resource*);

resourceListRec *getResourceChildren(resource*);

Boolean isResourceDescendent(resource *parent, resource *child);

resource *findChildResource(resource *parent, char *name);

int getResourceCount(resourceListRec*);

resource *getNthResource(resourceListRec*, int n);

resourceInfo *getResourceInfo(resource*);

resourceListRec *createResourceList();

Boolean addResourceList(resourceListRec*, resource*);

resource *newResource(resource *parent,
		      void *handle,
		      stringHandle abstraction,
		      const char *name,
		      timeStamp creation,
		      Boolean unique);

/*
 * manipulate user handle (a single void * to permit mapping between low level
 *   resource's and the resource consumer.
 *
 */
void *getResourceHandle(resource*);

void setResourceHandle(resource*, void*);

resourceListRec *findFocus(int count, char **focusString);

/*
 * Get the static configuration information.
 *
 */
metricListRec *getMetricList();

/*
 * looks for a specifc metric instance in an application context.
 *
 */
metric *findMetric(char *name);

/*
 * Metric utility functions.
 *
 */
char *getMetricName(metric*);

/*
 * Get metric out of a metric instance.
 *
 */
metric *getMetric(metricDefinitionNode*);

extern resource *findResource(const char *name);

#endif


