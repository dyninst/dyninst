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

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Symbol.h"
#include "symtabAPI/h/AddrLookup.h"
#include "symtabAPI/h/SymtabReader.h"

#include "common/src/addrtranslate.h"


#include <vector>
#include <algorithm>
#include <string>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

dyn_hash_map<string, std::vector<Symbol *> > AddressLookup::syms;

AddressLookup *AddressLookup::createAddressLookup(PID pid, ProcessReader *reader)
{
   AddressTranslate *trans = AddressTranslate::createAddressTranslator(pid, reader, getSymtabReaderFactory());

   if (!trans)
      return NULL;
   AddressLookup *ar = new AddressLookup(trans);

   if (!ar)
      return NULL;
   return ar;
}

AddressLookup *AddressLookup::createAddressLookup(ProcessReader *reader)
{
   AddressTranslate *trans = AddressTranslate::createAddressTranslator(reader, getSymtabReaderFactory());
   if (!trans) {
      return NULL;
   }
   AddressLookup *ar = new AddressLookup(trans);
   if (!ar) {
      return NULL;
   }
   return ar;
}

AddressLookup *AddressLookup::createAddressLookup(const std::vector<LoadedLibrary> &/*name_addrs*/)
{
   assert(0); //TODO Implement
   return NULL;
}

bool AddressLookup::getAddress(Symtab *tab, Offset off, Address &addr)
{
   LoadedLib *ll = getLoadedLib(tab);
   if (!ll)
      return false;
   addr = ll->offToAddress(off);
   return true;
}

bool AddressLookup::getAddress(Symtab *tab, Symbol *sym, Address &addr)
{
   LoadedLib *ll = getLoadedLib(tab);
   if (!ll)
      return false;
   addr = symToAddress(ll, sym);
   return true;
}

bool sort_by_addr(const Symbol* a, const Symbol* b)
{
   return a->getOffset() < b->getOffset();
}

vector<Symbol *> *AddressLookup::getSymsVector(LoadedLib *lib)
{
   string str = lib->getName();
   if (syms.find(str) != syms.end()) {
      return &(syms[str]);
   }
   
   Symtab *tab = getSymtab(lib);
   
   if (!tab) {
      return NULL;
   }

   vector<Symbol *> &symbols = syms[str];
   tab->getAllSymbolsByType(symbols, Symbol::ST_UNKNOWN);
   std::sort(symbols.begin(), symbols.end(), sort_by_addr);

   return &(syms[str]);
}

bool AddressLookup::getOffset(Address addr, Symtab* &tab, Offset &off)
{
   LoadedLib *lib;
   bool result;

   result = translator->getLibAtAddress(addr, lib);
   if (!result || !lib) {
      return false;
   }

   off = lib->addrToOffset(addr);
   tab = getSymtab(lib);
   return true;
}

bool AddressLookup::getOffset(Address addr, LoadedLibrary &ll, Offset &off)
{
   LoadedLib *lib;
   bool result;

   result = translator->getLibAtAddress(addr, lib);
   if (!result || !lib) {
      return false;
   }

   off = lib->addrToOffset(addr);
   ll.name = lib->getName();
   ll.codeAddr = lib->getCodeLoadAddr();
   ll.dataAddr = lib->getDataLoadAddr();
   return true;
}

bool AddressLookup::getSymbol(Address addr, Symbol* &sym, Symtab* &tab, bool close)
{
   LoadedLib *lib;
   bool result;

   result = translator->getLibAtAddress(addr, lib);
   if (!result || !lib) {
      return false;
   }

   tab = getSymtab(lib);
   vector<Symbol *> *symbols = getSymsVector(lib);
   if (!symbols) {
      return false;
   }
   
   unsigned min = 0;
   unsigned max = symbols->size();
   unsigned mid, last_mid;
   last_mid = max+1;

   Symbol *closest = NULL;
   unsigned long closest_dist = 0;

   addr -= lib->addrToOffset(addr);

   for (;;) {
      mid = (min + max) / 2;
      if (mid == last_mid)
         break;
      last_mid = mid;
      
      Offset cur_off = (*symbols)[mid]->getOffset();
      
      if (addr == cur_off) {
         sym = (*symbols)[mid];
         return true;
      }
      if (addr < cur_off) {
         max = mid;
         continue;
      }
      if (close && (!closest || closest_dist > (cur_off - addr)))
      {
         closest_dist = cur_off - addr;
         closest = (*symbols)[mid];
      }
      if (addr > cur_off) {
         min = mid;
         continue;
      }
   }

   if (close && closest)
   {
      sym = (*symbols)[mid];
      return true;
   }

   return false;
}

bool AddressLookup::getAllSymtabs(std::vector<Symtab *> &tabs)
{
   vector<LoadedLib *> libs;
   bool result = translator->getLibs(libs);
   if (!result)
      return false;

   for (unsigned i=0; i<libs.size(); i++)
   {
      Symtab *symt = getSymtab(libs[i]);
      if (symt)
         tabs.push_back(symt);
   }

   return true;
}

bool AddressLookup::getLoadAddress(Symtab* sym, Address &load_addr)
{
   LoadedLib *ll = getLoadedLib(sym);
   if (!ll)
      return false;
   load_addr = ll->getCodeLoadAddr();
   return true;
}

bool AddressLookup::getDataLoadAddress(Symtab* sym, Address &load_addr)
{
   LoadedLib *ll = getLoadedLib(sym);
   if (!ll)
      return false;
   load_addr = ll->getDataLoadAddr();
   return true;
}

bool AddressLookup::getLoadAddresses(std::vector<LoadedLibrary> &name_addrs)
{
   vector<LoadedLib *> libs;
   bool result = translator->getLibs(libs);
   if (!result)
      return false;

   for (unsigned i=0; i<libs.size(); i++)
   {
      LoadedLibrary l;
      libs[i]->getOutputs(l.name, l.codeAddr, l.dataAddr);
      name_addrs.push_back(l);
   }

   return true;
}

Address AddressLookup::getLibraryTrapAddrSysV()
{
   return translator->getLibraryTrapAddrSysV();
}

AddressLookup::AddressLookup(AddressTranslate *at) :
   translator(at)
{
}

AddressLookup::~AddressLookup()
{
}

bool AddressLookup::refresh()
{
   return translator->refresh();
}

bool AddressLookup::getExecutable(LoadedLibrary &lib)
{
   LoadedLib *llib = translator->getExecutable();
   if (!llib)
      return false;
   llib->getOutputs(lib.name, lib.codeAddr, lib.dataAddr);
   return true;
}

Dyninst::Address AddressLookup::symToAddress(LoadedLib *ll, Symbol *sym)
{
   return ll->getCodeLoadAddr() + sym->getOffset();
}

LoadedLib *AddressLookup::getLoadedLib(Symtab *sym)
{
   std::map<Symtab *, LoadedLib *>::iterator i = sym_to_ll.find(sym);
   if (i != sym_to_ll.end()) {
      return i->second;
   }
   
   vector<LoadedLib *> libs;
   translator->getLibs(libs);
   for (auto ll : libs) {
      if (sym->file() == ll->getName() || 
          sym->name() == ll->getName())
      {
         ll_to_sym[ll] = sym;
         sym_to_ll[sym] = ll;
         return ll;
      }
   }
   return NULL;
}

Symtab *AddressLookup::getSymtab(LoadedLib *ll)
{
   std::map<LoadedLib *, Symtab *>::iterator i = ll_to_sym.find(ll);
   if (i != ll_to_sym.end()) {
      return i->second;
   }
   
   Symtab *sym;
   bool result = Symtab::openFile(sym, ll->getName());
   if (!result) {
      return NULL;
   }
   ll_to_sym[ll] = sym;
   sym_to_ll[sym] = ll;
   return sym;
}
