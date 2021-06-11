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
 
#include "Graph.h"
#include "Node.h"
#include "Edge.h"
#include <string>
#include <stdio.h>
 
using namespace Dyninst;
using namespace std;
 
bool Graph::printDOT(const std::string& fileName) {
	
    FILE *file = fopen(fileName.c_str(), "w");
    if (file == NULL) {
        return false;
    }
    fprintf(file, "digraph G {\n");

    NodeSet visited;
    std::queue<Node::Ptr> worklist;

    NodeIterator entryBegin, entryEnd;
    entryNodes(entryBegin, entryEnd);

     // Initialize visitor worklist
    for (NodeIterator iter = entryBegin; iter != entryEnd; ++iter) {
        worklist.push(*iter);
    }

    // Put the entry nodes on their own (minimum) rank
    fprintf(file, "  { rank = min;");
    for (NodeIterator iter = entryBegin; iter != entryEnd; ++iter) {
      fprintf(file, "\"%p\"; ", (void*)(*iter).get());
    }
    fprintf(file, "}\n");

    NodeIterator exitBegin, exitEnd;
    exitNodes(exitBegin, exitEnd);

    // Put the entry nodes on their own (minimum) rank
    fprintf(file, "  { rank = max;");
    for (NodeIterator iter = exitBegin; iter != exitEnd; ++iter) {
      fprintf(file, "\"%p\"; ", (void*)(*iter).get());
    }
    fprintf(file, "}\n");
    

    while (!worklist.empty()) {
        Node::Ptr source = worklist.front();

        worklist.pop();

         //fprintf(stderr, "Considering node %s\n", source->format().c_str());

         // We may have already treated this node...
        if (visited.find(source) != visited.end()) {
             //fprintf(stderr, "\t skipping previously visited node\n");
            continue;
        }
         //fprintf(stderr, "\t inserting %s into visited set, %d elements pre-insert\n", source->format().c_str(), visited.size());
        visited.insert(source);

        fprintf(file, "\t%s\n", source->DOTshape().c_str());
        fprintf(file, "\t%s\n", source->DOTrank().c_str());
	fprintf(file, "\t\"%p\" [label=\"%s\"];\n", 
		(void*)source.get(), source->DOTname().c_str());

        NodeIterator outBegin, outEnd;
        source->outs(outBegin, outEnd);

        for (; outBegin != outEnd; ++outBegin) {
            Node::Ptr target = *outBegin;
            if (!target->DOTinclude()) continue;
            //fprintf(file, "\t %s -> %s;\n", source->DOTname().c_str(), target->DOTname().c_str());
	    fprintf(file, "\t \"%p\" -> \"%p\";\n", (void*)source.get(), (void*)target.get());
            if (visited.find(target) == visited.end()) {
                 //fprintf(stderr, "\t\t adding child %s\n", target->format().c_str());
                worklist.push(target);
            }
            else {
                 //fprintf(stderr, "\t\t skipping previously visited child %s\n", 
                 //target->format().c_str());
            }
        }
    }
    fprintf(file, "}\n\n\n");
    fclose(file);

    return true;
}

std::string Node::DOTname() const {
    return format();
}

std::string Node::DOTshape() const {
    // Use defaults...
    return std::string("// ") + format() + std::string(" [shape=ellipse];");
}

std::string Node::DOTrank() const {
    // Use defaults...
    return std::string("// ") + format();
}

std::string VirtualNode::DOTshape() const {
    return format() + std::string(" [shape=box];"); 
}
