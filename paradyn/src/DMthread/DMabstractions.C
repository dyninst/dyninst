/*
 * DMabstractions.C: code to handle programming abstractions
 *
 * $Log: DMabstractions.C,v $
 * Revision 1.1  1994/09/30 19:17:40  rbi
 * Abstraction interface change.
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "dataManager.h"
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

abstraction *AMfind(char *aname) 
{
  abstraction *ret = NULL;
  stringHandle aName;

  aName = abstraction::names.findAndAdd(aname);
  ret = abstraction::allAbstractions.find(aName);

  return ret;
}

void AMnewMapping(char *abstr, char *type, char *key, char *value) 
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
void AMnewResource(char *parent, char *name, char *abstr)
{
  abstraction *a;

  /* Find the abstraction */
  a = AMfind(abstr);
  if (a) {
    printf("AMnewResource: received new resource for abstraction '%s'\n", 
	   (char *) a->getName());
  }
}

