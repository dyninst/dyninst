/*
 * 
 * $Log: stringPool.h,v $
 * Revision 1.4  1994/09/22 03:17:00  markc
 * Changed private pointers to char* from void* since void* math is
 * illegal for ANSI
 *
 * Revision 1.3  1994/09/02  10:43:58  markc
 * Moved typedef for stringHandle outside of stringPool.h
 *
 * Revision 1.2  1994/08/05  16:01:54  hollings
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

#include "util/h/stringDecl.h"

#define TAB_SIZE 10004
#define PAGE_SIZE 4090

typedef struct _stringEntry {
    char *data;
    struct _stringEntry *next;
} stringEntry;

class stringPool {
    public:
	stringPool();
	stringHandle find(const char *);
	stringHandle findAndAdd(const char *);
    private:
	stringEntry *table[TAB_SIZE];
	char *currPage;
	char *currPos;
	char *getSpace(int);
};

#endif /* STRINGPOOL_H */
