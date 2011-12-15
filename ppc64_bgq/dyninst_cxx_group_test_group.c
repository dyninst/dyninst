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
#ifdef __cplusplus
extern "C" {
#endif
#include "../src/mutatee_call_info.h"

extern int test5_1_mutatee();
extern int test5_2_mutatee();
extern int test5_3_mutatee();
extern int test5_4_mutatee();
extern int test5_5_mutatee();
extern int test5_7_mutatee();
extern int test5_8_mutatee();
extern int test5_9_mutatee();

mutatee_call_info_t mutatee_funcs[] = {
  {"test5_1", test5_1_mutatee, GROUPED, "test5_1"},
  {"test5_2", test5_2_mutatee, GROUPED, "test5_2"},
  {"test5_3", test5_3_mutatee, GROUPED, "test5_3"},
  {"test5_4", test5_4_mutatee, GROUPED, "test5_4"},
  {"test5_5", test5_5_mutatee, GROUPED, "test5_5"},
  {"test5_7", test5_7_mutatee, GROUPED, "test5_7"},
  {"test5_8", test5_8_mutatee, GROUPED, "test5_8"},
  {"test5_9", test5_9_mutatee, GROUPED, "test5_9"}
};

int max_tests = 8;
int runTest[8];
int passedTest[8];
#ifdef __cplusplus
}
#endif
