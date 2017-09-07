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

#include "IndirectAnalyzer.h"
#include "IA_aarch32Details.h"
#include "Visitor.h"
#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "debug_parse.h"
#include <deque>
#include <boost/bind.hpp>
#include <algorithm>
#include <iterator>
#include <boost/iterator/indirect_iterator.hpp>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::InsnAdapter;
using namespace Dyninst::ParseAPI;

namespace Dyninst {
namespace InsnAdapter {
namespace detail {
    class TOCandOffsetExtractor : public Dyninst::InstructionAPI::Visitor
    {
      public:
	TOCandOffsetExtractor(Address TOCvalue)
            : result(0),
              toc_contents(TOCvalue)
        {}

	virtual ~TOCandOffsetExtractor() {}

	virtual void visit(BinaryFunction* b)
        {
            Address arg1 = m_stack.front();
            m_stack.pop_front();
            Address arg2 = m_stack.front();
            m_stack.pop_front();
            if (b->isAdd()) {
                result = arg1 + arg2;
            }
            else if (b->isMultiply()) {
                result = arg1 * arg2;
            }
            else {
                assert(!"unexpected binary function!");
                result = 0;
            }

            parsing_printf("\tTOC visitor visiting binary function,"
                           " result is 0x%lx\n", result);
            m_stack.push_front(result);
	}

	virtual void visit(Immediate* i)
        {
	  Address tmp = i->eval().convert<Address>();
	  result = tmp;
	  parsing_printf("\tTOC visitor visiting immediate,"
                         " result is 0x%lx\n", result);
	  m_stack.push_front(tmp);
        }

        virtual void visit(RegisterAST* r)
        {
            if (r->getID() == toc_reg->getID()) {
                m_stack.push_front(toc_contents);
            }
            else {
                m_stack.push_front(0);
            }
            result = m_stack.front();
            parsing_printf("\tTOC visitor visiting register,"
                           " result is 0x%lx\n", result);
        }

        virtual void visit(Dereference*) {}

        void clear()
        {
            m_stack.clear();
            result = 0;
        }

        std::deque<Address> m_stack;
        Address result;
        Address toc_contents;
        RegisterAST::Ptr toc_reg;
    };

} // End namespace detail
} // End namespace InsnAdapter
} // End namespace Dyninst

bool IA_aarch32Details::findTableAddrNoTOC(const IA_IAPI* /*blockToCheck*/)
{
    assert(0);
    return tableStartAddress == 0;
}

bool IA_aarch32Details::parseRelativeTableIdiom()
{
    assert(0);
    return true;
}

namespace detail_aarch32 {

bool isNonCallEdge(ParseAPI::Edge* e)
{
    assert(0);
    return e->type() != CALL;
}

bool leadsToVisitedBlock(ParseAPI::Edge* e, const std::set<Block*>& visited)
{
    assert(0);
    Block* src = e->src();
    return visited.find(src) != visited.end();
}

void processPredecessor(Dyninst::ParseAPI::Edge* /*e*/,
                        std::set<Block*>& /*visited*/,
                        std::deque<Block*>& /*worklist*/)
{
    assert(0);
}

} // End namespace detail_aarch32

bool IA_aarch32Details::scanForAdjustOrBase(IA_IAPI::allInsns_t::const_iterator /*start*/,
                                            IA_IAPI::allInsns_t::const_iterator /*end*/,
                                            RegisterAST::Ptr &/*jumpAddrReg*/)
{
    assert(0);
    return true;
}

// Like the above, but a wider net
bool IA_aarch32Details::findTableBase(IA_IAPI::allInsns_t::const_iterator /*start*/,
                                      IA_IAPI::allInsns_t::const_iterator /*end*/)
{
    assert(0);
    return true;
}

// This should only be called on a known indirect branch...
bool IA_aarch32Details::parseJumpTable(Block* /*currBlk*/,
                                       std::vector<std::pair< Address, EdgeTypeEnum> >& /*outEdges*/)
{
    assert(0);
    return true;
}

bool IA_aarch32Details::parseJumpTable(Dyninst::ParseAPI::Function* currFunc,
                                       Dyninst::ParseAPI::Block* currBlk,
				       std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges)
{
	IndirectControlFlowAnalyzer icfa(currFunc, currBlk);
	bool ret = icfa.NewJumpTableAnalysis(outEdges);

	parsing_printf("Jump table parser returned %d, %d edges\n", ret, outEdges.size());
	for (auto oit = outEdges.begin(); oit != outEdges.end(); ++oit) parsing_printf("edge target at %lx\n", oit->first);
	// Update statistics
	currBlk->obj()->cs()->incrementCounter(PARSE_JUMPTABLE_COUNT);
	if (!ret) currBlk->obj()->cs()->incrementCounter(PARSE_JUMPTABLE_FAIL);

	return ret;
}
