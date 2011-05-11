/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <stdlib.h>

/* These are copied in test1.mutatee.c and libtestB.c */
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

/* Keep this function at the start of this file to kludgily ensure
// that its base address differs from its counterpart in libtestB.c
*/
void call21_1()
{
     printf("This function was not meant to be called!\n");
}

void call22_4(int x)
{
     globalVariable22_2 += x;
     globalVariable22_2 += MAGIC22_4;
}

void call22_5(int x)
{
     globalVariable22_3 += x;
     globalVariable22_3 += MAGIC22_5A;
}

void call22_6(int x)
{
     globalVariable22_4 += x;
     globalVariable22_4 += MAGIC22_6;
}
