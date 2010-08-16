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
 
// $Id: reloc-func.h,v 1.5 2008/01/16 22:02:04 legendre Exp $

#if !defined(FUNC_RELOC_H)
#define FUNC_RELOC_H

#include "common/h/Types.h"
#include "arch-forward-decl.h" // instruction

class int_basicBlock;


// MOVE ME SOMEWHERE ELSE
// Signature for a modification of a function or basic block.
class funcMod {
 public:
    virtual bool modifyBBL(int_basicBlock *block,
                           pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &insns, 
                           bblInstance &bbl) = 0;
    virtual bool update(int_basicBlock *block,
                        pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &insns,
                        unsigned size) = 0;
    virtual ~funcMod() {}
};

class enlargeBlock : public funcMod {
 public:
    enlargeBlock(int_basicBlock *targetBlock, unsigned targetSize) :
        targetBlock_(targetBlock),
        targetSize_(targetSize) {}
    virtual ~enlargeBlock() {};

    virtual bool modifyBBL(int_basicBlock *block,
                           pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &,
                           bblInstance &bbl);

    virtual bool update(int_basicBlock *block,
                        pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &,
                        unsigned size);

    int_basicBlock *targetBlock_;
    unsigned targetSize_;
};

#endif
