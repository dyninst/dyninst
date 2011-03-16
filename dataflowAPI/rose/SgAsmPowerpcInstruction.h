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
//
// C++ Interface: SgAsmPowerpcInstruction
//
// Description: 
//
//
// Author: Bill Williams <bill@follis.cs.wisc.edu>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#if !defined(SG_ASM_POWERPC_INSN_H)
#define SG_ASM_POWERPC_INSN_H

#include "external/rose/rose-compat.h"
#include "typedefs.h"
#include "external/rose/powerpcInstructionEnum.h"
#include "SgAsmInstruction.h"

// Class Definition for SgAsmPowerpcInstruction
class SgAsmPowerpcInstruction : public SgAsmInstruction
{

    public:
        /*! \brief returns a string representing the class name */
        virtual std::string class_name() const;

        /*! \brief returns new style SageIII enum values */
        virtual VariantT variantT() const; // MS: new variant used in tree traversal

        /*! \brief static variant value */
        static const VariantT static_variant = V_SgAsmPowerpcInstruction;

        /* the generated cast function */
        /*! \brief Casts pointer from base class to derived class */
        friend       SgAsmPowerpcInstruction* isSgAsmPowerpcInstruction(       SgNode * s );
        /*! \brief Casts pointer from base class to derived class (for const pointers) */
        friend const SgAsmPowerpcInstruction* isSgAsmPowerpcInstruction( const SgNode * s );

    public:
        PowerpcInstructionKind get_kind() const;
        void set_kind(PowerpcInstructionKind kind);


    public:
        virtual ~SgAsmPowerpcInstruction() {}


    public:
        SgAsmPowerpcInstruction(rose_addr_t address = 0, std::string mnemonic = "", PowerpcInstructionKind kind =
                powerpc_unknown_instruction);

    protected:
        unsigned int p_address;
        PowerpcInstructionKind p_kind;
        SgAsmOperandList* p_operandList;
        std::string p_mnemonic;
        SgUnsignedCharList p_raw_bytes;
};

   const char* regclassToString(PowerpcRegisterClass n);

   
#endif //!defined(SG_ASM_POWERPC_INSN_H)
