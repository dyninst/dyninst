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

void fill_n__H3ZPP11int_functionZUlZP11int_function_X01X11RCX21_X01() {}
void fill_n__H3ZPP11int_functionZUiZP11int_function_X01X11RCX21_X01() { }

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
