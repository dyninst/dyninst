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

#if !defined(addrtranslate_sysv_h_)
#define addrtranslate_sysv_h_

#include "symtabAPI/src/addrtranslate.h"
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/util.h"

namespace Dyninst {
namespace SymtabAPI {

class FCNode
{
   friend class FileCache;
protected:
   string filename;
   dev_t device;
   ino_t inode;

   bool parsed_file;
   bool parse_error;

   string interpreter_name;
   vector<Region> regions;
   unsigned addr_size;
   Offset r_debug_offset;
   Symtab *symtable;

   void parsefile();
public:
   FCNode(string f, dev_t d, ino_t i);

   string getFilename();

   string getInterpreter();
   void getRegions(vector<Region> &regs);
   unsigned getAddrSize();
   Offset get_r_debug();
   Symtab *getSymtab();
};

class FileCache
{
private:
   vector<FCNode *> nodes;
   
public:
   FileCache();
   
   FCNode *getNode(const string &filename);
};

extern FileCache files;

class AddressTranslateSysV : public AddressTranslate
{
public:
   bool setInterpreter();
   bool setAddressSize();
   bool setInterpreterBase();
   
   virtual bool init();
   virtual bool refresh();

   LoadedLib *getAOut();
   AddressTranslateSysV(int pid, ProcessReader *reader_);
   AddressTranslateSysV();
private:
   ProcessReader *reader;
   Address interpreter_base;
   bool set_interp_base;
   int address_size;
   FCNode *interpreter;

   unsigned previous_r_state;
   unsigned current_r_state;

   Address r_debug_addr;

   ProcessReader *createDefaultDebugger(int pid);
};

}
}

#endif


