#ifndef __AddrLookup_H__
#define __AddrLookup_H__

#include "util.h"
#include "Symtab.h"

#include <string>

namespace Dyninst {
namespace SymtabAPI {

class AddressTranslate;
class LoadedLib;

//Needed for Linux
class ProcessReader {
 public:
   int pid;

   ProcessReader(PID pid_);

   virtual bool start() = 0;
   virtual bool readAddressSpace(Address inTraced, unsigned amount, 
                                 void *inSelf) = 0;
   virtual bool done() = 0;
   virtual ~ProcessReader() {}
};

typedef struct {
   std::string name;
   Address codeAddr;
   Address dataAddr;
} LoadedLibrary;

class AddressLookup 
{
 private:
   AddressTranslate *translator;
   AddressLookup(AddressTranslate *trans);
   static hash_map<std::string, std::vector<Symbol *> > syms;
   int getSymsVector(std::string str);
   std::vector<Symbol *> *getSymsVector(LoadedLib *lib);


 public:
   DLLEXPORT static AddressLookup *createAddressLookup(ProcessReader *reader = NULL);
   DLLEXPORT static AddressLookup *createAddressLookup(PID pid, ProcessReader *reader = NULL);
   DLLEXPORT static AddressLookup *createAddressLookup(const std::vector<LoadedLibrary> &name_addrs);
   
   DLLEXPORT bool getAddress(Symtab *tab, Symbol *sym, Address &addr);
   DLLEXPORT bool getSymbol(Address addr, Symbol* &sym, Symtab* &tab, bool close = false);
   
   DLLEXPORT bool getAllSymtabs(std::vector<Symtab *> &tabs);
   DLLEXPORT bool getLoadAddress(Symtab* sym, Address &load_addr);
   DLLEXPORT bool getDataLoadAddress(Symtab* sym, Address &load_addr);

   DLLEXPORT bool getLoadAddresses(std::vector<LoadedLibrary> &name_addrs);

   DLLEXPORT bool refresh();
   
   DLLEXPORT virtual ~AddressLookup();
};

}
}

#endif
