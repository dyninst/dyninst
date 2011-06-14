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

extern int patch1_1_mutatee();
extern int patch1_2_mutatee();
extern int patch1_3_mutatee();
extern int patch2_1_mutatee();
extern int patch3_1_mutatee();
extern int patch3_2_mutatee();

mutatee_call_info_t mutatee_funcs[] = {
  {"patch1_1", patch1_1_mutatee, GROUPED, "patch1_1"},
  {"patch1_2", patch1_2_mutatee, GROUPED, "patch1_2"},
  {"patch1_3", patch1_3_mutatee, GROUPED, "patch1_3"},
  {"patch2_1", patch2_1_mutatee, GROUPED, "patch2_1"},
  {"patch3_1", patch3_1_mutatee, GROUPED, "patch3_1"},
  {"patch3_2", patch3_2_mutatee, GROUPED, "patch3_2"}
};

int max_tests = 6;
int runTest[6];
int passedTest[6];
#ifdef __cplusplus
}
#endif
