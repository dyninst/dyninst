/*
 * Generate all the templates in one file.
 *
 */

/* 
 * $Log: templatesPVM.C,v $
 * Revision 1.1  1995/12/15 22:27:07  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Changed syntax of MDL resource lists
 *
 * Revision 1.2  1994/09/22  02:59:17  markc
 * Added stronger compiler warnings
 * Removed compiler warnings
 *
 * Revision 1.1  1994/06/29  03:00:37  hollings
 * New file for PVM specific template classes.
 *
 *
 */
#pragma implementation  "list.h"

#include "util/h/list.h"

typedef struct task;

typedef List<task *> v1;
