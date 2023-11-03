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

#include "SymLite-elf.h"
#include "common/src/headers.h"
#include "unaligned_memory_access.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <iostream> 

using namespace std;
using namespace Dyninst;

SymElf::SymElf(std::string file_) :
   elf(NULL),
   fd(-1),
   need_odp(false),
   odp_section(NULL),
   file(file_),
   buffer(NULL),
   buffer_size(0),
   cache(NULL),
   cache_size(0),
   sym_sections(NULL),
   sym_sections_size(0),
   ref_count(0),
   construction_error(false)
{
   fd = open(file_.c_str(), O_RDONLY);
   if (fd == -1) {
      construction_error = true;
      return;
   }
   elf = Elf_X::newElf_X(fd, ELF_C_READ, NULL, file_);
   if (!elf->isValid()) {
      construction_error = true;
      close(fd);
      fd = -1;
      return;
   }
   init();
}

SymElf::SymElf(const char *buffer_, unsigned long buffer_size_) :
   elf(NULL),
   fd(-1),
   need_odp(false),
   odp_section(NULL),
   file(),
   buffer(buffer_),
   buffer_size(buffer_size_),
   cache(NULL),
   cache_size(0),
   sym_sections(NULL),
   sym_sections_size(0),
   ref_count(0),
   construction_error(false)
{
   elf = Elf_X::newElf_X(const_cast<char *>(buffer_), (size_t) buffer_size);
   if (!elf->isValid()) {
      construction_error = true;
      return;
   }
   init();
}

SymElf::~SymElf()
{
   if (!elf) return;
   if (elf->isValid())
      elf->end();
   if (fd != -1) {
      close(fd);
      fd = -1;
   }
   if (cache) {
      for (unsigned int i = 0; i < cache_size; ++i)  {
         if (cache[i].demangled_name)  {
            free(const_cast<char*>(cache[i].demangled_name));
         }
      }
      free(cache);
      cache = NULL;
      cache_size = 0;
   }
   if (sym_sections) {
      free(sym_sections);
      sym_sections = NULL;
      sym_sections_size = 0;
   }
}

void SymElf::init()
{
   if (elf->e_machine() == EM_PPC64) {
      unsigned short stridx = elf->e_shstrndx();
      Elf_X_Shdr strshdr = elf->get_shdr(stridx);
      Elf_X_Data strdata = strshdr.get_data();
      const char *names = (const char *) strdata.d_buf();
      
      for (unsigned i=0; i < elf->e_shnum(); i++) {
         Elf_X_Shdr &shdr = elf->get_shdr(i);
         if (strcmp(names + shdr.sh_name(), ".opd") != 0)
            continue;
         odp_section = & shdr;
         need_odp = true;
         break;
      }
   }
}

#define INVALID_SYM_CODE ((int) 0xffffffff)
#define UNSET_INDEX_CODE ((int) 0xfffffffe)

#define FOR_EACH_SYMBOL(shdr, symbols, str_buffer, idx) \
   Elf_X_Data FES_sym_data = shdr.get_data(); \
   Elf_X_Sym symbols = FES_sym_data.get_sym(); \
   int FES_str_index = shdr.sh_link(); \
   Elf_X_Shdr str_shdr = elf->get_shdr(FES_str_index); \
   if (!str_shdr.isValid()) { \
      continue; \
   } \
   Elf_X_Data FES_str_data = str_shdr.get_data(); \
   const char *str_buffer = (const char *) FES_str_data.d_buf(); \
   unsigned FES_sym_count = symbols.count(); \
   for (unsigned idx=0; idx<FES_sym_count; idx++)

#define MAKE_SYMBOL(name, idx, shdr, sym) \
   sym.v1 = (void *) (const_cast<char *>(name)); \
   sym.v2 = (void *) shdr.getScn(); \
   sym.i1 = (int) idx; \
   sym.i2 = UNSET_INDEX_CODE;

#define SET_SYM_CACHEINDEX(sym, idx) \
   sym.i2 = idx

#define GET_SYMBOL(sym, shdr, symbols, name, idx) \
   assert(sym.i2 != INVALID_SYM_CODE); \
   const char *name = (const char *) sym.v1; \
   Elf_X_Shdr shdr = Elf_X_Shdr(elf->wordSize() == 8, (Elf_Scn *) sym.v2); \
   unsigned idx = (unsigned) sym.i1; \
   Elf_X_Data sym_data = shdr.get_data(); \
   Elf_X_Sym symbols = sym_data.get_sym();
   
#define GET_INVALID_SYMBOL(sym) \
   sym.v1 = sym.v2 = NULL; \
   sym.i1 = 0; sym.i2 = INVALID_SYM_CODE;

Symbol_t SymElf::getSymbolByName(std::string symname)
{
   Symbol_t ret;
   for (unsigned i=0; i < elf->e_shnum(); i++) 
   {
      Elf_X_Shdr shdr = elf->get_shdr(i);
      if (shdr.sh_type() != SHT_SYMTAB && shdr.sh_type() != SHT_DYNSYM) {
         continue;
      } 

      FOR_EACH_SYMBOL(shdr, symbol, str_buffer, idx) 
      {
         unsigned str_loc = symbol.st_name(idx);
         if (strcmp(str_buffer+str_loc, symname.c_str()) != 0)
            continue;
	 if (symbol.st_shndx(idx) == 0) {
	   continue;
	 }

         MAKE_SYMBOL(str_buffer+str_loc, idx, shdr, ret);
         return ret;
      }
   }
   GET_INVALID_SYMBOL(ret);
   return ret;
}

Symbol_t SymElf::getContainingSymbol(Dyninst::Offset offset)
{
#if 1
   if (!cache) {
      createSymCache();
   }
   return lookupCachedSymbol(offset);

#else
   Dyninst::Offset nearest = 0;
   bool has_nearest = false;
   Symbol_t nearest_sym;

   for (unsigned i=0; i < elf->e_shnum(); i++) 
   {
      Elf_X_Shdr shdr = elf->get_shdr(i);
      if (shdr.sh_type() != SHT_SYMTAB && shdr.sh_type() != SHT_DYNSYM) {
         continue;
      } 

      FOR_EACH_SYMBOL(shdr, symbol, str_buffer, idx) 
      {
         Dyninst::Offset sym_offset = getSymOffset(symbol, idx);
         if (sym_offset <= offset && (!has_nearest || sym_offset > nearest)) {
            unsigned str_loc = symbol.st_name(idx);
            MAKE_SYMBOL(str_buffer+str_loc, idx, shdr, nearest_sym);
            has_nearest = true;
            nearest = sym_offset;
         }
      }
   }
   if (!has_nearest) {
      GET_INVALID_SYMBOL(nearest_sym);
   }

   return nearest_sym;
#endif
}

std::string SymElf::getInterpreterName()
{
   for (unsigned i=0; i < elf->e_phnum(); i++)
   {
      Elf_X_Phdr phdr = elf->get_phdr(i);
      if (phdr.p_type() != PT_INTERP)
         continue;
      Dyninst::Offset off = (Dyninst::Offset) phdr.p_offset();
      
      if (fd != -1) {
         char interp_buffer[4096];
         ssize_t result;
         do {
            result = pread(fd, interp_buffer, 4096, off);
         } while (result == -1 && errno == EINTR);
         if (result != -1) {
            return std::string(interp_buffer);
         }
      }
      else if (buffer) {
         return std::string(buffer + off);
      }

      //rawfile is expensive
      size_t filesize;
      const char *whole_file = elf->e_rawfile(filesize);
      if (filesize < off) {
         return std::string();
      }
      return std::string(whole_file + off);
   }
   return std::string();
}

unsigned SymElf::numSegments()
{
   return elf->e_phnum();
}

bool SymElf::getSegment(unsigned num, SymSegment &seg)
{
   if (num >= elf->e_phnum())
      return false;

   Elf_X_Phdr &phdr = elf->get_phdr(num);
   seg.file_offset = phdr.p_offset();
   seg.mem_addr = phdr.p_vaddr();
   seg.file_size = phdr.p_filesz();
   seg.mem_size = phdr.p_memsz();
   seg.type = phdr.p_type();
   seg.perms = phdr.p_flags() & 0x7;
   return true;
}

unsigned SymElf::getAddressWidth()
{
   return elf->wordSize();
}

bool SymElf::getABIVersion(int &major, int &minor) const
{
   if (elf->e_machine() == EM_PPC64 && elf->e_flags() == 0x2) {
      major = elf->e_flags();
      minor = 0;
      return true;
   }
   else {
      return false;
   }
}

bool SymElf::isBigEndianDataEncoding() const
{
   return (elf->e_endian() != 0);
}

Architecture SymElf::getArchitecture() const
{
    return elf->getArch();
}

unsigned long SymElf::getSymbolSize(const Symbol_t &sym)
{
   GET_SYMBOL(sym, shdr, symbol, name, idx);
   (void)name; //Silence warnings
   unsigned long size = symbol.st_size(idx);
   return size;
}

Dyninst::Offset SymElf::getSymbolOffset(const Symbol_t &sym)
{
   assert(sym.i2 != INVALID_SYM_CODE);
   if (sym.i2 != UNSET_INDEX_CODE) {
      int cache_index = sym.i2;
      return cache[cache_index].symaddress;
   }

   GET_SYMBOL(sym, shdr, symbols, name, idx);
   (void)name; //Silence warnings
   return getSymOffset(symbols, idx);
}

Dyninst::Offset SymElf::getSymbolTOC(const Symbol_t &sym)
{
   GET_SYMBOL(sym, shdr, symbols, name, idx);
   (void)name; //Silence warnings
   return getSymTOC(symbols, idx);
}

std::string SymElf::getSymbolName(const Symbol_t &sym)
{
   GET_SYMBOL(sym, shdr, symbols, name, idx);
   (void)idx; (void)symbols; //Silence warnings
   return std::string(name);
}

std::string SymElf::getDemangledName(const Symbol_t &sym)
{
   assert(sym.i2 != INVALID_SYM_CODE);
   int cache_index = -1;
   const char *name = NULL;
   if (sym.i2 != UNSET_INDEX_CODE) {
      cache_index = sym.i2;
      name = (const char *) sym.v1;
   }
   else {
      assert(0); //TODO: Lookup in cache
   }

   if (cache[cache_index].demangled_name)
      return std::string(cache[cache_index].demangled_name);

   std::string res = P_cplus_demangle(name, true);
   cache[cache_index].demangled_name = strdup(res.c_str());
   return cache[cache_index].demangled_name;
}

bool SymElf::isValidSymbol(const Symbol_t &sym)
{
   return (sym.i2 != INVALID_SYM_CODE);
}

static int symcache_cmp(const void *a, const void *b)
{
   const SymCacheEntry *aa = (const SymCacheEntry *) a;
   const SymCacheEntry *bb = (const SymCacheEntry *) b;
   if (aa->symaddress < bb->symaddress) return -1;
   else if (aa->symaddress > bb->symaddress) return 1;
   else return 0;
}

unsigned long SymElf::getSymOffset(const Elf_X_Sym &symbol, unsigned idx)
{
   if (need_odp && symbol.ST_TYPE(idx) == STT_FUNC) {
      unsigned long odp_addr = odp_section->sh_addr();
      unsigned long odp_size = odp_section->sh_size();
      const char *odp_data = (const char *) odp_section->get_data().d_buf();
      
      unsigned long sym_offset = symbol.st_value(idx);
      while (sym_offset >= odp_addr && sym_offset < odp_addr + odp_size)
         sym_offset = Dyninst::read_memory_as<uint64_t>(odp_data + sym_offset - odp_addr);
      return sym_offset;
   }

   return symbol.st_value(idx);
}

unsigned long SymElf::getSymTOC(const Elf_X_Sym &symbol, unsigned idx)
{
   if (need_odp && symbol.ST_TYPE(idx) == STT_FUNC) {
      unsigned long odp_addr = odp_section->sh_addr();
      unsigned long odp_size = odp_section->sh_size();
      const char *odp_data = (const char *) odp_section->get_data().d_buf();
      unsigned long sym_offset = symbol.st_value(idx);

      if (sym_offset < odp_addr || (sym_offset >= odp_addr + odp_size)) 
         return 0;

      auto toc = Dyninst::read_memory_as<uint64_t>(odp_data + (sym_offset - odp_addr + sizeof(long)));
      return toc;
   }

   return 0;
}

void SymElf::createSymCache()
{
   unsigned long sym_count = 0, cur_sym = 0, cur_sec = 0;
   
   if (!cache && sym_sections)
      return;

   assert(!cache);
   assert(!sym_sections);
   for (unsigned i=0; i < elf->e_shnum(); i++) 
   {
      Elf_X_Shdr shdr = elf->get_shdr(i);
      if (shdr.sh_type() != SHT_SYMTAB && shdr.sh_type() != SHT_DYNSYM) {
         continue;
      }
      Elf_X_Data sym_data = shdr.get_data();
      Elf_X_Sym symbols = sym_data.get_sym();
      sym_count += symbols.count();
      sym_sections_size++;
   }

   sym_sections = (Elf_X_Shdr *) malloc(sym_sections_size * sizeof(Elf_X_Shdr));
   if (sym_count)
      cache = (SymCacheEntry *) malloc(sym_count * sizeof(SymCacheEntry));
   
   for (unsigned i=0; i < elf->e_shnum(); i++) 
   {
      Elf_X_Shdr shdr = elf->get_shdr(i);
      if (shdr.sh_type() != SHT_SYMTAB && shdr.sh_type() != SHT_DYNSYM) {
         continue;
      }

      sym_sections[cur_sec] = shdr;
      cur_sec++;

      FOR_EACH_SYMBOL(shdr, symbols, str_buffer, idx)
      {
         (void)str_buffer; //Disable warnings
         unsigned char symtype = symbols.ST_TYPE(idx);
         if (symtype != STT_FUNC)
            continue;
         if (!symbols.st_value(idx))
            continue;
         cache[cur_sym].symaddress = getSymOffset(symbols, idx);
         cache[cur_sym].symloc = symbols.st_symptr(idx);
         cache[cur_sym].demangled_name = NULL;
         cur_sym++;
      }
   }
   cache_size = cur_sym;
   if (cache)
      cache = (SymCacheEntry *) realloc(cache, cur_sym  * sizeof(SymCacheEntry)); //Size reduction
   if (cache)
      qsort(cache, cache_size, sizeof(SymCacheEntry), symcache_cmp);
}

Symbol_t SymElf::lookupCachedSymbol(Dyninst::Offset off)
{
   unsigned min = 0;
   unsigned max = cache_size;
   unsigned cur = cache_size / 2;
   Symbol_t ret;
   
   if (!cache) {
      ret.i2 = INVALID_SYM_CODE;
      return ret;
   }

   for (;;) {
      if (max == min || min+1 == max)
         break;
      Dyninst::Offset cur_off = cache[cur].symaddress;
      if (cur_off < off) {
         min = cur;
      }
      else if (cur_off > off) {
         max = cur;
      }
      else {
         break;
      }
      cur = (min + max) / 2;
   }
   void *sym_ptr = cache[cur].symloc;

   for (unsigned i=0; i<sym_sections_size; i++) {
      Elf_X_Shdr &shdr = sym_sections[i];
      Elf_X_Data data = shdr.get_data();
      
      void *data_start = data.d_buf();
      signed long sym_offset = ((unsigned char *) sym_ptr) - ((unsigned char *) data_start);
      if (sym_offset < 0 || sym_offset >= (signed long) data.d_size())
         continue;

      //Calculate symbol index
      Elf_X_Sym syms = data.get_sym();
      unsigned sym_idx = sym_offset / syms.st_entsize();
      
      //Lookup symbol name
      unsigned int str_index = shdr.sh_link();
      Elf_X_Shdr str_shdr = elf->get_shdr(str_index);
      Elf_X_Data str_data = str_shdr.get_data();
      const char *str_buffer = (const char *) str_data.d_buf();
      const char *name = str_buffer + syms.st_name(sym_idx);
      
      MAKE_SYMBOL(name, sym_idx, shdr, ret);
      SET_SYM_CACHEINDEX(ret, cur);
      return ret;
   }
   assert(0);

   return ret;
}

Section_t SymElf::getSectionByName(std::string name)
{
   unsigned short stridx = elf->e_shstrndx();
   Elf_X_Shdr strshdr = elf->get_shdr(stridx);
   Elf_X_Data strdata = strshdr.get_data();
   const char *names = (const char *) strdata.d_buf();
   Section_t ret;
   ret.i1 = -1;

   for (unsigned i=0; i < elf->e_shnum(); i++) 
   {
      Elf_X_Shdr shdr = elf->get_shdr(i);
      const char *sname = names + shdr.sh_name();
      if (name == sname) {
         ret.i1 = i;
         break;
      }
   }
   
   return ret;
}

Section_t SymElf::getSectionByAddress(Dyninst::Address addr)
{
   Section_t ret;
   ret.i1 = -1;

   for (unsigned i=0; i < elf->e_shnum(); i++) 
   {
      Elf_X_Shdr shdr = elf->get_shdr(i);
      Dyninst::Address mem_start = shdr.sh_addr();
      unsigned long mem_size = shdr.sh_size();
      if (addr >= mem_start && addr < mem_start + mem_size) {
         ret.i1 = i;
         break;
      }
   }
   return ret;
}

Dyninst::Address SymElf::getSectionAddress(Section_t sec)
{
   assert(isValidSection(sec));
   Elf_X_Shdr shdr = elf->get_shdr(sec.i1);
   
   return shdr.sh_addr();
}

std::string SymElf::getSectionName(Section_t sec)
{
   assert(isValidSection(sec));
   Elf_X_Shdr shdr = elf->get_shdr(sec.i1);

   unsigned short stridx = elf->e_shstrndx();
   Elf_X_Shdr strshdr = elf->get_shdr(stridx);
   Elf_X_Data strdata = strshdr.get_data();
   const char *names = (const char *) strdata.d_buf();

   return std::string(names + shdr.sh_name());
}

bool SymElf::isValidSection(Section_t sec)
{
   return (sec.i1 != -1);
}

Dyninst::Offset SymElf::imageOffset() 
{ 
   assert(0); return 0;
}

Dyninst::Offset SymElf::dataOffset() 
{ 
   assert(0); return 0; 
}

void *SymElf::getElfHandle() {
   return (void *) elf;
}

namespace Dyninst {
extern map<string, SymElf *> *getSymelfCache();
}

SymElfFactory::SymElfFactory()
{
   open_symelfs = Dyninst::getSymelfCache();
   assert(open_symelfs);
}

SymElfFactory::~SymElfFactory()
{
}

SymReader *SymElfFactory::openSymbolReader(std::string pathname)
{
   SymElf *se = NULL;
   std::map<std::string, SymElf *>::iterator i = open_symelfs->find(pathname);
   if (i == open_symelfs->end()) {
      se = new SymElf(pathname);
      if (se->construction_error) {
         delete se;
         return NULL;
      }
      se->ref_count = 1;
      (*open_symelfs)[pathname] = se;
   }
   else {
      se = i->second;
      se->ref_count++;
   }
   return static_cast<SymReader *>(se);
}

SymReader *SymElfFactory::openSymbolReader(const char *buffer, unsigned long size)
{
   SymElf *se = new SymElf(buffer, size);
   if (se->construction_error) {
      delete se;
      return NULL;
   }
   se->ref_count = 1;
   return static_cast<SymReader *>(se);
}

bool SymElfFactory::closeSymbolReader(SymReader *sr)
{
   SymElf *ser = static_cast<SymElf *>(sr);
   std::map<std::string, SymElf *>::iterator i = open_symelfs->find(ser->file);
   if (i == open_symelfs->end()) {
      delete ser;
      return true;
   }

   ser->ref_count--;
   if (ser->ref_count == 0) {
      open_symelfs->erase(i);
      delete ser;
   }
   return true;
}

int SymElf::getFD()
{
  return fd;
  
}
