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

#ifndef auxvparser_h
#define auxvparser_h

#include "dyninst_visibility.h"
#include <map>
#include "dyntypes.h"

class DYNINST_EXPORT AuxvParser
{
 private:
   int pid{-1};
   unsigned ref_count{0};
   bool create_err{true};
   Dyninst::Address interpreter_base{0};
   Dyninst::Address vsyscall_base{0};
   Dyninst::Address vsyscall_text{0};
   Dyninst::Address vsyscall_end{0};
   bool found_vsyscall{false};
   Dyninst::Address phdrBase{0};
   Dyninst::Address phdrCount{0};
   Dyninst::Address phdrSize{0};

   unsigned page_size{0};
   unsigned addr_size{0};
   
   bool readAuxvInfo();
   void *readAuxvFromProc();
   void *readAuxvFromStack();
   Dyninst::Address getStackTop(bool &err);
   AuxvParser(int pid, unsigned asize); 

   static std::map<int, AuxvParser *> pid_to_parser;

 public:
   static AuxvParser *createAuxvParser(int pid, unsigned asize);

   void deleteAuxvParser();
   ~AuxvParser();

   Dyninst::Address getInterpreterBase()     { return interpreter_base; }
   bool parsedVsyscall()                     { return found_vsyscall; }
   Dyninst::Address getVsyscallBase()        { return vsyscall_base; }
   Dyninst::Address getVsyscallText()        { return vsyscall_text; }
   Dyninst::Address getVsyscallEnd()         { return vsyscall_end; }
   Dyninst::Address getProgramHeaderBase()   { return phdrBase; }
   Dyninst::Address getProgramHeaderCount()  { return phdrCount; }
   Dyninst::Address getProgramHeaderSize()   { return phdrSize; }
   Dyninst::Address getPageSize()            { return page_size; }
};


#endif
