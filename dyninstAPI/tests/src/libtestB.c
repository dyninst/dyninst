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

#include <stdio.h>
#include <stdlib.h>

/* These are copied in test1.mutatee.c and libtestA.c */
#define MAGIC22_1   2200100
#define MAGIC22_2   2200200
#define MAGIC22_3   2200300
#define MAGIC22_4   2200400
#define MAGIC22_5A  2200510
#define MAGIC22_5B  2200520
#define MAGIC22_6   2200600
#define MAGIC22_7   2200700

extern int globalVariable22_1;
extern int globalVariable22_2;
extern int globalVariable22_3;
extern int globalVariable22_4;

void call22_5(int x)
{
     globalVariable22_3 += x;
     globalVariable22_3 += MAGIC22_5B;
}
/* function to make regex (test 21)search non-trivial */
void cbll21_1()
{
     printf("This function was not meant to be called!\n");
}
/* function to make regex (test 21)search non-trivial */
void cbll22_1()
{
     printf("This function was not meant to be called!\n");
}

/* function to make regex (test21) search non-trivial */
void acbll22_1()
{
     printf("This function was not meant to be called!\n");
}

/* Keep this function at the end of this file to kludgily ensure that
   its base address differs from its counterpart in libtestA.c */
void call21_1()
{
     printf("This function was not meant to be called!\n");
}

