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

#include <vector>
#include <string>

#include "common/h/parseauxv.h"
#include "common/h/headers.h"

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/src/addrtranslate.h"
#include "symtabAPI/src/addrtranslate-sysv.h"

#if defined(os_linux)
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
}

template<class r_debug_X> 
r_debug_dyn<r_debug_X>::~r_debug_dyn() 
{
}

template<class r_debug_X> 
bool r_debug_dyn<r_debug_X>::is_valid() {
   return valid;
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
       link_name[0] = '\0';
       assert(0);
       return link_name;
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

ProcessReader::ProcessReader(int pid_) :
   pid(pid_)
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

   vector<Region> regs;
   fc->getRegions(regs);
   
   for (unsigned i=0; i<regs.size(); i++) {
      pair<Address, unsigned long> p(load_addr + regs[i].addr, regs[i].size);
      mapped_regions.push_back(p);
   }
   
   return &mapped_regions;
}

AddressTranslate *AddressTranslate::createAddressTranslator(int pid_, 
                                                            ProcessReader *reader_)
{
   AddressTranslate *at = new AddressTranslateSysV(pid_, reader_);
   
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
   return createAddressTranslator(getpid(), reader_);
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
   AddressTranslate(0),
   reader(NULL),
   interpreter_base(0),
   set_interp_base(0),
   address_size(0),
   interpreter(NULL),
   previous_r_state(0),
   current_r_state(0),
   r_debug_addr(0)
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
   r_debug_addr(0)
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

   result = refresh();
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
   if (!parser)
      return false;

   interpreter_base = parser->getInterpreterBase();
   set_interp_base = true;

   parser->deleteAuxvParser();
   return true;
}

bool AddressTranslateSysV::init()
{
   bool result;

   result = setInterpreter();
   if (!result)
      return false;
   result = setInterpreterBase();
   if (!result)
      return false;
   result = setAddressSize();
   if (!result)
      return false;

   if (interpreter)
      r_debug_addr = interpreter->get_r_debug() + interpreter_base;
   else
      r_debug_addr = 0;

   return true;
}

bool AddressTranslateSysV::refresh()
{
   link_map_xplat *link_elm;
   r_debug_dyn<r_debug_dyn32> *r_debug_32 = NULL;
   r_debug_dyn<r_debug> *r_debug_native = NULL;
   map_entries *maps = NULL;
   bool result = false;

   if (!pid)
      return true;

   for (unsigned i=0; i<libs.size(); i++) {
      if (libs[i])
         delete libs[i];
   }
   libs.clear();

   LoadedLib *exe = getAOut();
   if (exe)
     libs.push_back(exe);
   
   if (!r_debug_addr)
      return true; //Static library

   reader->start();

   if (address_size == sizeof(void*)) {
      r_debug_native = new r_debug_dyn<r_debug>(reader, r_debug_addr);
      if (!r_debug_native || !r_debug_native->is_valid())
         return false;
      link_elm = new link_map_dyn<link_map>(reader, r_debug_native->r_map());
   }
   else { //64-bit mutator, 32-bit mutatee
      r_debug_32 = new r_debug_dyn<r_debug_dyn32>(reader, r_debug_addr);
      if (!r_debug_32 || !r_debug_32->is_valid())
         return false;
      link_elm = new link_map_dyn<link_map_dyn32>(reader, r_debug_32->r_map());
   }

   do {
      if (!link_elm->l_name())
         continue;
      string obj_name(link_elm->l_name());
      Address text = link_elm->l_addr();
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
      libs.push_back(ll);
   } while (link_elm->load_next());

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
   parse_error(false)
{
   filename = deref_link(f.c_str());
}

string FCNode::getFilename() {
   return filename;
}

string FCNode::getInterpreter() {
   parsefile();

   return interpreter_name;
}

Symtab *FCNode::getSymtab()
{
   parsefile();

   return symtable;
}

void FCNode::getRegions(vector<Region> &regs) {
   parsefile();

   for (unsigned i=0; i<regions.size(); i++)
      regs.push_back(regions[i]);
}

unsigned FCNode::getAddrSize() {
   parsefile();

   return addr_size;
}

Offset FCNode::get_r_debug() {
   parsefile();

   return r_debug_offset;
}

void FCNode::parsefile() {
   bool result;

   if (parsed_file || parse_error)
      return;

   result = Symtab::openFile(symtable, filename);
   if (!result) {
      parse_error = true;
      return;
   }
   
   const char *name = symtable->getInterpreterName();
   if (name)
      interpreter_name = name;
   addr_size = symtable->getAddressWidth();
   symtable->getMappedRegions(regions);
   r_debug_offset = 0;
   parsed_file = true;

   vector<Symbol *> syms;
   result = symtable->findSymbolByType(syms, R_DEBUG_NAME, Symbol::ST_OBJECT);
   if (!result || syms.size() == 0)
     return;
   Symbol *rDebugSym = syms[0];   
   r_debug_offset = rDebugSym->getAddr();
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

