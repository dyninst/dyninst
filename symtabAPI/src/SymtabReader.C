/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "symtabAPI/h/SymtabReader.h"
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Symbol.h"
#include "symtabAPI/h/Function.h"

#include "symtabAPI/src/Object.h"

#include <sstream>
using std::stringstream;

using namespace Dyninst;
using namespace SymtabAPI;

SymtabReader::SymtabReader(std::string file_) :
   symtab(NULL),
   ref_count(1),
   mapped_regions(NULL),
   dwarf_handle(NULL),
   ownsSymtab(true)
{
   Symtab::openFile(symtab, file_);
}

SymtabReader::SymtabReader(const char *buffer, unsigned long size) :
   symtab(NULL),
   ref_count(1),
   mapped_regions(NULL),
   dwarf_handle(NULL),
   ownsSymtab(true)
{
   stringstream memName;
   memName << "memory_" << (unsigned long)(buffer) << "_" << size;
   Symtab::openFile(symtab, const_cast<char *>(buffer), 
                    size, memName.str());
}

SymtabReader::SymtabReader(Symtab *s) :
    symtab(s),
    ref_count(1),
    mapped_regions(NULL),
    dwarf_handle(NULL),
    ownsSymtab(false)
{}

SymtabReader::~SymtabReader()
{
   if (mapped_regions)
      delete mapped_regions;
   if (symtab && ownsSymtab)
      Symtab::closeSymtab(symtab);
   symtab = NULL;
   mapped_regions = NULL;
#if !defined(os_windows)
   if (dwarf_handle)
     delete dwarf_handle;
#endif
   dwarf_handle = NULL;
}


#define DEFN_SYMBOL_T(name) Symbol_t name; name.v1 = name.v2 = NULL; name.i1 = name.i2 = 0

Symbol_t SymtabReader::getSymbolByName(std::string symname)
{
   assert(symtab);
   DEFN_SYMBOL_T(ret);

   std::vector<Symbol *> syms;
   bool result = symtab->findSymbol(syms, symname);
   if (!result || syms.empty()) {
      return ret;
   }

   ret.v1 = symtab;
   ret.v2 = syms[0];
   return ret;
}

Symbol_t SymtabReader::getContainingSymbol(Dyninst::Offset offset)
{
   assert(symtab);
   DEFN_SYMBOL_T(ret);

   Function *f = NULL;
   bool result = symtab->getContainingFunction(offset, f);
   if (!result || !f) {
      return ret;
   }

   ret.v1 = symtab;
   ret.v2 = f->getFirstSymbol();
   return ret;
}

std::string SymtabReader::getInterpreterName()
{
   assert(symtab);
   const char *interp = NULL;
   interp = symtab->getInterpreterName();

   if (!interp)
      return std::string();
   return std::string(interp);
}

unsigned SymtabReader::getAddressWidth()
{
   assert(symtab);
   return symtab->getAddressWidth();
}
   
unsigned SymtabReader::numRegions()
{
   assert(symtab);
   if (!mapped_regions) {
      mapped_regions = new std::vector<Region *>();
      bool result = symtab->getMappedRegions(*mapped_regions);
      if (!result) {
         return 0;
      }
   }
   return mapped_regions->size();
}

bool SymtabReader::getRegion(unsigned num, SymRegion &reg)
{
   assert(symtab);
   if (!mapped_regions) {
      mapped_regions = new std::vector<Region *>();
      bool result = symtab->getMappedRegions(*mapped_regions);
      if (!result) {
         return false;
      }
   }

   if (num >= mapped_regions->size())
      return false;
   Region *region = (*mapped_regions)[num];
   reg.file_offset = region->getDiskOffset();
   reg.mem_addr = region->getMemOffset();
   reg.file_size = region->getDiskSize();
   reg.mem_size = region->getMemSize();
   reg.type = (int) region->getRegionType();
   return true;
}

Dyninst::Offset SymtabReader::getSymbolOffset(const Symbol_t &sym)
{
   assert(sym.v2);
   Symbol *symbol = (Symbol *) sym.v2;
   return symbol->getOffset();
}

std::string SymtabReader::getSymbolName(const Symbol_t &sym)
{
   assert(sym.v2);
   Symbol *symbol = (Symbol *) sym.v2;
   return symbol->getMangledName();
}

std::string SymtabReader::getDemangledName(const Symbol_t &sym) {
   assert(sym.v2);
   Symbol *symbol = (Symbol *) sym.v2;
   return symbol->getTypedName();
}

unsigned long SymtabReader::getSymbolSize(const Symbol_t &sym)
{
   assert(sym.v2);
   Symbol *symbol = (Symbol *) sym.v2;
   return symbol->getSize();
}

bool SymtabReader::isValidSymbol(const Symbol_t &sym)
{
   return (sym.v1 != NULL) && (sym.v2 != NULL);
}

Dyninst::Offset SymtabReader::imageOffset()
{
   return symtab->imageOffset();
}

Dyninst::Offset SymtabReader::dataOffset()
{
   return symtab->dataOffset();
}

Section_t SymtabReader::getSectionByName(std::string name)
{
   Region *region;
   Section_t ret;
   ret.v1 = NULL;
   bool result = symtab->findRegion(region, name);
   if (!result) {
      return ret;
   }
   ret.v1 = (void *) region;
   return ret;
}

Section_t SymtabReader::getSectionByAddress(Dyninst::Address addr)
{
   Region *region = symtab->findEnclosingRegion(addr);
   Section_t ret;
   ret.v1 = (void *) region;
   return ret;
}

Dyninst::Address SymtabReader::getSectionAddress(Section_t sec)
{
   Region *region = (Region *) sec.v1;
   assert(region);
   return region->getRegionAddr();
}

std::string SymtabReader::getSectionName(Section_t sec)
{
   Region *region = (Region *) sec.v1;
   assert(region);
   return region->getRegionName();
}

bool SymtabReader::isValidSection(Section_t sec)
{
   return (sec.v1 != NULL);
}

void *SymtabReader::getDebugInfo()
{
#if defined(os_solaris) || defined(os_linux) || defined(os_bg_ion) || defined(os_freebsd) || defined(os_vxworks)
  if (!dwarf_handle)
  {
    Object *obj = symtab->getObject();
    dwarf_handle = new DwarfHandle(obj);
  }
  Dwarf_Debug dbg = *(dwarf_handle->dbg());
  return (void *) dbg;
#else
  return NULL;
#endif
}

SymtabReaderFactory::SymtabReaderFactory()
{
}

SymtabReaderFactory::~SymtabReaderFactory()
{
}

SymReader *SymtabReaderFactory::openSymbolReader(std::string pathname)
{
   std::map<std::string, SymReader *>::iterator i = open_syms.find(pathname);
   if (i != open_syms.end()) {
      SymtabReader *symtabreader = static_cast<SymtabReader *>(i->second);
      symtabreader->ref_count++;
      return symtabreader;
   }
   SymtabReader *symtabreader = new SymtabReader(pathname);
   if (!symtabreader) { 
      return NULL;
   }
   open_syms[pathname] = symtabreader;
   return symtabreader;
}

SymReader *SymtabReaderFactory::openSymbolReader(const char *buffer, unsigned long size)
{
   SymtabReader *symtabreader = new SymtabReader(buffer, size);
   if (!symtabreader) 
      return NULL;
   return symtabreader;
}

bool SymtabReaderFactory::closeSymbolReader(SymReader *sr)
{
   SymtabReader *symreader = static_cast<SymtabReader *>(sr);
   assert(symreader->ref_count >= 1);
   symreader->ref_count--;
   if (symreader->ref_count == 0) {
      delete symreader;
   }
   return true;
}

SymbolReaderFactory *Dyninst::SymtabAPI::getSymtabReaderFactory()
{
   static SymtabReaderFactory *fact = NULL;
   if (!fact)
      fact = new SymtabReaderFactory();
   return static_cast<SymbolReaderFactory *>(fact);
}
