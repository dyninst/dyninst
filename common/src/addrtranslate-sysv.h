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

#if !defined(addrtranslate_sysv_h_)
#define addrtranslate_sysv_h_

#include <map>
#include <string>
#include <utility>
#include "common/src/addrtranslate.h"

namespace Dyninst {

class FCNode
{
   friend class FileCache;
protected:
   string filename;
   dev_t device;
   ino_t inode;

   bool parsed_file;
   bool parsed_file_fast;
   bool parse_error;
   bool is_interpreter;

   string interpreter_name;
   vector<SymSegment> segments;
   unsigned addr_size;
   Offset r_debug_offset;
   Offset r_trap_offset;
   SymReader *symreader;
   SymbolReaderFactory *factory;

   void parsefile();

public:
   FCNode(string f, dev_t d, ino_t i, SymbolReaderFactory *factory_);

   string getFilename();

   string getInterpreter();
   void markInterpreter();
   void getSegments(vector<SymSegment> &segs);
   unsigned getAddrSize();
   Offset get_r_debug();
   Offset get_r_trap();
};

class FileCache
{
private:
   vector<FCNode *> nodes;
   
public:
   FileCache();
   
   FCNode *getNode(const string &filename, Dyninst::SymbolReaderFactory *factory_);
};

extern FileCache files;

struct LibCmp
{
   bool operator()(const std::pair<Address, std::string> &a,
                   const std::pair<Address, std::string> &b) const
   {
      if (a.first != b.first)
         return a.first < b.first;
      return a.second < b.second;
   }
};

class AddressTranslateSysV : public AddressTranslate
{
public:
   bool init();
   virtual bool refresh();
   virtual Address getLibraryTrapAddrSysV();

   AddressTranslateSysV(int pid, ProcessReader *reader_, 
                        SymbolReaderFactory *reader_fact,
                        std::string exe_name,
                        Address interp_base);
   AddressTranslateSysV();
   virtual ~AddressTranslateSysV() {}

private:
   ProcessReader *reader;
   Address interpreter_base;
   Address program_base;
   Address page_size;
   bool set_interp_base;
   int address_size;
   FCNode *interpreter;

   unsigned previous_r_state;
   unsigned current_r_state;

   Address r_debug_addr;
   Address trap_addr;
   Address getTrapAddrFromRdebug();

   LoadedLib *getLoadedLibByNameAddr(Address addr, std::string name);
   typedef std::map<std::pair<Address, std::string>, LoadedLib *, LibCmp> sorted_libs_t;
   sorted_libs_t sorted_libs;

   /* platform-specific functions */
   std::string getExecName();
   ProcessReader *createDefaultDebugger(int pid);

   /* 
    * These functions need to set r_debug_addr, trap_addr and address_size
    */
   bool parseDTDebug();
   bool parseInterpreter();

   bool setInterpreter();
   bool setAddressSize();
   bool setInterpreterBase();
   LoadedLib *getAOut();

   // PPC64-Linux uses an AIX-style function pointer, so the trap addr
   // we're provided is a pointer to the actual address.

   bool plat_getTrapAddr();
   Address real_trap_addr;

   Address adjustForAddrSpaceWrap(Address addr, std::string name);

};

}

#endif


