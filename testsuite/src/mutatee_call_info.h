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

#ifndef MUTATEE_CALL_INFO_H
#define MUTATEE_CALL_INFO_H

typedef enum {
  SOLO = 0,
  GROUPED = 1
} grouped_mutatee_t;

typedef struct {
  const char *testname;
  int (*func)(void);
  grouped_mutatee_t grouped;
  const char *testlabel;
} mutatee_call_info_t;

typedef struct _mutatee_info {
  mutatee_call_info_t *funcs; /* Array of mutatee_call_info structures */
  int *runTest; /* Array of flags for tests to run */
  int *passedTest; /* Array of flags for passing tests */
  unsigned int size; /* Number of elements in the arrays */
  int groupable; /* Groupable mutatee flag: controls output */
} mutatee_info_t;

extern int max_tests;

#endif /* MUTATEE_CALL_INFO_H */
