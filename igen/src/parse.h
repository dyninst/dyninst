/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

#ifndef PARSE_H
#define PARSE_H

/*
 * Parse.h - define the classes that are used in parsing an interface.
 */

#if defined(i386_unknown_nt4_0)
// XXX kludge for bison.simple
#include <malloc.h>
#define alloca _alloca
#endif

#include "common/h/String.h"
/* trace data streams */

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include <fstream.h>
#include "remote_func.h"
#include "interface_spec.h"

extern void dump_to_dot_h(const char*);

typedef struct functype_data {
  remote_func::call_type call;
  bool is_virtual;
} Type_data;

typedef struct derived_data {
  bool is_derived;
  string *name;
} Derived_data;

typedef struct cl {
  bool b;
  bool abs;
} cl;

union parse_stack {
  string *cp;
  int i;
  unsigned u;
  float f;
  bool b;
  cl class_data;
  arg *args;
  functype_data fd;
  derived_data derived;
  vector<arg*> *arg_vector;
  interface_spec *spec;
  char *charp;
};

#endif
