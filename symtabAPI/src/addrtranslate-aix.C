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

#include "symtabAPI/src/addrtranslate.h"
#include "common/h/headers.h"

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"

#include <sys/procfs.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

namespace Dyninst {
namespace SymtabAPI {

class AddressTranslateAIX : public AddressTranslate
{
public:
   AddressTranslateAIX();
   AddressTranslateAIX(PID pid);

   virtual bool refresh();
   virtual ~AddressTranslateAIX();
};

class LoadedLibAIX : public LoadedLib {
protected:
   string object;

   Address real_codeBase;
   Address real_dataBase;
   Address imageOffset;
   Address dataOffset;
   
   bool reals_set;
   void setReals();
public:
   LoadedLibAIX(string name, Address load_addr, string object);
   
   virtual Symtab *getSymtab();
   virtual ~LoadedLibAIX();

   virtual Address symToAddress(Symbol *sym);
   virtual Address offToAddress(Offset off);
   virtual Offset addrToOffset(Address addr);

   virtual Address getCodeLoadAddr();
   virtual Address getDataLoadAddr();
   virtual void getOutputs(string &filename, Address &code, Address &data);
};

static int open_map_fd(PID pid)
{
   char file[64];
   snprintf(file, 64, "/proc/%d/map", pid);
   int fd = P_open(file, O_RDONLY, pid);
   return fd;
}

static char *deref_link(char *path)
{
   static char buffer[PATH_MAX], *p;
   buffer[PATH_MAX-1] = '\0';
   p = realpath(path, buffer);
   if (p)
      return p;
   return path;
}

bool AddressTranslateAIX::refresh()
{
   unsigned iter = 0;
   bool is_aout = true;
   int map_fd = -1;
   int result;
   if (!pid)
      return true;
   
   for (unsigned i=0; i<libs.size(); i++) {
      if (libs[i])
         delete libs[i];
   }
   libs.clear();
   
   map_fd = open_map_fd(pid);
   if (map_fd == -1)
      return false;

   prmap_t mapEntry;

   for (;;) {
      result = pread(map_fd, &mapEntry, sizeof(prmap_t), iter * sizeof(prmap_t));
      if (result != sizeof(prmap_t))
         break;
      if (mapEntry.pr_size == 0)
         break;

      char buf[512];
      if (mapEntry.pr_pathoff) {
         pread(map_fd, buf, 512, mapEntry.pr_pathoff);
      }
      
      printf("%lu\n" 
             "\taddr = %llx +%llu\n"
             "\tmapname = %s\n"
             "\toffset = %llu, flags = %d\n"
             "\tpr_pathoff = %d (%s)\n"
             "\tobject = %s\n"
             "\talias = %llx, gp = %llx\n",
             iter * sizeof(prmap_t), 
             mapEntry.pr_vaddr, mapEntry.pr_size, 
             mapEntry.pr_mapname,
             mapEntry.pr_off, mapEntry.pr_mflags,
             mapEntry.pr_pathoff, mapEntry.pr_pathoff ? buf : "NONE",
             mapEntry.pr_pathoff ? buf + strlen(buf) + 1 : "NONE",
             mapEntry.pr_alias, mapEntry.pr_gp);
      
      if (mapEntry.pr_pathoff) {
         string filename = buf;
         string object_name = buf + strlen(buf) + 1;
         LoadedLib *ll = new LoadedLibAIX(filename, 0, object_name);
         Symtab *s = ll->getSymtab();
         printf("\timageOffset = %ld, length = %lu\n"
                "\tdataOffset = %ld, length = %lu\n",
                s->imageOffset(), s->imageLength(),
                s->dataOffset(), s->dataLength());
      }

      printf("\n");
      iter++;
   }

   iter = 0;

   for (;;) {
      result = pread(map_fd, &mapEntry, sizeof(prmap_t), iter * sizeof(prmap_t));
      if (result != sizeof(prmap_t))
         break;
      if (mapEntry.pr_size == 0)
         break;
      
      string filename;
      string object_name;
      /*      
              if (is_aout) {
                char buf[128];
                sprintf(buf, "/proc/%d/object/a.out", pid);
                filename = deref_link(buf);
              }
      */
      if (mapEntry.pr_pathoff) {
         char buf[512];
         pread(map_fd, buf, 256, mapEntry.pr_pathoff);
         filename = deref_link(buf);
         object_name = buf + strlen(buf) + 1;
      }
      else {
         filename = deref_link(mapEntry.pr_mapname);
      }
      is_aout = false;
      
      LoadedLib *ll = new LoadedLibAIX(filename, mapEntry.pr_vaddr, object_name);

      iter++;
      ll->add_mapped_region(mapEntry.pr_vaddr, mapEntry.pr_size);
      libs.push_back(ll);

      prmap_t next;
      result = pread(map_fd, &next, sizeof(prmap_t), iter * sizeof(prmap_t));
      if (result != sizeof(prmap_t))
         break;
      if (strcmp(mapEntry.pr_mapname, next.pr_mapname))
         continue;

      iter++;
      ll->add_mapped_region(next.pr_vaddr, next.pr_size);
      ll->setDataLoadAddr(next.pr_vaddr);
   }
   
   P_close(map_fd);
   return true;
}

AddressTranslate *AddressTranslate::createAddressTranslator(PID pid_, 
                                                            ProcessReader *)
{
   AddressTranslate *at = new AddressTranslateAIX(pid_);
   
   if (!at) {
      return NULL;
   }
   else if (at->creation_error) {
      delete at;
      return NULL;
   }
   return at;
}

AddressTranslate *AddressTranslate::createAddressTranslator(ProcessReader *)
{
   return createAddressTranslator(getpid());
}

AddressTranslate *AddressTranslate::createAddressTranslator(const std::vector<LoadedLibrary> &name_addrs)
{
   AddressTranslate *at = new AddressTranslateAIX();
   
   if (!at) {
      return NULL;
   }
   else if (at->creation_error) {
      delete at;
      return NULL;
   }
   
   for (unsigned i=0; i<name_addrs.size(); i++)
   {
      string::size_type cpos = name_addrs[i].name.find(':');
      string archive_name, object_name;
      if (cpos == string::npos)
         archive_name = name_addrs[i].name;
      else
      {
         archive_name = name_addrs[i].name.substr(0, cpos);
         object_name = name_addrs[i].name.substr(cpos+1);
      }

      LoadedLibAIX *ll = new LoadedLibAIX(archive_name, name_addrs[i].codeAddr,
                                          object_name);
      ll->setDataLoadAddr(name_addrs[i].dataAddr);
      
      Symtab *st = ll->getSymtab();
      if (!st)
         continue;
      vector<Region> regs;
      bool result = st->getMappedRegions(regs);
      if (!result)
         continue;
      
      ll->add_mapped_region(name_addrs[i].codeAddr, regs[0].size);
      if (name_addrs[i].dataAddr) {
         ll->add_mapped_region(name_addrs[i].dataAddr, regs[1].size);
      }
      at->libs.push_back(ll);
   }
   return at;
}

AddressTranslateAIX::AddressTranslateAIX()
   : AddressTranslate(0)
{
}

AddressTranslateAIX::AddressTranslateAIX(PID pid)
   : AddressTranslate(pid)
{
   refresh();
}

AddressTranslateAIX::~AddressTranslateAIX()
{
}

vector< pair<Address, unsigned long> > *LoadedLib::getMappedRegions()
{
   return &mapped_regions;
}

static map<string, Symtab *> openedFiles;
static map<string, Archive *> openedArchives;

Symtab *LoadedLib::getSymtab()
{
   assert(0);
   return NULL;
}

Symtab *LoadedLibAIX::getSymtab()
{
   if (symtable)
      return symtable;

   if (object.length())
   {
      string hash_name = name + ":" + object;
      if (openedFiles.count(hash_name)) {
         symtable = openedFiles[hash_name];
         return symtable;
      }

      Archive *archive;
      if (openedArchives.count(name))
      {
         archive = openedArchives[name];
      }
      else
      {
         if (!Archive::openArchive(archive, name))
            return NULL;
         openedArchives[name] = archive;
      }
      
      bool result = archive->getMember(symtable, object);
      if (!result || !symtable)
         return NULL;
      openedFiles[hash_name] = symtable;

      return symtable;
   }

   if (openedFiles.count(name)) {
      symtable = openedFiles[name];
      return symtable;
   }

   bool result = Symtab::openFile(symtable, name);
   if (!result)
      return NULL;
   
   return symtable;
}

LoadedLibAIX::LoadedLibAIX(string name, Address load_addr, string obj)
   : LoadedLib(name, load_addr),
     object(obj),
     real_codeBase(0),
     real_dataBase(0),
     imageOffset(0),
     dataOffset(0),
     reals_set(false)
{
}

LoadedLibAIX::~LoadedLibAIX()
{
}

void LoadedLibAIX::setReals()
{
   if (reals_set)
      return;

   Symtab *sym = getSymtab();
   if (!sym)
      return;
   
   imageOffset = sym->imageOffset();
   dataOffset = sym->dataOffset();
   
   if (imageOffset > load_addr)
      real_codeBase = 0;
   else {
      real_codeBase = load_addr;
      if (imageOffset < 0x20000000)
         real_codeBase -= imageOffset;
      Section *sec;
      bool result = sym->findSection(sec, ".text");
      if (result && sec)
         real_codeBase += (Address) sec->getPtrToRawData() - sym->getBaseOffset();
   }

   if (dataOffset >= data_load_addr)
      real_dataBase = 0;
   else if (dataOffset < 0x30000000) {
      real_dataBase = data_load_addr - dataOffset;
   }

   reals_set = true;
}

Address LoadedLibAIX::offToAddress(Offset off)
{
   setReals();
   
   Address addr = off;

   if ((imageOffset < dataOffset && addr >= imageOffset && addr < dataOffset) ||
       (imageOffset > dataOffset && addr > imageOffset))
   {
      return addr + real_codeBase;
   }
   else 
   {
      return addr + real_dataBase;
   }
}

Address LoadedLibAIX::symToAddress(Symbol *sym)
{
   setReals();
   
   Address symAddr = sym->getAddr();

   if ((imageOffset < dataOffset && symAddr >= imageOffset && symAddr < dataOffset) ||
       (imageOffset > dataOffset && symAddr > imageOffset))
   {
      return symAddr + real_codeBase;
   }
   else 
   {
      return symAddr + real_dataBase;
   }
}

Offset LoadedLibAIX::addrToOffset(Address addr)
{
   setReals();

   if (addr >= mapped_regions[0].first && 
       addr < mapped_regions[0].first + mapped_regions[0].second)
      return addr - real_codeBase;
   else
      return addr - real_dataBase;  
}

Address LoadedLibAIX::getCodeLoadAddr()
{
   setReals();
   return real_codeBase;
}

Address LoadedLibAIX::getDataLoadAddr()
{
   setReals();
   return real_dataBase;
}

void LoadedLibAIX::getOutputs(string &filename, Address &code, Address &data)
{
   if (object.length())
      filename = name + ":" + object;
   else
      filename = name;
   code = load_addr;
   data = data_load_addr;
}

}
}
