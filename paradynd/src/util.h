/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * util.h - support functions.
 *
 * $Log: util.h,v $
 * Revision 1.10  1996/05/11 23:16:07  tamches
 * added addrHash
 *
 * Revision 1.9  1996/05/09 22:46:51  karavan
 * changed uiHash.
 *
 * Revision 1.8  1995/02/16  08:54:30  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.7  1995/02/16  08:35:05  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.6  1994/11/09  18:40:44  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.5  1994/11/02  11:18:52  markc
 * Remove old malloc wrappers.
 *
 * Revision 1.4  1994/09/22  02:27:52  markc
 * Change signature to intComp
 *
 * Revision 1.3  1994/07/28  22:40:50  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.2  1994/06/27  18:57:20  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.1  1994/01/27  20:31:49  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */
#ifndef UTIL_H
#define UTIL_H

#include "util/h/String.h"

extern void logLine(const char *line);
extern void statusLine(const char *line);
extern char errorLine[];

inline unsigned uiHash(const unsigned &val) {
  return val % 23;
}

unsigned addrHash(const unsigned &addr);

inline unsigned intHash(const int &val) {
  return val;
}

void
pd_log_perror(const char* msg);

typedef struct sym_data {
  string name;
  bool must_find;
} sym_data;


#endif /* UTIL_H */
