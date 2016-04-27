// Example ParseAPI program; produces a graph (in DOT format) of the
// control flow graph of the provided binary. 
//
// Improvements by E. Robbins (er209 at kent dot ac dot uk)
//

#include <stdio.h>
#include <map>
#include <vector>
#include <unordered_map>
#include <sstream>
#include "CodeObject.h"
#include "CFG.h"

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;

int main(int argc, char * argv[])
{
   map<Address, bool> seen;
   vector<Function *> funcs;
   SymtabCodeSource *sts;
   CodeObject *co;
   
   // Create a new binary code object from the filename argument
   sts = new SymtabCodeSource( argv[1] );
   co = new CodeObject( sts );
   
   // Parse the binary
   co->parse();
   cout << "digraph G {" << endl;
   
   // Print the control flow graph
   const CodeObject::funclist& all = co->funcs();
   auto fit = all.begin();
   for(int i = 0; fit != all.end(); ++fit, i++) { // i is index for clusters
      Function *f = *fit;
      
      // Make a cluster for nodes of this function
      cout << "\t subgraph cluster_" << i 
           << " { \n\t\t label=\""
           << f->name()
           << "\"; \n\t\t color=blue;" << endl;
      
      cout << "\t\t\"" << hex << f->addr() << dec
           << "\" [shape=box";
      if (f->retstatus() == NORETURN)
         cout << ",color=red";
      cout << "]" << endl;
      
      // Label functions by name
      cout << "\t\t\"" << hex << f->addr() << dec
           << "\" [label = \""
           << f->name() << "\\n" << hex << f->addr() << dec
           << "\"];" << endl;

      stringstream edgeoutput;
      
      auto bit = f->blocks().begin();
      for( ; bit != f->blocks().end(); ++bit) {
         Block *b = *bit;
         // Don't revisit blocks in shared code
         if(seen.find(b->start()) != seen.end())
            continue;
         
         seen[b->start()] = true;
         
         cout << "\t\t\"" << hex << b->start() << dec << 
            "\";" << endl;
         
         auto it = b->targets().begin();
         for( ; it != b->targets().end(); ++it) {
            
            std::string s = "";
            if((*it)->type() == CALL)
               s = " [color=blue]";
            else if((*it)->type() == RET)
               s = " [color=green]";

            // Store the edges somewhere to be printed outside of the cluster
            edgeoutput << "\t\"" 
                       << hex << (*it)->src()->start()
                       << "\" -> \""
                       << (*it)->trg()->start()
                       << "\"" << s << endl;
         }
      }
      // End cluster
      cout << "\t}" << endl;

      // Print edges
      cout << edgeoutput.str() << endl;
   }
   cout << "}" << endl;
}
