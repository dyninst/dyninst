/*
 * Copyright (c) 1996-1998 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: DMresource.h,v 1.46 2002/10/28 04:54:17 schendel Exp $

#ifndef DMresource_H 
#define DMresource_H
#ifndef NULL
#define NULL 0
#endif

extern "C" {
#include <malloc.h>
#include <assert.h>

#if !defined(i386_unknown_nt4_0)
#include <unistd.h>
#endif // !defined(i386_unknown_nt4_0)
}

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "dataManager.thread.h"
#include "dataManager.thread.SRVR.h"
#include "DMabstractions.h"
#include "DMinclude.h"

class CallGraph;

//
//class resource: the static items basically manage a "database" of all
//resources. the non-static stuff gives you information about a single resource.
//note: In the "database", resources can be created, but never destroyed.
//
class resource {
      friend class dataManager;
      friend class performanceStream;
      friend class resourceList;
      friend void printAllResources();
      friend class dynRPCUser;

      // call graph needs to be able to go from resource names to resource *s....
      friend class CallGraph;

      friend string DMcreateRLname(const vector<resourceHandle> &res);
      friend resourceHandle createResource(unsigned, vector<string>&, string&, unsigned);
      friend resourceHandle createResource_ncb(vector<string>&, string&, unsigned, 
					resourceHandle&, bool&);
      friend void getMemoryBounds(vector<metric_focus_pair> *request) ;

  public:
    vector<resourceHandle> *getChildren();
    const char *getName() const { 
	return(fullName[fullName.size()-1].c_str());}
    const char *getFullName() const { return(name.c_str()); }
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
    bool isMagnifySuppressed() { return (suppressMagnify); }
    void setSuppressMagnify() {suppressMagnify = true;}
    void clearSuppressMagnify() {suppressMagnify = false;}
    bool isStartFunction();
    bool isThreadType(unsigned *tid);

    void AddChild(resourceHandle r){ children += r; }
    resourceHandle getHandle() const {return(res_handle);}
    static bool string_to_handle (const string &res,resourceHandle *h);
    static const char *getName(resourceHandle);
    static const char *getFullName(resourceHandle);
    const vector<string>& getParts() const {return fullName;}
    static bool get_lib_constraints(vector<string>&, vector<unsigned>&);
    static bool get_func_constraints(vector< vector<string> >&, vector<unsigned>&);
    static void saveHierarchiesToFile (ofstream& foo);
    void saveHierarchyToFile (ofstream& foo);

    // Boolean value indicating whether the specified magnify type is
    //  applicable to the particular node.
    // Added when changes put in to support different PC searches
    //  necessitated some types of magnification which semantically
    //  apply to only some resources....
    bool MagnifyTypeApplies(magnifyType t) {
        assert(t == OriginalSearch || t == CallGraphSearch);
	if (t == OriginalSearch) return true;
	assert(t == CallGraphSearch);
	if (!strcmp(fullName[0].c_str(), "Code")) {
	    return true;
	}
	return false;
    }
    static resource *handle_to_resource(resourceHandle);
  protected:
    resource();
    resource(resourceHandle p_handle,
	     unsigned tempId, // the temporary id generated by the daemon
	     vector<string>& resource_name,
	     string& r_name,
	     string& abstr, unsigned type);
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

    bool suppressMagnify;  // true if mdl exclude_lib option specifies this 
    abstraction *abstr;  // TODO: change this to a handle later
    vector<resourceHandle> children; 
    static dictionary_hash<string, resource*> allResources;
    static dictionary_hash<unsigned, resource*> resources;  // indexed by resourceHandle
    static resource *rootResource;
    static vector<string> lib_constraints;
	static vector<unsigned> lib_constraint_flags;
    // each element is a module string and a function string
    static vector< vector<string> > func_constraints;
    static vector<unsigned> func_constraint_flags;
    static bool func_constraints_built;
    static bool lib_constraints_built;

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
      const char *getName() const {return(fullName.c_str());}

      bool convertToStringList(vector< vector<string> >& fs);
      bool convertToIDList(vector<resourceHandle>& flist);
      bool isSuppressed(){return(suppressed);}

      // Syntacticaly assuming here that all magnify types are equally
      //  applicable to all resources - conceptually this is not the case
      //  (CallGraphSearch applies to magnifying down the call graph only),
      //  - but this makes the interface more uniform and cleaner....
      // Therefore, when something which handles non-standard magnification
      //  types for a resource hierarchy which it does not handle, it
      //  should NOT crash, but rather should return an empty resource list....
      vector<rlNameId> *magnify(resourceHandle rh, magnifyType type, 
				resource *currentPath);
      vector<rlNameId> *magnify(magnifyType type);
      resourceListHandle *constrain(resourceHandle);

      // returns the thread id if the resource list includes a path through
      // a machine, process, and thread resource.
      // returns -1 if thread not defined
      int getThreadID();

      bool getMachineNameReferredTo(string &machName) const;
         // If this focus is specific to a machine, then fill in "machName"
	 // and return true.  Else, leave "machName" alone and return false.
         // What does it mean for a focus to be specific to a machine?
         // For one, if the focus is a descendant of a machine, then it's 
	 // obvious.  If the focus is a descendant of a process, then we can
	 // find a machine to which it's referring, too.
         // NOTE: If this routine gets confused or isn't sure whether the
	 // resource is  specific to a machine, it returns false.

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

