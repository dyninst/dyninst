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

#include <string>
#include <iostream>
#include <sstream>

#include "Immediate.h"
#include "../../common/src/singleton_object_pool.h"
#include "Visitor.h"
#include "ArchSpecificFormatters.h"
#include <boost/assign/list_of.hpp>

namespace Dyninst {
    namespace InstructionAPI {
        Immediate::Ptr Immediate::makeImmediate(const Result &val) {
            return make_shared(singleton_object_pool<Immediate>::construct(val));
        }


        Immediate::Immediate(const Result &val) : Expression(val.type) {
            setValue(val);
        }

        Immediate::~Immediate() {
        }

        void Immediate::getChildren(vector<InstructionAST::Ptr> &) const {
            return;
        }

        void Immediate::getChildren(vector<Expression::Ptr> &) const {
            return;
        }

        bool Immediate::isUsed(InstructionAST::Ptr findMe) const {
            return *findMe == *this;
        }

        void Immediate::getUses(std::set<InstructionAST::Ptr> &) {
            return;
        }

        std::string Immediate::format(Architecture arch, formatStyle) const {
            return ArchSpecificFormatter::getFormatter(arch).formatImmediate(eval().format());
        }

        std::string Immediate::format(formatStyle) const {
            return eval().format();
        }

        bool Immediate::isStrictEqual(const InstructionAST &rhs) const {

            return (rhs.eval() == eval());
        }

        void Immediate::apply(Visitor *v) {
            v->visit(this);
        }

        NamedImmediate::NamedImmediate(std::string name, const Result &val) : Immediate(val) , name_(name){
        }

        Immediate::Ptr NamedImmediate::makeNamedImmediate(std::string name, const Result &val) {
            Immediate::Ptr ret = make_shared(singleton_object_pool<NamedImmediate>::construct(name,val));
            return ret;
        }

        std::string NamedImmediate::format(Architecture, formatStyle f) const {
            return format(f);
        }

        std::string NamedImmediate::format(formatStyle f) const {
            return name_+std::string(":0x")+Immediate::format(f);
        }
        ArmConditionImmediate::ArmConditionImmediate(const Result &val) : Immediate(val) {
            m_condLookupMap = boost::assign::map_list_of(0, "eq")(1, "ne")(2, "cs")(3, "cc")(4, "mi")(5, "pl")(6, "vs")(7, "vc")
                    (8, "hi")(9, "ls")(10, "ge")(11, "lt")(12, "gt")(13, "le")(14, "al")(15, "nv").convert_to_container<std::map<unsigned int, std::string> >();
        }

        Immediate::Ptr ArmConditionImmediate::makeArmConditionImmediate(const Result &val) {
            Immediate::Ptr ret = make_shared(singleton_object_pool<ArmConditionImmediate>::construct(val));
            return ret;
        }

        std::string ArmConditionImmediate::format(Architecture, formatStyle f) const {
            return format(f);
        }

        std::string ArmConditionImmediate::format(formatStyle) const {
            unsigned int cond_val = eval().convert<unsigned int>();
            if(m_condLookupMap.count(cond_val) > 0)
        return m_condLookupMap.find(cond_val)->second;
        else
        return "Error: Invalid condition code for ARM64!";
        }

	ArmPrfmTypeImmediate::ArmPrfmTypeImmediate(const Result &val) : Immediate(val) {
	    m_prfmTypeLookupMap = boost::assign::map_list_of(0, "PLDL1KEEP")(1, "PLDL1STRM")(2, "PLDL2KEEP")(3, "PLDL2STRM")(4, "PLDL3KEEP")(5, "PLDL3STRM")(8, "PLIL1KEEP")(9, "PLIL1STRM")(10, "PLIL2KEEP")(11, "PLIL2STRM")(12, "PLIL3KEEP")(13, "PLIL3STRM")(16, "PSTL1KEEP")(17, "PSTL1STRM")(18, "PSTL2KEEP")(19, "PSTL2STRM")(20, "PSTL3KEEP")(21, "PSTL3STRM").convert_to_container<std::map<unsigned int, std::string> >();
	}

	Immediate::Ptr ArmPrfmTypeImmediate::makeArmPrfmTypeImmediate(const Result &val) {
	    Immediate::Ptr ret = make_shared(singleton_object_pool<ArmPrfmTypeImmediate>::construct(val));
	    return ret;
	}

	std::string ArmPrfmTypeImmediate::format(Architecture, formatStyle f) const {
	    return format(f);
	}

    std::string ArmPrfmTypeImmediate::format(formatStyle) const {
        unsigned prfm_type = eval().convert<unsigned int>();
        if(m_prfmTypeLookupMap.count(prfm_type) > 0)
        return m_prfmTypeLookupMap.find(prfm_type)->second;
        else
        return "Error: Invalid prefetech memory type for ARM64!";
    }


    }
}
