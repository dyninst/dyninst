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

#include "InstructionDecoder-aarch64.h"
#include "Immediate.h"
#include <boost/assign/list_of.hpp>
#include "../../common/src/singleton_object_pool.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
    typedef void (InstructionDecoder_aarch64::*operandFactory)();
    typedef std::vector< operandFactory > operandSpec;


    InstructionDecoder_aarch64::InstructionDecoder_aarch64(Architecture a)
      : InstructionDecoderImpl(a),
        insn(0),
        insn_in_progress(NULL),
	    isRAWritten(false),
        invertBranchCondition(false),
        isFPInsn(false),
        bcIsConditional(false)
    {
    }

    InstructionDecoder_aarch64::~InstructionDecoder_aarch64()
    {
    }

    void InstructionDecoder_aarch64::decodeOpcode(InstructionDecoder::buffer& b)
    {
      b.start += 4;
    }

    void InstructionDecoder_aarch64::mainDecode()
    {
        assert(0);
        return;
    }
  };
};




