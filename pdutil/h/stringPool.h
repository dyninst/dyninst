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
 * 
 * $Log: stringPool.h,v $
 * Revision 1.7  2000/07/26 23:02:46  hollings
 * Spilt util into common and pdutil.  util is still used (for now) to
 * build pdutil.  Changed included .C files in various templates to find
 * moved files.
 *
 * util/h files are now #include of the renamed files.   Next step is to fixup
 * existing includes.
 *
 * Revision 1.6  1996/08/16 21:30:47  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1995/02/16 09:27:14  markc
 * Modified code to remove compiler warnings.
 * Added #defines to simplify inlining.
 * Cleaned up Object file classes.
 *
 * Revision 1.4  1994/09/22  03:17:00  markc
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

#include "common/h/stringDecl.h"

#define TAB_SIZE 10004

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
