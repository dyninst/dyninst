/*
 * DMabstractions.C: code to handle programming abstractions
 *
 * $Log: DMabstractions.C,v $
 * Revision 1.6  1996/01/05 20:00:40  newhall
 * removed warnings
 *
 * Revision 1.5  1995/11/08 06:22:19  tamches
 * removed some warnings
 *
 * Revision 1.4  1995/06/02 20:48:12  newhall
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
 * Revision 1.3  1995/02/16  08:09:22  markc
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
#include "DMabstractions.h"
#include "DMresource.h"
#include "util/h/Dictionary.h"
#include "util/h/String.h"

abstraction::abstraction(const char *a)
{
  string aName = a;
  if(!(allAbstractions.defines(aName))){
     name = a;
     abstraction *abstr = this;
     allAbstractions[aName] = abstr;
     abstr = 0;
  }
}

void abstraction::print()
{
    printf("%s ", name.string_of());
}

abstraction *AMfind(const char *aname) 
{
  abstraction *ret = NULL;
  string aName = aname;

  if(abstraction::allAbstractions.defines(aName))
      ret = abstraction::allAbstractions[aName];
  return ret;
}

void AMnewMapping(const char *abstr, const char *, const char *,
		  const char *) 
{
  abstraction *a;

  /* Find the abstraction */
  a = AMfind(abstr);
  if (a) {
    printf("AMnewMapping: received new mapping for abstraction '%s'\n", 
	    a->getName());
  }
}

/*
 *  AMnewResource -- new resource 
 */
void AMnewResource(const char *, const char *, const char *abstr)
{
  abstraction *a;

  /* Find the abstraction */
  a = AMfind(abstr);
  if (a) {
    printf("AMnewResource: received new resource for abstraction '%s'\n", 
	   a->getName());
  }
}

