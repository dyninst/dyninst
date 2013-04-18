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

#include <vector>
#include <string>

#include "common/src/headers.h"
#include "addrtranslate.h"

using namespace std;

class AddressTranslateStatic : public AddressTranslate
{
public:
   virtual bool init();
   virtual bool refresh();
   AddressTranslateStatic(PID pid, PROC_HANDLE phand, std::string exename);
};

using namespace Dyninst;

AddressTranslate *AddressTranslate::createAddressTranslator(PID pid_, ProcessReader *, SymbolReaderFactory *, PROC_HANDLE phand, std::string exename, Address)
{
	AddressTranslateStatic *new_translate = new AddressTranslateStatic(pid_, phand, exename);
	return new_translate;
}

AddressTranslate *AddressTranslate::createAddressTranslator(ProcessReader *, SymbolReaderFactory *, std::string exename, Address)
{
	return createAddressTranslator(0, NULL, exename);
}

bool AddressTranslateStatic::init()
{
	return true;
}

bool AddressTranslateStatic::refresh()
{
	return true;
}

vector< pair<Address, unsigned long> > *LoadedLib::getMappedRegions()
{
   return &mapped_regions;
}

AddressTranslateStatic::AddressTranslateStatic(PID pid, PROC_HANDLE phand, std::string exename) :
	AddressTranslate(pid, phand, exename)
{
}
