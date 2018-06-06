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


#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include <fstream>
#include <set>
#include <string>
#include <vector>

// parseAPI
#include "CodeSource.h"
#include "CodeObject.h"
#include "CFG.h"

// symtabAPI
#include "Function.h"

// instructionAPI
#include "InstructionDecoder.h"
#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"

// dataflowAPI
#include "SymEval.h"
#include "slicing.h"
#include "types.h"

class trapLoc;
class SemanticDescriptorElem;

using namespace std;
using namespace Dyninst;

void getInsnInstances(ParseAPI::Block *block, Slicer::InsnVec &insns);

bool isSyscall(InstructionAPI::Instruction::Ptr & insn, Address & syscallTrampStore);

bool isCallToSyscallTrampStore(InstructionAPI::Instruction::Ptr & insn, Address & _syscallTramp);

Address getSyscallTrampStore(SymtabAPI::Symtab * symtab);

Address searchForSyscallTrampStore(ParseAPI::CodeObject::funclist & procedures);

#endif
