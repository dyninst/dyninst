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
#include <limits.h>

#include <vector>
#include <string>

#include "common/h/parseauxv.h"
#include "common/h/headers.h"

#include "common/h/addrtranslate.h"
#include "common/src/addrtranslate-sysv.h"

#if defined(os_linux) || defined(os_bg)
#define R_DEBUG_NAME "_r_debug"
#else
#define R_DEBUG_NAME "r_debug"
#endif

#if defined(os_linux)
#include "common/h/linuxKludges.h"
#endif

using namespace std;
using namespace Dyninst;

FileCache Dyninst::files;


class ProcessReaderSelf : public ProcessReader {
public:
   ProcessReaderSelf();
   bool start();
   bool ReadMem(Address inTraced, void *inSelf, unsigned amount);
   bool GetReg(MachRegister reg, MachRegisterVal &val);
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
   valid = proc->ReadMem(addr, &debug_elm, sizeof(debug_elm));
   if (!valid)
      return;

   translate_printf("[%s:%u] - r_debug_dyn valid = %d\n", __FILE__, __LINE__, valid?1:0);
   translate_printf("[%s:%u] -     Read rdebug structure.  Values were:\n", __FILE__, __LINE__);
   translate_printf("[%s:%u] -       r_brk:    %lx\n", __FILE__, __LINE__, (unsigned long)debug_elm.r_brk);
   translate_printf("[%s:%u] -       r_map:    %lx\n", __FILE__, __LINE__, (unsigned long)debug_elm.r_map);
#if !defined(os_freebsd)
   translate_printf("[%s:%u] -       r_ldbase: %lx\n", __FILE__, __LINE__, (unsigned long)debug_elm.r_ldbase);
#endif
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
   return (uint64_t)link_elm.l_addr;
}

template<class link_map_X>
char *link_map_dyn<link_map_X>::l_name() 
{
  if (loaded_name) return link_name;

  for (unsigned int i = 0; i < sizeof(link_name); ++i) {
     if (!proc->ReadMem((Address) (link_elm.l_name + i),
                        link_name + i, sizeof(char)))
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
   return const_cast<void *>(reinterpret_cast<const void *>(link_elm.l_ld)); 
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
   return proc->ReadMem(addr, &link_elm, sizeof(link_elm));
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

ProcessReaderSelf::ProcessReaderSelf() :
   ProcessReader() 
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

bool ProcessReaderSelf::ReadMem(Address inTraced, void *inSelf, unsigned amount)
{
   memcpy(inSelf, (void *) inTraced, amount);
   return true;
}

bool ProcessReaderSelf::GetReg(MachRegister /*reg*/, MachRegisterVal &/*val*/)
{
   assert(0);
   return false;
}


vector< pair<Address, unsigned long> > *LoadedLib::getMappedRegions()
{
   if (mapped_regions.size())
   {
      return &mapped_regions;
   }

   FCNode *fc = files.getNode(name, symreader_factory);
   if (!fc)
      return false;

   vector<SymRegion> regs;
   fc->getRegions(regs);
   
   for (unsigned i=0; i<regs.size(); i++) {
      pair<Address, unsigned long> p(load_addr + regs[i].mem_addr, 
                                     regs[i].mem_size);
      mapped_regions.push_back(p);
   }
   
   return &mapped_regions;
}

AddressTranslate *AddressTranslate::createAddressTranslator(int pid_, 
                                                            ProcessReader *reader_,
                                                            SymbolReaderFactory *symfactory_,
                                                            PROC_HANDLE,
                                                            std::string exename)
{
   translate_printf("[%s:%u] - Creating AddressTranslateSysV\n", __FILE__, __LINE__);
   AddressTranslate *at = new AddressTranslateSysV(pid_, reader_, symfactory_, exename);
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

AddressTranslate *AddressTranslate::createAddressTranslator(ProcessReader *reader_,
                                                            SymbolReaderFactory *factory_,
                                                            std::string exename)
{
   return createAddressTranslator(getpid(), reader_, factory_, INVALID_HANDLE_VALUE, exename);
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

AddressTranslateSysV::AddressTranslateSysV(int pid, ProcessReader *reader_, 
                                           SymbolReaderFactory *reader_fact,
                                           std::string exename) :
   AddressTranslate(pid, INVALID_HANDLE_VALUE, exename),
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
   symfactory = reader_fact;
   result = init();
   if (!result) {
      creation_error = true;
      return;
   }
}

bool AddressTranslateSysV::parseDTDebug() {
    //TODO this could possibly be used on other platforms
#if !defined(os_freebsd)
    return false;
#else
    if( !setAddressSize() ) {
        translate_printf("[%s:%u] - Failed to set address size.\n", __FILE__, __LINE__);
        return false;
    }

    // This information is derived from the DT_DEBUG field in the
    // executable's program headers -- however, the value needs to
    // be read from the loaded executable image so determine the
    // address of the DT_DEBUG field and then read it from the
    // process

    const char *l_err = "Failed to determine trap address.";

    getExecName();
    if( exec_name.empty() ) {
        translate_printf("[%s:%u] - %s\n", __FILE__, __LINE__, l_err);
        return false;
    }

    SymReader *exe = symfactory->openSymbolReader(exec_name);
    if( !exe ) {
        translate_printf("[%s:%u] - %s\n", __FILE__, __LINE__, l_err);
        return false;
    }
        
    // Need to get the address of the DYNAMIC segment
    Address dynAddress = 0;
    size_t dynSize = 0;
    unsigned numRegs = exe->numRegions();
    for(unsigned i = 0; i < numRegs; ++i) {
        SymRegion reg;
        exe->getRegion(i, reg);

        if( PT_DYNAMIC == reg.type ) {
            dynAddress = reg.mem_addr;
            dynSize = reg.mem_size;
            break;
        }
    }
    symfactory->closeSymbolReader(exe);

    if( !dynAddress || !dynSize ) {
        // This is okay for static binaries
        return false;
    }

    if( !reader->start() ) {
        translate_printf("[%s:%u] - %s\n", __FILE__, __LINE__, l_err);
        return false;
    }

    // Read the DYNAMIC segment from the process
    void *dynData = malloc(dynSize);
    if( !dynData || !reader->ReadMem(dynAddress, dynData, dynSize) ) {
        translate_printf("[%s:%u] - %s\n", __FILE__, __LINE__, l_err);
        if( dynData ) free(dynData);
        return false;
    }
    
    if( address_size == 8 ) {
        Elf64_Dyn *dynDataElf = (Elf64_Dyn *)dynData;
        for(unsigned i = 0; i < (dynSize / sizeof(Elf64_Dyn)); ++i) {
            if( DT_DEBUG == dynDataElf[i].d_tag ) {
                r_debug_addr = (Address) dynDataElf[i].d_un.d_ptr;
                break;
            }
        }
    }else{
        Elf32_Dyn *dynDataElf = (Elf32_Dyn *)dynData;
        for(unsigned i = 0; i < (dynSize / sizeof(Elf32_Dyn)); ++i) {
            if( DT_DEBUG == dynDataElf[i].d_tag ) {
                r_debug_addr = (Address) dynDataElf[i].d_un.d_ptr;
                break;
            }
        }
    }
    free(dynData);

    // When a process is initializing, the DT_DEBUG value could be zero
    // This function needs to indicate an error so the trap address can
    // be parsed from other sources (i.e., the interpreter)

    if( r_debug_addr ) {
        trap_addr = getTrapAddrFromRdebug();
        if( trap_addr == 0 ) {
            reader->done();
            return false;
        }
    }

    reader->done();
    
    return ( r_debug_addr != 0 );
#endif
}

bool AddressTranslateSysV::parseInterpreter() {
    bool result;

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
        if( interpreter->get_r_debug() ) {
            r_debug_addr = interpreter->get_r_debug() + interpreter_base;

            if( !reader->start() ) {
                translate_printf("[%s:%u] - Failed to initialize process reader\n", __FILE__, __LINE__);
                return false;
            }

            trap_addr = getTrapAddrFromRdebug();

            if( !reader->done() ) {
                translate_printf("[%s:%u] - Failed to finalize process reader\n", __FILE__, __LINE__);
                return false;
            }

            if( trap_addr == 0 ) {
                trap_addr = interpreter->get_r_trap() + interpreter_base;
            }
        }
        else
        {
            r_debug_addr = 0;
            trap_addr = interpreter->get_r_trap() + interpreter_base;
        }
    } else {
        r_debug_addr = 0;
        trap_addr = 0;
    }

    return true;
}

bool AddressTranslateSysV::init() {
   translate_printf("[%s:%u] - Initing AddressTranslateSysV\n", __FILE__, __LINE__);

   // Try to use DT_DEBUG first, falling back to parsing the interpreter binary if possible
   if( !parseDTDebug() ) {
       if( !parseInterpreter() ) {
           translate_printf("[%s:%u] - Failed to determine r_debug address\n", 
                   __FILE__, __LINE__);
           return false;
       }
   }

   translate_printf("[%s:%u] - trap_addr = 0x%lx, r_debug_addr = 0x%lx\n", __FILE__, __LINE__, trap_addr, r_debug_addr);
   translate_printf("[%s:%u] - Done with AddressTranslateSysV::init()\n", __FILE__, __LINE__);

   return true;
}

LoadedLib *AddressTranslateSysV::getLoadedLibByNameAddr(Address addr, std::string name)
{
   std::pair<Address, std::string> p(addr, name);
   sorted_libs_t::iterator i = sorted_libs.find(p);
   LoadedLib *ll = NULL;
   if (i != sorted_libs.end()) {
      ll = i->second;
   }
   else {
      ll = new LoadedLib(name, addr);
      ll->setFactory(symfactory);
      assert(ll);
      sorted_libs[p] = ll;
   }
   ll->setShouldClean(false);
   return ll;
}

Address AddressTranslateSysV::getTrapAddrFromRdebug() {
    Address retVal = 0;
    assert( r_debug_addr && address_size );

    if( address_size == sizeof(void *) ) {
        r_debug_dyn<r_debug> *r_debug_native = new r_debug_dyn<r_debug>(reader, r_debug_addr);
        if( !r_debug_native ) {
            translate_printf("[%s:%u] - Failed to parse r_debug struct.\n", __FILE__, __LINE__);
            return 0;
        }
        if( !r_debug_native->is_valid() ) {
            return 0;
        }
        retVal = (Address) r_debug_native->r_brk();
        delete r_debug_native;
    }else{
        r_debug_dyn<r_debug_dyn32> *r_debug_32 = new r_debug_dyn<r_debug_dyn32>(reader, r_debug_addr);
        if( !r_debug_32 ) {
            translate_printf("[%s:%u] - Failed to parse r_debug struct.\n", __FILE__, __LINE__);
            return 0;
        }
        if( !r_debug_32->is_valid() ) {
            return 0;
        }
        retVal = (Address) r_debug_32->r_brk();
        delete r_debug_32;
    }

    return retVal;
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

   if (!r_debug_addr) {
       // On systems that use DT_DEBUG to determine r_debug_addr, DT_DEBUG might
       // not be set right away -- read DT_DEBUG now and see if it is set
       if( !parseDTDebug() && !interpreter ) {
          translate_printf("[%s:%u] - Working with static binary, no libraries to refresh\n",
                  __FILE__, __LINE__);
          libs.clear();
          if (!exec) {
             exec = getAOut();
          }
          getArchLibs(libs);
          return true;
       }
   }

   std::vector<LoadedLib *>::iterator i;
   for (i = libs.begin(); i != libs.end(); i++)
      (*i)->setShouldClean(true);
   libs.clear();

   if (!exec) {
      exec = getAOut();
   }   
   exec->setShouldClean(false);
   libs.push_back(exec);
   getArchLibs(libs);
   
   if( !reader->start() ) {
       translate_printf("[%s:%u] - Failed to refresh libraries\n", __FILE__, __LINE__);
       return false;
   }

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
      else if (!r_debug_native->is_valid() && read_abort)
      {
         result = false;
         goto all_done;
      }
      else if (!r_debug_native->is_valid())
      {
         if (interpreter) {
            libs.push_back(getLoadedLibByNameAddr(interpreter_base,
                                                  interpreter->getFilename()));
         }
         result = true;
         goto done;
      }
      link_elm = new link_map_dyn<link_map>(reader, r_debug_native->r_map());
   }
   else {//64-bit mutator, 32-bit mutatee
      r_debug_32 = new r_debug_dyn<r_debug_dyn32>(reader, r_debug_addr);
      if (!r_debug_32)
      {
         result = true;
         goto done;
      }
      else if (!r_debug_32->is_valid() && read_abort)
      {
         result = false;
         goto all_done;
      }
      else if (!r_debug_32->is_valid())
      {
         if (interpreter) {
            libs.push_back(getLoadedLibByNameAddr(interpreter_base,
                                                  interpreter->getFilename()));
         }
         result = true;
         goto done;
      }
      link_elm = new link_map_dyn<link_map_dyn32>(reader, r_debug_32->r_map());
   }

   if (!link_elm->is_valid() && read_abort) {
      result = false;
      goto all_done;
   }
   if (!link_elm->is_valid()) {
      result = true;
      goto done;
   }

   do {
      if (!link_elm->l_name()) {
         if (read_abort) {
            result = false;
            goto all_done;
         }
         continue;
      }
      string obj_name(link_elm->l_name());

      // Don't re-add the executable, it has already been added
      if( getExecName() == string(deref_link(obj_name.c_str())) )
          continue;

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
      LoadedLib *ll = getLoadedLibByNameAddr(text, s);
      loaded_lib_count++;
      translate_printf("[%s:%u] -     New Loaded Library: %s(%lx)\n", __FILE__, __LINE__, s.c_str(), text);

      libs.push_back(ll);
   } while (link_elm->load_next());
   
   if (read_abort) {
      result = false;
      goto all_done;
   }

   translate_printf("[%s:%u] - Found %d libraries.\n", __FILE__, __LINE__, loaded_lib_count);

   result = true;
 done:
   reader->done();
   
   //Erase old elements from the sorted_libs
   sorted_libs.clear();
   for (vector<LoadedLib *>::iterator i = libs.begin(); i != libs.end(); i++)
   {
      LoadedLib *ll = *i;
      sorted_libs[pair<Address, string>(ll->getCodeLoadAddr(), ll->getName())] = ll;
   }

  all_done:

   if (read_abort) {
      translate_printf("[%s:%u] - refresh aborted due to async read\n", __FILE__, __LINE__);
   }
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

FCNode::FCNode(string f, dev_t d, ino_t i, SymbolReaderFactory *factory_) :
   device(d),
   inode(i),
   parsed_file(false),
   parse_error(false),
   is_interpreter(false),
   r_debug_offset(0),
   r_trap_offset(0),
   symreader(NULL),
   factory(factory_)
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

void FCNode::getRegions(vector<SymRegion> &regs) {
   parsefile();

   regs = regions;
}

unsigned FCNode::getAddrSize() {
   parsefile();

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

void FCNode::markInterpreter() {
   if (is_interpreter)
      return;

   assert(!parsed_file);
   is_interpreter = true;
}

#define NUM_DBG_BREAK_NAMES 3
const char *dbg_break_names[] = { "_dl_debug_state",
                                  "r_debug_state",
                                  "_r_debug_state" };

void FCNode::parsefile()
{
   if (parsed_file || parse_error)
      return;
   parsed_file = true;
   
   assert(!symreader);
   symreader = factory->openSymbolReader(filename);
   if (!symreader) {
      parse_error = true;
      translate_printf("[%s:%u] - Failed to open %s\n", __FILE__, __LINE__,
                       filename.c_str());
      return;
   }

   if (is_interpreter) {
#if !defined(os_freebsd)
      //We're parsing the interpreter, don't confuse this with
      // parsing the interpreter link info (which happens below).
      Symbol_t r_debug_sym = symreader->getSymbolByName(R_DEBUG_NAME);
      if (!symreader->isValidSymbol(r_debug_sym)) {
         translate_printf("[%s:%u] - Failed to find r_debug symbol in %s\n",
                          __FILE__, __LINE__, filename.c_str());
         parse_error = true;
      }
      r_debug_offset = symreader->getSymbolOffset(r_debug_sym);
#endif
      r_trap_offset = 0;
      for(unsigned i = 0; i < NUM_DBG_BREAK_NAMES; ++i) {
          Symbol_t r_trap_sym = symreader->getSymbolByName(dbg_break_names[i]);
          if( symreader->isValidSymbol(r_trap_sym) ) {
              r_trap_offset = symreader->getSymbolOffset(r_trap_sym);
              break;
          }
      }

      if( !r_trap_offset ) {
          translate_printf("[%s:%u] - Failed to find debugging trap symbol in %s\n",
                  __FILE__, __LINE__, filename.c_str());
          parse_error = true;
      }
   }

   addr_size = symreader->getAddressWidth();   
   interpreter_name = symreader->getInterpreterName();
   
   unsigned num_regions = symreader->numRegions();
   for (unsigned i=0; i<num_regions; i++) {
      SymRegion sr;
      bool result = symreader->getRegion(i, sr);
      if (!result) {
         translate_printf("[%s:%u] - Failed to get region info\n",
                          __FILE__, __LINE__);
         parse_error = true;
         break;
      }
      
      regions.push_back(sr);
   }
   /*factory->closeSymbolReader(symreader);
     symreader = NULL;*/
}

FCNode *FileCache::getNode(const string &filename, SymbolReaderFactory *factory)
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

   FCNode *fc = new FCNode(filename, buf.st_dev, buf.st_ino, factory);
   nodes.push_back(fc);

   return fc;
}

FileCache::FileCache()
{
}

Address AddressTranslateSysV::getLibraryTrapAddrSysV() {
  return trap_addr;
}
