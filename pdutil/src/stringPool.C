/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
 * 
 * $Log: stringPool.C,v $
 * Revision 1.13  2004/03/23 01:12:42  eli
 * Updated copyright string
 *
 * Revision 1.12  2003/01/02 21:43:34  schendel
 * renamed list.h to List.h because Solaris native compiler was mistakenly
 * including the STL list.h;
 *
 * Revision 1.11  2000/07/28 17:22:35  pcroth
 * Updated #includes to reflect util library split
 *
 * Revision 1.10  1996/11/12 17:50:17  mjrg
 * Removed warnings, changes for compiling with Visual C++ and xlc
 *
 * Revision 1.9  1996/08/16 21:32:05  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.8  1995/11/28 15:58:35  naim
 * Minor fix. Chaning constant 4090 to SP_PAGE_SIZE - naim
 *
 * Revision 1.7  1995/02/16  09:28:13  markc
 * Removed compiler warnings.
 * Changed Boolean to bool
 *
 * Revision 1.6  1994/09/22  03:19:13  markc
 * Changed private pointers to char*
 *
 * Revision 1.5  1994/08/05  16:02:05  hollings
 * More consistant use of stringHandle vs. char *.
 *
 * Revision 1.4  1994/07/28  22:22:06  krisna
 * changed definitions of ReadFunc and WriteFunc to conform to prototypes
 *
 * Revision 1.3  1994/07/14  23:43:14  hollings
 * added abort for malloc failure.
 *
 * Revision 1.2  1994/01/26  04:53:43  hollings
 * Change to using <module>/h/{*.h}
 *
 */

#include <stdlib.h>
#include <string.h>

#include "common/h/List.h"
#include "pdutil/h/stringPool.h"

#define SP_PAGE_SIZE 4090

/*
 * Hash Function from Aho, Sethi, Ulman _Compilers_ (Second Edition)
 *   page 436.
 *
 */
static int hash(const char *ch, int size)
{
    register unsigned int h = 0, g;

    for (; *ch != '\0'; ch++) {
        h = (h << 4) + (*ch);
        if ((g = (h & 0xf0000000))) {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }
    return(h % size);
}

stringPool::stringPool()
{
    memset(table, '\0', sizeof(table));
    currPage = new char[SP_PAGE_SIZE];
    currPos = currPage;
}

stringHandle stringPool::find(const char *data)
{
    int hid;
    stringEntry *curr;

    hid = hash(data, TAB_SIZE);
    for (curr=table[hid]; curr; curr=curr->next) {
	if (!strcmp(data, curr->data)) {
	    return(curr->data);
	}
    }
    return(NULL);
}

char *stringPool::getSpace(int size)
{
    char *ret;

    if ((int)(currPos - currPage) + size > SP_PAGE_SIZE) {
      // create a new page.
      currPage = new char[SP_PAGE_SIZE];
      if (!currPage) abort();
      currPos = currPage;
    }
    ret = currPos;
    currPos += size;
    return(ret);
}

stringHandle stringPool::findAndAdd(const char *data)
{
    int hid;
    stringHandle val;
    stringEntry *temp;

    val = find(data);
    if (!val) {
	// add it.
	hid = hash(data, TAB_SIZE);
	temp = new stringEntry;
	temp->data = getSpace(strlen(data)+1);
	strcpy(temp->data, data);
	temp->next = table[hid];
	table[hid] = temp;
	val = (stringHandle) temp->data;
    }
    return(val);
}
