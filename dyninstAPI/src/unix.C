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

// $Id: unix.C,v 1.243 2008/06/30 17:33:31 legendre Exp $

#include "os.h"
#include "debug.h"
#include "mapped_object.h"
#include "pcProcess.h"
#include "pcThread.h"
#include "function.h"

using namespace Dyninst::ProcControlAPI;

// Functions for all Unices //

int_function *PCThread::mapInitialFunc(int_function *ifunc) {
    return ifunc;
}

bool PCProcess::hideDebugger()
{
    return false;
}

bool OS::executableExists(const std::string &file) 
{
   struct stat file_stat;
   int stat_result;

   const char *fn = file.c_str();
   stat_result = stat(fn, &file_stat);
   return (stat_result != -1);
}

std::string PCProcess::createExecPath(const std::string &file, const std::string &dir) {
    std::string ret = file;
    if (dir.length() > 0) {
        if (!(file[0] == ('/'))) {
            // file does not start  with a '/', so it is a relative pathname
            // we modify it to prepend the given directory
            if (dir[dir.length()-1 ] == ('/') ) {
                // the dir already has a trailing '/', so we can
                // just concatenate them to get an absolute path
                ret =  dir + file;
            } else {
                // the dir does not have a trailing '/', so we must
                // add a '/' to get the absolute path
                ret =  dir + "/" + file;
            }
        } else {
            // file starts with a '/', so it is an absolute pathname
            // DO NOT prepend the directory, regardless of what the
            // directory variable holds.
            // nothing to do in this case
        }

    }
    return ret;
}

// If true is passed for ignore_if_mt_not_set, then an error won't be
// initiated if we're unable to determine if the program is multi-threaded.
// We are unable to determine this if the daemon hasn't yet figured out what
// libraries are linked against the application.  Currently, we identify an
// application as being multi-threaded if it is linked against a thread
// library (eg. libpthreads.a on AIX).  There are cases where we are querying
// whether the app is multi-threaded, but it can't be determined yet but it
// also isn't necessary to know.
bool PCProcess::multithread_capable(bool ignoreIfMtNotSet) {
#if !defined(cap_threads)
    return false;
#endif

    if( mt_cache_result_ != not_cached ) {
        if( mt_cache_result_ == cached_mt_true) return true;
        else return false;
    }

    if( mapped_objects.size() <= 1 ) {
        assert( ignoreIfMtNotSet && "Can't query MT state" );
        return false;
    }

    if(    findObject("libthread.so*", true) // Solaris
        || findObject("libpthreads.*", true) // AIX
        || findObject("libpthread.so*", true)) // Linux
    {
        mt_cache_result_ = cached_mt_true;
        return true;
    }

    mt_cache_result_ = cached_mt_false;
    return false;
}

bool PCEventHandler::shouldStopForSignal(int signal) {
    if( signal == SIGSTOP || signal == SIGINT ) return true;
    return false;
}

PCEventHandler::RTSignalResult
PCEventHandler::handleRTSignal_NP(EventSignal::const_ptr ev,
        PCProcess *evProc, Address rt_arg, int status) const
{
    /* Okay... we use both DYNINST_BREAKPOINT_SIGNUM and sigstop,
       depending on what we're trying to stop. So we have to check the
       flags against the signal
    */
    // This is split into two to make things easier
    if (ev->getSignal() == SIGSTOP) {
        // We only use stop on fork...
        if (status != DSE_forkExit) {
            proccontrol_printf("%s[%d]: SIGSTOP wasn't due to fork exit\n",
                    FILE__, __LINE__);
            return NotRTSignal;
        }
        // ... of the child
        if (rt_arg != 0) {
            proccontrol_printf("%s[%d]: parent %d received SIGSTOP\n",
                    FILE__, __LINE__, evProc->getPid());
            return NotRTSignal;
        }
    } else if (ev->getSignal() == DYNINST_BREAKPOINT_SIGNUM) {
        if ((status == DSE_forkExit) && (rt_arg == 0)) {
            proccontrol_printf("%s[%d]: child %d received signal %d\n",
                    FILE__ ,__LINE__, evProc->getPid(), DYNINST_BREAKPOINT_SIGNUM);
            return NotRTSignal;
        }
    } else {
        proccontrol_printf("%s[%d]: signal wasn't sent by RT library\n",
                FILE__, __LINE__);
        return NotRTSignal;
    }

    BPatch_process *bproc = BPatch::bpatch->getProcessByPid(evProc->getPid());
    if( bproc == NULL ) {
        proccontrol_printf("%s[%d]: no corresponding BPatch_process for process %d\n",
                FILE__, __LINE__, evProc->getPid());
        return ErrorInDecoding;
    }

    // See pcEventHandler.h (SYSCALL HANDLING) for a description of what
    // is going on here

    EventType reportEt;
    std::string eventStr;

    switch(status) {
    case DSE_forkEntry:
        proccontrol_printf("%s[%d]: decodeRTSignal_NP decoded forkEntry, arg = %lx\n",
                      FILE__, __LINE__, rt_arg);
        reportEt = EventType(EventType::Pre, EventType::Fork);
        eventStr = string("fork entry");
        switch(PCEventHandler::getCallbackBreakpointCase(reportEt)) {
            case BreakpointOnly: 
                proccontrol_printf("%s[%d]: reporting fork entry event to BPatch layer\n",
                        FILE__, __LINE__);
                break;
            default:
                break;
        }
        break;
    case DSE_forkExit:
        proccontrol_printf("%s[%d]: decodeRTSignal_NP decoded forkExit, arg = %lx\n",
                      FILE__, __LINE__, rt_arg);
        // TODO
        break;
    case DSE_execEntry:
        proccontrol_printf("%s[%d]: decodeRTSignal_NP decoded execEntry, arg = %lx\n",
                      FILE__, __LINE__, rt_arg);
        // TODO
        break;
    case DSE_execExit:
        proccontrol_printf("%s[%d]: decodeRTSignal_NP decoded execExit, arg = %lx\n",
                      FILE__, __LINE__, rt_arg);
        // This is not currently used by Dyninst internals for anything
        return ErrorInDecoding;
    case DSE_exitEntry:
        proccontrol_printf("%s[%d]: decodeRTSignal_NP decoded exitEntry, arg = %lx\n",
                      FILE__, __LINE__, rt_arg);
        /* Entry of exit, used for the callback. We need to trap before
           the process has actually exited as the callback may want to
           read from the process */
        // TODO
        break;
    case DSE_loadLibrary:
        proccontrol_printf("%s[%d]: decodeRTSignal_NP decoded loadLibrary (error), arg = %lx\n",
                      FILE__, __LINE__, rt_arg);
        // This is no longer used 
        return ErrorInDecoding;
    case DSE_lwpExit:
        proccontrol_printf("%s[%d]: decodeRTSignal_NP decoded lwpExit, arg = %lx\n",
                      FILE__, __LINE__, rt_arg);
        // This is not currently used on any platform
        return ErrorInDecoding;
    case DSE_snippetBreakpoint:
        proccontrol_printf("%s[%d]: decodeRTSignal_NP decoded snippetBreak, arg = %lx\n",
                      FILE__, __LINE__, rt_arg);
        bproc->setLastSignal(ev->getSignal());
        break;
    case DSE_stopThread:
        proccontrol_printf("%s[%d]: decodeRTSignal_NP decoded stopThread, arg = %lx\n",
                      FILE__, __LINE__, rt_arg);
        if( !handleStopThread(evProc, rt_arg) ) {
            proccontrol_printf("%s[%d]: failed to handle stopped thread event\n",
                    FILE__, __LINE__);
            return ErrorInDecoding;
        }
        bproc->setLastSignal(ev->getSignal());
        break;
    default:
        return NotRTSignal;
    }

    // Behavior common to all syscalls
    if( reportEt.code() != EventType::Unset ) {
        switch(PCEventHandler::getCallbackBreakpointCase(reportEt)) {
            case CallbackOnly:
                proccontrol_printf("%s[%d]: mistakenly received fork entry event from RT lib\n",
                        FILE__, __LINE__);
                return ErrorInDecoding;
            case BothCallbackBreakpoint: 
            {
                // Report the event to ProcControlAPI
                break;
            }
            default:
                // Don't need to do anything else for BreakpointOnly
                break;
        }
    }

    // Don't deliver any of the RT library signals to the process
    ev->clearSignal();
    return IsRTSignal;
}

mapped_object *PCProcess::createObjectNoFile(Address) {
    assert(0); // not implemented on UNIX
    return NULL;
}

// The following functions are only implemented on some Unices //

#if defined(os_linux) || defined(os_solaris) || defined(os_freebsd)

#include "dyninstAPI/src/binaryEdit.h"
#include "symtabAPI/h/Archive.h"

using namespace Dyninst::SymtabAPI;

std::map<std::string, BinaryEdit*> BinaryEdit::openResolvedLibraryName(std::string filename) {
    std::map<std::string, BinaryEdit *> retMap;

    std::vector<std::string> paths;
    std::vector<std::string>::iterator pathIter;

    // First, find the specified library file
    bool resolved = getResolvedLibraryPath(filename, paths);

    // Second, create a set of BinaryEdits for the found library
    if ( resolved ) {
        startup_printf("[%s:%u] - Opening dependent file %s\n",
                       FILE__, __LINE__, filename.c_str());

        Symtab *origSymtab = getMappedObject()->parse_img()->getObject();

        // Dynamic case
        if ( !origSymtab->isStaticBinary() ) {
            for(pathIter = paths.begin(); pathIter != paths.end(); ++pathIter) {
                BinaryEdit *temp = BinaryEdit::openFile(*pathIter);
                if (temp && temp->getAddressWidth() == getAddressWidth()) {
                    retMap.insert(std::make_pair(*pathIter, temp));
                    return retMap;
                }
                delete temp;
            }
        } else {
            // Static executable case

            /* 
             * Alright, this is a kludge, but even though the Archive is opened
             * twice (once here and once by the image class later on), it is
             * only parsed once because the Archive class keeps track of all
             * open Archives.
             *
             * This is partly due to the fact that Archives are collections of
             * Symtab objects and their is one Symtab for each BinaryEdit. In
             * some sense, an Archive is a collection of BinaryEdits.
             */
            for(pathIter = paths.begin(); pathIter != paths.end(); ++pathIter) {
                Archive *library;
                Symtab *singleObject;
                if (Archive::openArchive(library, *pathIter)) {
                    std::vector<Symtab *> members;
                    if (library->getAllMembers(members)) {
                        std::vector <Symtab *>::iterator member_it;
                        for (member_it = members.begin(); member_it != members.end();
                             ++member_it) 
                        {
                            BinaryEdit *temp = BinaryEdit::openFile(*pathIter, (*member_it)->memberName());
                            if (temp && temp->getAddressWidth() == getAddressWidth()) {
                                std::string mapName = *pathIter + string(":") +
                                    (*member_it)->memberName();
                                retMap.insert(std::make_pair(mapName, temp));
                            }else{
                                if(temp) delete temp;
                                retMap.clear();
                                break;
                            }
                        }

                        if (retMap.size() > 0) {
                            origSymtab->addLinkingResource(library);
                            return retMap;
                        }
                        //if( library ) delete library;
                    }
                } else if (Symtab::openFile(singleObject, *pathIter)) {
                    BinaryEdit *temp = BinaryEdit::openFile(*pathIter);
                    if (temp && temp->getAddressWidth() == getAddressWidth()) {
                        if( singleObject->getObjectType() == obj_SharedLib ||
                            singleObject->getObjectType() == obj_Executable ) 
                        {
                          startup_printf("%s[%d]: cannot load dynamic object(%s) when rewriting a static binary\n", 
                                  FILE__, __LINE__, pathIter->c_str());
                          std::string msg = std::string("Cannot load a dynamic object when rewriting a static binary");
                          showErrorCallback(71, msg.c_str());

                          delete singleObject;
                        }else{
                            retMap.insert(std::make_pair(*pathIter, temp));
                            return retMap;
                        }
                    }
                    if(temp) delete temp;
                }
            }
        }
    }

    startup_printf("[%s:%u] - Creation error opening %s\n",
                   FILE__, __LINE__, filename.c_str());
    retMap.clear();
    retMap.insert(std::make_pair("", static_cast < BinaryEdit * >(NULL)));
    return retMap;
}

#endif

#if defined(os_linux) || defined(os_freebsd)

#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/image-func.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/addressSpace.h"
#include "symtabAPI/h/Symtab.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/pcProcess.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/debug.h"
#include <elf.h>

// The following functions were factored from linux.C to be used
// on both Linux and FreeBSD

// findCallee: finds the function called by the instruction corresponding
// to the instPoint "instr". If the function call has been bound to an
// address, then the callee function is returned in "target" and the 
// instPoint "callee" data member is set to pt to callee's int_function.  
// If the function has not yet been bound, then "target" is set to the 
// int_function associated with the name of the target function (this is 
// obtained by the PLT and relocation entries in the image), and the instPoint
// callee is not set.  If the callee function cannot be found, (ex. function
// pointers, or other indirect calls), it returns false.
// Returns false on error (ex. process doesn't contain this instPoint).
int_function *instPoint::findCallee() {
   using namespace Dyninst::SymtabAPI;
   // Already been bound
   if (callee_) {
      return callee_;
   }  

   /*  if (ipType_ != callSite) {
      // Assert?
      return NULL; 
      }*/
   if (isDynamic()) {
      return NULL;
   }

   if( img_p_ == NULL ) return NULL;

   assert(img_p_);
   image_func *icallee = img_p_->getCallee(); 
   if (icallee && !icallee->isPLTFunction()) {
     callee_ = proc()->findFuncByInternalFunc(icallee);
     //callee_ may be NULL if the function is unloaded

       //fprintf(stderr, "%s[%d]:  returning %p\n", FILE__, __LINE__, callee_);
     return callee_;
   }

   // Do this the hard way - an inter-module jump
   // get the target address of this function
   Address target_addr = img_p_->callTarget();
   if(!target_addr) {
      // this is either not a call instruction or an indirect call instr
      // that we can't get the target address
       //fprintf(stderr, "%s[%d]:  returning NULL\n", FILE__, __LINE__);
      return NULL;
   }

   // get the relocation information for this image
   Symtab *obj = func()->obj()->parse_img()->getObject();
   pdvector<relocationEntry> fbt;
   vector <relocationEntry> fbtvector;
   if (!obj->getFuncBindingTable(fbtvector)) {

       //fprintf(stderr, "%s[%d]:  returning NULL\n", FILE__, __LINE__);
        return NULL;
        }

   /**
    * Object files and static binaries will not have a function binding table
    * because the function binding table holds relocations used by the dynamic
    * linker
    */
   if (!fbtvector.size() && !obj->isStaticBinary() && 
           obj->getObjectType() != obj_RelocatableFile ) 
   {
      fprintf(stderr, "%s[%d]:  WARN:  zero func bindings\n", FILE__, __LINE__);
   }

   for (unsigned index=0; index< fbtvector.size();index++)
        fbt.push_back(fbtvector[index]);
  
   Address base_addr = func()->obj()->codeBase();
   dictionary_hash<Address, std::string> *pltFuncs =
       func()->ifunc()->img()->getPltFuncs();

   // find the target address in the list of relocationEntries
   if (pltFuncs->defines(target_addr)) {
      for (u_int i=0; i < fbt.size(); i++) {
         if (fbt[i].target_addr() == target_addr) 
         {
            // check to see if this function has been bound yet...if the
            // PLT entry for this function has been modified by the runtime
            // linker
            int_function *target_pdf = 0;
            if (proc()->hasBeenBound(fbt[i], target_pdf, base_addr)) {
               callee_ = target_pdf;
               img_p_->setCalleeName(target_pdf->symTabName());
               //fprintf(stderr, "%s[%d]:  returning %p\n", FILE__, __LINE__, callee_);
               return callee_;  // target has been bound
            }
         }
      }

      const char *target_name = (*pltFuncs)[target_addr].c_str();
      PCProcess *dproc = dynamic_cast<PCProcess *>(proc());
      BinaryEdit *bedit = dynamic_cast<BinaryEdit *>(proc());
      img_p_->setCalleeName(std::string(target_name));
      pdvector<int_function *> pdfv;
      if (dproc) {
         bool found = proc()->findFuncsByMangled(target_name, pdfv);
         if (found) {
            return pdfv[0];
         }
      }
      else if (bedit) {
         std::vector<BinaryEdit *>::iterator i;
         for (i = bedit->getSiblings().begin(); i != bedit->getSiblings().end(); i++)
         {
            bool found = (*i)->findFuncsByMangled(target_name, pdfv);
            if (found) {
               return pdfv[0];
            }
         }
      }
      else 
         assert(0);
   }

   //fprintf(stderr, "%s[%d]:  returning NULL: target addr = %p\n", FILE__, __LINE__, (void *)target_addr);
   return NULL;
}

void BinaryEdit::makeInitAndFiniIfNeeded()
{
    using namespace Dyninst::SymtabAPI;

    Symtab* linkedFile = getAOut()->parse_img()->getObject();

    // Disable this for .o's and static binaries
    if( linkedFile->isStaticBinary() || 
        linkedFile->getObjectType() == obj_RelocatableFile ) 
    {
        return;
    }

    bool foundInit = false;
    bool foundFini = false;
    vector <Function *> funcs;
    if (linkedFile->findFunctionsByName(funcs, "_init")) {
        foundInit = true;
    }
    if (linkedFile->findFunctionsByName(funcs, "_fini")) {
        foundFini = true;
    }
    if( !foundInit )
    {
        Offset initOffset = linkedFile->getInitOffset();
        Region *initsec = linkedFile->findEnclosingRegion(initOffset);
        if(!initOffset || !initsec)
        {
            unsigned char* emptyFunction = NULL;
            int emptyFuncSize = 0;
#if defined(arch_x86) || defined(arch_x86_64)
            static unsigned char empty_32[] = { 0x55, 0x89, 0xe5, 0xc9, 0xc3 };
            static unsigned char empty_64[] = { 0x55, 0x48, 0x89, 0xe5, 0xc9, 0xc3 };
            if(linkedFile->getAddressWidth() == 8)
            {
                emptyFunction = empty_64;
                emptyFuncSize = 6;
            }
            else
            {
                emptyFunction = empty_32;
                emptyFuncSize = 5;
            }
#elif defined (arch_power)
            static unsigned char empty[] = { 0x4e, 0x80, 0x00, 0x20};
             emptyFunction = empty;
             emptyFuncSize = 4;
#endif //defined(arch_x86) || defined(arch_x86_64)
            linkedFile->addRegion(highWaterMark_, (void*)(emptyFunction), emptyFuncSize, ".init.dyninst",
                                  Dyninst::SymtabAPI::Region::RT_TEXT, true);
            highWaterMark_ += emptyFuncSize;
            lowWaterMark_ += emptyFuncSize;
            linkedFile->findRegion(initsec, ".init.dyninst");
            assert(initsec);
            linkedFile->addSysVDynamic(DT_INIT, initsec->getRegionAddr());
            startup_printf("%s[%d]: creating .init.dyninst region, region addr 0x%lx\n",
                           FILE__, __LINE__, initsec->getRegionAddr());
        }
        startup_printf("%s[%d]: ADDING _init at 0x%lx\n", FILE__, __LINE__, initsec->getRegionAddr());
        Symbol *initSym = new Symbol( "_init",
                                      Symbol::ST_FUNCTION,
                                      Symbol::SL_GLOBAL,
                                      Symbol::SV_DEFAULT,
                                      initsec->getRegionAddr(),
                                      linkedFile->getDefaultModule(),
                                      initsec,
                                      UINT_MAX );
        linkedFile->addSymbol(initSym);
    }
    if( !foundFini )
    {
        Offset finiOffset = linkedFile->getFiniOffset();
        Region *finisec = linkedFile->findEnclosingRegion(finiOffset);
        if(!finiOffset || !finisec)
        {
            unsigned char* emptyFunction = NULL;
            int emptyFuncSize = 0;
#if defined(arch_x86) || defined(arch_x86_64)
            static unsigned char empty_32[] = { 0x55, 0x89, 0xe5, 0xc9, 0xc3 };
            static unsigned char empty_64[] = { 0x55, 0x48, 0x89, 0xe5, 0xc9, 0xc3 };
            if(linkedFile->getAddressWidth() == 8)
            {
                emptyFunction = empty_64;
                emptyFuncSize = 6;
            }
            else
            {
                emptyFunction = empty_32;
                emptyFuncSize = 5;
            }

#elif defined (arch_power)
            static unsigned char empty[] = { 0x4e, 0x80, 0x00, 0x20};
             emptyFunction = empty;
             emptyFuncSize = 4;
#endif //defined(arch_x86) || defined(arch_x86_64)
            linkedFile->addRegion(highWaterMark_, (void*)(emptyFunction), emptyFuncSize, ".fini.dyninst",
                                  Dyninst::SymtabAPI::Region::RT_TEXT, true);
            highWaterMark_ += emptyFuncSize;
            lowWaterMark_ += emptyFuncSize;
            linkedFile->findRegion(finisec, ".fini.dyninst");
            assert(finisec);
            linkedFile->addSysVDynamic(DT_FINI, finisec->getRegionAddr());
            startup_printf("%s[%d]: creating .fini.dyninst region, region addr 0x%lx\n",
                           FILE__, __LINE__, finisec->getRegionAddr());

        }
        startup_printf("%s[%d]: ADDING _fini at 0x%lx\n", FILE__, __LINE__, finisec->getRegionAddr());
        Symbol *finiSym = new Symbol( "_fini",
                                      Symbol::ST_FUNCTION,
                                      Symbol::SL_GLOBAL,
                                      Symbol::SV_DEFAULT,
                                      finisec->getRegionAddr(),
                                      linkedFile->getDefaultModule(),
                                      finisec,
                                      UINT_MAX );
        linkedFile->addSymbol(finiSym);
    }
}

#endif
