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

#ifndef addrtranslate_h_
#define addrtranslate_h_

#include <string>
#include <vector>
#include <utility>

#include <unordered_map>
#include "common/h/dyntypes.h"
#include "common/h/SymReader.h"
#include "common/h/ProcReader.h"
#include "common/src/debug_common.h"

using namespace std;

namespace Dyninst {

class COMMON_EXPORT LoadedLib {
   friend class AddressTranslate;
   friend class AddressTranslateSysV;
 protected:
   string name;
   Address load_addr;
   Address data_load_addr;
   Address dynamic_addr;
   Address map_addr;
   bool should_clean;
   vector< pair<Address, unsigned long> > mapped_regions;
   SymReader *symreader;
   SymbolReaderFactory *symreader_factory;
   void *up_ptr;

 public:
    LoadedLib(string name, Address load_addr);
    virtual ~LoadedLib();
    void add_mapped_region(Address addr, unsigned long size);
   
    string getName() const;
    void setDataLoadAddr(Address a);
    vector< pair<Address, unsigned long> > *getMappedRegions();

    virtual Address offToAddress(Offset off);
    virtual Offset addrToOffset(Address addr);

    virtual Address getCodeLoadAddr() const;
    virtual Address getDataLoadAddr() const;
    virtual Address getDynamicAddr() const;
    virtual void getOutputs(string &filename, Address &code, Address &data);

    Address getMapAddr() const { return map_addr; }

    void* getUpPtr();
    void setUpPtr(void *v);

    void setShouldClean(bool b);
    bool shouldClean();

    void setFactory(SymbolReaderFactory *factory);
};

struct LoadedLibCmp
{
   COMMON_EXPORT bool operator()(const LoadedLib *a, const LoadedLib *b) const
   {
      if (a->getCodeLoadAddr() != b->getCodeLoadAddr()) 
         return a->getCodeLoadAddr() < b->getCodeLoadAddr();
      return (a->getName() < b->getName());
   }
};

class COMMON_EXPORT AddressTranslate {
 protected:
   PID pid;
   PROC_HANDLE phandle;
   bool creation_error;

   AddressTranslate(PID pid, PROC_HANDLE phand = INVALID_HANDLE_VALUE, std::string exename = std::string(""));
   vector<LoadedLib *> libs;
   std::string exec_name;
   LoadedLib *exec;
   SymbolReaderFactory *symfactory;
   bool read_abort;
 public:

    static AddressTranslate *createAddressTranslator(PID pid_,
                                         ProcessReader *reader_ = NULL,
                                         SymbolReaderFactory *symfactory_ = NULL,
                                         PROC_HANDLE phand = INVALID_HANDLE_VALUE,
                                         std::string exename = std::string(""),
                                         Address interp_base = (Address) -1);
    static AddressTranslate *createAddressTranslator(ProcessReader *reader_ = NULL,
                                         SymbolReaderFactory *symfactory_ = NULL,
                                         std::string exename = std::string(""),
                                         Address interp_base = (Address) -1);
   
    virtual bool refresh() = 0;
    virtual ~AddressTranslate();
  
    PID getPid();
    bool getLibAtAddress(Address addr, LoadedLib* &lib);
    bool getLibs(vector<LoadedLib *> &libs_);
    bool getArchLibs(vector<LoadedLib *> &olibs);
    LoadedLib *getLoadedLib(std::string name);
    LoadedLib *getLoadedLib(SymReader *sym);
    LoadedLib *getExecutable();

    virtual Address getLibraryTrapAddrSysV();
   
    void setReadAbort(bool b);
};

}


#endif
