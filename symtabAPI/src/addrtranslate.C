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

#include "addrtranslate.h"

using namespace Dyninst;
using namespace SymtabAPI;
using namespace std;

AddressTranslate::AddressTranslate(PID pid_) :
   pid(pid_),
   creation_error(false)
{
}

bool AddressTranslate::getLibAtAddress(Address addr, LoadedLib* &lib)
{
   for (unsigned i=0; i<libs.size(); i++)
   {
      LoadedLib *l = libs[i];
      if (!l)
         continue;
      vector<pair<Address, unsigned long> > *addresses = l->getMappedRegions();
      for (unsigned j = 0; j<addresses->size(); j++) {
         if (addr >= (*addresses)[j].first && 
             addr < (*addresses)[j].first + (*addresses)[j].second)
         {
            lib = l;
            return true;
         }
      }
   }
   return false;
}

bool AddressTranslate::getLibs(vector<LoadedLib *> &libs_)
{
   libs_.clear();
   for (unsigned i=0; i<libs.size(); i++)
      libs_.push_back(libs[i]);
   return true;
}

PID AddressTranslate::getPid()
{
   return pid;
}

LoadedLib *AddressTranslate::getLoadedLib(std::string name)
{
   for (unsigned i=0; i<libs.size(); i++)
   {
      if (libs[i]->getName() == name)
      {
         return libs[i];
      }
   }
   return NULL;
}

LoadedLib *AddressTranslate::getLoadedLib(Symtab *sym)
{
   for (unsigned i=0; i<libs.size(); i++)
   {
      if (libs[i]->symtable == sym)
      {
         return libs[i];
      }
   }
   return getLoadedLib(sym->file());
}

AddressTranslate::~AddressTranslate()
{
}

string LoadedLib::getName() const {
   return name;
}

void LoadedLib::add_mapped_region(Address addr, unsigned long size)
{
   mapped_regions.push_back(pair<Address, unsigned long>(addr, size));   
}

LoadedLib::LoadedLib(string n, Address la) :
   name(n),
   load_addr(la),
   data_load_addr(0),
   symtable(NULL)
{
}

LoadedLib::~LoadedLib()
{
}

void LoadedLib::setDataLoadAddr(Address a)
{
   data_load_addr = a;
}

Address LoadedLib::symToAddress(Symbol *sym)
{
   return sym->getAddr() + getCodeLoadAddr();
}

Address LoadedLib::offToAddress(Offset off)
{
   return off + getCodeLoadAddr();
}

Offset LoadedLib::addrToOffset(Address addr)
{
   return addr - getCodeLoadAddr();
}

Address LoadedLib::getCodeLoadAddr()
{
   return load_addr;
}

Address LoadedLib::getDataLoadAddr()
{
   return data_load_addr;
}

void LoadedLib::getOutputs(string &filename, Address &code, Address &data)
{
   filename = name;
   code = load_addr;
   data = 0;
}
