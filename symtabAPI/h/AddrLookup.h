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

#ifndef __AddrLookup_H__
#define __AddrLookup_H__

#include "Annotatable.h"
#include <string>
#include <vector>
#include <map>

namespace Dyninst {

class AddressTranslate;
class LoadedLib;

namespace SymtabAPI {

typedef struct {
   std::string name;
   Address codeAddr;
   Address dataAddr;
} LoadedLibrary;

class SYMTAB_EXPORT AddressLookup : public AnnotatableSparse
{
 private:
   AddressTranslate *translator;
   AddressLookup(AddressTranslate *trans);
   static dyn_hash_map<std::string, std::vector<Symbol *> > syms;
   int getSymsVector(std::string str);
   std::vector<Symbol *> *getSymsVector(LoadedLib *lib);

   std::map<Symtab *, LoadedLib *> sym_to_ll;
   std::map<LoadedLib *, Symtab *> ll_to_sym;

   LoadedLib *getLoadedLib(Symtab *sym);
   Dyninst::Address symToAddress(LoadedLib *ll, Symbol *sym);
   Symtab *getSymtab(LoadedLib *);
 public:
   static AddressLookup *createAddressLookup(ProcessReader *reader = NULL);
   static AddressLookup *createAddressLookup(PID pid, ProcessReader *reader = NULL);
   static AddressLookup *createAddressLookup(const std::vector<LoadedLibrary> &name_addrs);
   
   bool getAddress(Symtab *tab, Symbol *sym, Address &addr);
   bool getAddress(Symtab *tab, Offset off, Address &addr);

   bool getSymbol(Address addr, Symbol* &sym, Symtab* &tab, bool close = false);
   bool getOffset(Address addr, Symtab* &tab, Offset &off);
   
   bool getAllSymtabs(std::vector<Symtab *> &tabs);
   bool getLoadAddress(Symtab* sym, Address &load_addr);
   bool getDataLoadAddress(Symtab* sym, Address &load_addr);

   bool getLoadAddresses(std::vector<LoadedLibrary> &name_addrs);
   bool getExecutable(LoadedLibrary &lib);
   bool getOffset(Address addr, LoadedLibrary &lib, Offset &off);

   bool refresh();

   Address getLibraryTrapAddrSysV();
   
   virtual ~AddressLookup();
};

}
}

#endif
