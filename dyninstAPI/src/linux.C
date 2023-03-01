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

// $Id: linux.C,v 1.279 2008/09/03 06:08:44 jaw Exp $

#include "binaryEdit.h"
#include "dynProcess.h"
#include "image.h"
#include "function.h"
#include "instPoint.h"
#include "baseTramp.h"
#include "os.h"
#include "debug.h"
#include "mapped_object.h"
#include "mapped_module.h"
#include "linux.h"
#include <dlfcn.h>

#include <exception>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "boost/shared_ptr.hpp"

#include "pcEventMuxer.h"

#include "common/src/headers.h"
#include "common/src/linuxKludges.h"

#include "symtabAPI/h/Symtab.h"
using namespace Dyninst::SymtabAPI;
using namespace Dyninst::ProcControlAPI;

using std::string;

bool get_linux_version(int &major, int &minor, int &subvers)
{
    int subsub;
    return get_linux_version(major,minor,subvers,subsub); 
}

bool get_linux_version(int &major, int &minor, int &subvers, int &subsubvers)
{
   static int maj = 0, min = 0, sub = 0, subsub = 0;
   int result;
   FILE *f;
   if (maj)
   {
      major = maj;
      minor = min;
      subvers = sub;
      subsubvers = subsub;
      return true;
   }
   f = P_fopen("/proc/version", "r");
   if (!f) goto error;
   result = fscanf(f, "Linux version %d.%d.%d.%d", &major, &minor, &subvers,
                    &subsubvers);
   fclose(f);
   if (result != 3 && result != 4) goto error;

   maj = major;
   min = minor;
   sub = subvers;
   subsub = subsubvers;
   return true;

 error:
   //Assume 2.4, which is the earliest version we support
   major = maj = 2;
   minor = min = 4;
   subvers = sub = 0;
   subsubvers = subsub = 0;
   return false;
}

void PCProcess::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
        inferiorHeapType /* type */ ) 
{
    if (near) {
#if !defined(arch_x86_64) && !defined(arch_power) && !defined(arch_aarch64)
        lo = region_lo(near);
        hi = region_hi(near);
#else
        if (getAddressWidth() == 8) {
            lo = region_lo_64(near);
            hi = region_hi_64(near);
        } else {
            lo = region_lo(near);
            hi = region_hi(near);
        }
#endif
    }
}

inferiorHeapType PCProcess::getDynamicHeapType() const {
    return anyHeap;
}

void loadNativeDemangler() {}

bool PCProcess::dumpImage(string) {
    return false;
}

bool PCProcess::dumpCore(string) {
    return false;
}

bool PCProcess::skipHeap(const heapDescriptor &) {
    return false;
}

bool AddressSpace::usesDataLoadAddress() const {
    return false;
}

bool PCProcess::copyDanglingMemory(PCProcess *) {
    return true;
}


bool PCEventMuxer::useBreakpoint(Dyninst::ProcControlAPI::EventType et)
{
    // This switch statement can be derived from the EventTypes and Events
    // table in the ProcControlAPI manual -- it states what Events are
    // available on each platform

   // Pre-events are breakpoint
   // Post-events are callback
   if (et.time() == EventType::Pre &&
       ((et.code() == EventType::Fork) ||
	(et.code() == EventType::Exec))) return true;
   return false;
}

bool PCEventMuxer::useCallback(Dyninst::ProcControlAPI::EventType et)
{
    // This switch statement can be derived from the EventTypes and Events
    // table in the ProcControlAPI manual -- it states what Events are
    // available on each platform

   // Pre-events are breakpoint
   // Post-events are callback
  if(et.code() == EventType::Exit) return true;
  
  if (et.time() == EventType::Post &&
       ((et.code() == EventType::Fork) ||
        (et.code() == EventType::Exec))) return true;
  
   return false;
}

namespace
{
template <typename ContainerT = std::vector<std::string>,
          typename PredicateT = std::string (*)(const std::string&)>
inline ContainerT
delimit(
    const std::string& line, const std::string& delimiters = ":",
    PredicateT&& predicate = [](const std::string& s) -> std::string { return s; })
{
    size_t     _beginp = 0;  // position that is the beginning of the new string
    size_t     _delimp = 0;  // position of the delimiter in the string
    ContainerT _result = {};
    while(_beginp < line.length() && _delimp < line.length())
    {
        // find the first character (starting at _delimp) that is not a delimiter
        _beginp = line.find_first_not_of(delimiters, _delimp);
        // if no a character after or at _end that is not a delimiter is not found
        // then we are done
        if(_beginp == std::string::npos)
            break;
        // starting at the position of the new string, find the next delimiter
        _delimp = line.find_first_of(delimiters, _beginp);
        std::string _tmp{};
        try
        {
            // starting at the position of the new string, get the characters
            // between this position and the next delimiter
            if(_beginp < line.length())
                _tmp = line.substr(_beginp, _delimp - _beginp);
        } catch(std::exception& e)
        {
            // print the exception but don't fail, unless maybe it should?
            fprintf(stderr, "[%s:%i] %s (delimiters: %s) :: %s\n", __FILE__, __LINE__,
                    line.c_str(), delimiters.c_str(), e.what());
        }
        // don't add empty strings
        if(!_tmp.empty())
        {
            _result.emplace_back(std::forward<PredicateT>(predicate)(_tmp));
        }
    }
    return _result;
}
}  // namespace

bool
BinaryEdit::getResolvedLibraryPath(const string& filename, std::vector<string>& paths)
{
    auto _path_exists = [](const std::string& _filename) {
        struct stat dummy;
        return (_filename.empty()) ? false : (stat(_filename.c_str(), &dummy) == 0);
    };

    auto _emplace_if_exists = [&paths, filename,
                               _path_exists](const std::string& _directory) {
        auto _filename = _directory + "/" + filename;
        if(_path_exists(_filename))
            paths.emplace_back(std::move(_filename));
    };

    // prefer qualified file paths
    if(_path_exists(filename))
        paths.emplace_back(filename);

    // For cross-rewriting
    char* dyn_path = getenv("DYNINST_REWRITER_PATHS");
    if(dyn_path)
    {
        for(const auto& itr : delimit(dyn_path, ":"))
            _emplace_if_exists(itr);
    }

    // search paths from environment variables
    char* ld_path = getenv("LD_LIBRARY_PATH");
    if(ld_path)
    {
        for(const auto& itr : delimit(ld_path, ":"))
            _emplace_if_exists(itr);
    }

#ifdef DYNINST_COMPILER_SEARCH_DIRS
    // search compiler-specific library paths
    // NB1: DYNINST_COMPILER_SEARCH_DIRS is defined at build time
    // NB2: This is explicitly done _after_ adding directories from
    //		LD_LIBRARY_PATH so that the user can override these paths.
    {
#    define xstr(s) str(s)
#    define str(s) #    s

        for(const auto& itr : delimit(xstr(DYNINST_COMPILER_SEARCH_DIRS), ":"))
            _emplace_if_exists(itr);

#    undef str
#    undef xstr
    }
#endif

    // search ld.so.cache
    // apparently ubuntu doesn't like pclosing NULL, so a shared pointer custom
    // destructor is out. Ugh.
    FILE* ldconfig = popen("/sbin/ldconfig -p", "r");
    if(ldconfig)
    {
        constexpr size_t buffer_size = 512;
        char             buffer[buffer_size];
        // ignore first line
        if(fgets(buffer, buffer_size, ldconfig))
        {
            // each line constaining relevant info should be in form:
            //      <LIBRARY_BASENAME> (...) => <RESOLVED_ABSOLUTE_PATH>
            // example:
            //      libz.so (libc6,x86-64) => /lib/x86_64-linux-gnu/libz.so
            auto _get_entry = [](const std::string& _inp) {
                auto _paren_pos = _inp.find('(');
                auto _arrow_pos = _inp.find("=>", _paren_pos);
                if(_arrow_pos == std::string::npos || _paren_pos == std::string::npos)
                    return std::string{};
                if(_arrow_pos + 2 < _inp.length())
                {
                    auto _pos = _inp.find_first_not_of(" \t", _arrow_pos + 2);
                    if(_pos < _inp.length())
                        return _inp.substr(_pos);
                }
                return std::string{};
            };

            auto _data = std::stringstream{};
            while(fgets(buffer, buffer_size, ldconfig) != nullptr)
            {
                _data << buffer;
                auto _len = strnlen(buffer, buffer_size);
                if(_len > 0 && buffer[_len - 1] == '\n')
                {
                    auto _v = _data.str();
                    if(!_v.empty())
                    {
                        _v = _v.substr(_v.find_first_not_of(" \t"));
                        if(_v.length() > 1)
                        {
                            auto _entry = _get_entry(_v.substr(0, _v.length() - 1));
                            if(!_entry.empty())
                                _emplace_if_exists(_entry);
                        }
                    }
                    _data = std::stringstream{};
                }
            }
        }
        pclose(ldconfig);
    }

    // search hard-coded system paths
    for(const char* itr :
        { "/usr/local/lib", "/usr/share/lib", "/usr/lib", "/usr/lib64",
          "/usr/lib/x86_64-linux-gnu", "/lib", "/lib64", "/lib/x86_64-linux-gnu",
          "/usr/lib/i386-linux-gnu", "/usr/lib32" })
    {
        _emplace_if_exists(itr);
    }

    return (!paths.empty());
}

bool BinaryEdit::archSpecificMultithreadCapable() {
    /*
     * The heuristic for this check on Linux is that some symbols provided by
     * pthreads are only defined when the binary contains pthreads. Therefore,
     * check for these symbols, and if they are defined in the binary, then
     * conclude that the binary is multithread capable.
     */
    const int NUM_PTHREAD_SYMS = 4;
    const char *pthreadSyms[NUM_PTHREAD_SYMS] = 
        { "pthread_cancel", "pthread_once", 
          "pthread_mutex_unlock", "pthread_mutex_lock" 
        };
    if( mobj->isStaticExec() ) {
        int numSymsFound = 0;
        for(int i = 0; i < NUM_PTHREAD_SYMS; ++i) {
            const std::vector<func_instance *> *tmpFuncs = 
                mobj->findFuncVectorByPretty(pthreadSyms[i]);
            if( tmpFuncs != NULL && tmpFuncs->size() ) numSymsFound++;
        }

        if( numSymsFound == NUM_PTHREAD_SYMS ) return true;
    }

    return false;
}

// Temporary remote debugger interface.
// I assume these will be removed when procControlAPI is complete.
bool OS_isConnected(void)
{
    return true;  // We're always connected to the child on this platform.
}

bool OS_connect(BPatch_remoteHost &/*remote*/)
{
    return true;  // We're always connected to the child on this platform.
}

bool OS_getPidList(BPatch_remoteHost &/*remote*/,
                   BPatch_Vector<unsigned int> &/*tlist*/)
{
    return false;  // Not implemented.
}

bool OS_getPidInfo(BPatch_remoteHost &/*remote*/,
                   unsigned int /*pid*/, std::string &/*pidStr*/)
{
    return false;  // Not implemented.
}

bool OS_disconnect(BPatch_remoteHost &/*remote*/)
{
    return true;
}
