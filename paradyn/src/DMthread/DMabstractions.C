/*
 * DMabstractions.C: code to handle programming abstractions
 *
 * $Log: DMabstractions.C,v $
 * Revision 1.3  1995/02/16 08:09:22  markc
 * Made char* args const char*
 *
 * Revision 1.2  1995/01/26  17:58:06  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.1  1994/09/30  19:17:40  rbi
 * Abstraction interface change.
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "dataManager.thread.h"
#include "DMresource.h"
#include "util/h/list.h"

stringPool abstraction::names;
HTable<abstraction*> abstraction::allAbstractions;
abstraction *baseAbstr = new abstraction("BASE");
abstraction *cmfAbstr = new abstraction("CMF");

abstraction::abstraction(const char *a)
{
  stringHandle aName;

  aName = names.findAndAdd(a);
  allAbstractions.add(this, (void *) aName);
  name = aName;
}

void abstraction::print()
{
    printf("%s ", (char *) name);
}

abstraction *AMfind(const char *aname) 
{
  abstraction *ret = NULL;
  stringHandle aName;

  aName = abstraction::names.findAndAdd(aname);
  ret = abstraction::allAbstractions.find(aName);

  return ret;
}

void AMnewMapping(const char *abstr, const char *type, const char *key,
		  const char *value) 
{
  abstraction *a;

  /* Find the abstraction */
  a = AMfind(abstr);
  if (a) {
    printf("AMnewMapping: received new mapping for abstraction '%s'\n", 
	   (char *) a->getName());
  }
}

/*
 *  AMnewResource -- new resource 
 */
void AMnewResource(const char *parent, const char *name, const char *abstr)
{
  abstraction *a;

  /* Find the abstraction */
  a = AMfind(abstr);
  if (a) {
    printf("AMnewResource: received new resource for abstraction '%s'\n", 
	   (char *) a->getName());
  }
}

