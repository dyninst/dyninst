/*
 * DMresource.h - define the resource data abstraction.
 *
 * $Log: DMresource.h,v $
 * Revision 1.1  1994/02/02 00:42:36  hollings
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
}

#include "util/h/list.h"
#include "util/h/stringPool.h"

class resource;

class resourceList {
  public:
      resourceList() { count = 0; elements = NULL; maxItems = 0; }
      resource *getNth(int n) {
	  if (n < count) {
	      return(elements[n]);
	  } else {
	      return(NULL);
	  }
      }
      void add(resource *r) {
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
	return;
      }
      resource *find(char *name);
      int getCount()	{ return(count); }
      void print();
      char **convertToStringList();
  private:
      int count;
      int maxItems;
      resource **elements;
};

class resource {
      friend resource *createResource(resource *parent, char *name);
      friend class resourceList;
  public:
    resourceList *getChildren() { return(&children); }
    char *getName() { return(name); }
    char *getFullName() { return(fullName); }
    int match(char *ptr) { return(ptr == name); }
    Boolean isDescendent(resource *child);
    void print();

    resource *parent;         /* parent of this resourceBase */

    /* children of this resourceBase */
    resourceList children;  	

    // global variables common to all resourceBase.
    static stringPool names;
    static resource *rootResource;
    static HTable<resource*> allResources;
    List<performanceStream*> notify;

  protected:
    resource();
    resource(resource *parent, char *name);

  private:
    char *name;
    char *fullName;
};
