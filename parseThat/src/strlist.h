/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
#ifndef __STRLIST_H__
#define __STRLIST_H__

#include "compiler_annotations.h"

struct strlist_elm {
    char *data;
    strlist_elm *next;
};

struct strlist {
    strlist_elm *head, *tail;
    unsigned count;
};

#define STRLIST_INITIALIZER { NULL, NULL, 0 }

strlist *strlist_alloc() DYNINST_MALLOC_ANNOTATION;
void strlist_clear(strlist *);

// Alpha-numeric sorted insert.
bool strlist_insert(strlist *, const char *);

// Queue/Stack implementation.
bool strlist_push_front(strlist *, const char *);
bool strlist_pop_front(strlist *);
bool strlist_push_back(strlist *, const char *);
bool strlist_pop_back(strlist *);

// Data Retrieval
char *strlist_get(strlist *, unsigned);

bool strlist_cmp(strlist *, strlist *);
strlist char2strlist(char *);
char *strlist2char(strlist *);

#endif
