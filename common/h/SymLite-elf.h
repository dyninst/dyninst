#include "dynutil/h/SymReader.h"
#include "common/h/Elf_X.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include <map>

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
 public:
   virtual Symbol_t getSymbolByName(std::string symname);
   virtual Symbol_t getContainingSymbol(Dyninst::Offset offset);
   virtual std::string getInterpreterName();

   virtual unsigned numRegions();
   virtual bool getRegion(unsigned num, SymRegion &reg); 

   virtual Dyninst::Offset getSymbolOffset(const Symbol_t &sym);
   virtual std::string getSymbolName(const Symbol_t &sym);
   virtual bool isValidSymbol(const Symbol_t &sym);
   virtual unsigned getAddressWidth();
   virtual unsigned long getSymbolSize(const Symbol_t &sym);

   virtual Dyninst::Offset imageOffset() { assert(0); return 0; };
   virtual Dyninst::Offset dataOffset() { assert(0); return 0; }
   int ref_count;
   bool construction_error;
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
}

#define VALID_SYM_CODE ((int) 0xbeef2324)

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
   sym.i2 = VALID_SYM_CODE;

#define GET_SYMBOL(sym, shdr, symbols, name, idx) \
   assert(sym.i2 == VALID_SYM_CODE); \
   const char *name = (const char *) sym.v1; \
   Elf_X_Shdr shdr = Elf_X_Shdr(elf.wordSize() == 8, (Elf_Scn *) sym.v2); \
   unsigned idx = (unsigned) sym.i1; \
   Elf_X_Data sym_data = shdr.get_data(); \
   Elf_X_Sym symbols = sym_data.get_sym();
   
#define GET_INVALID_SYMBOL(sym) \
   sym.v1 = sym.v2 = NULL; \
   sym.i1 = sym.i2 = 0 

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
         if (sym_offset >= offset && (!has_nearest || sym_offset < nearest)) {
            unsigned str_loc = symbol.st_name(idx);
            MAKE_SYMBOL(str_buffer+str_loc, idx, shdr, nearest_sym);
            has_nearest = true;
         }
      }
   }
   if (!has_nearest) 
      GET_INVALID_SYMBOL(nearest_sym);

   return nearest_sym;
}

inline std::string SymElf::getInterpreterName()
{
   for (unsigned i=0; i < elf.e_phnum(); i++)
   {
      Elf_X_Phdr phdr = elf.get_phdr(i);
      if (phdr.p_type() != PT_INTERP)
         continue;
      Dyninst::Offset off = (Dyninst::Offset) phdr.p_offset();
      
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

inline bool SymElf::isValidSymbol(const Symbol_t &sym)
{
   return (sym.i2 == VALID_SYM_CODE);
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

