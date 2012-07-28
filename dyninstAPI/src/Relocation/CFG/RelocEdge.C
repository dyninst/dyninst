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
#include "RelocBlock.h"
#include "RelocTarget.h"
#include "RelocEdge.h"
#include <iostream>

using namespace Dyninst;
using namespace Relocation;
using namespace std;

RelocEdge::~RelocEdge() { 
   if (src) delete src;
   if (trg) delete trg;
}

RelocEdge *RelocEdges::find(ParseAPI::EdgeTypeEnum e) {
   // Returns the first one
   for (iterator iter = begin(); iter != end(); ++iter) {
     if ((*iter)->type == e) {
       return *iter;
     }
   }
   return NULL;
}

void RelocEdges::erase(RelocEdge *e) {
   for (iterator iter = begin(); iter != end(); ++iter) {
      if ((*iter) == e) {
         edges.erase(iter);
         return;
      }
   }
}

bool RelocEdges::contains(ParseAPI::EdgeTypeEnum e) {
   return (find(e) != NULL);
}

