/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include <assert.h>
#include <link.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <vector>
#include <string>

#include "common/h/parseauxv.h"
#include "common/h/headers.h"

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/src/addrtranslate.h"
#include "symtabAPI/src/addrtranslate-sysv.h"
#include "debug.h"

#if defined(os_linux) || defined(os_bg)
#define R_DEBUG_NAME "_r_debug"
#else
#define R_DEBUG_NAME "r_debug"
#endif

using namespace std;

using namespace Dyninst;
using namespace SymtabAPI;

FileCache Dyninst::SymtabAPI::files;


class ProcessReaderSelf : public ProcessReader {
public:
   ProcessReaderSelf();
   bool start();
   bool readAddressSpace(Address inTraced, unsigned amount,
                         void *inSelf);
   bool done();

  virtual ~ProcessReaderSelf();
};

struct link_map_dyn32
{
   Elf32_Addr l_addr;
   uint32_t l_name;
   uint32_t l_ld;
   uint32_t l_next, l_prev;
};

struct r_debug_dyn32
{
    int r_version;
    uint32_t r_map;
    Elf32_Addr r_brk;
    enum
    {
       RT_CONSISTENT,
       RT_ADD,
       RT_DELETE
    } r_state;
    Elf32_Addr r_ldbase;
};

class link_map_xplat 
{
public:
   virtual size_t size() = 0;
   virtual uint64_t l_addr() = 0;
   virtual char *l_name() = 0;
   virtual void *l_ld() = 0;
   virtual bool is_last() = 0;
   virtual bool load_next() = 0;   
   virtual bool is_valid() = 0;   
   virtual bool load_link(Address addr) = 0;
   virtual ~link_map_xplat() {};
};

template<class link_map_X> 
class link_map_dyn : public link_map_xplat 
{
public: 
   link_map_dyn(ProcessReader *proc_, Address addr);
   ~link_map_dyn();
   virtual size_t size();
   virtual uint64_t l_addr();
   virtual char *l_name();
   virtual void *l_ld();
   virtual bool is_last();
   virtual bool load_next();   
   virtual bool is_valid();   
   virtual bool load_link(Address addr);

protected:
   ProcessReader *proc;
   char link_name[256];
   bool loaded_name;
   bool valid;
   link_map_X link_elm;
};

template<class r_debug_X> 
class r_debug_dyn {
public:
   r_debug_dyn(ProcessReader *proc_, Address addr);
   ~r_debug_dyn();
   void *r_brk();
   Address r_map();
   int r_state();
   bool is_valid();
protected:
   ProcessReader *proc;
   bool valid;
   r_debug_X debug_elm;
};

template<class r_debug_X> 
r_debug_dyn<r_debug_X>::r_debug_dyn(ProcessReader *proc_, Address addr)
   : proc(proc_) 
{
   valid = proc->readAddressSpace(addr, sizeof(debug_elm), &debug_elm);

   translate_printf("[%s:%u] -     Read rdebug structure.  Values were:\n", __FILE__, __LINE__);
   translate_printf("[%s:%u] -       r_brk:    %lx\n", __FILE__, __LINE__, (unsigned long)debug_elm.r_brk);
   translate_printf("[%s:%u] -       r_map:    %lx\n", __FILE__, __LINE__, (unsigned long)debug_elm.r_map);
   translate_printf("[%s:%u] -       r_ldbase: %lx\n", __FILE__, __LINE__, (unsigned long)debug_elm.r_ldbase);
}

template<class r_debug_X> 
r_debug_dyn<r_debug_X>::~r_debug_dyn() 
{
}

template<class r_debug_X> 
bool r_debug_dyn<r_debug_X>::is_valid() {
   if (0 == r_map()) return false;
   else return valid;
}

template<class r_debug_X> 
Address r_debug_dyn<r_debug_X>::r_map() {
	return (Address) debug_elm.r_map;
}

template<class r_debug_X> 
void *r_debug_dyn<r_debug_X>::r_brk() { 
   return reinterpret_cast<void *>(debug_elm.r_brk); 
}

template<class r_debug_X> 
int r_debug_dyn<r_debug_X>::r_state() { 
   return (int)debug_elm.r_state; 
}
 
template<class link_map_X>
link_map_dyn<link_map_X>::link_map_dyn(ProcessReader *proc_, Address addr_) :
   proc(proc_),
   loaded_name(false)
{
   valid = load_link(addr_);
}

template<class link_map_X>
link_map_dyn<link_map_X>::~link_map_dyn() {
}

template<class link_map_X>
bool link_map_dyn<link_map_X>::is_valid() {
   return valid;
}

template<class link_map_X>
size_t link_map_dyn<link_map_X>::size()
{
   return sizeof(link_elm);
}

template<class link_map_X>
uint64_t link_map_dyn<link_map_X>::l_addr() 
{
   return link_elm.l_addr;
}

template<class link_map_X>
char *link_map_dyn<link_map_X>::l_name() 
{
  if (loaded_name) return link_name;

  for (unsigned int i = 0; i < sizeof(link_name); ++i) {
    if (!proc->readAddressSpace((Address) (link_elm.l_name + i),
                             sizeof(char), link_name + i))
    {
       valid = false;
       return NULL;
    }
    if (link_name[i] == '\0') break;
  }
  link_name[sizeof(link_name) - 1] = '\0';

  loaded_name = true;  
  return link_name;
}

template<class link_map_X>
void *link_map_dyn<link_map_X>::l_ld() 
{ 
   return reinterpret_cast<void *>(link_elm.l_ld); 
}

template<class link_map_X>
bool link_map_dyn<link_map_X>::is_last() 
{ 
   return (link_elm.l_next == 0); 
}

template<class link_map_X>
bool link_map_dyn<link_map_X>::load_next() 
{
	if (is_last()) {
      return false;
   }
	if (load_link((Address) link_elm.l_next)) {
      loaded_name = false;
      return true;
	}
	return false;
}

template<class link_map_X>
bool link_map_dyn<link_map_X>::load_link(Address addr) 
{
   return proc->readAddressSpace(addr, sizeof(link_elm), &link_elm);
}

static const char *deref_link(const char *path)
{
   static char buffer[PATH_MAX], *p;
   buffer[PATH_MAX-1] = '\0';
   p = realpath(path, buffer);
   if (!p)
      return path;
   return p;
}

ProcessReader::ProcessReader(int pid_, string exe) :
   pid(pid_), executable(exe)
{
}

ProcessReader::ProcessReader() :
   pid(0)
{
}

ProcessReaderSelf::ProcessReaderSelf() :
   ProcessReader(getpid()) 
{
}

ProcessReaderSelf::~ProcessReaderSelf()
{
}

bool ProcessReaderSelf::start() {
   return true;
}

bool ProcessReaderSelf::done() {
   return true;
}

bool ProcessReaderSelf::readAddressSpace(Address inTraced, unsigned amount,
                                         void *inSelf) 
{
   memcpy(inSelf, (void *) inTraced, amount);
   return true;
}

Symtab *LoadedLib::getSymtab()
{
   if (symtable)
      return symtable;

   FCNode *fc = files.getNode(name);
   if (!fc)
      return NULL;
   symtable = fc->getSymtab();

   return symtable;
}

vector< pair<Address, unsigned long> > *LoadedLib::getMappedRegions()
{
   if (mapped_regions.size())
   {
      return &mapped_regions;
   }
   
   FCNode *fc = files.getNode(name);
   if (!fc)
      return false;

   vector<Region *> regs;
   fc->getRegions(regs);
   
   for (unsigned i=0; i<regs.size(); i++) {
      pair<Address, unsigned long> p(load_addr + regs[i]->getRegionAddr(), regs[i]->getRegionSize());
      mapped_regions.push_back(p);
   }
   
   return &mapped_regions;
}

AddressTranslate *AddressTranslate::createAddressTranslator(int pid_, 
                                                            ProcessReader *reader_,
															PROC_HANDLE)
{
   translate_printf("[%s:%u] - Creating AddressTranslateSysV\n", __FILE__, __LINE__);
   AddressTranslate *at = new AddressTranslateSysV(pid_, reader_);
   translate_printf("[%s:%u] - Created: %lx\n", __FILE__, __LINE__, (long)at);
   
   if (!at) {
      return NULL;
   }
   else if (at->creation_error) {
      delete at;
      return NULL;
   }
   return at;
}

AddressTranslate *AddressTranslate::createAddressTranslator(ProcessReader *reader_)
{
   return createAddressTranslator(getpid(), reader_, INVALID_HANDLE_VALUE);
}

AddressTranslate *AddressTranslate::createAddressTranslator(const std::vector<LoadedLibrary> &name_addrs)
{
   AddressTranslate *at = new AddressTranslateSysV();
   
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
      at->libs.push_back(ll);
   }
   return at;
}

AddressTranslateSysV::AddressTranslateSysV() :
   AddressTranslate(NULL_PID),
   reader(NULL),
   interpreter_base(0),
   set_interp_base(0),
   address_size(0),
   interpreter(NULL),
   previous_r_state(0),
   current_r_state(0),
   r_debug_addr(0),
   trap_addr(0)
{
}

AddressTranslateSysV::AddressTranslateSysV(int pid, ProcessReader *reader_) :
   AddressTranslate(pid),
   reader(reader_),
   interpreter_base(0),
   set_interp_base(0),
   address_size(0),
   interpreter(NULL),
   previous_r_state(0),
   current_r_state(0),
   r_debug_addr(0),
   trap_addr(0)
{
   bool result;
   if (!reader) {
      if (pid == getpid())
         reader = new ProcessReaderSelf();
      else
         reader = createDefaultDebugger(pid);
   }

   result = init();
   if (!result) {
      creation_error = true;
      return;
   }
}

bool AddressTranslateSysV::setInterpreterBase()
{
   if (set_interp_base)
     return true;

   AuxvParser *parser = AuxvParser::createAuxvParser(pid, address_size);
   if (!parser) {
      return false;
   }

   interpreter_base = parser->getInterpreterBase();
   set_interp_base = true;

   parser->deleteAuxvParser();
   return true;
}

bool AddressTranslateSysV::init()
{
   bool result;
   translate_printf("[%s:%u] - Initing AddressTranslateSysV\n", __FILE__, __LINE__);

   result = setInterpreter();

   if (!result) {
     translate_printf("[%s:%u] - Failed to set interpreter.\n", __FILE__, __LINE__);
     return false;
   }

   result = setAddressSize();
   if (!result) {
      translate_printf("[%s:%u] - Failed to set address size.\n", __FILE__, __LINE__);
      return false;
   }

   result = setInterpreterBase();
   if (!result) {
      translate_printf("[%s:%u] - Failed to set interpreter base.\n", __FILE__, __LINE__);
      return false;
   }

   if (interpreter) {
      r_debug_addr = interpreter->get_r_debug() + interpreter_base;
      trap_addr = interpreter->get_r_trap() + interpreter_base;
   }
   else {
      r_debug_addr = 0;
      trap_addr = 0;
   }
   
   translate_printf("[%s:%u] - Done with AddressTranslateSysV::init()\n", __FILE__, __LINE__);

   return true;
}

bool AddressTranslateSysV::refresh()
{
   link_map_xplat *link_elm = NULL;
   r_debug_dyn<r_debug_dyn32> *r_debug_32 = NULL;
   r_debug_dyn<r_debug> *r_debug_native = NULL;
   map_entries *maps = NULL;
   bool result = false;
   size_t loaded_lib_count = 0;

   translate_printf("[%s:%u] - Refreshing Libraries\n", __FILE__, __LINE__);

   if (pid == NULL_PID)
      return true;

   for (unsigned i=0; i<libs.size(); i++) {
      if (libs[i])
         delete libs[i];
      if (libs[i] == exec)
         exec = NULL;
   }
   libs.clear();

   if (!exec) {
      exec = getAOut();
   }
   if (exec) {
      libs.push_back(exec);
   }
   
   if (!r_debug_addr) {
      return true; //Static library
   }

   reader->start();

   translate_printf("[%s:%u] -     Starting refresh.\n", __FILE__, __LINE__);
   translate_printf("[%s:%u] -       trap_addr:    %lx\n", __FILE__, __LINE__, trap_addr);
   translate_printf("[%s:%u] -       r_debug_addr: %lx\n", __FILE__, __LINE__, r_debug_addr);

   if (address_size == sizeof(void*)) {
      r_debug_native = new r_debug_dyn<r_debug>(reader, r_debug_addr);
      if (!r_debug_native)
      {
        result = true;
        goto done;
      }
      else if (!r_debug_native->is_valid())
      {
         libs.push_back(new LoadedLib(interpreter->getFilename(),
                                      interpreter_base));
         result = true;
         goto done;
      }
      link_elm = new link_map_dyn<link_map>(reader, r_debug_native->r_map());
   }
   else { //64-bit mutator, 32-bit mutatee
      r_debug_32 = new r_debug_dyn<r_debug_dyn32>(reader, r_debug_addr);
      if (!r_debug_32)
      {
         result = true;
         goto done;
      }
      else if (!r_debug_32->is_valid())
      {
         libs.push_back(new LoadedLib(interpreter->getFilename(),
                                      interpreter_base));
         result = true;
         goto done;
      }
      link_elm = new link_map_dyn<link_map_dyn32>(reader, r_debug_32->r_map());
   }

   if (!link_elm->is_valid()) {
      result = true;
      goto done;
   }

   do {
      if (!link_elm->l_name())
         continue;
      string obj_name(link_elm->l_name());
      Address text = (Address) link_elm->l_addr();
      if (obj_name == "" && !text)
         continue;
      if (!link_elm->is_valid())
         goto done;

#if defined(os_linux)
      unsigned maps_size;
      if (obj_name == "") { //Augment using maps
         if (!maps)
            maps = getLinuxMaps(pid, maps_size);
         for (unsigned i=0; maps && i<maps_size; i++) {
            if (text == maps[i].start) {
               obj_name = maps[i].path;
               break;
            }
         }
      }
      if (obj_name.c_str()[0] == '[')
         continue;
#endif
      
      string s(deref_link(obj_name.c_str()));
      LoadedLib *ll = new LoadedLib(s, text);
      loaded_lib_count++;
      translate_printf("[%s:%u] -     New Loaded Library: %s(%lx)\n", __FILE__, __LINE__, s.c_str(), text);

      libs.push_back(ll);
   } while (link_elm->load_next());

   translate_printf("[%s:%u] - Found %d libraries.\n", __FILE__, __LINE__, loaded_lib_count);

   result = true;
 done:
   reader->done();

   if (link_elm)
      delete link_elm;
   if (r_debug_32)
      delete r_debug_32;
   if (r_debug_native)
      delete r_debug_native;
   if (maps)
      free(maps);

   return result;
}

FCNode::FCNode(string f, dev_t d, ino_t i) :
   device(d),
   inode(i),
   parsed_file(false),
   parsed_file_fast(false),
   parse_error(false)
{
   filename = deref_link(f.c_str());
}

string FCNode::getFilename() {
   return filename;
}

string FCNode::getInterpreter() {
   parsefile_fast();

   return interpreter_name;
}

Symtab *FCNode::getSymtab()
{
   parsefile();

   return symtable;
}

void FCNode::getRegions(vector<Region *> &regs) {
   parsefile_fast();

   for (unsigned i=0; i<regions.size(); i++)
      regs.push_back(regions[i]);
}

unsigned FCNode::getAddrSize() {
   parsefile_fast();

   return addr_size;
}

Offset FCNode::get_r_debug() {
   parsefile();

   return r_debug_offset;
}

Offset FCNode::get_r_trap() {
   parsefile();

   return r_trap_offset;
}

#define NUM_DBG_BREAK_NAMES 3
const char *dbg_break_names[] = { "_dl_debug_state",
                                  "r_debug_state",
                                  "_r_debug_state" };
void FCNode::parsefile() {
   bool result;

   if (parsed_file || parse_error)
      return;

   translate_printf("[%s:%u] - Parsing file in FCNode: %s.\n", __FILE__, __LINE__, filename.c_str());

   result = Symtab::openFile(symtable, filename);
   if (!result) {
      parse_error = true;
      return;
   }
   
   if (!parsed_file_fast)
   {
      const char *name = symtable->getInterpreterName();
      if (name) {
         interpreter_name = name;
         translate_printf("[%s:%u] - Interpreter was: %s.\n", __FILE__, __LINE__, name);
      }
      
      addr_size = symtable->getAddressWidth();
      symtable->getMappedRegions(regions);
   }

   r_debug_offset = 0;
   r_trap_offset = 0;
   parsed_file = true;
   parsed_file_fast = true;

   vector<Symbol *> syms;
   result = symtable->findSymbolByType(syms, 
                                       R_DEBUG_NAME, 
                                       Symbol::ST_OBJECT,
                                       anyName);
   if (result && syms.size() != 0) {
      Symbol *rDebugSym = syms[0];   
      r_debug_offset = rDebugSym->getAddr();
   }
   syms.clear();

   for (unsigned i=0; i<NUM_DBG_BREAK_NAMES; i++) {
      result = symtable->findSymbolByType(syms, dbg_break_names[i], 
                                          Symbol::ST_FUNCTION,
                                          anyName);
      if (!result || !syms.size())
         continue;
      Symbol *rTrapSym = syms[0];
      r_trap_offset = rTrapSym->getAddr();
      break;
   }
}

#include "Elf_X.h"
#include "symtabAPI/h/Region.h"
#include <elf.h>

void FCNode::parsefile_fast() {
   if (parsed_file_fast || parse_error)
      return;

   parsed_file_fast = true;
   off_t interp_disk_offset = 0;
   unsigned interp_size = 0;
   int result;
   char *buffer = NULL;
   int fd = -1;

   fd = open(filename.c_str(), O_RDONLY);
   if (fd == -1) {
      int error = errno;
      translate_printf("[%s:%u] - Error opening file %s: %s\n",
                       __FILE__, __LINE__, filename.c_str(), strerror(error));
      parse_error = true;
      goto done;
   }

   {//Binding to destruct 'elf' when done.
      Elf_X elf(fd, ELF_C_READ);

      addr_size = elf.wordSize();

      for (unsigned i=0; i<elf.e_phnum(); i++)
      {
         Elf_X_Phdr phdr = elf.get_phdr(i);
         if (phdr.p_type() == PT_LOAD)
         {
            Region::perm_t perms;
            switch (elf.e_flags()) {
               case 4: perms = Region::RP_R; break;
               case 5: perms = Region::RP_RX; break;
               case 6: perms = Region::RP_RW; break;
               case 7: perms = Region::RP_RWX; break;
               default:
                  perms = Region::RP_RWX; break;
            }
            Region *region = Region::createRegion(phdr.p_vaddr(), perms, Region::RT_DATA, 
                                                  phdr.p_filesz(), phdr.p_vaddr(), 
                                                  phdr.p_memsz());
            regions.push_back(region);
         }
         if (phdr.p_type() == PT_INTERP)
         {
            interp_disk_offset = (size_t) phdr.p_offset();
            interp_size = (unsigned) phdr.p_filesz();
         }
      }
      elf.end();
   }

   if (!interp_disk_offset || !interp_size) {
      translate_printf("[%s:%u] - Did not find interp section\n", __FILE__, __LINE__);
      goto done;
   }

   result = lseek(fd, interp_disk_offset, SEEK_SET);
   if (result == (off_t)-1) {
      translate_printf("[%s:%u] - Failed to seek to interp section\n", 
                       __FILE__, __LINE__);
      parse_error = true;
      goto done;
   }
   buffer = (char *) malloc(interp_size+1);
   memset(buffer, 0, interp_size+1);

   result = read(fd, buffer, interp_size);
   if (result == -1) {
      free(buffer);
      translate_printf("[%s:%u] - Could not read interp string from file\n",
                       __FILE__, __LINE__);
      parse_error = true;
      goto done;
   }
   interpreter_name = std::string(buffer);

 done:
   if (buffer)
      free(buffer);
   if (fd != -1)
      close(fd);
}

FCNode *FileCache::getNode(const string &filename)
{
   struct stat buf;
   int result = stat(filename.c_str(), &buf);
   if (result == -1)
      return NULL;
   if (!filename.length())
      return NULL;

   for (unsigned i=0; i<nodes.size(); i++)
   {
      if (nodes[i]->inode == buf.st_ino &&
          nodes[i]->device == buf.st_dev)
      {
         return nodes[i];
      }
   }

   FCNode *fc = new FCNode(filename, buf.st_dev, buf.st_ino);
   nodes.push_back(fc);

   return fc;
}

FileCache::FileCache()
{
}

Address AddressTranslateSysV::getLibraryTrapAddrSysV() {
  return trap_addr;
}
