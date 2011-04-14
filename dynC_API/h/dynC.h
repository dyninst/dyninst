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
//command.h
#ifndef DYN_C_H
#define DYN_C_H

#include "BPatch.h"
#include "BPatch_snippet.h"
#include <fstream>
#include "stdio.h"
#include "snippetGen.h"
#include <sstream>
#include <string>

namespace dynC_API{
   BPatch_snippet *createSnippet(const char *s, BPatch_point &point, const char *name = "");
   BPatch_snippet *createSnippet(FILE *f, BPatch_point &point, const char *name = "");
   BPatch_snippet *createSnippet(std::string str, BPatch_point &point, const char *name = "");
   BPatch_snippet *createSnippet(const char *s, BPatch_addressSpace &addSpace, const char *name = "");
   BPatch_snippet *createSnippet(FILE *f, BPatch_addressSpace &addSpace, const char *name = "");
   BPatch_snippet *createSnippet(std::string str, BPatch_addressSpace &addSpace, const char *name = "");

//   BPatch_snippet *createSnippet(std::ifstream *is, BPatch_point *point, char *name = "");
   
   std::string mangle(const char *varName, const char *snippetName, const char *typeName);
   std::string getMangledStub(const char *varName, const char *snippetName);
//   std::string getSnippetName(const char *mangled);
//   BPatch_variableExpr demangle(const char *mangledName);
}

#endif
