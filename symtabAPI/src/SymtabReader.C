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

#include "symtabAPI/h/SymtabReader.h"
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Symbol.h"
#include "symtabAPI/h/Function.h"

#include "symtabAPI/src/Object.h"
#include <queue>
#include <iostream>
using namespace std;

#include <sstream>
using std::stringstream;

using namespace Dyninst;
using namespace SymtabAPI;

SymtabReader::SymtabReader(std::string file_) :
   symtab(NULL),
   ref_count(1),
   ownsSymtab(true)
{
  // We'd throw, but...
  (void)Symtab::openFile(symtab, file_);
}

SymtabReader::SymtabReader(const char *buffer, unsigned long size) :
   symtab(NULL),
   ref_count(1),
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
    ownsSymtab(false)
{}

SymtabReader::~SymtabReader()
{
   if (symtab && ownsSymtab)
      Symtab::closeSymtab(symtab);
   symtab = NULL;
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
   
bool SymtabReader::getABIVersion(int &major, int &minor) const
{
   assert(symtab);
   return symtab->getABIVersion(major, minor);
}

bool SymtabReader::isBigEndianDataEncoding() const
{
   assert(symtab);
   return symtab->isBigEndianDataEncoding();
}

Architecture SymtabReader::getArchitecture() const
{
    return symtab->getArchitecture();
}

unsigned SymtabReader::numSegments()
{
   buildSegments();
   return segments.size();
}

bool SymtabReader::getSegment(unsigned num, SymSegment &seg)
{
   buildSegments();
   if (num >= segments.size()) return false;
   seg = segments[num];

   return true;
}

void SymtabReader::buildSegments() {
   if (!segments.empty()) return;

   // We want ELF segments; contiguous areas of the 
   // binary loaded into memory. 
   symtab->getSegmentsSymReader(segments);
}


Dyninst::Offset SymtabReader::getSymbolOffset(const Symbol_t &sym)
{
   assert(sym.v2);
   Symbol *symbol = (Symbol *) sym.v2;
   return symbol->getOffset();
}

Dyninst::Offset SymtabReader::getSymbolTOC(const Symbol_t &sym)
{
   assert(sym.v2);
   Symbol *symbol = (Symbol *) sym.v2;
   return symbol->getSymtab()->getTOCoffset(symbol->getOffset());
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
   return region->getMemOffset();
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

void *SymtabReader::getElfHandle()
{
#if defined(os_linux) || defined(os_freebsd)
   Object *obj = symtab->getObject();
   return obj->getElfHandle();
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
      SymtabReader *symtabreader = dynamic_cast<SymtabReader *>(i->second);
      symtabreader->ref_count++;
      return symtabreader;
   }
   SymtabReader *symtabreader = new SymtabReader(pathname);
   if (!symtabreader) { 
      return NULL;
   }
   if(!symtabreader->symtab) {
     delete symtabreader;
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
     // We need to remove this from the big map, but we don't 
     // store the path. So crawl and look. 
     std::queue<std::string> toDelete;
     std::map<std::string, SymReader *>::iterator i;
     for (i = open_syms.begin(); i != open_syms.end(); ++i) {
       if (i->second == symreader) {
	 toDelete.push(i->first);
       }
     }
     while (!toDelete.empty()) {
       open_syms.erase(toDelete.front());
       toDelete.pop();
     }

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
