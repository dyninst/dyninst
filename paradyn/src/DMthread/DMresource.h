/*
 * DMresource.h - define the resource data abstraction.
 *
 * $Log: DMresource.h,v $
 * Revision 1.29  1996/05/02 16:17:34  tamches
 * added getMachineNameReferredTo
 * cleaned up class decls by making appropriate member fns const
 *
 * Revision 1.28  1996/04/30 18:54:00  newhall
 * changes to make enabling and disabling data asynchronous
 *
 * Revision 1.27  1996/03/01  22:47:05  mjrg
 * Added type to resources.
 *
 * Revision 1.26  1996/02/02 02:14:53  karavan
 * changed resource::magnify to return struct like magnify2.
 *
 * removed obsolete friend classes for compatibility with new PC.
 *
 *
 * Revision 1.25  1995/12/11 02:25:13  newhall
 * changed magnify2 to return the resourceList label with each
 * magnified focus
 *
 * Revision 1.24  1995/10/17  20:42:56  tamches
 * Removed reference to the now-obsolete class "dag"
 *
 * Revision 1.23  1995/10/13 22:07:01  newhall
 * Added code to change sampling rate as bucket width changes (this is not
 * completely implemented in daemon code yet, so now it has no effect).
 * Purify fixes.  Added phaseType parameter to sampleDataCallbackFunc
 * Added 2 new DM interface routines: getResourceName, getResourceLabelName
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
//  class resource: the static items basically manage a "database" of all resources.
//                  the non-static stuff gives you information about a single resource.
//  note: In the "database", resources can be created, but never destroyed.
//
class resource {
      friend class dataManager;
      friend class performanceStream;
      friend class resourceList;
      friend void printAllResources();
      friend class dynRPCUser;
      friend string DMcreateRLname(const vector<resourceHandle> &res);
      friend resourceHandle createResource(vector<string>&, string&, unsigned);

  public:
    vector<resourceHandle> *getChildren();
    const char *getName() const { return(fullName[fullName.size()-1].string_of());}
    const char *getFullName() const { return(name.string_of()); }
    const char *getAbstractionName() const { 
	if (abstr) return(abstr->getName());
        return 0;	
    }
    unsigned getType() const { return type; }
    resourceHandle *findChild(const char*) const;
    bool match(const string &ptr) const { return(ptr == name); }
    bool isDescendent(resourceHandle child) const;
    bool isDescendantOf(const resource &other) const;
    bool sameRoot(resourceHandle other) const;
    void print();
    resourceHandle getParent() const { return(parent); }
    void setSuppress(bool newValue){ suppressSearch = newValue; }
    void setSuppressChildren(bool newValue){ suppressChildSearch = newValue;}
    bool getSuppress() const { return(suppressSearch); }
    bool getSuppressChildren() const { return(suppressChildSearch); }
    abstraction *getAbstraction() { return(abstr); }

    void AddChild(resourceHandle r){ children += r; }
    resourceHandle getHandle() const {return(res_handle);}
    static bool string_to_handle (const string &res,resourceHandle *h);
    static const char *getName(resourceHandle);
    static const char *getFullName(resourceHandle);
    const vector<string>& getParts() const {return fullName;}
  protected:
    resource();
    resource(resourceHandle p_handle,
	     vector<string>& resource_name,
	     string& r_name,
	     string& abstr, unsigned type);
    ~resource(){}
  private:
    string name;
    unsigned type;   // MDL type of this resource (MDL_T_INT, MDL_T_STRING, etc)
    resourceHandle res_handle;  
    resourceHandle parent;  
    vector<string> fullName; 
    bool suppressSearch;
    bool suppressChildSearch;
    abstraction *abstr;  // TODO: change this to a handle later
    vector<resourceHandle> children; 
    static dictionary_hash<string, resource*> allResources;
    static vector<resource*> resources;  // indexed by resourceHandle
    static resource *rootResource;

    static resource *handle_to_resource(resourceHandle);
    static resource *string_to_resource(const string &);
};

// 
// a resourceList can be created, but never destroyed
//
class resourceList {
      friend class metricInstance;
      friend class dataManager;
      friend class paradynDaemon;
  public:
      // resourceList(string name); 
      resourceList(const vector<resourceHandle> &resources); 
      resourceList(const vector<string> &names); 
      bool getNth(int n,resourceHandle *h) const {
	  if(n < (int)(elements.size())){
	      *h = (elements[n])->getHandle();
	      return TRUE;
          }
	  return FALSE;
      }
      resourceListHandle getHandle() const {return(id);}
      int getCount() const { return(elements.size()); }
      void print();
      const char *getName() const {return(fullName.string_of());}

      bool convertToStringList(vector< vector<string> >& fs);
      bool convertToIDList(vector<resourceHandle>& flist);
      bool isSuppressed(){return(suppressed);}

      vector<rlNameId> *magnify(resourceHandle rh);
      vector<rlNameId> *magnify();
      resourceListHandle *constrain(resourceHandle);

      bool getMachineNameReferredTo(string &machName) const;
         // If this focus is specific to a machine, then fill in "machName" and return
         // true.  Else, leave "machName" alone and return false.
         // What does it mean for a focus to be specific to a machine?
         // For one, if the focus is a descendant of a machine, then it's obvious.
         // If the focus is a descendant of a process, then we can find a
         // machine to which it's referring, too.
         // NOTE: If this routine gets confused or isn't sure whether the resource is
         // specific to a machine, it returns false.

      static const char *getName(resourceListHandle rh);
      static bool convertToIDList(resourceListHandle rh,
				  vector<resourceHandle> &rl);
      static vector<resourceHandle> *getResourceHandles(resourceListHandle);
         // returns NULL if getFocus(resourceListHandle) fails, else
         // returns a vector allocated with new (which the caller must delete!)

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

