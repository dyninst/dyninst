
/*
 * dataManagerImpl.C - provide the interface methods for the dataManager thread
 *   remote class.
 *
 * $Log: DMpublic.C,v $
 * Revision 1.9  1994/04/06 21:26:41  markc
 * Added "include <assert.h>"
 *
 * Revision 1.8  1994/04/01  20:45:05  hollings
 * Added calls to query bucketWidth and max number of bins.
 *
 * Revision 1.7  1994/03/31  01:39:01  markc
 * Added dataManager continue/pause Process.
 *
 * Revision 1.6  1994/03/20  01:49:49  markc
 * Gave process structure a buffer to allow multiple writers.  Added support
 * to register name of paradyn daemon.  Changed addProcess to return type int.
 *
 * Revision 1.5  1994/03/08  17:39:34  hollings
 * Added foldCallback and getResourceListName.
 *
 * Revision 1.4  1994/02/24  04:36:32  markc
 * Added an upcall to dyninstRPC.I to allow paradynd's to report information at
 * startup.  Added a data member to the class that igen generates.
 * Make depend differences due to new header files that igen produces.
 * Added support to allow asynchronous starts of paradynd's.  The dataManager has
 * an advertised port that new paradynd's can connect to.
 *
 * Revision 1.3  1994/02/08  17:20:29  hollings
 * Fix to not core dump when parent is null.
 *
 * Revision 1.2  1994/02/03  23:26:59  hollings
 * Changes to work with g++ version 2.5.2.
 *
 * Revision 1.1  1994/02/02  00:42:34  hollings
 * Changes to the Data manager to reflect the file naming convention and
 * to support the integration of the Performance Consultant.
 *
 * Revision 1.1  1994/01/28  01:34:18  hollings
 * The initial version of the Data Management thread.
 *
 *
 */
extern "C" {
#include <malloc.h>
}

#include "dataManager.SRVR.h"
#include "dataManager.CLNT.h"
#include "dyninstRPC.CLNT.h"
#include "DMinternals.h"
#include <assert.h>

// the argument list passed to paradynds
char **paradynDaemon::args = 0;

applicationContext *dataManager::createApplicationContext(errorHandler foo)
{
  appContext = new applicationContext(foo);
  return appContext;
}

Boolean dataManager::addExecutable(applicationContext *app,
				   char  *machine,
				   char *login,
				   char *name,
				   int argc,
				   char **argv)
{
    return(app->addExecutable(machine, login, name, argc, argv));
}

Boolean dataManager::applicationDefined(applicationContext *app)
{
    return(app->applicationDefined());
}

Boolean dataManager::startApplication(applicationContext *app)
{
    return(app->startApplication());
}

Boolean dataManager::pauseApplication(applicationContext *app)
{
    return(app->pauseApplication());
}

Boolean dataManager::pauseProcess(applicationContext *app, int pid)
{
    return(app->pauseProcess(pid));
}

Boolean dataManager::continueApplication(applicationContext *app)
{
    return(app->continueApplication());
}

Boolean dataManager::continueProcess(applicationContext *app, int pid)
{
    return(app->continueProcess(pid));
}

Boolean dataManager::detachApplication(applicationContext *app, Boolean pause)
{
   return(app->detachApplication(pause));
}

// 
// Informs that instance of the name of the paradynd for execs
//
void dataManager::sendParadyndName (String the_name)
{
   assert(the_name);
   paradynd_name = strdup(the_name);

  // setup arg list to pass
  // to prevent memory leaks this list could be freed by the destructor
  // this list is null terminated
  assert (paradynDaemon::args =
	  RPC_make_arg_list (the_name, AF_INET, SOCK_STREAM, socket, 1));
}

performanceStream *dataManager::createPerformanceStream(applicationContext *ap,
						        dataType dt,
						        dataCallback dc,
						        controlCallback cc)
{
    performanceStream *ps;

    ps = new performanceStream(ap, dt, dc, cc, getRequestingThread());
    ap->streams.add(ps);

    return(ps);
}

String_Array dataManager::getAvailableMetrics(applicationContext *ap)
{
    return(ap->getAvailableMetrics());
}

metric *dataManager::findMetric(applicationContext *ap, char *name)
{
    return(ap->findMetric(name));
}

resourceList *dataManager::getRootResources()
{
    return(resource::rootResource->getChildren());
}

resource *dataManager::getRootResource()
{
    return(resource::rootResource);
}

char *dataManager::getResourceName(resource *r)
{
    return(r->getFullName());
}

resource *dataManager::getResourceParent(resource *r)
{
    return(r->parent);
}

resourceList *dataManager::getResourceChildren(resource *r)
{
    return(&r->children);
}

Boolean dataManager::isResourceDescendent(resource *parent, resource *child)
{
    return(parent->isDescendent(child));
}

resource *dataManager::findChildResource(resource *parent, char *name)
{
    if (parent) {
	return(parent->getChildren()->find(name));
    } else {
	return(NULL);
    }
}

int dataManager::getResourceCount(resourceList *rl)
{
    return(rl ? rl->getCount(): 0);
}

resource *dataManager::getNthResource(resourceList *rl, int n)
{
    return(rl->getNth(n));
}

resourceList *dataManager::createResourceList()
{
    resourceList *ret;

    ret = new resourceList;
    return(ret);
}

void dataManager::addResourceList(resourceList *rl, resource *r)
{
    rl->add(r);
}

char *dataManager::getResourceListName(resourceList *rl)
{
    int i;
    int count;
    int total;
    char *ret;
    char **temp;
    extern int strCompare(char **a, char **b);

    count = rl->getCount();
    temp = rl->convertToStringList();
    qsort(temp, count, sizeof(char *), strCompare);

    total = 2;
    for (i=0; i < count; i++) total += strlen(temp[i])+2;

    ret = new (char[total]);
    strcpy(ret, "<");
    for (i=0; i < count; i++) {
	if (i) strcat(ret, ",");
	strcat(ret, temp[i]);
    }
    strcat(ret, ">");

    return(ret);
}

metricInstance *dataManager::enableDataCollection(performanceStream *ps,
						  resourceList *rl,
						  metric *m)
{
    return(ps->enableDataCollection(rl, m));
}

void dataManager::disableDataCollection(performanceStream *ps, 
					metricInstance *mi)
{
    ps->disableDataCollection(mi);
}

void dataManager::enableResourceCreationNotification(performanceStream *ps, 
							resource *r)
{
    ps->enableResourceCreationNotification(r);
}

void dataManager::disableResourceCreationNotification(performanceStream *ps, 
							 resource *r)
{
    ps->disableResourceCreationNotification(r);
}

metric *dataManager::getMetric(metricInstance *mi)
{
    return(mi->met);
}

char *dataManager::getMetricNameFromMI(metricInstance *mi)
{
    return(mi->met->getName());
}

char *dataManager::getMetricName(metric *m)
{
    return(m->getName());
}

float dataManager::getMetricValue(metricInstance *mi)
{
    return(mi->getValue());
}

void dataManager::setSampleRate(performanceStream *ps, timeStamp rate)
{
    ps->setSampleRate(rate);
}

float dataManager::getPredictedDataCost(applicationContext *a, 
					resourceList *rl, 
					metric *m)
{
    return(a->getPredictedDataCost(rl, m));
}

void dataManager::printResources()
{
    HTable<resource*> curr;

    for (curr=  resource::allResources; *curr; curr++) {
	(*curr)->print();
	printf("\n");
    }
}

void dataManager::printStatus(applicationContext *appl)
{
    appl->printStatus();
}

void dataManager::coreProcess(applicationContext *app, int pid)
{
    app->coreProcess(pid);
}

//
// Now for the upcalls.  We provide code that get called in the thread that
//   requested the call back.
//
void dataManagerUser::newMetricDefined(metricInfoCallback cb,
				  performanceStream *ps,
				  metric *m)
{
    (cb)(ps, m);
}

void dataManagerUser::newResourceDefined(resourceInfoCallback cb,
					 performanceStream *ps,
					 resource *parent,
					 resource *newResource,
					 char *name)
{
    (cb)(ps, parent, newResource, name);
}

void dataManagerUser::histFold(histFoldCallback cb,
			       performanceStream *ps,
			       timeStamp width)
{
    (cb)(ps, width);
}

void dataManagerUser::newPerfData(sampleDataCallbackFunc func,
                             performanceStream *ps,
                             metricInstance *mi,
                             timeStamp st,
                             timeStamp end,
                             sampleValue value)
{
    (func)(ps, mi, st, end, value);
}

metricInfo *dataManager::getMetricInfo(metric *met) {
    return(met->getInfo());
}

resource *dataManager::newResource(applicationContext *app, 
			      resource *res, 
			      char *name)
{
    resource *child;

    child = createResource(res, name);
    app->tellDaemonsOfResource(res->getFullName(), name);
    return(child);
}

timeStamp dataManager::getCurrentBucketWidth()
{
    return(Histogram::bucketSize);
}

int dataManager::getMaxBins()
{
    return(Histogram::numBins);
}
