
#ifndef MAIN_H
#define MAIN_H

/* 
 * $Log: main.h,v $
 * Revision 1.3  1995/02/16 08:53:39  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.2  1995/02/16  08:33:41  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.1  1994/11/01  16:58:03  markc
 * Prototypes
 *
 */

#include "comm.h"

extern pdRPC *tp;
#endif
