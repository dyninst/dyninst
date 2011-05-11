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

extern int test_anno_basic_types_mutatee();
extern int test_line_info_mutatee();
extern int test_lookup_func_mutatee();
extern int test_lookup_var_mutatee();
extern int test_module_mutatee();
extern int test_relocations_mutatee();
extern int test_symtab_ser_funcs_mutatee();
extern int test_type_info_mutatee();

mutatee_call_info_t mutatee_funcs[] = {
  {"test_anno_basic_types", test_anno_basic_types_mutatee, GROUPED, "test_anno_basic_types"},
  {"test_line_info", test_line_info_mutatee, GROUPED, "test_line_info"},
  {"test_lookup_func", test_lookup_func_mutatee, GROUPED, "test_lookup_func"},
  {"test_lookup_var", test_lookup_var_mutatee, GROUPED, "test_lookup_var"},
  {"test_module", test_module_mutatee, GROUPED, "test_module"},
  {"test_relocations", test_relocations_mutatee, GROUPED, "test_relocations"},
  {"test_symtab_ser_funcs", test_symtab_ser_funcs_mutatee, GROUPED, "test_symtab_ser_funcs"},
  {"test_type_info", test_type_info_mutatee, GROUPED, "test_type_info"}
};

int max_tests = 8;
int runTest[8];
int passedTest[8];
#ifdef __cplusplus
}
#endif
