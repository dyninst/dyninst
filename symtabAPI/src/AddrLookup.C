#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Symbol.h"
#include "symtabAPI/h/AddrLookup.h"

#include "symtabAPI/src/addrtranslate.h"

#include <vector>
#include <algorithm>
#include <string>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

hash_map<string, std::vector<Symbol *> > AddressLookup::syms;

AddressLookup *AddressLookup::createAddressLookup(PID pid, ProcessReader *reader)
{
   AddressTranslate *trans = AddressTranslate::createAddressTranslator(pid, reader);
   if (!trans)
      return NULL;
   AddressLookup *ar = new AddressLookup(trans);
   if (!ar)
      return NULL;
   return ar;
}

AddressLookup *AddressLookup::createAddressLookup(ProcessReader *reader)
{
   AddressTranslate *trans = AddressTranslate::createAddressTranslator(reader);
   if (!trans)
      return NULL;
   AddressLookup *ar = new AddressLookup(trans);
   if (!ar)
      return NULL;
   return ar;
}

AddressLookup *AddressLookup::createAddressLookup(const std::vector<LoadedLibrary> &name_addrs)
{
   AddressTranslate *trans = AddressTranslate::createAddressTranslator(name_addrs);
   if (!trans)
      return NULL;
   AddressLookup *ar = new AddressLookup(trans);
   if (!ar)
      return NULL;
   return ar;
}

bool AddressLookup::getAddress(Symtab *tab, Offset off, Address &addr)
{
   LoadedLib *ll = translator->getLoadedLib(tab);
   if (!ll)
      return false;
   addr = ll->offToAddress(off);
   return true;
}

bool AddressLookup::getAddress(Symtab *tab, Symbol *sym, Address &addr)
{
   LoadedLib *ll = translator->getLoadedLib(tab);
   if (!ll)
      return false;
   addr = ll->symToAddress(sym);
   return true;
}

bool sort_by_addr(const Symbol* a, const Symbol* b)
{
   return a->getAddr() < b->getAddr();
}

vector<Symbol *> *AddressLookup::getSymsVector(LoadedLib *lib)
{
   string str = lib->getName();
   if (syms.find(str) != syms.end()) {
      return &(syms[str]);
   }
   
   Symtab *tab = lib->getSymtab();
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
   tab = lib->getSymtab();
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

   tab = lib->getSymtab();
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
      
      Offset cur_off = (*symbols)[mid]->getAddr();
      
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
      tabs.push_back(libs[i]->getSymtab());
   }

   return true;
}

bool AddressLookup::getLoadAddress(Symtab* sym, Address &load_addr)
{
   LoadedLib *ll = translator->getLoadedLib(sym);
   if (!ll)
      return false;
   load_addr = ll->getCodeLoadAddr();
   return true;
}

bool AddressLookup::getDataLoadAddress(Symtab* sym, Address &load_addr)
{
   LoadedLib *ll = translator->getLoadedLib(sym);
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
