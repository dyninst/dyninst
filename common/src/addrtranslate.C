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

#include "common/src/addrtranslate.h"

#include <cstdio>

using namespace Dyninst;
using namespace std;

AddressTranslate::AddressTranslate(PID pid_, PROC_HANDLE phand, std::string exename) :
   pid(pid_),
   phandle(phand),
   creation_error(false),
   exec_name(exename),
   exec(NULL),
   symfactory(NULL),
   read_abort(false)
{
}

bool AddressTranslate::getLibAtAddress(Address addr, LoadedLib* &lib)
{

   /**
    * This code sets lower_lib_index and higher_lib_index to point
    * to the indexes in libs[] that have dynamic section addresses
    * that are the closest to the target address.
    **/
   Address lower_dynamic_addr = 0, higher_dynamic_addr = 0;
   int lower_lib_index = -1, higher_lib_index = -1;
   for (unsigned i=0; i<libs.size(); i++)
   {
      LoadedLib *l = libs[i];
      if (!l) continue;
      Address daddr = l->getDynamicAddr();
      if (!daddr) continue;
      if (daddr < addr) {
         if (daddr > lower_dynamic_addr) {
            lower_dynamic_addr = daddr;
            lower_lib_index = i;
         }
      }
      else if (daddr > addr) {
         if (!higher_dynamic_addr || daddr < higher_dynamic_addr) {
            higher_dynamic_addr = daddr;
            higher_lib_index = i;
         }
      }
   }
   
   /**
    * Let's search this list in a not-stupid order.  We'll
    * check lower_lib_index and higher_lib_index first
    * (the -1 and -2 cases).  If neither of those look right,
    * then we'll just have to walk the entire list.
    **/
   for (int i=-2; i< (int) libs.size(); i++)
   {
      LoadedLib *l = NULL;

      if (i == -2) {
         if (lower_lib_index ==  -1)
            continue;
         l = libs[lower_lib_index];
      }
      else if (i == -1) {
         if (higher_lib_index ==  -1)
            continue;
         l = libs[higher_lib_index];
      }
      else if (i == lower_lib_index || i == higher_lib_index) {
         continue;
      }
      else {
         l = libs[i];
      }
      if (!l)
         continue;

      vector<pair<Address, unsigned long> > *addresses = l->getMappedRegions();
      if (!addresses)
         continue;
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

#include <iostream>
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

void AddressTranslate::setReadAbort(bool b)
{
   read_abort = b;
}

AddressTranslate::~AddressTranslate()
{
   for (vector<LoadedLib *>::iterator i = libs.begin(); i != libs.end(); i++)
   {
      if (*i == exec)
         exec = NULL;
      delete *i;
   }
   if (exec) {
      delete exec;
      exec = NULL;
   }
}

LoadedLib *AddressTranslate::getExecutable()
{
   return exec;
}

bool AddressTranslate::getArchLibs(std::vector<LoadedLib *> &)
{
   return true;
}

string LoadedLib::getName() const {
   return name;
}

void LoadedLib::add_mapped_region(Address addr, unsigned long size)
{
   mapped_regions.push_back(pair<Address, unsigned long>(addr, size));   
}

void LoadedLib::setShouldClean(bool b)
{
   should_clean = b;
}

bool LoadedLib::shouldClean()
{
   return should_clean;
}

LoadedLib::LoadedLib(string n, Address la) :
   name(n),
   load_addr(la),
   data_load_addr(0),
   dynamic_addr(0),
   map_addr(0),
   should_clean(false),
   symreader(NULL),
   symreader_factory(NULL),
   up_ptr(NULL)
{
}

LoadedLib::~LoadedLib()
{
}

void LoadedLib::setDataLoadAddr(Address a)
{
   data_load_addr = a;
}

Address LoadedLib::offToAddress(Offset off)
{
   return off + getCodeLoadAddr();
}

Offset LoadedLib::addrToOffset(Address addr)
{
   return addr - getCodeLoadAddr();
}

Address LoadedLib::getCodeLoadAddr() const
{
   return load_addr;
}

Address LoadedLib::getDataLoadAddr() const
{
   return data_load_addr;
}

Address LoadedLib::getDynamicAddr() const
{
   return dynamic_addr;
}

void LoadedLib::setFactory(SymbolReaderFactory *factory)
{
   symreader_factory = factory;
}

void LoadedLib::getOutputs(string &filename, Address &code, Address &data)
{
   filename = name;
   code = load_addr;
   data = 0;
}

void* LoadedLib::getUpPtr()
{
   return up_ptr;
}

void LoadedLib::setUpPtr(void *v)
{
   up_ptr = v;
}

#include <stdarg.h>

//#if !defined(os_linux)
//This definition is for all the non-System V systems
Address AddressTranslate::getLibraryTrapAddrSysV()
{
   return 0x0;
}

