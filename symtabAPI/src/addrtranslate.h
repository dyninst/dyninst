/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef addrtranslate_h_
#define addrtranslate_h_

#include <string>
#include <vector>
#include <utility>

#include "common/h/Dictionary.h"
#include "common/h/Types.h"
#include "dynutil/h/dyntypes.h"
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/AddrLookup.h"

using namespace std;

namespace Dyninst {
namespace SymtabAPI {

class LoadedLib {
   friend class AddressTranslate;
 protected:
   string name;
   Address load_addr;
   Address data_load_addr;
   vector< pair<Address, unsigned long> > mapped_regions;
   Symtab *symtable;

 public:
   LoadedLib(string name, Address load_addr);
   void add_mapped_region(Address addr, unsigned long size);
   
   string getName() const;
   void setDataLoadAddr(Address a);
   vector< pair<Address, unsigned long> > *getMappedRegions();
   virtual Symtab *getSymtab();
   virtual ~LoadedLib();

   virtual Address symToAddress(Symbol *sym);
   virtual Address offToAddress(Offset off);
   virtual Offset addrToOffset(Address addr);

   virtual Address getCodeLoadAddr();
   virtual Address getDataLoadAddr();
   virtual void getOutputs(string &filename, Address &code, Address &data);
};

class AddressTranslate {
 protected:
   PID pid;
   bool creation_error;

   AddressTranslate(PID pid);
   vector<LoadedLib *> libs;
 public:

   static AddressTranslate *createAddressTranslator(PID pid_,
                                                    ProcessReader *reader_ = NULL);
   static AddressTranslate *createAddressTranslator(ProcessReader *reader_ = NULL);
   static AddressTranslate *createAddressTranslator(const std::vector<LoadedLibrary> &name_addrs);
   
   virtual bool refresh() = 0;
   virtual ~AddressTranslate();
  
   PID getPid();
   bool getLibAtAddress(Address addr, LoadedLib* &lib);
   bool getLibs(vector<LoadedLib *> &libs_);
   LoadedLib *getLoadedLib(std::string name);
   LoadedLib *getLoadedLib(Symtab *sym);
};

}
}

#endif
