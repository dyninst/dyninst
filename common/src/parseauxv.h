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

#include "common/src/Types.h"
#include <map>

class COMMON_EXPORT AuxvParser {
 private:
  int pid;
  unsigned ref_count;
  bool create_err;
  Address interpreter_base;
  Address vsyscall_base;
  Address vsyscall_text;
  Address vsyscall_end;
  bool found_vsyscall;
  Address phdr;

  unsigned page_size;
  unsigned addr_size;

  bool readAuxvInfo();
  void *readAuxvFromProc();
  void *readAuxvFromStack();
  Address getStackTop(bool &err);
  AuxvParser(int pid, unsigned asize);

  static std::map<int, AuxvParser *> pid_to_parser;

 public:
  static AuxvParser *createAuxvParser(int pid, unsigned asize);

  void deleteAuxvParser();
  ~AuxvParser();

  Address getInterpreterBase();
  bool parsedVsyscall();
  Address getVsyscallBase();
  Address getVsyscallText();
  Address getVsyscallEnd();
  Address getProgramBase();
  Address getPageSize();
};

#endif
