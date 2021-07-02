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

#include "stackwalk/src/libstate.h"

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/walker.h"

#include <set>
#include <algorithm>
#include <iterator>

#include <string.h>

using namespace Dyninst;
using namespace Stackwalker;
using namespace std;

SymbolReaderFactory *TrackLibState::symfactory = NULL;

TrackLibState::TrackLibState(ProcessState *parent, std::string executable_) : 
   LibraryState(parent),
   needs_update(true),
   has_updated(false),
   translate(NULL),
   procreader(parent, executable_)
{
   PID pid = procstate->getProcessId();
   
   sw_printf("[%s:%d] - Creating a TrackLibState on pid %d\n",
             FILE__, __LINE__, pid);
   if (!symfactory)
      symfactory = Walker::getSymbolReader();

   if (procstate->isFirstParty()) {
      translate = AddressTranslate::createAddressTranslator(&procreader, symfactory, executable_);
   }
   else {
      translate = AddressTranslate::createAddressTranslator(pid, &procreader, symfactory, INVALID_HANDLE_VALUE, executable_);
   }
   if (!translate) {
      sw_printf("[%s:%d] - Creation of AddressTranslate failed "
                "on pid %d!\n", FILE__, __LINE__, pid);
   }
   assert(translate);
}

void TrackLibState::getCurList(std::set<LibAddrPair> &list)
{
   vector<LoadedLib *> libs;
   bool result = translate->getLibs(libs);
   if (!result) {
      return;
   }
   vector<LoadedLib *>::iterator i;
   for (i = libs.begin(); i != libs.end(); i++) {
      LibAddrPair o;
      o.first = (*i)->getName();
      o.second = (*i)->getCodeLoadAddr();
      list.insert(o);
   }
}

bool TrackLibState::updateLibs()
{
   if (!needs_update)
      return true;
   needs_update = false;

   std::set<LibAddrPair> pre, post, diff;
   if (has_updated) {
      getCurList(pre);
   }
   has_updated = true;

   PID pid = procstate->getProcessId();
   bool result = translate->refresh();
   if (!result) {
      sw_printf("[%s:%d] - Could not get load addresses out of SymtabAPI for %d."
                "This may happen during process create before libs have be set up\n",
                 FILE__, __LINE__, pid);
      needs_update = true;
   }

   vector<pair<LibAddrPair, unsigned int> > libs;
   if (!updateLibsArch(libs)) {
#if !defined(os_linux) && !defined(arch_x86_64)
      sw_printf("[%s:%d] - updateLibsArch failed\n",  FILE__, __LINE__);
#endif
   }

   getCurList(post);
   
   StepperGroup *group = procstate->getWalker()->getStepperGroup();
   set_difference(pre.begin(), pre.end(),
                 post.begin(), post.end(), 
                 inserter(diff, diff.begin()));
   for (set<LibAddrPair>::iterator i = diff.begin(); i != diff.end(); i++) {
      LibAddrPair la = *i;
      group->newLibraryNotification(&la, library_unload);
   }
   diff.clear();

   set_difference(post.begin(), post.end(),
                 pre.begin(), pre.end(), 
                 inserter(diff, diff.begin()));
   for (set<LibAddrPair>::iterator i = diff.begin(); i != diff.end(); i++) {
      LibAddrPair la = *i;
      group->newLibraryNotification(&la, library_load);
   }
                 
   return true;
}

bool TrackLibState::getLibraryAtAddr(Address addr, LibAddrPair &olib)
{
   bool result = refresh();
   if (!result) {
      sw_printf("[%s:%d] - Failed to refresh library.\n", FILE__, __LINE__);
      setLastError(err_symtab, "Failed to refresh library list");
      return false;
   }
   
   Address load_addr;
   
   std::vector<std::pair<LibAddrPair, unsigned> >::iterator i;
   for (i = arch_libs.begin(); i != arch_libs.end(); i++) {
      load_addr = (*i).first.second;
      unsigned size = (*i).second;
      if ((addr >= load_addr) && (addr < load_addr + size)) {
         olib = (*i).first;
         return true;
      }
   }

   LoadedLib *ll;
   result = translate->getLibAtAddress(addr, ll);
   if (!result) {
      sw_printf("[%s:%d] - no file loaded at %lx\n", FILE__, __LINE__, addr);
      setLastError(err_nofile, "No file loaded at specified address");
      return false;
   }

   olib.first = ll->getName();
   olib.second = ll->getCodeLoadAddr();
   
   return true;
}

bool TrackLibState::getLibraries(std::vector<LibAddrPair> &olibs, bool allow_refresh)
{
   bool result;
   if (allow_refresh) {
      result = refresh();
      if (!result) {
         setLastError(err_symtab, "Failed to refresh library list");
         return false;
      }
   }

   vector<LoadedLib *> libs;
   result = translate->getLibs(libs);
   if (!result) {
      setLastError(err_symtab, "No objects in process");
      return false;
   }
   
   olibs.clear();
   vector<LoadedLib *>::iterator i;
   for (i = libs.begin(); i != libs.end(); i++) {
      LibAddrPair o;
      o.first = (*i)->getName();
      o.second = (*i)->getCodeLoadAddr();
      olibs.push_back(o);
   }
   return true;
}

bool TrackLibState::refresh()
{
   bool result = updateLibs();
   if (!result)
      return false;
   //TODO: Determine difference, notify steppergroup of diff
   return true;
}

Address TrackLibState::getLibTrapAddress() {
   return translate->getLibraryTrapAddrSysV();
}

TrackLibState::~TrackLibState() {
  delete translate;
  
}

void TrackLibState::notifyOfUpdate() {
   //This may be called under a signal handler, keep simple
   needs_update = true;
}

static bool libNameMatch(const char *s, const char *libname)
{
   // A poor-man's regex match for */lib<s>[0123456789-.]*.so*
   const char *filestart = strrchr(libname, '/');
   if (!filestart)
      filestart = libname;
   const char *lib_start = strstr(filestart, "lib");
   if (lib_start != filestart+1)
      return false;
   const char *libname_start = lib_start+3;
   int s_len = strlen(s);
   if (strncmp(s, libname_start, s_len) != 0) {
      return false;
   }

   const char *cur = libname_start + s_len;
   const char *valid_chars = "0123456789-.";
   while (*cur) {
      if (!strchr(valid_chars, *cur)) {
         cur--;
         if (strstr(cur, ".so") == cur) {
            return true;
         }
         return false;
      }
      cur++;
   }
   return false;
}

bool LibraryState::getLibc(LibAddrPair &addr_pair)
{
   std::vector<LibAddrPair> libs;
   getLibraries(libs, false);
   if (libs.size() == 1) {
      //Static binary.
      addr_pair = libs[0];
      return true;
   }
   for (std::vector<LibAddrPair>::iterator i = libs.begin(); i != libs.end(); i++)
   {
      if (libNameMatch("c", i->first.c_str())) {
         addr_pair = *i;
         return true;
      }
   }
   return false;
}

bool LibraryState::getLibthread(LibAddrPair &addr_pair)
{
   std::vector<LibAddrPair> libs;
   getLibraries(libs, false);
   if (libs.size() == 1) {
      //Static binary.
      addr_pair = libs[0];
      return true;
   }
   for (std::vector<LibAddrPair>::iterator i = libs.begin(); i != libs.end(); i++)
   {
      if (libNameMatch("pthread", i->first.c_str()) || libNameMatch("thr", i->first.c_str()))  {
         addr_pair = *i;
         return true;
      }
   }
   return false;
}

bool TrackLibState::getAOut(LibAddrPair &addr_pair)
{
   LoadedLib *aout;
   updateLibs();
   aout = translate->getExecutable();
   if (!aout) {
      sw_printf("[%s:%d] - Error.  SymtabAPI getAOut failed\n",
                FILE__, __LINE__);
      return false;
   }
   addr_pair = LibAddrPair(aout->getName(), aout->getCodeLoadAddr());
   return true;
}

swkProcessReader::swkProcessReader(ProcessState *pstate, std::string /*executable*/) :
   procstate(pstate)
{
}

bool swkProcessReader::ReadMem(Address inTraced, void *inSelf, unsigned amount)
{
  return procstate->readMem(inSelf, inTraced, amount);
}

swkProcessReader::~swkProcessReader()
{
}

bool swkProcessReader::start()
{
   return true;
}

bool swkProcessReader::done()
{
   return true;
}

static LibraryWrapper libs;

SymReader *LibraryWrapper::getLibrary(std::string filename)
{
   std::map<std::string, SymReader *>::iterator i = libs.file_map.find(filename);
   if (i != libs.file_map.end()) {
      return i->second;
   }
   
   SymbolReaderFactory *fact = Walker::getSymbolReader();
   SymReader *reader = fact->openSymbolReader(filename);
   if (!reader)
      return NULL;
   libs.file_map[filename] = reader;
   return reader;
}

void LibraryWrapper::registerLibrary(SymReader *reader, std::string filename)
{
   libs.file_map[filename] = reader;
}
 
SymReader *LibraryWrapper::testLibrary(std::string filename)
{
   std::map<std::string, SymReader *>::iterator i = libs.file_map.find(filename);
   if (i != libs.file_map.end()) {
      return i->second;
   }
   return NULL;
}

StaticBinaryLibState::StaticBinaryLibState(ProcessState *parent, std::string executable) :
   LibraryState(parent)
{
   the_exe.first = executable;
   the_exe.second = 0x0;
}

StaticBinaryLibState::~StaticBinaryLibState()
{
}

bool StaticBinaryLibState::getLibraryAtAddr(Address /*addr*/, LibAddrPair &olib)
{
   olib = the_exe;
   return true;
}

bool StaticBinaryLibState::getLibraries(std::vector<LibAddrPair> &olibs, bool /*allow_refresh*/)
{
   olibs.push_back(the_exe);
   return true;
}

bool StaticBinaryLibState::getLibc(LibAddrPair &lc)
{
   lc = the_exe;
   return true;
}

bool StaticBinaryLibState::getLibthread(LibAddrPair &lt)
{
   lt = the_exe;
   return true;
}

bool StaticBinaryLibState::getAOut(LibAddrPair &ao)
{
   ao = the_exe;
   return true;
}

void StaticBinaryLibState::notifyOfUpdate()
{
}

Address StaticBinaryLibState::getLibTrapAddress()
{
   return 0x0;
}
