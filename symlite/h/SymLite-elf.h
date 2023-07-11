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

#include "SymReader.h"
#include "Elf_X.h"

#include <string>
#include <map>

namespace Dyninst {

struct SYMLITE_EXPORT SymCacheEntry {
   Dyninst::Offset symaddress;
   void *symloc;
   const char *demangled_name;
};

class SYMLITE_EXPORT SymElf : public Dyninst::SymReader
{
   friend class SymElfFactory;
 private:
   Elf_X *elf;
   int fd;
   bool need_odp;
   Elf_X_Shdr *odp_section;

   std::string file;
   const char *buffer;
   unsigned long buffer_size;

   SymElf(std::string file_);
   SymElf(const char *buffer_, unsigned long size_);
   virtual ~SymElf();

   SymCacheEntry *cache;
   unsigned cache_size;

   Elf_X_Shdr *sym_sections;
   unsigned sym_sections_size;
   
   void createSymCache();
   Symbol_t lookupCachedSymbol(Dyninst::Offset offset);
   
   void init();
   unsigned long getSymOffset(const Elf_X_Sym &symbol, unsigned idx);   
   unsigned long getSymTOC(const Elf_X_Sym &symbol, unsigned idx);   
 public:
   virtual Symbol_t getSymbolByName(std::string symname);
   virtual Symbol_t getContainingSymbol(Dyninst::Offset offset);
   virtual std::string getInterpreterName();

   virtual unsigned numSegments();
   virtual bool getSegment(unsigned num, SymSegment &reg); 

   virtual Dyninst::Offset getSymbolOffset(const Symbol_t &sym);
   virtual Dyninst::Offset getSymbolTOC(const Symbol_t &sym);
   virtual std::string getSymbolName(const Symbol_t &sym);
   virtual std::string getDemangledName(const Symbol_t &sym);

   virtual bool isValidSymbol(const Symbol_t &sym);
   virtual unsigned getAddressWidth();
   virtual bool getABIVersion(int &major, int &minor) const;
   virtual bool isBigEndianDataEncoding() const;
   virtual Architecture getArchitecture() const;

   virtual unsigned long getSymbolSize(const Symbol_t &sym);
   virtual Section_t getSectionByName(std::string name);
   virtual Section_t getSectionByAddress(Dyninst::Address addr);
   virtual Dyninst::Address getSectionAddress(Section_t sec);
   virtual std::string getSectionName(Section_t sec);
   virtual bool isValidSection(Section_t sec);

   virtual Dyninst::Offset imageOffset();
   virtual Dyninst::Offset dataOffset();
   int ref_count;
   bool construction_error;
   
   void *getElfHandle();
   int getFD();
   
};

class SYMLITE_EXPORT SymElfFactory : public Dyninst::SymbolReaderFactory
{
private:
   std::map<std::string, SymElf *> *open_symelfs;
public:
   SymElfFactory();
   virtual ~SymElfFactory();
   virtual SymReader *openSymbolReader(std::string pathname);
   virtual SymReader *openSymbolReader(const char *buffer, unsigned long size);
   virtual bool closeSymbolReader(SymReader *sr);
};

}
