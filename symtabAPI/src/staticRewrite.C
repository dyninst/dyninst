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

#include <stdio.h>
#include <stdlib.h>

#include "Symtab.h"
#include "Archive.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;


/* findDotOs - finds the list of .o files from archive 'arf' which
 * when added satisfy all the symbol references.
 *
 * We start with two different sets of symbols defined, undefined set
 * of symbols and a set of .o's that we need. Initially all the symbols
 * defined in the static executable fall under defined set and undefined fall
 * under the undefined set. We iterate over all the undefined symbols and try to find
 * the member in the archive which has the definition and add it to the list of .o's.
 * All the undefined symbols in this .o are added to undefined set and defined symbols
 * are added to the defined set. This process goes on until we do not have any undefined symbols.
 * Otherwise we return an error
 */
bool findDotOs(Symtab *obj, std::vector<Archive *>arfs, vector<Symtab *>&members){
    vector<Symbol *> undefSyms;

    //Initialize undefSyms with all the undefined members in the static executable
    obj->getAllUndefinedSymbols(undefSyms);

    while(undefSyms.size() != 0){
        //get the first undefined symbol
        Symbol *sym = undefSyms[0];
        undefSyms.erase(undefSyms.begin());
        Symtab *tab; // Symtab object for member that contains the definition
        vector<Symbol *> foundsyms;
        //first check if the symbol is already defined in the static executable
        if(obj->findSymbolByType(foundsyms, sym->getName(), Symbol::ST_UNKNOWN, true))
            continue;
        //check all archives starting from the beginning
        for(unsigned i=0;i<arfs.size();i++){
            if(arfs[i]->findMemberWithDefinition(tab, sym->getName())){
                members.push_back(tab);
                vector<Symbol *>undefs;
                if(tab->getAllUndefinedSymbols(undefs))
                    undefSyms.insert(undefSyms.end(), undefs.begin(), undefs.end());
                continue;
            }
        }
        //reached if there are undefined symbols that are not defined in any of the archives
        return false;
    }
    return true;
}
