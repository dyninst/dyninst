/*
 * 
 * $Log: stringPool.C,v $
 * Revision 1.6  1994/09/22 03:19:13  markc
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
 * Revision 1.1  1994/01/25  20:50:27  hollings
 * First real version of utility library.
 *
 * Revision 1.4  1993/08/05  18:58:08  hollings
 * new includes
 *
 * Revision 1.3  1993/05/07  20:19:17  hollings
 * Upgrade to use dyninst interface.
 *
 * Revision 1.2  1993/01/28  19:32:44  hollings
 * bzero changed to memset.
 *
 * Revision 1.1  1992/08/03  20:42:59  hollings
 * Initial revision
 *
 *
 */
#include <stdlib.h>
#include <string.h>

#include "util/h/list.h"
#include "util/h/stringPool.h"

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
        if (g = (h & 0xf0000000)) {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }
    return(h % size);
}

stringPool::stringPool()
{
    memset(table, '\0', sizeof(table));
    currPage = new char[4090];
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

    if ((int)(currPos - currPage) + size > PAGE_SIZE) {
      // create a new page.
      currPage = new char[4090];
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
