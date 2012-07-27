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
 * wrapper.C
 *
 * Function wrapping mutator for Dyninst
 *
 * This program implements a simple function wrapping mutator for Dyninst.
 * 
 * Caveats: 
 *   * Currently works only with the binary rewriter. 
 *   * The wrapper must call the original function using a different
 *     name, so that we can determine the mangled name to rename the
 *     original to. If the wrapper will not call the original, use
 *     function replacement instead.
 *   * Requires that the specified names match single functions, so users
 *     must use mangled names if functions are overloaded. 
 *
 * Usage: the program takes the following arguments:
 *   -i <input executable>  : the file containing functions to be wrapped.
 *   -o <output executable> : the file that will be produced. Should be
 *                            used instead of the original input.
 *   -l <library>           : file containing wrapper functions. May be repeated.
 *   -s <specfile>          : specifies wrapping actions. Each line must contain
 *                            three space-separated names:
 *                              original wrapper clone
 *                            where original is the function to be wrapped, 
 *                            wrapper is the name of the wrapper function, 
 *                            and clone is the new name we give original to allow
 *                            it to be called directly. 
 *                            These names may be mangled or demangled, but must
 *                            follow the SymtabAPI naming specifications for each.
 */

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

#include "BPatch.h"
#include "BPatch_function.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_callbacks.h"
#include "BPatch_basicBlock.h"
#include "BPatch_flowGraph.h"
#include "CodeSource.h"
#include "PatchCFG.h"
#include "CFG.h"

#include <map>

using namespace std;
using namespace Dyninst;
using namespace SymtabAPI;

typedef vector<BPatch_function *> FuncVec;
typedef vector<Symbol *> SymbolVec;

BPatch bpatch;
BPatch_binaryEdit *app = NULL;
string output;

vector<string> specFiles;

void usage() {
   cerr << "Usage: wrapper <arguments>" << endl;
   cerr << "\t -i <input file name>" << endl;
   cerr << "\t -o <output file name>" << endl;
   cerr << "\t -l <wrapper library> (may be specified multiple times)" << endl;
   cerr << "\t -s <specification file> (may be specified multiple times)" << endl;
}

// Look up the Symbol representing the clone. We use this instead of 
// a string directly so that we can use the compiler name mangler. 
Symbol *findCloneSymbol(Symtab *symtab, string clone) {
   // Look up the provided name in each opened Symtab;
   // we're looking for an undefined symbol.
   Symbol *sym = NULL;
   SymbolVec candidates;

   if (symtab->findSymbol(candidates, clone, Symbol::ST_UNKNOWN,
                          anyName, false, false, true)) {
      // We can get multiple hits if the symbol is in both the static and dynamic 
      // symbol tables. SymtabAPI doesn't have a filter mechanism (yet...) so
      // go through it now.
      for (SymbolVec::iterator c = candidates.begin(); c != candidates.end(); ++c) {
         if ((*c)->isInDynSymtab()) {
            if (sym && sym->isInDynSymtab()) {
               cerr << "Error: multiple candidates found for clone " << clone << endl;
               exit(0);
            }
         }
         // Take any option we have, just prefer the dynamic symbol table one. 
         sym = *c;
      }
      if (candidates.empty()) {
         cerr << "Error: internal error in SymtabAPI" << endl;
         exit(0);
      }
   }
   if (!sym) {
      cerr << "Error: could not find symbol for clone " << clone << endl;
      exit(0);
   }
   return sym;
}

// Do the work of wrapping a function: look up the BPatch_function objects
// for the original and wrapper, the Symbol for the clone, and call wrapFunction.
//
// Requires that there be a single match for each. 
bool wrap(string original, string wrapper, string clone) {
   FuncVec o;
   app->getImage()->findFunction(original.c_str(), o);
   if (o.empty()) return false;
   if (o.size() > 1) { 
      cerr << "Error: multiple matches found for original function " << original << endl;
      exit(0);
   }


   FuncVec w;
   app->getImage()->findFunction(wrapper.c_str(), w, false, false, true);
   if (w.empty()) return false;
   if (w.size() > 1) {
      cerr << "Error: multiple matches found for wrapper function " << wrapper << endl;
      exit(0);
   }

   ParseAPI::CodeObject *co = w[0]->getPatchAPIFunc()->function()->obj();
   ParseAPI::SymtabCodeSource *src = dynamic_cast<ParseAPI::SymtabCodeSource *>(co->cs());
   if (!src) {
      cerr << "Error: wrapper function created from non-SymtabAPI code source" << endl;
      exit(0);
   }

   // Find the symbol for the clone
   Symbol *c = findCloneSymbol(src->getSymtabObject(), clone);
   if (!c) exit(0);

   return app->wrapFunction(o[0], w[0], c);
}

// Simple option parser

bool parseOptions(int argc, char **argv) {
   bool outputSet = false;

   int c;
   while ((c = getopt(argc, argv, "i:o:l:s:h")) != -1) {
      switch(c) {
         case 'i':
            if (app) {
               cerr << "Error: must specify only one input file" << endl;
               exit(0);
            }
            app = bpatch.openBinary(optarg, false);
            break;
         case 'o':
            outputSet = true;
            output = optarg;
            break;
         case 'l': {
            if (!app) {
               cerr << "Error: must specify input file before loading wrapper libraries" << endl;
               exit(0);
            }
            app->loadLibrary(optarg);
            break;
         }
         case 's':
            specFiles.push_back(optarg);
            break;
         case 'h':
            usage();
            exit(0);
            break;
         default:
            cerr << "Unknown argument " << (char) c << endl;
            usage();
            exit(0);
            break;
      }
   }
   if (!outputSet) {
      cerr << "Error: no output file name given" << endl;
      usage();
      exit(0);
   }
   if (!app) {
      cerr << "Error: no input file name given" << endl;
      usage();
      exit(0);
   }
   if (specFiles.empty()) {
      cerr << "Warning: no spec files for wrapping provided" << endl;
   }

   return true;
}

// And read and parse the spec file. Each line must have the format
// original wrapper clone
// and we're pretty dumb about how we handle it. 
void handleSpecFile(string specfile) {
   ifstream file;
   file.open(specfile.c_str());
   if (!file.is_open()) {
      cerr << "Error: failed to open specfile " << specfile << endl;
      exit(0);
   }
   while (file.good()) {
      string original, wrapper, clone;
      file >> original;
      file >> wrapper;
      file >> clone;
      // We get a trailing blank line...
      if (original == "" || wrapper == "" || clone == "") continue;      

      wrap(original, wrapper, clone);
   }
   file.close();
}


int main(int argc, char** argv)
{    
   parseOptions(argc, argv);

   for (vector<string>::iterator iter = specFiles.begin(); 
        iter != specFiles.end(); ++iter) {
      handleSpecFile(*iter);
   }
   
   app->writeFile(output.c_str());
   
   return 1;
}
