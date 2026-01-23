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

#ifndef DYNINST_DATAFLOWAPI_ROSEAST_H
#define DYNINST_DATAFLOWAPI_ROSEAST_H

#include "AST.h"
// #include "ASTVisitor.h"
#include "dyninst_visibility.h"

#include <boost/make_shared.hpp>
#include <ostream>
#include <sstream>
#include <string>

namespace Dyninst { namespace DataflowAPI {

  struct DYNINST_EXPORT ROSEOperation {

    typedef enum {
      nullOp,
      extractOp,
      invertOp,
      negateOp,
      signExtendOp,
      equalToZeroOp,
      generateMaskOp,
      LSBSetOp,
      MSBSetOp,
      concatOp,
      andOp,
      orOp,
      xorOp,
      addOp,
      rotateLOp,
      rotateROp,
      shiftLOp,
      shiftROp,
      shiftRArithOp,
      derefOp,
      writeRepOp,
      writeOp,
      ifOp,
      sMultOp,
      uMultOp,
      sDivOp,
      sModOp,
      uDivOp,
      uModOp,
      extendOp,
      extendMSBOp
    } Op;

    ROSEOperation() = default;

    explicit ROSEOperation(Op o) : op(o), size(0UL) {}

    ROSEOperation(Op o, size_t s) : op(o), size(s) {}

    bool operator==(const ROSEOperation &rhs) const {
      return ((rhs.op == op) && (rhs.size == size));
    }

    std::string format() const {
      std::stringstream ret;
      ret << "<";
      switch(op) {
        case nullOp:
          ret << "null";
          break;
        case extractOp:
          ret << "extract";
          break;
        case invertOp:
          ret << "invert";
          break;
        case negateOp:
          ret << "negate";
          break;
        case signExtendOp:
          ret << "signExtend";
          break;
        case equalToZeroOp:
          ret << "eqZero?";
          break;
        case generateMaskOp:
          ret << "genMask";
          break;
        case LSBSetOp:
          ret << "LSB?";
          break;
        case MSBSetOp:
          ret << "MSB?";
          break;
        case concatOp:
          ret << "concat";
          break;
        case andOp:
          ret << "and";
          break;
        case orOp:
          ret << "or";
          break;
        case xorOp:
          ret << "xor";
          break;
        case addOp:
          ret << "add";
          break;
        case rotateLOp:
          ret << "rotL";
          break;
        case rotateROp:
          ret << "rotR";
          break;
        case shiftLOp:
          ret << "shl";
          break;
        case shiftROp:
          ret << "shr";
          break;
        case shiftRArithOp:
          ret << "shrA";
          break;
        case derefOp:
          ret << "deref";
          break;
        case writeRepOp:
          ret << "writeRep";
          break;
        case writeOp:
          ret << "write";
          break;
        case ifOp:
          ret << "if";
          break;
        case sMultOp:
          ret << "sMult";
          break;
        case uMultOp:
          ret << "uMult";
          break;
        case sDivOp:
          ret << "sDiv";
          break;
        case sModOp:
          ret << "sMod";
          break;
        case uDivOp:
          ret << "uDiv";
          break;
        case uModOp:
          ret << "uMod";
          break;
        case extendOp:
          ret << "ext";
          break;
        case extendMSBOp:
          ret << "extMSB";
          break;
        default:
          ret << " ??? ";
          break;
      };
      if(size) {
        ret << ":" << size;
      }
      ret << ">";
      return ret.str();
    }

    friend std::ostream &operator<<(std::ostream &stream, const ROSEOperation &c) {
      return stream << c.format();
    }

    Op op{nullOp};
    size_t size{};
  };

  class RoseAST : public AST {
  public:
    using Ptr = boost::shared_ptr<RoseAST>;

    RoseAST() = default;

    RoseAST(ROSEOperation t, AST::Ptr a) : t_{std::move(t)} {
      kids_.push_back(a);
    }

    RoseAST(ROSEOperation t, AST::Ptr a, AST::Ptr b) : t_{std::move(t)} {
      kids_.push_back(a);
      kids_.push_back(b);
    }

    RoseAST(ROSEOperation t, AST::Ptr a, AST::Ptr b, AST::Ptr c) : t_{std::move(t)} {
      kids_.push_back(a);
      kids_.push_back(b);
      kids_.push_back(c);
    }

    RoseAST(ROSEOperation t, Children kids) : t_{std::move(t)}, kids_{std::move(kids)} {}

    static Ptr create(ROSEOperation t, AST::Ptr a) {
      return boost::make_shared<RoseAST>(std::move(t), a);
    }

    static Ptr create(ROSEOperation t, AST::Ptr a, AST::Ptr b) {
      return boost::make_shared<RoseAST>(std::move(t), a, b);
    }

    static Ptr create(ROSEOperation t, AST::Ptr a, AST::Ptr b, AST::Ptr c) {
      return boost::make_shared<RoseAST>(std::move(t), a, b, c);
    }

    static Ptr create(ROSEOperation t, Children c) {
      return boost::make_shared<RoseAST>(std::move(t), std::move(c));
    }

    std::string format() const override {
      std::stringstream ret;
      ret << t_ << "(";
      for(Children::const_iterator i = kids_.begin(); i != kids_.end(); ++i) {
        ret << (*i)->format() << ",";
      }
      ret << ")";
      return ret.str();
    }

    AST::Ptr child(unsigned i) const override {
      return kids_[i];
    }

    unsigned numChildren() const override {
      return kids_.size();
    }

    AST::Ptr accept(ASTVisitor *v) override {
      return v->visit(this);
    }

    AST::ID getID() const override {
      return V_RoseAST;
    }

    static Ptr convert(AST::Ptr a) {
      if(a->getID() == V_RoseAST) {
        return boost::static_pointer_cast<RoseAST>(a);
      }
      return Ptr{};
    }

    const ROSEOperation &val() const {
      return t_;
    }

    void setChild(int i, AST::Ptr a) override {
      kids_[i] = a;
    }

  private:
    bool isStrictEqual(Dyninst::AST const &rhs) const override {
      auto const *ptr = dynamic_cast<RoseAST const *>(&rhs);
      if(!ptr) {
        return false;
      }
      if(!(t_ == ptr->t_)) {
        return false;
      }
      if(kids_.size() != ptr->kids_.size()) {
        return false;
      }
      for(unsigned i = 0; i < kids_.size(); ++i) {
        if(!(kids_[i]->equals(ptr->kids_[i]))) {
          return false;
        }
      }
      return true;
    }

    ROSEOperation t_{};
    Children kids_{};
  };

}}

#endif
