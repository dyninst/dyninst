/*
 * Copyright (c) 1996 Barton P. Miller
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

/*
 * DMabstractions.C: code to handle programming abstractions
 *
 * $Log: DMabstractions.C,v $
 * Revision 1.10  2003/07/15 22:45:26  schendel
 * rename string to pdstring
 *
 * Revision 1.9  2002/05/13 19:52:49  mjbrim
 * update string class to eliminate implicit number conversions
 * and replace all use of string_of with c_str  - - - - - - - - - - - - - -
 * change implicit number conversions to explicit conversions,
 * change all use of string_of to c_str
 *
 * Revision 1.8  2000/07/28 17:21:41  pcroth
 * Updated #includes to reflect util library split
 *
 * Revision 1.7  1996/08/16 21:01:31  tamches
 * updated copyright for release 1.1
 *
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
#include "common/h/Dictionary.h"
#include "common/h/String.h"

abstraction::abstraction(const char *a)
{
  pdstring aName = a;
  if(!(allAbstractions.defines(aName))){
     name = a;
     abstraction *abstr = this;
     allAbstractions[aName] = abstr;
     abstr = 0;
  }
}

void abstraction::print()
{
    printf("%s ", name.c_str());
}

abstraction *AMfind(const char *aname) 
{
  abstraction *ret = NULL;
  pdstring aName = aname;

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

