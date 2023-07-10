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

#if !defined(LIBSTATE_H_)
#define LIBSTATE_H_

#include "common/h/ProcReader.h"
#include "common/h/SymReader.h"
#include "stackwalk/h/procstate.h"
#include "common/src/addrtranslate.h"
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <set>

namespace Dyninst {
namespace Stackwalker {

class swkProcessReader : public ProcessReader {
 private:
   ProcessState *procstate;
 public:
   swkProcessReader(ProcessState *pstate, std::string executable_);
   virtual bool start();
   virtual bool ReadMem(Address inTraced, void *inSelf, unsigned amount);
   virtual bool GetReg(Dyninst::MachRegister, Dyninst::MachRegisterVal&) { return false; }
   virtual bool done();
   virtual ~swkProcessReader();
};

class TrackLibState : public LibraryState {
 private:
   bool needs_update;
   bool has_updated;
   AddressTranslate *translate;
   static SymbolReaderFactory *symfactory;
   swkProcessReader procreader;
   
   bool updateLibs();
   bool refresh();
   std::vector<std::pair<LibAddrPair, unsigned> > arch_libs;

   void getCurList(std::set<LibAddrPair> &list);
 public:
   TrackLibState(ProcessState *parent, std::string executable = "");
   virtual bool getLibraryAtAddr(Address addr, LibAddrPair &olib);
   virtual bool getLibraries(std::vector<LibAddrPair> &olibs, bool allow_refresh = true);
   virtual bool getAOut(LibAddrPair &ao);
   virtual void notifyOfUpdate();
   virtual Address getLibTrapAddress();
   virtual ~TrackLibState();
};

class StaticBinaryLibState : public LibraryState {
   LibAddrPair the_exe;
 public:
   StaticBinaryLibState(ProcessState *parent, std::string executable = "");
   ~StaticBinaryLibState();
   virtual bool getLibraryAtAddr(Address addr, LibAddrPair &olib);
   virtual bool getLibraries(std::vector<LibAddrPair> &olibs, bool allow_refresh = true);
   virtual bool getLibc(LibAddrPair &lc);
   virtual bool getLibthread(LibAddrPair &lt);
   virtual bool getAOut(LibAddrPair &ao);
   virtual void notifyOfUpdate();
   virtual Address getLibTrapAddress();
};

SymbolReaderFactory *getDefaultSymbolReader();

class LibraryWrapper {
  private:
   std::map<std::string, SymReader *> file_map;
  public:
   static SymReader *testLibrary(std::string filename);
   static SymReader *getLibrary(std::string filename);
   static void registerLibrary(SymReader *reader, std::string filename);
};

}
}

#endif
