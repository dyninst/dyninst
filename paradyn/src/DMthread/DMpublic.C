
/*
 * dataManagerImpl.C - provide the interface methods for the dataManager thread
 *   remote class.
 *
 * $Log: DMpublic.C,v $
 * Revision 1.3  1994/02/08 17:20:29  hollings
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

#include "dataManager.h"
#include "DMinternals.h"

applicationContext *dataManager::createApplicationContext(errorHandler foo)
{
    return(new applicationContext(foo));
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

Boolean dataManager::continueApplication(applicationContext *app)
{
    return(app->continueApplication());
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

metricInstance *dataManager::enableDataCollection(performanceStream *ps,
						  resourceList *rl,
						  metric *m)
{
    return(ps->enableDataCollection(rl, m));
}

void dataManager::disableDataCollection(performanceStream *ps, 
					metricInstance *mi)
{
    return(ps->disableDataCollection(mi));
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
