
/*
 * dataManagerImpl.C - provide the interface methods for the dataManager thread
 *   remote class.
 *
 * $Log: DMpublic.C,v $
 * Revision 1.34  1994/11/09 18:39:36  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.33  1994/11/07  08:24:37  jcargill
 * Added ability to suppress search on children of a resource, rather than
 * the resource itself.
 *
 * Revision 1.32  1994/11/04  16:30:41  rbi
 * added getAvailableDaemons()
 *
 * Revision 1.31  1994/11/02  11:46:21  markc
 * Changed shadowing nam.
 *
 * Revision 1.30  1994/09/30  19:17:47  rbi
 * Abstraction interface change.
 *
 * Revision 1.29  1994/09/22  00:56:05  markc
 * Added const to args to addExecutable()
 *
 * Revision 1.28  1994/08/22  15:59:07  markc
 * Add interface calls to support daemon definitions.
 *
 * Revision 1.27  1994/08/11  02:17:42  newhall
 * added dataManager interface routine destroyPerformanceStream
 *
 * Revision 1.26  1994/08/08  20:15:20  hollings
 * added suppress instrumentation command.
 *
 * Revision 1.25  1994/08/05  16:03:59  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.24  1994/07/25  14:55:37  hollings
 * added suppress resource option.
 *
 * Revision 1.23  1994/07/14  23:45:54  hollings
 * added hybrid cost model.
 *
 * Revision 1.22  1994/07/07  03:29:35  markc
 * Added interface function to start a paradyn daemon
 *
 * Revision 1.21  1994/07/02  01:43:12  markc
 * Removed all uses of type aggregation from enableDataCollection.
 * The metricInfo structure now contains the aggregation operator.
 *
 * Revision 1.20  1994/06/27  21:23:29  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.19  1994/06/17  22:08:00  hollings
 * Added code to provide upcall for resource batch mode when a large number
 * of resources is about to be added.
 *
 * Revision 1.18  1994/06/14  15:23:17  markc
 * Added support for aggregation.
 *
 * Revision 1.17  1994/06/02  16:08:16  hollings
 * fixed duplicate naming problem for printResources.
 *
 * Revision 1.16  1994/05/31  19:11:33  hollings
 * Changes to permit direct access to resources and resourceLists.
 *
 * Revision 1.15  1994/05/10  03:57:38  hollings
 * Changed data upcall to return array of buckets.
 *
 * Revision 1.14  1994/05/09  20:56:22  hollings
 * added changeState callback.
 *
 * Revision 1.13  1994/04/21  23:24:27  hollings
 * removed process name from calls to RPC_make_arg_list.
 *
 * Revision 1.12  1994/04/20  15:30:11  hollings
 * Added error numbers.
 * Added data manager function to get histogram buckets.
 *
 * Revision 1.11  1994/04/19  22:08:38  rbi
 * Added getTotValue method to get non-normalized metric data.
 *
 * Revision 1.10  1994/04/18  22:28:32  hollings
 * Changes to create a canonical form of a resource list.
 *
 * Revision 1.9  1994/04/06  21:26:41  markc
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

void dataManager::setResourceSearchSuppress(applicationContext *app,
					    resource *res, Boolean newValue)
{
    if (res) res->setSuppress(newValue);
}

void dataManager::setResourceSearchChildrenSuppress(applicationContext *app,
						    resource *res, 
						    Boolean newValue)
{
    if (res) res->setSuppressChildren(newValue);
}

void dataManager::setResourceInstSuppress(applicationContext *app,
				      resource *res, Boolean newValue)
{
    if (res) app->setInstSuppress(res, newValue);
}

Boolean dataManager::addDaemon(applicationContext *app,
			       char *machine, char *login, char *name)
{
  return (app->getDaemon(machine, login, name));
}

Boolean dataManager::defineDaemon(applicationContext *app,
				  const char *command,
				  const char *dir,
				  const char *login,
				  const char *name,
				  const char *machine,
				  int flavor)
{
  return (app->defineDaemon(command, dir, login, name, machine, flavor));
}

Boolean dataManager::addExecutable(applicationContext *app,
				   char  *machine,
				   char *login,
				   char *name,
				   char *dir,
				   int argc,
				   char **argv)
{
    return(app->addExecutable(machine, login, name, dir, argc, argv));
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

performanceStream *dataManager::createPerformanceStream(applicationContext *ap,
						        dataType dt,
						        dataCallback dc,
						        controlCallback cc)
{
    int td;
    performanceStream *ps;

    td = getRequestingThread();
    ps = new performanceStream(ap, dt, dc, cc, td);
    ap->streams.add(ps);

    return(ps);
}

int dataManager::destroyPerformanceStream(applicationContext *ap,
                                          performanceStream *ps){
    int ok = 1;
    performanceStream *temp;

    if(temp = ap->streams.find(ps)){
      ok = ap->streams.remove(ps);
    }
    return(ok);
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

metricInstance *dataManager::enableDataCollection(performanceStream *ps,
						  resourceList *rl,
						  metric *m)
{
    return(ps->enableDataCollection(rl, m));
}

void dataManager::disableDataCollection(performanceStream *ps, 
					metricInstance *mi)
{
    if (mi) ps->disableDataCollection(mi);
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

stringHandle dataManager::getMetricNameFromMI(metricInstance *mi)
{
    return(mi->met->getName());
}

stringHandle dataManager::getMetricName(metric *m)
{
    return(m->getName());
}

float dataManager::getMetricValue(metricInstance *mi)
{
    return(mi->getValue());
}

float dataManager::getTotValue(metricInstance *mi)
{
    return(mi->getTotValue());
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

float dataManager::getCurrentHybridCost(applicationContext *a) 
{
    return(a->getCurrentHybridCost());
}

int dataManager::getSampleValues(metricInstance *mi,
				 sampleValue *buckets,
				 int numberOfBuckets,
				 int first)
{
    Histogram *hist;

    hist = mi->data;

    if (!hist) return(-1);

    return(hist->getBuckets(buckets, numberOfBuckets, first));
}

void dataManager::printResources()
{
    printAllResources();
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
					 stringHandle name)
{
    (cb)(ps, parent, newResource, name);
}

void dataManagerUser::changeResourceBatchMode(resourceBatchModeCallback cb,
					 performanceStream *ps,
					 batchMode mode)
{
    (cb)(ps, mode);
}

void dataManagerUser::histFold(histFoldCallback cb,
			       performanceStream *ps,
			       timeStamp width)
{
    (cb)(ps, width);
}

void dataManagerUser::changeState(appStateChangeCallback cb,
			          performanceStream *ps,
			          appState state)
{
    (cb)(ps, state);
}

void dataManagerUser::newPerfData(sampleDataCallbackFunc func,
                             performanceStream *ps,
                             metricInstance *mi,
			     sampleValue *buckets,
			     int count,
			     int first)
{
    (func)(ps, mi, buckets, count, first);
}

metricInfo *dataManager::getMetricInfo(metric *met) {
    return(met->getInfo());
}

resource *dataManager::newResource(applicationContext *app, 
				   resource *res, 
				   const char *name)
{
    resource *child;

    // rbi: kludge 
    // calls to this method should specify an abstraction,
    // but that involves a bunch of other changes that I don't want 
    // to make right now.
    // the kludge works because we know that all calls to this method 
    // are for BASE abstraction resources.  
    child = createResource(res, name, "BASE");  
    app->tellDaemonsOfResource((const char *) res->getFullName(), name);
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

void dataManager::printDaemons(applicationContext *app)
{
  app->printDaemons();
}

String_Array dataManager::getAvailableDaemons(applicationContext *ap)
{
    return(ap->getAvailableDaemons());
}

double dataManager::firstSampleTime(int program, double first) {
  if (!firstTime)
    firstTime = first;
  return firstTime;
}

