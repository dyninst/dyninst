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
// This information is cut from a generated SAGEIII file. 
// Since we only need the data structure definition I've
// extracted that from the larger generated file.

// All methods that do not appear to be used by x86InstructionSemantics.h
// have been commented out. 

#if !defined(SG_ASM_INSN_H)
#define SG_ASM_INSN_H

#include "external/rose/rose-compat.h"
#include "typedefs.h"
#include "SgNode.h"

#include "SgAsmOperandList.h"
#include <string>

class SgAsmInstruction : public SgNode {
 public:

    std::string get_mnemonic() const;
    void set_mnemonic(std::string mnemonic);
    
    SgUnsignedCharList get_raw_bytes() const;
    void set_raw_bytes(SgUnsignedCharList raw_bytes);
    
    SgAsmOperandList* get_operandList() const;
    void set_operandList(SgAsmOperandList* operandList);
    

    std::string get_comment() const;
    void set_comment(std::string comment);
    
    rose_addr_t get_address() const;
    void set_address(rose_addr_t address);

 protected:
    std::string p_mnemonic;
    
    SgUnsignedCharList p_raw_bytes;
    
    SgAsmOperandList* p_operandList;

    rose_addr_t p_address;
          
    std::string p_comment;
};

#endif
