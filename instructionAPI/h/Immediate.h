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

#if !defined(IMMEDIATE_H)
#define IMMEDIATE_H

#include "Expression.h"

#include <map>
#include <sstream>
#include <string>

namespace Dyninst { namespace InstructionAPI {
  class DYNINST_EXPORT Immediate : public Expression {
  public:
    Immediate(const Result& val);

    virtual ~Immediate();

    virtual bool isUsed(Expression::Ptr findMe) const override;

    virtual std::string format(Architecture, formatStyle) const override;
    virtual std::string format(formatStyle) const override;

    static Immediate::Ptr makeImmediate(const Result& val);
    virtual void apply(Visitor* v) override;

  protected:
    virtual bool isStrictEqual(const Expression& rhs) const override;
  };

  class DYNINST_EXPORT NamedImmediate : public Immediate {
  public:
    NamedImmediate(std::string name, const Result& val);

    static NamedImmediate::Ptr makeNamedImmediate(std::string name, const Result& val);
    virtual std::string format(Architecture, formatStyle) const override;
    virtual std::string format(formatStyle) const override;

  private:
    std::string name_;
  };

  class DYNINST_EXPORT ArmConditionImmediate : public Immediate {
  public:
    ArmConditionImmediate(const Result& val);

    static ArmConditionImmediate::Ptr makeArmConditionImmediate(const Result& val);
    virtual std::string format(Architecture, formatStyle) const override;
    virtual std::string format(formatStyle) const override;

  private:
    std::map<unsigned int, std::string> m_condLookupMap;
  };

  class DYNINST_EXPORT ArmPrfmTypeImmediate : public Immediate {
  public:
    ArmPrfmTypeImmediate(const Result& val);

    static Immediate::Ptr makeArmPrfmTypeImmediate(const Result& val);
    virtual std::string format(Architecture, formatStyle) const override;
    virtual std::string format(formatStyle) const override;

  private:
    std::map<unsigned int, std::string> m_prfmTypeLookupMap;
  };
}}

#endif
