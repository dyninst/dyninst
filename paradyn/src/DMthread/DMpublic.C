
/*
 * dataManagerImpl.C - provide the interface methods for the dataManager thread
 *   remote class.
 *
 * $Log: DMpublic.C,v $
 * Revision 1.42  1995/08/01 02:11:18  newhall
 * complete implementation of phase interface:
 *   - additions and changes to DM interface functions
 *   - changes to DM classes to support data collection at current or
 *     global phase granularity
 * added alphabetical ordering to foci name creation
 *
 * Revision 1.41  1995/07/06  01:52:53  newhall
 * update for new version of Histogram library, removed compiler warnings
 *
 * Revision 1.40  1995/06/02  20:48:27  newhall
 * * removed all pointers to datamanager class objects from datamanager
 *    interface functions and from client threads, objects are now
 *    refered to by handles or by passing copies of DM internal data
 * * removed applicationContext class from datamanager
 * * replaced List and HTable container classes with STL containers
 * * removed global variables from datamanager
 * * remove redundant lists of class objects from datamanager
 * * some reorginization and clean-up of data manager classes
 * * removed all stringPools and stringHandles
 * * KLUDGE: there are PC friend members of DM classes that should be
 *    removed when the PC is re-written
 *
 * Revision 1.38  1995/02/26  02:14:07  newhall
 * added some of the phase interface support
 *
 * Revision 1.37  1995/02/16  19:10:44  markc
 * Removed start slash from comments
 *
 * Revision 1.36  1995/02/16  08:16:42  markc
 * Changed Bool to bool
 * Changed igen-xdr functions to use string/vectors rather than char igen-arrays
 *
 * Revision 1.35  1995/01/26  17:58:23  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.34  1994/11/09  18:39:36  rbi
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

#include <assert.h>
#include "dataManager.thread.h"
#include "dataManager.thread.SRVR.h"
#include "dataManager.thread.CLNT.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "visi.xdr.h"
#include "util/h/sys.h"
#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "util/h/makenan.h"
#include "DMmetric.h"
#include "DMdaemon.h"
#include "DMresource.h"
#include "DMperfstream.h"
#include "DMphase.h"
#include "DMinclude.h"

// the argument list passed to paradynds
vector<string> paradynDaemon::args = 0;

extern bool parse_metrics(string metric_file);
void dataManager::kludge(char *file) {
  parse_metrics(file);
}

void histDataCallBack(sampleValue *buckets,
                      timeStamp start_time,
		      int count,
		      int first,
		      void *arg)
{
    metricInstance *mi = (metricInstance *) arg;
    performanceStream *ps = 0;
    unsigned i;

    if(start_time == 0.0) { 
	// update global data
        for (i=0; i < mi->global_users.size(); i++) {
	    ps = performanceStream::find(mi->global_users[i]); 
	    if(ps)
	        ps->callSampleFunc(mi->getHandle(), buckets, count, first);
        }
	// update curr. phase data if curr. phase started at time 0.0
	if (phaseInfo::GetLastPhaseStart() == 0.0) {
            for (i=0; i < mi->users.size(); i++) {
	        ps = performanceStream::find(mi->users[i]); 
	        if(ps)
	            ps->callSampleFunc(mi->getHandle(), buckets, count, first);
            }
	}
    }

    else {  // update just curr. phase data
        for (i=0; i < mi->users.size(); i++) {
	    ps = performanceStream::find(mi->users[i]); 
	    if(ps)
	        ps->callSampleFunc(mi->getHandle(), buckets, count, first);
        }
    }

    for(i=first; i < count; i++){
        if(buckets[i] < 0) printf("bucket %d : %f \n",i,buckets[i]);
    }
}

//
// start_time specifies the phaseType (globalType starts at 0.0) 
//
void histFoldCallBack(timeStamp width, void *arg,timeStamp start_time)
{

    // need to check if current phase also starts at 0.0
    // if it does, then fold applies to both global and curr phase
    if(start_time == 0.0){
        timeStamp curr_start =  phaseInfo::GetLastPhaseStart(); 
        if(curr_start == 0.0){
	    if(metricInstance::GetCurrWidth() != width) {
	        metricInstance::SetCurrWidth(width);
	        performanceStream::foldAll(width,CurrentPhase);
	    }
	    phaseInfo::setCurrentBucketWidth(width);
        }
	if(metricInstance::GetGlobalWidth() != width) {
	    metricInstance::SetGlobalWidth(width);
	    performanceStream::foldAll(width,GlobalPhase);
	}
    }
    else {  // fold applies to current phase
	if(metricInstance::GetCurrWidth() != width) {
	    metricInstance::SetCurrWidth(width);
	    performanceStream::foldAll(width,CurrentPhase);
	}
	phaseInfo::setCurrentBucketWidth(width);
    }
}


void dataManager::setResourceSearchSuppress(resourceHandle res, bool newValue)
{
    resource *r = resource::handle_to_resource(res);
    if(r)
        r->setSuppress(newValue);
}

void dataManager::setResourceSearchChildrenSuppress(resourceHandle res, 
						    bool newValue)
{
    resource *r = resource::handle_to_resource(res);
    if(r) r->setSuppressChildren(newValue);
}

void dataManager::setResourceInstSuppress(resourceHandle res, bool newValue)
{
    resource *r = resource::handle_to_resource(res);
    if (r) paradynDaemon::setInstSuppress(r, newValue);
}

bool dataManager::addDaemon(const char *machine,
			    const char *login,
			    const char *name)
{
  // fix args so that if char * points to "" then it pts to NULL
  string m = 0;
  if(machine && strlen(machine))  m = machine;
  string l = 0;
  if(login && strlen(login)) l = login;
  string n = 0;
  if(!name) { 
      char *temp = 0; 
      if(!paradynDaemon::setDefaultArgs(temp))
	  return false;
      n = temp;
      delete temp;
  }
  else {
      n = name;
  }
  return (paradynDaemon::getDaemon(m, l, n));
}

#ifdef n_def
//
// define a new entry for the daemon dictionary
//
bool dataManager::defineDaemon(const char *command,
			       const char *dir,
			       const char *login,
			       const char *name,
			       const char *machine,
			       const char *flavor)
{
  if(!name || !command)
      return false;
  string c = command;
  string d = dir;
  string l = login;
  string n = name;
  string m = machine;
  string f = flavor;
  return (paradynDaemon::defineDaemon(c, d, l, n, m, f));
}
#endif

bool dataManager::addExecutable(const char *machine,
				const char *login,
				const char *name,
				const char *dir,
				const vector<string> *argv)
{
    string m = machine;
    string l = login;
    string n = name;
    string d = dir;
    return(paradynDaemon::newExecutable(m, l, n, d, *argv));
}

bool dataManager::applicationDefined()
{
    return(paradynDaemon::applicationDefined());
}

bool dataManager::startApplication()
{
    return(paradynDaemon::startApplication());
}

bool dataManager::pauseApplication()
{
    return(paradynDaemon::pauseAll());
}

bool dataManager::pauseProcess(int pid)
{
    return(paradynDaemon::pauseProcess(pid));
}

bool dataManager::continueApplication()
{
    return(paradynDaemon::continueAll());
}

bool dataManager::continueProcess(int pid)
{
    return(paradynDaemon::continueProcess(pid));
}

bool dataManager::detachApplication(bool pause)
{
   return(paradynDaemon::detachApplication(pause));
}

perfStreamHandle dataManager::createPerformanceStream(dataType dt,
						      dataCallback dc,
						      controlCallback cc)
{
    int td;
    performanceStream *ps;

    td = getRequestingThread();
    ps = new performanceStream(dt, dc, cc, td);
    return(ps->Handle());
    ps = 0;
}

int dataManager::destroyPerformanceStream(perfStreamHandle handle){

    performanceStream *ps = performanceStream::find(handle);
    if(!ps) return(0);
    delete ps;
    return(1);
}

vector<string> *dataManager::getAvailableMetrics()
{
    return(metric::allMetricNames());
}

vector<met_name_id> *dataManager::getAvailableMetInfo()
{
    return(metric::allMetricNamesIds());
}


metricHandle *dataManager::findMetric(const char *name)
{
    string n = name;
    const metricHandle *met = metric::find(n);
    if(met){
	metricHandle *ret = new metricHandle;
	*ret = *met;
	return(ret);
    }
    return 0;
}

vector<resourceHandle> *dataManager::getRootResources()
{
    return(resource::rootResource->getChildren());
}

resourceHandle *dataManager::getRootResource()
{
    resourceHandle *rh = new resourceHandle;
    *rh = resource::rootResource->getHandle();
    return(rh);
}

#ifdef ndef
// OLD VERSION
metricInstInfo *dataManager::enableDataCollection(perfStreamHandle ps_handle,
					const vector<resourceHandle> *focus, 
					metricHandle m, phaseType type,
					unsigned persistent_data,
					unsigned persistent_collection)
{
    if(!focus || !focus->size()){
        if(focus) 
	    printf("error in enableDataCollection size = %d\n",focus->size());
        else
	    printf("error in enableDataCollection focus is NULL\n");
        return 0;
    } 
    resourceListHandle rl = resourceList::getResourceList(*focus);
    // is this this metric/focus combination already enable?
     metricInstance *mi = metricInstance::find(m,rl);
     if(mi){
	 mi->addUser(ps_handle); // add this ps to users list
     }
     else {
	// TODO: pass persistence flags and phaseType
        mi = (paradynDaemon::enableData(rl,m)
     }
     if(mi){
	mi->addUser(ps_handle); // add this ps to users list
        metricInstInfo *temp = new metricInstInfo;
	assert(temp);
	temp->mi_id = mi->getHandle();
	temp->m_id = m;
	temp->r_id = rl;
        metric *m_temp = metric::getMetric(m);
	resourceList *rl_temp = resourceList::getFocus(rl);
	temp->metric_name = m_temp->getName();
	temp->metric_units = m_temp->getUnits();
	temp->focus_name = rl_temp->getName();
	return(temp);
	temp = 0;
     }
     else {
        printf("error in DMenable\n");
	for(unsigned i=0; i < focus->size(); i++){
	    printf("resource %d\n",(*focus)[i]);  
	}
	printf("metric = %d\n",m);
     }
     return 0;
}
#endif


// TODO: implement phaseType and persistent options
metricInstInfo *dataManager::enableDataCollection(perfStreamHandle ps_handle,
					const vector<resourceHandle> *focus, 
					metricHandle m,
					phaseType type,
					unsigned persistent_data,
					unsigned persistent_collection)
{
    if(!focus || !focus->size()){
        if(focus) 
	    printf("error in enableDataCollection size = %d\n",focus->size());
        else
	    printf("error in enableDataCollection focus is NULL\n");
        return 0;
    } 
    resourceListHandle rl = resourceList::getResourceList(*focus);

    // does this this metric/focus combination already exist? 
     metricInstance *mi = metricInstance::find(m,rl);

    if (!mi) {  // create new metricInstance
	if(!(mi = new metricInstance(rl,m,phaseInfo::CurrentPhaseHandle()))) {
            return 0;
    }}

    if ( !(mi->isEnabled()) ){  // enable data collection for this MI
        if (!(paradynDaemon::enableData(rl,m,mi))) { // TODO: pass phaseType?
	    return 0;
    }}

    metric *metricptr = metric::getMetric(m);

    // update appropriate MI info. 
    if (type == CurrentPhase) {
	 mi->newCurrDataCollection(metricptr->getStyle(),
				   histDataCallBack,
				   histFoldCallBack);
	 mi->newGlobalDataCollection(metricptr->getStyle(),
				   histDataCallBack,
				   histFoldCallBack);
         mi->addCurrentUser(ps_handle);
    }
    else {
	 mi->newGlobalDataCollection(metricptr->getStyle(),
				   histDataCallBack,
				   histFoldCallBack);
         mi->addGlobalUser(ps_handle);

    }

    // update persistence flags:  the OR of new and previous values
    if(persistent_data)
	mi->setPersistentData();
    if(persistent_collection)
	mi->setPersistentCollection();


    metricInstInfo *temp = new metricInstInfo;
    assert(temp);
    temp->mi_id = mi->getHandle();
    temp->m_id = m;
    temp->r_id = rl;
    resourceList *rl_temp = resourceList::getFocus(rl);
    temp->metric_name = metricptr->getName();
    temp->metric_units = metricptr->getUnits();
    temp->focus_name = rl_temp->getName();
    return(temp);
    temp = 0;
}


// data is really disabled when there are no current or global users and
// when the persistent_collection flag is clear
// when persistent_data flag is clear:
// current histogram is destroyed when there are no curr users 
// global histogram is destroyed whern there are no curr or gloabl users
// TODO: add persistent data support: clear active flag on archived
// histograms rather than deleting them
void dataManager::disableDataCollection(perfStreamHandle handle, 
					metricInstanceHandle mh,
					phaseType type)
{

    metricInstance *mi = metricInstance::getMI(mh);
    if (!mi) return;

    
    if (mi->isCollectionPersistent()) {
        // just remove handle from appropriate client list
        if (type == GlobalPhase){
	    mi->removeGlobalUser(handle);
        }
        else {
            mi->removeCurrUser(handle);
        }
        return;
    }
    
    // remove user from appropriate list
    if (type == CurrentPhase) {
        mi->removeCurrUser(handle); 
    }
    else {
        mi->removeGlobalUser(handle);
    }

    // really disable MI data collection?  
    if (!(mi->currUsersCount())) {
	if (!(mi->isDataPersistent())){
	    // remove histogram
	    mi->deleteCurrHistogram();
	}
	else {  //TODO: clear active flag on histogram

	}
	if (!(mi->globalUsersCount())) {
	    // paradynDaemon::disableData(mi);
	    mi->dataDisable();  // makes disable call to daemons
	    if (!(mi->isDataPersistent())){
	        delete mi;	
	    }
	}
    }
    return;
}



// TODO: implement these: setting and clearing persistentCollection may have
// enable/disable side effects, clearing persistentData may cause MI to be 
// deleted
void dataManager::setPersistentCollection(metricInstanceHandle){}
void dataManager::clearPersistentCollection(metricInstanceHandle){}
void dataManager::setPersistentData(metricInstanceHandle){}
void dataManager::clearPersistentData(metricInstanceHandle){}

metricHandle *dataManager::getMetric(metricInstanceHandle mh)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if(!mi) return 0;

    metricHandle *handle = new metricHandle;
    *handle = mi->getMetricHandle();
    return(handle);
}

string *dataManager::getMetricNameFromMI(metricInstanceHandle mh)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if(mi){ 
	string *name = new string(metric::getName(mi->getMetricHandle()));
        return(name);
    }
    return 0;
}

string *dataManager::getMetricName(metricHandle m)
{
    string *name = new string(metric::getName(m));
    if(name->string_of())
        return(name);
    return 0;
}

sampleValue dataManager::getMetricValue(metricInstanceHandle mh)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if(mi) 
	return(mi->getValue());
    float ret = PARADYN_NaN;
    return(ret);
}

sampleValue dataManager::getTotValue(metricInstanceHandle mh)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if(mi) 
	return(mi->getTotValue());
    float ret = PARADYN_NaN;
    return(ret);
}

void dataManager::setSampleRate(perfStreamHandle handle, timeStamp rate)
{
    performanceStream *ps = performanceStream::find(handle);
    if(ps)
      ps->setSampleRate(rate);
}



//
// converts from a vector of resourceHandles to a resourceListHandle
//
resourceListHandle dataManager::getResourceList(const vector<resourceHandle> *h)
{
  
    resourceListHandle r = resourceList::getResourceList(*h);
    return r;
}


//
// converts from a resourceListHandle to a vector of resourceHandles
//
vector<resourceHandle> *dataManager::getResourceHandles(resourceListHandle h)
{
    return resourceList::getResourceHandles(h);
}

//
// converts from a resource name to a resourceHandle
//
resourceHandle *dataManager::findResource(const char *name){

    resourceHandle *rl = new resourceHandle;
    string r_name = name;
    if(resource::string_to_handle(r_name,rl)){
        return(rl);
    }
    return 0;
}

//
// returns resource name 
//
string *dataManager::getResourceName(resourceHandle h){

     const char *s = resource::getName(h);
     if(s){
         string *name = new string(s);
	 return(name);
     }
     return 0;
}

//
// converts from a focus name to a resourceListHandle
//
resourceListHandle *dataManager::findResourceList(const char *name){

    string n = name;
    const resourceListHandle *temp = resourceList::find(n);
    if(temp){
        resourceListHandle *h = new resourceListHandle;
	*h = *temp;
	return(h);
    }
    return 0;
}


float dataManager::getPredictedDataCost(resourceListHandle rl_handle, 
					metricHandle m_handle)
{
    metric *m = metric::getMetric(m_handle);
    if(m){
	resourceList *rl = resourceList::getFocus(rl_handle);
        if(rl){
            return(paradynDaemon::predictedDataCost(rl, m));
    } }
    float ret = PARADYN_NaN;
    return(ret);
}

float dataManager::getCurrentHybridCost() 
{
    return(paradynDaemon::currentHybridCost());
}

// TODO: implement phase option
// caller provides array of sampleValue to be filled
int dataManager::getSampleValues(metricInstanceHandle mh,
				 sampleValue *buckets,
				 int numberOfBuckets,
				 int first,
				 phaseType phase)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if(mi) 
        return(mi->getSampleValues(buckets, numberOfBuckets, first, phase));
    return(0); 
}


//  TODO: implement this
// fill the passed array of buckets with the archived histogram values
// of the passed metricInstance
int dataManager::getArchiveValues(metricInstanceHandle mi,
		     sampleValue *buckets,
		     int numberOfBuckets,
		     int first,
		     phaseHandle phase_id){

    return 0;
}


void dataManager::printResources()
{
    printAllResources();
}

void dataManager::printStatus()
{
    paradynDaemon::printStatus();
}

void dataManager::coreProcess(int pid)
{
    paradynDaemon::dumpCore(pid);
}

void dataManager::StartPhase(timeStamp start_Time, const char *name)
{
    string n = name;
    phaseInfo::startPhase(start_Time,n);
}

vector<T_visi::phase_info> *dataManager::getAllPhaseInfo(){
    return(phaseInfo::GetAllPhaseInfo());
}

//
// Now for the upcalls.  We provide code that get called in the thread that
//   requested the call back.
//
void dataManagerUser::newMetricDefined(metricInfoCallback cb,
				  perfStreamHandle p_handle,
				  const char *name,
				  int style,
				  int aggregate,
				  const char *units,
				  metricHandle handle)
{
    
    (cb)(p_handle, name, style, aggregate, units, handle);
}

void dataManagerUser::newResourceDefined(resourceInfoCallback cb,
					 perfStreamHandle handle,
					 resourceHandle parent,
					 resourceHandle newResource,
					 const char *name,
					 const char *abstr)
{
    (cb)(handle, parent, newResource, name, abstr);
}

void dataManagerUser::changeResourceBatchMode(resourceBatchModeCallback cb,
					 perfStreamHandle handle,
					 batchMode mode)
{
    (cb)(handle, mode);
}

void dataManagerUser::histFold(histFoldCallback cb,
			       perfStreamHandle handle,
			       timeStamp width,
			       phaseType phase_type)
{
    (cb)(handle, width, phase_type);
}

void dataManagerUser::changeState(appStateChangeCallback cb,
			          perfStreamHandle handle,
			          appState state)
{
    (cb)(handle, state);
}

void dataManagerUser::newPerfData(sampleDataCallbackFunc func,
                             perfStreamHandle handle,
                             metricInstanceHandle mi,
			     sampleValue *buckets,
			     int count,
			     int first)
{
    (func)(handle, mi, buckets, count, first);
}

void dataManagerUser::newPhaseInfo(newPhaseCallback cb,
				   perfStreamHandle handle,
				   const char *name,
				   phaseHandle phase,
				   timeStamp begin,
				   timeStamp end,
				   float bucketwidth) {

    (cb)(handle,name,phase,begin,end,bucketwidth);
}


T_dyninstRPC::metricInfo *dataManager::getMetricInfo(metricHandle m_handle) {

    const T_dyninstRPC::metricInfo *met = metric::getInfo(m_handle);
    if(met){ 
	T_dyninstRPC::metricInfo *copy = new T_dyninstRPC::metricInfo;
	copy->style = met->style;
	copy->units = met->units;
	copy->name = met->name;
	copy->aggregate = met->aggregate;
	copy->handle = met->handle;
	return(copy);
    }
    return 0;
}

#ifdef n_def
resourceHandle dataManager::newResource(resourceHandle parent, 
					const char *newResource) {
    // rbi: kludge 
    // calls to this method should specify an abstraction,
    // but that involves a bunch of other changes that I don't want 
    // to make right now.
    // the kludge works because we know that all calls to this method 
    // are for BASE abstraction resources.  

    // TEMP: until this routine is called with vector of strings for new res
    string res = resource::resources[parent]->getFullName();
    res += string("/");
    res += string(newResource);
    char *word = strdup(res.string_of());
    string next;
    vector<string> temp;
    unsigned j=1;
    for(unsigned i=1; i < res.length(); i++){
	if(word[i] == '/'){
	    word[i] = '\0';
	    next = &word[j];
	    temp += next;
	    j = i+1;
        }
    }
    next = &word[j];
    temp += next;
    string base = string("BASE");
    resourceHandle r = createResource(parent, temp, res, base);  
    paradynDaemon::tellDaemonsOfResource(res.string_of(),newResource);
    return(r);
}
#endif

resourceHandle dataManager::newResource(resourceHandle parent,
			                const char *name)
{

    // rbi: kludge
    // calls to this method should specify an abstraction,
    // but that involves a bunch of other changes that I don't want
    // to make right now.
    // the kludge works because we know that all calls to this method
    // are for BASE abstraction resources.
    
    string abs = "BASE";
    resource *parent_res = resource::handle_to_resource(parent);
    vector<string> res_name = parent_res->getParts();
    /*
    for(unsigned i=0; i < res_name.size(); i++){
        printf("parent part %d: %s\n",i,res_name[i].string_of());
    }
    */
    res_name += name;
    resourceHandle child = createResource(res_name,abs);
    paradynDaemon::tellDaemonsOfResource(parent_res->getHandle(), 
			       		 child, 
			                 name);
    return(child);

}

timeStamp dataManager::getGlobalBucketWidth()
{
    return(Histogram::getGlobalBucketWidth());
}

timeStamp dataManager::getCurrentBucketWidth()
{
    return(phaseInfo::GetLastBucketWidth());
}

timeStamp dataManager::getCurrentStartTime() 
{
    return(phaseInfo::GetLastPhaseStart());
}

u_int dataManager::getCurrentPhaseId() 
{
    return(phaseInfo::CurrentPhaseHandle());
}



int dataManager::getMaxBins()
{
    return(Histogram::getNumBins());
}

void dataManager::printDaemons()
{
  paradynDaemon::printDaemons();
}

vector<string> *dataManager::getAvailableDaemons()
{
    return(paradynDaemon::getAvailableDaemons());
}
