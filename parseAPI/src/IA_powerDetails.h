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

#if !defined(IA_POWER_DETAILS_H)
#define IA_POWER_DETAILS_H

#include "CFG.h"
#include "dyntypes.h"
#include "IA_platformDetails.h"

namespace Dyninst
{
    namespace InsnAdapter
    {
        namespace detail
        {
            class TOCandOffsetExtractor;
        }
        
        class IA_powerDetails : public IA_platformDetails
        {
            friend IA_platformDetails* makePlatformDetails(Dyninst::Architecture Arch, const IA_IAPI* cb);
            protected:
                IA_powerDetails(const IA_IAPI* cb) :
                    IA_platformDetails(cb),
                    tableIsRelative(false), tableStartAddress(0),
                    adjustTableStartAddress(0), jumpStartAddress(0),
                    adjustEntry(0), foundAdjustEntry(false), TOC_address(0)
                {}
            public:
                virtual ~IA_powerDetails() {}
                virtual bool parseJumpTable(Dyninst::ParseAPI::Block* currBlk,
                                            std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges);
            private:
                bool findTableAddrNoTOC(const IA_IAPI* blockToCheck);
                bool parseRelativeTableIdiom();
		bool scanForAdjustOrBase(IA_IAPI::allInsns_t::const_iterator start,
					 IA_IAPI::allInsns_t::const_iterator end,
					 Dyninst::InstructionAPI::RegisterAST::Ptr &jumpAddrReg);

		bool findTableBase(IA_IAPI::allInsns_t::const_iterator start,
				   IA_IAPI::allInsns_t::const_iterator end);
					 
                
                boost::shared_ptr<detail::TOCandOffsetExtractor> toc_visitor;
		std::set<int> dfgregs;
                //std::map<Address, Dyninst::InstructionAPI::Instruction::Ptr>::const_iterator patternIter;
                IA_IAPI::allInsns_t::const_iterator patternIter;
                
                bool tableIsRelative;
                Address tableStartAddress;
                Address adjustTableStartAddress;
                Address jumpStartAddress;
                Address adjustEntry;
                bool foundAdjustEntry;
                Address TOC_address;
                Dyninst::InstructionAPI::RegisterAST::Ptr toc_reg;
        };
    }
}

#endif //!defined(IA_POWER_DETAILS_H)
