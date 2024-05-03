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

#ifndef DYN_C_H
#define DYN_C_H

#include "BPatch.h"
#include "BPatch_snippet.h"
#include <fstream>
#include "stdio.h"
#include "snippetGen.h"
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include "dyninst_visibility.h"

namespace dynC_API{
   DYNINST_EXPORT BPatch_snippet *createSnippet(const char *s, BPatch_point &point);
   DYNINST_EXPORT std::map<BPatch_point *, BPatch_snippet *> *createSnippet(const char *s, std::vector<BPatch_point *> points);

   DYNINST_EXPORT BPatch_snippet *createSnippet(FILE *f, BPatch_point &point);
   DYNINST_EXPORT std::map<BPatch_point *, BPatch_snippet *> *createSnippet(FILE *f, std::vector<BPatch_point *> points);

   DYNINST_EXPORT BPatch_snippet *createSnippet(std::string str, BPatch_point &point);
   DYNINST_EXPORT std::map<BPatch_point *, BPatch_snippet *> *createSnippet(std::string str, std::vector<BPatch_point *> points);

   DYNINST_EXPORT BPatch_snippet *createSnippet(const char *s, BPatch_addressSpace &addSpace);
   DYNINST_EXPORT BPatch_snippet *createSnippet(FILE *f, BPatch_addressSpace &addSpace);
   DYNINST_EXPORT BPatch_snippet *createSnippet(std::string str, BPatch_addressSpace &addSpace);
   
//dynC internal
   std::string mangle(const char *varName, const char *snippetName, const char *typeName);
   std::string getMangledStub(const char *varName, const char *snippetName);
}

#endif
