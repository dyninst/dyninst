/*
 * DMresource.h - define the resource data abstraction.
 *
 * $Log: DMresource.h,v $
 * Revision 1.3  1994/05/31 19:11:35  hollings
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
#ifndef NULL
#define NULL 0
#endif

extern "C" {
#include <malloc.h>
#include <unistd.h>
#include <assert.h>
}

#include "util/h/list.h"
#include "util/h/stringPool.h"

class resource;

class resourceList {
  public:
      resourceList() { count = 0; 
			elements = NULL; 
			maxItems = 0; 
			fullName = NULL;
			locked = 0;
		     }
      resource *getNth(int n) {
	  lock();
	  if (n < count) {
	      unlock();
	      return(elements[n]);
	  } else {
	      unlock();
	      return(NULL);
	  }
      }
      void add(resource *r) {
	lock();
	if (count == maxItems) {
	    maxItems += 10;
	    if (elements) {
		elements = (resource**) realloc(elements, sizeof(resource*) * maxItems);
	    } else {
		elements = (resource* *) malloc(sizeof(resource*) * maxItems);
	    }
	}
	elements[count] = r;
	count++;
	if (fullName) {
	   fullName = NULL;
	}
	unlock();
	return;
      }
      resource *find(char *name);
      char *getCanonicalName();
      int getCount()	{ return(count); }
      void print();
      char **convertToStringList();
  private:
      // provide mutex so we can support shared access.
      void lock() { assert(!locked); locked = 1; }
      void unlock() { assert(locked); locked = 0; } 
      volatile int locked;

      int count;
      int maxItems;
      resource **elements;
      char *fullName;

      // names space of all resource list canonical names
      static stringPool names;
};

class resource {
      friend resource *createResource(resource *parent, char *name);
      friend class dataManager;
      friend class performanceStream;
      friend class resourceList;
      friend void printResources();
      friend class dynRPCUser;
  public:
    resourceList *getChildren() { return(&children); }
    char *getName() { return(name); }
    char *getFullName() { return(fullName); }
    resource *findChild(char *name) { return(children.find(name)); }
    int match(char *ptr) { return(ptr == name); }
    Boolean isDescendent(resource *child);
    void print();
    resource *getParent()	{ return(parent); }
    static resource *rootResource;

  protected:
    resource();
    resource(resource *parent, char *name);

  private:
    resource *parent;         /* parent of this resourceBase */

    /* children of this resourceBase */
    resourceList children;  	

    char *name;
    char *fullName;

    List<performanceStream*> notify;

    // global variables common to all resourceBase.
    static stringPool names;
    static HTable<resource*> allResources;
};
