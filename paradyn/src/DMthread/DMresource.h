/*
 * DMresource.h - define the resource data abstraction.
 *
 * $Log: DMresource.h,v $
 * Revision 1.24  1995/10/17 20:42:56  tamches
 * Removed reference to the now-obsolete class "dag"
 *
 * Revision 1.23  1995/10/13 22:07:01  newhall
 * Added code to change sampling rate as bucket width changes (this is not
 * completely implemented in daemon code yet, so now it has no effect).
 * Purify fixes.  Added phaseType parameter to sampleDataCallbackFunc
 * Added 2 new DM interface routines: getResourceName, getResourceLabelName
 *
 * Revision 1.22  1995/09/18  18:22:16  newhall
 * changes to avoid for-scope problem
 *
 * Revision 1.21  1995/09/05  16:24:19  newhall
 * added DM interface routines for PC, added resourceList method functions
 *
 * Revision 1.20  1995/08/05  17:08:42  krisna
 * updated friend entries for histDataCallback() and createResource()
 *
 * Revision 1.19  1995/08/01 02:11:21  newhall
 * complete implementation of phase interface:
 *   - additions and changes to DM interface functions
 *   - changes to DM classes to support data collection at current or
 *     global phase granularity
 * added alphabetical ordering to foci name creation
 *
 * Revision 1.18  1995/06/02  20:48:30  newhall
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
 * Revision 1.16  1995/02/16  08:17:54  markc
 * Changed Boolean to bool
 *
 * Revision 1.15  1995/01/26  17:58:26  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.14  1994/11/07  08:24:42  jcargill
 * Added ability to suppress search on children of a resource, rather than
 * the resource itself.
 *
 * Revision 1.13  1994/11/02  11:46:55  markc
 * Made sure that functions that have a return type, return that type.
 *
 * Revision 1.12  1994/09/30  21:17:46  newhall
 * changed convertToStringList method function return value from
 * stringHandle * to char**
 *
 * Revision 1.11  1994/09/30  19:17:52  rbi
 * Abstraction interface change.
 *
 * Revision 1.10  1994/09/25  01:56:27  newhall
 * added #ifndef's
 *
 * Revision 1.9  1994/09/22  00:58:02  markc
 * Added const to const char* for createResource()
 *
 * Revision 1.8  1994/08/05  16:04:01  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.7  1994/07/25  14:55:40  hollings
 * added suppress resource option.
 *
 * Revision 1.6  1994/06/27  21:23:33  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.5  1994/06/14  15:25:03  markc
 * Added new call (sameRoot) to the resource class.  This call is used to
 * determine if two resources have the same parent but are not in an
 * ancestor-descendant relationship.  Such a relationship implies a conflict
 * in the two foci.
 *
 * Revision 1.4  1994/06/02  16:08:18  hollings
 * fixed duplicate naming problem for printResources.
 *
 * Revision 1.3  1994/05/31  19:11:35  hollings
 * Changes to permit direct access to resources and resourceLists.
 *
 * Revision 1.2  1994/04/18  22:28:34  hollings
 * Changes to create a canonical form of a resource list.
 *
 * Revision 1.1  1994/02/02  00:42:36  hollings
 * Changes to the Data manager to reflect the file naming convention and
 * to support the integration of the Performance Consultant.
 *
 *
 */
#ifndef DMresource_H 
#define DMresource_H
#ifndef NULL
#define NULL 0
#endif

extern "C" {
#include <malloc.h>
#include <unistd.h>
#include <assert.h>
}

#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "dataManager.thread.h"
#include "dataManager.thread.SRVR.h"
#include "DMabstractions.h"
#include "DMinclude.h"

//
//  resources can be created, but never destroyed
//
class resource {
      friend class dataManager;
      friend class performanceStream;
      friend class resourceList;
      friend void printAllResources();
      friend class dynRPCUser;
      friend string DMcreateRLname(const vector<resourceHandle> &res);
      friend resourceHandle createResource(vector<string>&, string&);

      // TODO: these should go when PC is re-written *******************
      friend class testValue;
      friend class focus;
      friend void initResources();
//      friend class dag;

      // ***************************************************************
  public:
    vector<resourceHandle> *getChildren();
    const char *getName() { return(fullName[fullName.size()-1].string_of());}
    const char *getFullName() { return(name.string_of()); }
    const char *getAbstractionName(){ 
	if (abstr) return(abstr->getName());
        return 0;	
    }
    resourceHandle *findChild(const char*);
    int match(string ptr) { return(ptr == name); }
    bool isDescendent(resourceHandle child);
    bool sameRoot(resourceHandle other);
    void print();
    resourceHandle getParent(){ return(parent); }
    void setSuppress(bool newValue){ suppressSearch = newValue; }
    void setSuppressChildren(bool newValue){ suppressChildSearch = newValue;}
    bool getSuppress()	{ return(suppressSearch); }
    bool getSuppressChildren()	{ return(suppressChildSearch); }
    abstraction *getAbstraction() { return(abstr); }

    void AddChild(resourceHandle r){ children += r; }
    resourceHandle getHandle(){return(res_handle);}
    static bool string_to_handle(string res,resourceHandle *h);
    static const char *getName(resourceHandle);
    static const char *getFullName(resourceHandle);
    vector<string>& getParts(){return fullName;}
  protected:
    resource();
    resource(resourceHandle p_handle,
	     vector<string>& resource_name,
	     string& r_name,
	     string& abstr);
    ~resource(){}
  private:
    string name;
    resourceHandle res_handle;  
    resourceHandle parent;  
    vector<string> fullName; 
    bool suppressSearch;
    bool suppressChildSearch;
    abstraction *abstr;  // TODO: change this to a handle latter
    vector<resourceHandle> children; 
    static dictionary_hash<string, resource*> allResources;
    static vector<resource*> resources;  // indexed by resourceHandle
    static resource *rootResource;

    static resource *handle_to_resource(resourceHandle);
    static resource *string_to_resource(string);
};

// 
// a resourceList can be created, but never destroyed
//
class resourceList {
      friend class metricInstance;
      friend class dataManager;
      friend class paradynDaemon;
      // TODO: these should go when PC is re-written *******************
      friend class datum;
      friend class focus;
      friend void initResources();
      // ***************************************************************
  public:
      // resourceList(string name); 
      resourceList(const vector<resourceHandle> &resources); 
      resourceList(const vector<string> &names); 
      bool getNth(int n,resourceHandle *h) {
	  if(n < (int)(elements.size())){
	      *h = (elements[n])->getHandle();
	      return TRUE;
          }
	  return FALSE;
      }
      resourceListHandle getHandle(){return(id);}
      int getCount() { return(elements.size()); }
      void print();
      const char *getName(){return(fullName.string_of());}

      bool convertToStringList(vector< vector<string> >& fs);
      bool convertToIDList(vector<u_int>& flist);
      bool isSuppressed(){return(suppressed);}

      vector<resourceListHandle> *magnify(resourceHandle rh);
      vector<resourceListHandle> *magnify();
      resourceListHandle *constrain(resourceHandle);

      static const char *getName(resourceListHandle rh);
      static vector<resourceHandle> *getResourceHandles(resourceListHandle);
      static const resourceListHandle *find(const string &name);
      // creates new resourceList if one doesn't already exist 
      static resourceListHandle getResourceList(const vector<resourceHandle>&);
  private:
      resourceListHandle id;
      vector<resource*> elements;
      string fullName;
      bool suppressed;
      static vector<resourceList *> foci;  // indexed by resourceList id
      static dictionary_hash<string,resourceList *> allFoci;
      static resourceList *getFocus(resourceListHandle handle){
            if(handle < foci.size())
		    return(foci[handle]);
	    return 0;
      }
      static resourceList *findRL(const char *name);
};

#endif

