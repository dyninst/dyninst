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

#include <stdio.h>
#include <windows.h>

#include <vector>
#include <string>

#include "symtabAPI/h/Symtab.h"
#include "addrtranslate.h"
#include "Psapi.h"

using namespace std;

namespace Dyninst {
namespace SymtabAPI {

class AddressTranslateWin : public AddressTranslate
{
private:
   bool no_proc;
public:
   virtual bool init();
   virtual bool refresh();
   AddressTranslateWin(PID pid);
   void setNoProc(bool b);
};

}
}

using namespace Dyninst;
using namespace SymtabAPI;

void AddressTranslateWin::setNoProc(bool b)
{
   no_proc = b;
}

AddressTranslate *AddressTranslate::createAddressTranslator(PID pid_, ProcessReader *)
{
	AddressTranslateWin *new_translate = new AddressTranslateWin(pid_);
	if (!new_translate)
		return NULL;
	if (new_translate->creation_error)
		return NULL;
	return new_translate;
}

AddressTranslate *AddressTranslate::createAddressTranslator(ProcessReader *)
{
	return createAddressTranslator(GetCurrentProcess(), NULL);
}

bool AddressTranslateWin::init()
{
	bool result = refresh();
	if (!result)
		creation_error = true;
	return true;
}

void printSysError(unsigned errNo) {
    char buf[1000];
    bool result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errNo, 
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  buf, 1000, NULL);
    if (!result) {
        fprintf(stderr, "Couldn't print error message\n");
        printSysError(GetLastError());
    }
    fprintf(stderr, "*** System error [%d]: %s\n", errNo, buf);
    fflush(stderr);
}

bool AddressTranslateWin::refresh()
{
	HANDLE currentProcess = pid;
	int result;

   if (no_proc)
      return true;

   for (unsigned i=0; i<libs.size(); i++)
      if (libs[i]) delete libs[i];
	libs.clear();
   
	//Calculate the number of modules
	DWORD num_modules_needed, total_space;
   result = EnumProcessModules(currentProcess,
                               NULL,
                               0,
                               &total_space);
	if (!result) {
		return false;
	}
	num_modules_needed = total_space / sizeof(HMODULE);
    
	//Get modules
	HMODULE* loadedModules = new HMODULE[num_modules_needed];
   result = EnumProcessModules(currentProcess,
                               loadedModules,
                               sizeof(HMODULE) * num_modules_needed,
                               &total_space);
	if (!result) {
      printSysError(GetLastError());
		return false;
	}
   
	//Add modules to libs
	for (HMODULE *i = loadedModules; i < loadedModules + num_modules_needed; i++)
	{
		MODULEINFO info;
		TCHAR filename[MAX_PATH];
		
		result = GetModuleInformation(currentProcess, *i, &info, sizeof(info));
		if (!result)
			continue;
      result = GetModuleFileNameEx(currentProcess, *i, filename, MAX_PATH);
      if (!result)
         continue;
      
      LoadedLib *ll = new LoadedLib(std::string(filename), (Address) info.lpBaseOfDll);
      ll->add_mapped_region((Address) info.lpBaseOfDll, info.SizeOfImage);
      libs.push_back(ll);
	}
	
	delete [] loadedModules;
	return true;
}

vector< pair<Address, unsigned long> > *LoadedLib::getMappedRegions()
{
   return &mapped_regions;
}

AddressTranslateWin::AddressTranslateWin(PID pid) :
	AddressTranslate(pid),
   no_proc(false)
{
	init();
}

static map<string, Symtab *> openedFiles;

Symtab *LoadedLib::getSymtab()
{
   if (symtable)
      return symtable;

   if (openedFiles.count(name))
   {
      symtable = openedFiles[name];
      return symtable;
   }

   bool result = Symtab::openFile(symtable, name);
   if (!result)
      return NULL;
   
   return symtable;
}

AddressTranslate *AddressTranslate::createAddressTranslator(const std::vector<LoadedLibrary> &name_addrs)
{
   AddressTranslateWin *at = new AddressTranslateWin(INVALID_HANDLE_VALUE);
   at->setNoProc(true);
   
   if (!at) {
      return NULL;
   }
   else if (at->creation_error) {
      delete at;
      return NULL;
   }
   
   for (unsigned i=0; i<name_addrs.size(); i++)
   {
      LoadedLib *ll = new LoadedLib(name_addrs[i].name, name_addrs[i].codeAddr);
      Symtab *st = ll->getSymtab();
      if (!st)
         continue;

      vector<Region> regs;
      st->getMappedRegions(regs);

      ll->add_mapped_region(name_addrs[i].codeAddr + regs[0].addr, regs[0].size);
      ll->add_mapped_region(name_addrs[i].dataAddr + regs[1].addr, regs[1].size);
   }
   return at;
}
