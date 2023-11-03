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

#include "common/src/parseauxv.h"
#include <elf.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <vector>

std::map<int, AuxvParser *> AuxvParser::pid_to_parser;

AuxvParser *AuxvParser::createAuxvParser(int pid, unsigned asize)
{
   AuxvParser *newparser = NULL;
   if (pid_to_parser.count(pid))
   {
      newparser = pid_to_parser[pid];
   }
   else
   {
      newparser = new AuxvParser(pid, asize);
      if (!newparser)
      {
         return NULL;
      }
      if (newparser->create_err)
      {
         delete newparser;
         return NULL;
      }
      pid_to_parser[pid] = newparser;
   }

   newparser->ref_count++;
   return newparser;
}

void AuxvParser::deleteAuxvParser()
{
   assert(ref_count);
   ref_count--;
   if (!ref_count) {
      delete this;
      return;
   }
}

AuxvParser::~AuxvParser()
{
   pid_to_parser.erase(pid);
}

AuxvParser::AuxvParser(int pid_, unsigned addr_size_) :
   pid(pid_),
   ref_count(0),
   interpreter_base(0x0),
   vsyscall_base(0x0),
   vsyscall_text(0x0),
   vsyscall_end(0x0),
   found_vsyscall(false),
   phdr(0x0),
   page_size(0x0),
   addr_size(addr_size_)
{
   create_err = !readAuxvInfo();
}
 
Dyninst::Address AuxvParser::getInterpreterBase()
{
   return interpreter_base;
}
 
bool AuxvParser::parsedVsyscall()
{
   return found_vsyscall;
}

Dyninst::Address AuxvParser::getVsyscallBase()
{
   return vsyscall_base;
}

Dyninst::Address AuxvParser::getVsyscallText()
{
   return vsyscall_text;
}

Dyninst::Address AuxvParser::getVsyscallEnd()
{
   return vsyscall_end;
}   

Dyninst::Address AuxvParser::getProgramBase()
{
   return phdr;
}

Dyninst::Address AuxvParser::getPageSize()
{
   return page_size;
}
