/*
 * DMresource.h - define the resource data abstraction.
 *
 * $Log: DMresource.h,v $
 * Revision 1.9  1994/09/22 00:58:02  markc
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
#include "dataManager.h"

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
      resource *find(const char *name);
      stringHandle getCanonicalName();
      int getCount()	{ return(count); }
      void print();
      stringHandle *convertToStringList();
  private:
      // provide mutex so we can support shared access.
      void lock() { assert(!locked); locked = 1; }
      void unlock() { assert(locked); locked = 0; } 
      volatile int locked;

      int count;
      int maxItems;
      resource **elements;
      stringHandle fullName;

      // names space of all resource list canonical names
      static stringPool names;
};

class resource {
      friend resource *createResource(resource *parent, const char *name,
				      abstractionType at);
      friend class dataManager;
      friend class performanceStream;
      friend class resourceList;
      friend void printAllResources();
      friend class dynRPCUser;
  public:
    resourceList *getChildren() { return(&children); }
    stringHandle getName() { return(name); }
    stringHandle getFullName() { return(fullName); }
    resource *findChild(char *name) { return(children.find(name)); }
    int match(char *ptr) { return(ptr == name); }
    Boolean isDescendent(resource *child);
    Boolean sameRoot(resource *child);
    void print();
    resource *getParent()	{ return(parent); }
    static resource *rootResource;
    setSuppress(Boolean nv)	{ suppressSearch = nv; }
    getSuppress()		{ return(suppressSearch); }

  protected:
    resource();
    resource(resource *parent, char *name, abstractionType at);

  private:
    resource *parent;         /* parent of this resourceBase */

    /* children of this resourceBase */
    resourceList children;  	

    stringHandle name;
    stringHandle fullName;
    abstractionType abstraction;

    List<performanceStream*> notify;

    // global variables common to all resourceBase.
    static stringPool names;
    static HTable<resource*> allResources;

    Boolean suppressSearch;		// user want's to ignore this one.
};

