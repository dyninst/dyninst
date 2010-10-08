#include "dynutil/h/SymReader.h"
#include "common/h/Elf_X.h"
#include "common/h/headers.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include <map>

struct SymCacheEntry {
   Dyninst::Offset symaddress;
   void *symloc;
   const char *demangled_name;
};

class SymElf : public Dyninst::SymReader
{
   friend class SymElfFactory;
 private:
   Elf_X elf;
   int fd;
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
   
 public:
   virtual Symbol_t getSymbolByName(std::string symname);
   virtual Symbol_t getContainingSymbol(Dyninst::Offset offset);
   virtual std::string getInterpreterName();

   virtual unsigned numRegions();
   virtual bool getRegion(unsigned num, SymRegion &reg); 

   virtual Dyninst::Offset getSymbolOffset(const Symbol_t &sym);
   virtual std::string getSymbolName(const Symbol_t &sym);
   virtual std::string getDemangledName(const Symbol_t &sym);

   virtual bool isValidSymbol(const Symbol_t &sym);
   virtual unsigned getAddressWidth();
   virtual unsigned long getSymbolSize(const Symbol_t &sym);

   virtual Section_t getSectionByName(std::string name);
   virtual Section_t getSectionByAddress(Dyninst::Address addr);
   virtual Dyninst::Address getSectionAddress(Section_t sec);
   virtual std::string getSectionName(Section_t sec);
   virtual bool isValidSection(Section_t sec);

   virtual Dyninst::Offset imageOffset() { assert(0); return 0; };
   virtual Dyninst::Offset dataOffset() { assert(0); return 0; }
   int ref_count;
   bool construction_error;
   
   Elf_X *getElfHandle() { return &elf; }
};

class SymElfFactory : public Dyninst::SymbolReaderFactory
{
private:
   std::map<std::string, SymElf *> open_symelfs;
public:
   SymElfFactory();
   virtual ~SymElfFactory();
   virtual SymReader *openSymbolReader(std::string pathname);
   virtual SymReader *openSymbolReader(const char *buffer, unsigned long size);
   virtual bool closeSymbolReader(SymReader *sr);
};


inline SymElf::SymElf(std::string file_) :
   fd(-1),
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

   elf = Elf_X(fd, ELF_C_READ);
   if (!elf.isValid()) {
      construction_error = true;
      close(fd);
      fd = -1;
      return;
   }
}

inline SymElf::SymElf(const char *buffer_, unsigned long buffer_size_) :
   fd(-1),
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
   elf = Elf_X(const_cast<char *>(buffer_), (size_t) buffer_size);
   if (!elf.isValid()) {
      construction_error = true;
      return;
   }
}

inline SymElf::~SymElf()
{
   if (elf.isValid())
      elf.end();
   if (fd != -1) {
      close(fd);
      fd = -1;
   }
   if (cache) {
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

#define INVALID_SYM_CODE ((int) 0xffffffff)
#define UNSET_INDEX_CODE ((int) 0xfffffffe)

#define FOR_EACH_SYMBOL(shdr, symbols, str_buffer, idx) \
   Elf_X_Data sym_data = shdr.get_data(); \
   Elf_X_Sym symbols = sym_data.get_sym(); \
   int str_index = shdr.sh_link(); \
   Elf_X_Shdr str_shdr = elf.get_shdr(str_index); \
   if (!str_shdr.isValid()) { \
      continue; \
   } \
   Elf_X_Data str_data = str_shdr.get_data(); \
   const char *str_buffer = (const char *) str_data.d_buf(); \
   unsigned sym_count = symbols.count(); \
   for (unsigned idx=0; idx<sym_count; idx++)

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
   Elf_X_Shdr shdr = Elf_X_Shdr(elf.wordSize() == 8, (Elf_Scn *) sym.v2); \
   unsigned idx = (unsigned) sym.i1; \
   Elf_X_Data sym_data = shdr.get_data(); \
   Elf_X_Sym symbols = sym_data.get_sym();
   
#define GET_INVALID_SYMBOL(sym) \
   sym.v1 = sym.v2 = NULL; \
   sym.i1 = 0; sym.i2 = INVALID_SYM_CODE;

inline Symbol_t SymElf::getSymbolByName(std::string symname)
{
   Symbol_t ret;
   for (unsigned i=0; i < elf.e_shnum(); i++) 
   {
      Elf_X_Shdr shdr = elf.get_shdr(i);
      if (shdr.sh_type() != SHT_SYMTAB && shdr.sh_type() != SHT_DYNSYM) {
         continue;
      } 

      FOR_EACH_SYMBOL(shdr, symbol, str_buffer, idx) 
      {
         unsigned str_loc = symbol.st_name(idx);
         if (strcmp(str_buffer+str_loc, symname.c_str()) != 0)
            continue;
         MAKE_SYMBOL(str_buffer+str_loc, idx, shdr, ret);
         return ret;
      }
   }
   GET_INVALID_SYMBOL(ret);
   return ret;
}

inline Symbol_t SymElf::getContainingSymbol(Dyninst::Offset offset)
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

   for (unsigned i=0; i < elf.e_shnum(); i++) 
   {
      Elf_X_Shdr shdr = elf.get_shdr(i);
      if (shdr.sh_type() != SHT_SYMTAB && shdr.sh_type() != SHT_DYNSYM) {
         continue;
      } 

      FOR_EACH_SYMBOL(shdr, symbol, str_buffer, idx) 
      {
         Dyninst::Offset sym_offset = symbol.st_value(idx);
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

inline std::string SymElf::getInterpreterName()
{
   for (unsigned i=0; i < elf.e_phnum(); i++)
   {
      Elf_X_Phdr phdr = elf.get_phdr(i);
      if (phdr.p_type() != PT_INTERP)
         continue;
      Dyninst::Offset off = (Dyninst::Offset) phdr.p_offset();
      
      if (fd != -1) {
         off_t old_offset = lseek(fd, 0, SEEK_CUR);
         lseek(fd, off, SEEK_SET);
         char interp_buffer[4096];
         ssize_t result;
         do {
            result = read(fd, interp_buffer, 4096);
         } while (result == -1 && errno == EINTR);
         lseek(fd, old_offset, SEEK_SET);
         if (result != -1) {
            return std::string(interp_buffer);
         }
      }
      else if (buffer) {
         return std::string(buffer + off);
      }

      //rawfile is expensive
      size_t filesize;
      const char *whole_file = elf.e_rawfile(filesize);
      if (filesize < off) {
         return std::string();
      }
      return std::string(whole_file + off);
   }
   return std::string();
}

inline unsigned SymElf::numRegions()
{
   return elf.e_phnum();
}

inline bool SymElf::getRegion(unsigned num, SymRegion &reg)
{
   if (num >= elf.e_phnum())
      return false;

   Elf_X_Phdr phdr = elf.get_phdr(num);
   reg.file_offset = phdr.p_offset();
   reg.mem_addr = phdr.p_vaddr();
   reg.file_size = phdr.p_filesz();
   reg.mem_size = phdr.p_memsz();
   reg.type = phdr.p_type();
   reg.perms = phdr.p_flags() & 0x7;
   return true;
}

inline unsigned SymElf::getAddressWidth()
{
   return elf.wordSize();
}

unsigned long SymElf::getSymbolSize(const Symbol_t &sym)
{
   GET_SYMBOL(sym, shdr, symbol, name, idx);
   name = NULL; //Silence warnings
   unsigned long size = symbol.st_size(idx);
   return size;
}

inline Dyninst::Offset SymElf::getSymbolOffset(const Symbol_t &sym)
{
   GET_SYMBOL(sym, shdr, symbols, name, idx);
   name = NULL; //Silence warnings
   Dyninst::Offset sym_offset = symbols.st_value(idx);
   return sym_offset;
}

inline std::string SymElf::getSymbolName(const Symbol_t &sym)
{
   GET_SYMBOL(sym, shdr, symbols, name, idx);
   idx = 0; //Silence warnings
   return std::string(name);
}

inline std::string SymElf::getDemangledName(const Symbol_t &sym)
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
   char *res = P_cplus_demangle(name, false, true);
   if (!res) {
      //Try native demangler
      res = P_cplus_demangle(name, true, true);
   }

   cache[cache_index].demangled_name = res ? res : name;
   return cache[cache_index].demangled_name;
}

inline bool SymElf::isValidSymbol(const Symbol_t &sym)
{
   return (sym.i2 != INVALID_SYM_CODE);
}

static int symcache_cmp(const void *a, const void *b)
{
   SymCacheEntry *aa = (SymCacheEntry *) a;
   SymCacheEntry *bb = (SymCacheEntry *) b;
   if (aa->symaddress < bb->symaddress) return -1;
   else if (aa->symaddress > bb->symaddress) return 1;
   else return 0;
}

inline void SymElf::createSymCache()
{
   unsigned long sym_count = 0, cur_sym = 0, cur_sec = 0;
   
   assert(!cache);
   assert(!sym_sections);
   for (unsigned i=0; i < elf.e_shnum(); i++) 
   {
      Elf_X_Shdr shdr = elf.get_shdr(i);
      if (shdr.sh_type() != SHT_SYMTAB && shdr.sh_type() != SHT_DYNSYM) {
         continue;
      }
      Elf_X_Data sym_data = shdr.get_data();
      Elf_X_Sym symbols = sym_data.get_sym();
      sym_count += symbols.count();
      sym_sections_size++;
   }

   cache = (SymCacheEntry *) malloc(sym_count * sizeof(SymCacheEntry));
   sym_sections = (Elf_X_Shdr *) malloc(sym_sections_size * sizeof(Elf_X_Shdr));
   
   for (unsigned i=0; i < elf.e_shnum(); i++) 
   {
      Elf_X_Shdr shdr = elf.get_shdr(i);
      if (shdr.sh_type() != SHT_SYMTAB && shdr.sh_type() != SHT_DYNSYM) {
         continue;
      }

      sym_sections[cur_sec] = shdr;
      cur_sec++;

      FOR_EACH_SYMBOL(shdr, symbols, str_buffer, idx)
      {
         str_buffer = NULL; //Disable warnings
         unsigned char symtype = symbols.ST_TYPE(idx);
         if (symtype != STT_FUNC)
            continue;
         if (!symbols.st_value(idx))
            continue;
         cache[cur_sym].symaddress = symbols.st_value(idx);
         cache[cur_sym].symloc = symbols.st_symptr(idx);
         cache[cur_sym].demangled_name = NULL;
         cur_sym++;
      }
   }
   cache_size = cur_sym;
   cache = (SymCacheEntry *) realloc(cache, cur_sym  * sizeof(SymCacheEntry)); //Size reduction

   qsort(cache, cache_size, sizeof(SymCacheEntry), symcache_cmp);
}

inline Symbol_t SymElf::lookupCachedSymbol(Dyninst::Offset off)
{
   unsigned min = 0;
   unsigned max = cache_size;
   unsigned cur = cache_size / 2;
   
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

   Symbol_t ret;
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
      Elf_X_Shdr str_shdr = elf.get_shdr(str_index);
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

inline Section_t SymElf::getSectionByName(std::string name)
{
   unsigned short stridx = elf.e_shstrndx();
   Elf_X_Shdr strshdr = elf.get_shdr(stridx);
   Elf_X_Data strdata = strshdr.get_data();
   const char *names = (const char *) strdata.d_buf();
   Section_t ret;
   ret.i1 = -1;

   for (unsigned i=0; i < elf.e_shnum(); i++) 
   {
      Elf_X_Shdr shdr = elf.get_shdr(i);
      const char *sname = names + shdr.sh_name();
      if (name == sname) {
         ret.i1 = i;
         break;
      }
   }
   
   return ret;
}

inline Section_t SymElf::getSectionByAddress(Dyninst::Address addr)
{
   Section_t ret;
   ret.i1 = -1;

   for (unsigned i=0; i < elf.e_shnum(); i++) 
   {
      Elf_X_Shdr shdr = elf.get_shdr(i);
      Dyninst::Address mem_start = shdr.sh_addr();
      unsigned long mem_size = shdr.sh_size();
      if (addr >= mem_start && addr < mem_start + mem_size) {
         ret.i1 = i;
         break;
      }
   }
   return ret;
}

inline Dyninst::Address SymElf::getSectionAddress(Section_t sec)
{
   assert(isValidSection(sec));
   Elf_X_Shdr shdr = elf.get_shdr(sec.i1);
   
   return shdr.sh_addr();
}

inline std::string SymElf::getSectionName(Section_t sec)
{
   assert(isValidSection(sec));
   Elf_X_Shdr shdr = elf.get_shdr(sec.i1);

   unsigned short stridx = elf.e_shstrndx();
   Elf_X_Shdr strshdr = elf.get_shdr(stridx);
   Elf_X_Data strdata = strshdr.get_data();
   const char *names = (const char *) strdata.d_buf();

   return std::string(names + shdr.sh_name());
}

inline bool SymElf::isValidSection(Section_t sec)
{
   return (sec.i1 != -1);
}

inline SymElfFactory::SymElfFactory()
{
}

inline SymElfFactory::~SymElfFactory()
{
}

SymReader *SymElfFactory::openSymbolReader(std::string pathname)
{
   SymElf *se = NULL;
   std::map<std::string, SymElf *>::iterator i = open_symelfs.find(pathname);
   if (i == open_symelfs.end()) {
      se = new SymElf(pathname);
      if (se->construction_error) {
         delete se;
         return NULL;
      }
      se->ref_count = 1;
      open_symelfs[pathname] = se;
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
   std::map<std::string, SymElf *>::iterator i = open_symelfs.find(ser->file);
   if (i == open_symelfs.end()) {
      delete ser;
   }

   ser->ref_count--;
   if (ser->ref_count == 0) {
      open_symelfs.erase(i);
      delete ser;
   }
   return true;
}
