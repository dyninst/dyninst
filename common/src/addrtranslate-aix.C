/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#include "common/h/addrtranslate.h"
#include "common/h/headers.h"

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"

#include <sys/procfs.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

namespace Dyninst {

class AddressTranslateAIX : public AddressTranslate
{
public:
   AddressTranslateAIX(SymbolReaderFactory *fact_);
   AddressTranslateAIX(PID pid, SymbolReaderFactory *fact_);

   virtual bool refresh();
   virtual ~AddressTranslateAIX();
   Address getLibraryTrapAddrSysV();
};

class LoadedLibAIX : public LoadedLib {
protected:
   string object;

   mutable Address real_codeBase;
   mutable Address real_dataBase;
   mutable Address imageOffset;
   mutable Address dataOffset;   
   mutable bool reals_set;
   void setReals() const;
public:
   LoadedLibAIX(string name, Address load_addr, string object);
   
   virtual ~LoadedLibAIX();

   virtual Address offToAddress(Offset off);
   virtual Offset addrToOffset(Address addr);

   virtual Address getCodeLoadAddr() const;
   virtual Address getDataLoadAddr() const;
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

#if defined(DEBUG_PRINT)
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
#endif

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
      
      LoadedLib *ll = new LoadedLibAIX(filename, (unsigned long)mapEntry.pr_vaddr, object_name);

      iter++;
      ll->add_mapped_region((unsigned long)mapEntry.pr_vaddr, mapEntry.pr_size);
      libs.push_back(ll);

      prmap_t next;
      result = pread(map_fd, &next, sizeof(prmap_t), iter * sizeof(prmap_t));
      if (result != sizeof(prmap_t))
         break;
      if (strcmp(mapEntry.pr_mapname, next.pr_mapname))
         continue;

      iter++;
      ll->add_mapped_region((unsigned long)next.pr_vaddr, next.pr_size);
      ll->setDataLoadAddr((unsigned long)next.pr_vaddr);
   }
   
   P_close(map_fd);
   return true;
}

AddressTranslate *AddressTranslate::createAddressTranslator(PID pid_, 
                                                            ProcessReader *,
                                                            SymbolReaderFactory *fact,
                                                            PROC_HANDLE, 
							    std::string)
{
   AddressTranslate *at = new AddressTranslateAIX(pid_, fact);
   
   if (!at) {
      return NULL;
   }
   else if (at->creation_error) {
      delete at;
      return NULL;
   }
   return at;
}

AddressTranslate *AddressTranslate::createAddressTranslator(ProcessReader *, SymbolReaderFactory *fact, std::string exename)
{
   return createAddressTranslator(getpid(), NULL, fact);
}

AddressTranslateAIX::AddressTranslateAIX(SymbolReaderFactory *fact)
   : AddressTranslate(0)
{
   symfactory = fact;
}
   
AddressTranslateAIX::AddressTranslateAIX(PID pid, SymbolReaderFactory *fact)
   : AddressTranslate(pid)
{
   symfactory = fact;
   refresh();
}

AddressTranslateAIX::~AddressTranslateAIX()
{
}

vector< pair<Address, unsigned long> > *LoadedLib::getMappedRegions()
{
   return &mapped_regions;
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

void LoadedLibAIX::setReals() const
{
   if (reals_set)
      return;

   SymReader *sreader = symreader_factory->openSymbolReader(name);
   imageOffset = sreader->imageOffset();
   dataOffset = sreader->dataOffset();
   real_codeBase = 0;
   real_dataBase = 0;
   /*
   Symtab *sym = getSymtab();
   if (!sym)
      return;

   if (imageOffset > load_addr)
      real_codeBase = 0;
   else {
      real_codeBase = load_addr;
      if (imageOffset < 0x20000000)
         real_codeBase -= imageOffset;
      Region *sec;
      bool result = sym->findRegion(sec, ".text");
      if (result && sec)
         real_codeBase += (Address) sec->getPtrToRawData() - sym->getBaseOffset();
   }

   if (dataOffset >= data_load_addr)
      real_dataBase = 0;
   else if (dataOffset < 0x30000000) {
      real_dataBase = data_load_addr - dataOffset;
   }
   */
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

Offset LoadedLibAIX::addrToOffset(Address addr)
{
   setReals();

   if (addr >= mapped_regions[0].first && 
       addr < mapped_regions[0].first + mapped_regions[0].second)
      return addr - real_codeBase;
   else
      return addr - real_dataBase;  
}

Address LoadedLibAIX::getCodeLoadAddr() const
{
   setReals();
   return real_codeBase;
}

Address LoadedLibAIX::getDataLoadAddr() const
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

Address AddressTranslateAIX::getLibraryTrapAddrSysV()
{
   return 0x0;
}

}
