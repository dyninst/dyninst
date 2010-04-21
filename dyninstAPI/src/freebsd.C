/*
 * Copyright (c) 1996-2010 Barton P. Miller
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

#include <cassert>

#include "freebsd.h"
#include "dyn_lwp.h"
#include "process.h"
#include "syscallNotification.h"
#include "dynamiclinking.h"
#include "signalgenerator.h"
#include "binaryEdit.h"
#include "mapped_object.h"

void loadNativeDemangler() {}

bool BinaryEdit::getResolvedLibraryPath(const std::string &filename, std::vector<std::string> &paths) {
    char *libPathStr, *libPath;
    std::vector<std::string> libPaths;
    struct stat dummy;
    FILE *ldconfig;
    char buffer[512];

    // prefer qualified file paths
    if (stat(filename.c_str(), &dummy) == 0) {
        paths.push_back(filename);
    }

    // search paths from environment variables
    libPathStr = strdup(getenv("LD_LIBRARY_PATH"));
    libPath = strtok(libPathStr, ":");
    while (libPath != NULL) {
        libPaths.push_back(std::string(libPath));
        libPath = strtok(NULL, ":");
    }
    free(libPathStr);

    for (unsigned int i = 0; i < libPaths.size(); i++) {
        std::string str = libPaths[i] + "/" + filename;
        if (stat(str.c_str(), &dummy) == 0) {
            paths.push_back(str);
        }
    }

    // search ld.so hints file
    ldconfig = popen("/sbin/ldconfig -r", "r");
    if( ldconfig ) {
        // ignore first and second line
        fgets(buffer, 512, ldconfig);
        fgets(buffer, 512, ldconfig);

        // Here is the expected format:
        // [^/]* => (path)/(filename)
        while(fgets(buffer, 512, ldconfig) != NULL) {
            size_t fileBegin, pathBegin;

            // Remove any whitespace at the end
            std::string strBuf(buffer);
            strBuf = strBuf.substr(0, strBuf.find_last_not_of(" \t\n\r")+1);

            // Locate the filename
            fileBegin = strBuf.rfind("/");
            if( fileBegin == std::string::npos ||
                fileBegin+1 >= strBuf.length() ) continue;

            if( strBuf.substr(fileBegin+1) == filename ) {
                // Locate the path
                pathBegin = strBuf.find("/");
                if( pathBegin == std::string::npos ) continue;
                paths.push_back(strBuf.substr(pathBegin));
            }
        }

        pclose(ldconfig);
    }

    // search hard-coded system paths
    libPaths.clear();
    libPaths.push_back("/lib");
    libPaths.push_back("/usr/lib");
    libPaths.push_back("/usr/local/lib");
    for (unsigned int i = 0; i < libPaths.size(); i++) {
        std::string str = libPaths[i] + "/" + filename;
        if (stat(str.c_str(), &dummy) == 0) {
            paths.push_back(str);
        }
    }

    return ( 0 < paths.size() );
}

// TODO This was blindly copied from Linux
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

int_function *instPoint::findCallee() 
{

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

   // find the target address in the list of relocationEntries
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

         const char *target_name = fbt[i].name().c_str();
         process *dproc = dynamic_cast<process *>(proc());
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
         break;
      }
   }
       //fprintf(stderr, "%s[%d]:  returning NULL: target addr = %p\n", FILE__, __LINE__, (void *)target_addr);
   return NULL;
}



sharedLibHook::~sharedLibHook() {}

/* START unimplemented functions */

#define FREEBSD_NOT_IMPLEMENTED "This function is not implemented on FreeBSD"

void initPrimitiveCost() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
}

void dyninst_yield() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
}

bool dyn_lwp::stop_() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::installPostFork() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::dumpImage(std::string) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::realLWP_attach_() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::trapAtEntryPointOfMain(dyn_lwp*, unsigned long) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dynamic_linking::installTracing() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::removePreLwpExit() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

Frame dyn_lwp::getActiveFrame() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    Frame nullFrame;
    return nullFrame;
}

bool dyn_lwp::waitUntilStopped() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::hasPassedMain() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::writeTextWord(char*, int) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::handleTrapAtLibcStartMain(dyn_lwp*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::getRegisters_(dyn_saved_regs*, bool) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool SignalGenerator::decodeEvents(std::vector<EventRecord, std::allocator<EventRecord> >&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dynamic_linking::initialize() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::continueLWP_(int) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

syscallNotification::syscallNotification(syscallNotification*, process*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
}

bool dynamic_linking::decodeIfDueToSharedObjectMapping(EventRecord&, unsigned int&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::installPostExec() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::loadDYNINSTlibCleanup(dyn_lwp*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::changePC(unsigned long, dyn_saved_regs*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::insertTrapAtEntryPointOfMain() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::restoreRegisters_(dyn_saved_regs const&, bool) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::readDataSpace(void const*, unsigned int, void*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

Address dyn_lwp::readRegister(unsigned int) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::determineLWPs(std::vector<unsigned int, std::allocator<unsigned int> >&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::writeTextSpace(void*, unsigned int, void const*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

void dyn_lwp::realLWP_detach_() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
}

bool dyn_lwp::writeDataSpace(void*, unsigned int, void const*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

Frame dyn_thread::getActiveFrameMT() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    Frame nullFrame;
    return nullFrame;
}

bool syscallNotification::removePostExec() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

std::string process::tryToFindExecutable(std::string const&, int) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return std::string("");
}


bool process::unsetProcessFlags() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::decodeStartupSysCalls(EventRecord&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool SignalGeneratorCommon::getExecFileDescriptor(std::string, int, bool, int&, fileDescriptor&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dynamic_linking::handleIfDueToSharedObjectMapping(EventRecord&, std::vector<mapped_object*, std::allocator<mapped_object*> >&, 
        std::vector<bool, std::allocator<bool> >&) 
{
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

void OS::osTraceMe() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
}

bool Frame::setPC(unsigned long) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::instrumentLibcStartMain() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

void dyn_lwp::representativeLWP_detach_() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
}

bool process::handleTrapAtEntryPointOfMain(dyn_lwp*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::trapDueToDyninstLib(dyn_lwp*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::removePreExec() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dynamic_linking::processLinkMaps(std::vector<fileDescriptor, std::allocator<fileDescriptor> >&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::removePostFork() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::dumpCore_(std::string) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::removePreFork() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

dyn_lwp* process::createRepresentativeLWP() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return NULL;
}

bool syscallNotification::removePreExit() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::installPreExec() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::installPreFork() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::loadDYNINSTlib() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool SignalHandler::handleProcessExitPlat(EventRecord&, bool&) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::installPreLwpExit() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::representativeLWP_attach_() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::isRunning_() const {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::hasBeenBound(Dyninst::SymtabAPI::relocationEntry const&, int_function*&, unsigned long) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool syscallNotification::installPreExit() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool dyn_lwp::readTextSpace(const void*, unsigned int, void*) {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

bool process::setProcessFlags() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return false;
}

terminateProcStatus_t process::terminateProc_() {
    assert(!FREEBSD_NOT_IMPLEMENTED);
    return terminateFailed;
}

/* END unimplemented functions */
