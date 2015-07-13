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

#include "DynAST.h"
#include "../../dyninstAPI/src/debug.h"
#include "../../common/src/singleton_object_pool.h"

using namespace Dyninst; 
const int NOT_VISITED = 0;
const int BEING_VISITED = 1;
const int DONE_VISITED = 2;

AST::Ptr AST::substitute(AST::Ptr in, AST::Ptr a, AST::Ptr b) {
  if (!in) return in;

  if (*in == *a)
    return b;

  Children newKids;
  for (unsigned i = 0; i < in->numChildren(); ++i) {
    in->setChild(i, substitute(in->child(i), a, b));
  }
  return in;
}

AST::Ptr AST::accept(ASTVisitor *v) {
  return v->visit(this);
}

// AST cycle detector.

void AST::hasCycle(AST::Ptr in,std::map<AST::Ptr, int> &visited) {
if(!in)
	return;
  std::map<AST::Ptr, int>::iterator ssit = visited.find(in);
  if(ssit != visited.end() && (*ssit).second == BEING_VISITED) {
	//printf("Cycle Detected %p \n", (*ssit));
	  fprintf(stderr,"\nCycle detected haha\n");
	// printf("Cycle detected <- %s\n",((*ssit).first)->format());
	//assert(0);
  }
  if (ssit != visited.end() && (*ssit).second == DONE_VISITED) return;
   
  if(ssit == visited.end() || (*ssit).second == NOT_VISITED) {
	  visited.insert(std::pair<AST::Ptr, int>(in,BEING_VISITED));
	  ssit=visited.find(in);
	   for (unsigned i = 0; i < in->numChildren(); ++i) {
		hasCycle(in->child(i),visited);
		if(in->child(i)){
		//printf(" <- %p", in->child(i));
		 // printf(" <- %s\n", in->child(i)->getID());
	  }
	
	}
     (*ssit).second = DONE_VISITED;
    }
  
  return;
}


