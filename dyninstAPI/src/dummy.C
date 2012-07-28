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
/*
 * $Id: dummy.C,v 1.8 2005/01/21 23:44:17 bernat Exp $
 *
 * Miscellaneous functions and globals that are defined in paradynd modules
 * which aren't being included in the library.
 */

#include <stdlib.h>
#include <stdio.h>

int traceSocket = 0;
int traceSocket_fd = 0;

void cleanUpAndExit(int status)
{
    exit(status);
}

extern "C" {

void __uninitialized_copy_aux__H2ZPCt6vector2ZUiZt9allocator1ZUiZPt6vector2ZUiZt9allocator1ZUi_X01X01X11G12__false_type_X11() { abort(); }
void __uninitialized_copy_aux__H2ZPC7FERNodeZP7FERNode_X01X01X11G12__false_type_X11() { abort(); }

void fill_n__H3ZPP11func_instanceZUlZP11func_instance_X01X11RCX21_X01() {}
void fill_n__H3ZPP11func_instanceZUiZP11func_instance_X01X11RCX21_X01() { }

void fill_n__H3ZPP7AstNodeZUiZP7AstNode_X01X11RCX21_X01() { }

void fill_n__H3ZPP19BPatch_variableExprZUiZP19BPatch_variableExpr_X01X11RCX21_X01() {abort(); }
void fill__H2ZPP19BPatch_variableExprZP19BPatch_variableExpr_X01X01RCX11_v() {abort(); }

void fill__H2ZPP19BPatch_variableExprZP19BPatch_variableExpr_X01T0RCX11_v() { abort(); }

void fill_n__H3ZPP8pdmoduleZUiZP8pdmodule_X01X11RCX21_X01() { }

#ifdef notdef
void __uninitialized_copy_aux__H2ZPC6stringZP6string_X01X01X11G12__false_type_X11() { 
    printf("call fill of string\n");
}
#endif

void __uninitialized_copy_aux__H2ZPCt6vector2ZUlZt9allocator1ZUlZPt6vector2ZUlZt9allocator1ZUl_X01X01X11G12__false_type_X11() {}

void fill_n__H3ZPP8pdmoduleZUlZP8pdmodule_X01X11RCX21_X01() {}

void fill_n__H3ZPP19BPatch_variableExprZUlZP19BPatch_variableExpr_X01X11RCX21_X01() {}

}
