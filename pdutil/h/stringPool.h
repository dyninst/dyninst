/*
 * 
 * $Log: stringPool.h,v $
 * Revision 1.2  1994/08/05 16:01:54  hollings
 * More consistant use of stringHandle vs. char *.
 *
 * Revision 1.1  1994/01/25  20:49:42  hollings
 * First real version of utility library.
 *
 * Revision 1.1  1992/08/03  20:45:54  hollings
 * Initial revision
 *
 *
 */

#ifndef STRINGPOOL_H
#define STRINGPOOL_H

typedef void *stringHandle;

#define TAB_SIZE 10004
#define PAGE_SIZE 4090

typedef struct _stringEntry {
    stringHandle data;
    struct _stringEntry *next;
} stringEntry;

class stringPool {
    public:
	stringPool();
	stringHandle find(char *);
	stringHandle findAndAdd(char *);
    private:
	stringEntry *table[TAB_SIZE];
	stringHandle head;
	stringHandle currPage;
	stringHandle currPos;
	stringHandle getSpace(int);
};

#endif /* STRINGPOOL_H */
