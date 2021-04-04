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
#include "ExpressionConversionVisitor.h"

#include "Register.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "Dereference.h"

#include <list>

#include "../rose/SgAsmExpression.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace DataflowAPI;

void ExpressionConversionVisitor::visit(InstructionAPI::Immediate *immed) {
    // no children

    const Result &value = immed->eval();

    // TODO rose doesn't distinguish signed/unsigned within the value itself,
    // only at operations?

    // TODO rose doesn't handle large values (XMM?)

    // build different kind of rose value object based on type
    if(arch == Arch_aarch64 || arch == Arch_ppc32 || arch == Arch_ppc64 || arch == Arch_amdgpu_vega) {
        bool isSigned = false;
        switch (value.type) {
            case s8:
                isSigned = true;
            case u8:
                roseExpression = new SgAsmIntegerValueExpression(value.val.u8val,
                        new SgAsmIntegerType(ByteOrder::ORDER_UNSPECIFIED, 8,
                            isSigned));
                break;
            case s16:
                isSigned = true;
            case u16:
                roseExpression = new SgAsmIntegerValueExpression(value.val.u16val,
                        new SgAsmIntegerType(ByteOrder::ORDER_LSB, 16,
                            isSigned));
                break;
            case s32:
                isSigned = true;
            case u32:
                roseExpression = new SgAsmIntegerValueExpression(value.val.u32val,
                        new SgAsmIntegerType(ByteOrder::ORDER_LSB, 32,
                            isSigned));
                break;
            case s48:
                isSigned = true;
            case u48:
                roseExpression = new SgAsmIntegerValueExpression(value.val.u32val,
                        new SgAsmIntegerType(ByteOrder::ORDER_LSB, 32,
                            isSigned));
                break;
            case s64:
                isSigned = true;
            case u64:
                roseExpression = new SgAsmIntegerValueExpression(value.val.u64val,
                        new SgAsmIntegerType(ByteOrder::ORDER_LSB, 64,
                            isSigned));
                break;
            case sp_float:
                roseExpression = new SgAsmSingleFloatValueExpression(value.val.floatval);
                break;
            case dp_float:
                roseExpression = new SgAsmDoubleFloatValueExpression(value.val.dblval);
                break;
            default:
                roseExpression = NULL;
                assert(0);
        }
    } else {
        switch (value.type) {
            case s8:
            case u8:
                roseExpression = new SgAsmByteValueExpression(value.val.u8val);
                break;
            case s16:
            case u16:
                roseExpression = new SgAsmWordValueExpression(value.val.u16val);
                break;
            case s32:
            case u32:
                roseExpression = new SgAsmDoubleWordValueExpression(value.val.u32val);
                break;
            case s48:
            case u48:
                // This only happens with far calls. ROSE appears to be set up to
                // expect a 32-bit absolute destination (or doesn't handle far call at
                // all), so give it what it wants.
                roseExpression = new SgAsmDoubleWordValueExpression(value.val.u32val);
                break;
            case s64:
            case u64:
                roseExpression = new SgAsmQuadWordValueExpression(value.val.u64val);
                break;
            case sp_float:
                roseExpression = new SgAsmSingleFloatValueExpression(value.val.floatval);
                break;
            case dp_float:
                roseExpression = new SgAsmDoubleFloatValueExpression(value.val.dblval);
                break;
            default:
                roseExpression = NULL;
                assert(0);
                // error!
        }
    }
    m_stack.push_front(roseExpression);
}


void ExpressionConversionVisitor::visit(RegisterAST *regast) {
    // has no children
    SgAsmExpression* reg = archSpecificRegisterProc(regast, addr, size);
    if (reg == NULL) {
        roseExpression = NULL;
        return;
    }
    m_stack.push_front(reg);
    roseExpression = m_stack.front();
    return;
}

void ExpressionConversionVisitor::visit(Dereference *deref) {
    // get child
    assert(m_stack.size());
    SgAsmExpression *toderef = m_stack.front();
    m_stack.pop_front();
    if (toderef == NULL) {
        roseExpression = NULL;
        return;
    }
    SgAsmType *type;

    // TODO fix some mismatched types?
    // pick correct type
    if(arch == Arch_aarch64 || arch == Arch_ppc32 || arch == Arch_ppc64 || arch == Arch_amdgpu_vega) {
        bool isSigned = false;
        switch (deref->eval().type) {
            case s8:
                isSigned = true;
            case u8:
                type = new SgAsmIntegerType(ByteOrder::ORDER_LSB, 8, isSigned);
                break;
            case s16:
                isSigned = true;
            case u16:
                type = new SgAsmIntegerType(ByteOrder::ORDER_LSB, 16, isSigned);
                break;
            case s32:
                isSigned = true;
            case u32:
                type = new SgAsmIntegerType(ByteOrder::ORDER_LSB, 32, isSigned);
                break;
            case s64:
                isSigned = true;
            case u64:
                type = new SgAsmIntegerType(ByteOrder::ORDER_LSB, 64, isSigned);
                break;
            case sp_float:
                type = new SgAsmFloatType(ByteOrder::ORDER_LSB, 64,
                        SgAsmFloatType::BitRange::baseSize(0, 52),  // significand
                        SgAsmFloatType::BitRange::baseSize(52, 11), // exponent
                        63,                                         // sign bit
                        1023,                                       // exponent bias
                        SgAsmFloatType::NORMALIZED_SIGNIFICAND | SgAsmFloatType::GRADUAL_UNDERFLOW);
                break;
            case dp_float:
                type = new SgAsmFloatType(ByteOrder::ORDER_LSB, 80,
                        SgAsmFloatType::BitRange::baseSize(0, 64),  // significand
                        SgAsmFloatType::BitRange::baseSize(64, 15), // exponent
                        79,                                         // sign bit
                        16383,                                      // exponent bias
                        SgAsmFloatType::NORMALIZED_SIGNIFICAND | SgAsmFloatType::GRADUAL_UNDERFLOW);
                break;
            default:
                type = NULL;
        }
    } else {
        switch (deref->eval().type) {
            case s8:
            case u8:
                type = new SgAsmTypeByte();
                break;
            case s16:
            case u16:
                type = new SgAsmTypeWord();
                break;
            case s32:
            case u32:
                type = new SgAsmTypeDoubleWord();
                break;
            case s64:
            case u64:
                type = new SgAsmTypeQuadWord();
                break;
            case sp_float:
                type = new SgAsmTypeSingleFloat();
                break;
            case dp_float:
                type = new SgAsmTypeDoubleFloat();
                break;
            default:
                type = NULL;
                // error
        }
    }

    SgAsmExpression *segReg = makeSegRegExpr();
    SgAsmMemoryReferenceExpression *result = new SgAsmMemoryReferenceExpression(toderef, segReg);
    result->set_type(type);
    roseExpression = result;
}

SgAsmExpression *ExpressionConversionVisitor::archSpecificRegisterProc(InstructionAPI::RegisterAST *regast,
        uint64_t addr, uint64_t size) {
    
    MachRegister machReg = regast->getID();

    //std::cout << " in " << __func__ << " idx = " << machReg << " arch = " << arch   << std::endl;
    switch (arch) {
        case Arch_x86:
        case Arch_x86_64: {
                              int regClass;
                              int regNum;
                              int regPos;

                              MachRegister machReg = regast->getID();
                              if (machReg.isPC()) {
                                  // ideally this would be symbolic
                                  // When ip is read, the value read is not the address of the current instruction,
                                  // but the address of the next instruction.
                                  SgAsmExpression *constAddrExpr;
                                  if (arch == Arch_x86)
                                      constAddrExpr = new SgAsmDoubleWordValueExpression(addr + size);
                                  else
                                      constAddrExpr = new SgAsmQuadWordValueExpression(addr + size);

                                  return constAddrExpr;
                              }
                              machReg.getROSERegister(regClass, regNum, regPos);
                              if (regClass < 0) return NULL;
                              return new SgAsmx86RegisterReferenceExpression((X86RegisterClass) regClass,
                                      regNum,
                                      (X86PositionInRegister) regPos);
                          }
        case Arch_ppc32: 
        case Arch_ppc64: {
                             int regClass;
                             int regNum;
                             int regPos;
                             SgAsmDirectRegisterExpression *dre;
                             machReg.getROSERegister(regClass, regNum, regPos);
                             if (regClass < 0) return NULL;
                             if (regClass == powerpc_regclass_cr) {
                                 // ROSE treats CR as one register, so regNum is always 0. 
                                 // CR0 to CR7 are 8 subfields within CR.
                                 // CR0 has register offset 0
                                 // CR1 has register offset 4
                                 dre = new SgAsmDirectRegisterExpression(RegisterDescriptor(regClass, regNum, regPos * 4, 4));
                                 dre->set_type(new SgAsmIntegerType(ByteOrder::ORDER_LSB, 4, false));
                             } else {
                                 dre = new SgAsmDirectRegisterExpression(RegisterDescriptor(regClass, regNum, regPos, machReg.size() * 8));
                                 dre->set_type(new SgAsmIntegerType(ByteOrder::ORDER_LSB, machReg.size() * 8, false));
                             }
                             return dre;
                         }
        case Arch_aarch64: {
                               int regClass;
                               int regNum;
                               int regPos;

                               machReg.getROSERegister(regClass, regNum, regPos);
                               if (regClass < 0) return NULL;
                               SgAsmDirectRegisterExpression *dre = new SgAsmDirectRegisterExpression(RegisterDescriptor(regClass, regNum, regPos, machReg.size() * 8));
                               dre->set_type(new SgAsmIntegerType(ByteOrder::ORDER_LSB, machReg.size() * 8, false));
                               return dre;
                           }
        case Arch_amdgpu_vega: {
                               int regClass;
                               int regNum;
                               int regPos;

                               machReg.getROSERegister(regClass, regNum, regPos);
                               if (regClass < 0) return NULL;
                               //std::cout << " after get rose register, regClass = " << regClass << " regNum = " << regNum    << std::endl;
                               // TODO : it is not clear how regsize adn such should be set, for now we just follow aarch64's implementation
                               SgAsmDirectRegisterExpression *dre = new SgAsmDirectRegisterExpression(RegisterDescriptor(regClass, regNum, regPos, machReg.size() * 8));
                               dre->set_type(new SgAsmIntegerType(ByteOrder::ORDER_LSB, machReg.size() * 8, false));
                               return dre;
                           }
 
        default:
                          return NULL;
    }
}

SgAsmExpression *ExpressionConversionVisitor::makeSegRegExpr() {
    if (arch == Arch_x86 || arch == Arch_x86_64) {
        return new SgAsmx86RegisterReferenceExpression(x86_regclass_segment,
                x86_segreg_none, x86_regpos_all);
    }
    else {
        return NULL;
    }
}

/////////////// Visitor class /////////////////

void ExpressionConversionVisitor::visit(BinaryFunction *binfunc) {
    assert(m_stack.size() >= 2);
    SgAsmExpression *rhs = m_stack.front();
    m_stack.pop_front();
    SgAsmExpression *lhs = m_stack.front();
    m_stack.pop_front();
    // If the RHS didn't convert, that means it should disappear
    // And we are just left with the LHS
    if (!rhs && !lhs) {
        roseExpression = NULL;
    }
    else if (!rhs) {
        roseExpression = lhs;
    }
    else if (!lhs) {
        roseExpression = rhs;
    }
    else {
        // now build either add or multiply
        if (binfunc->isAdd())
            roseExpression = new SgAsmBinaryAdd(lhs, rhs);
        else if (binfunc->isMultiply())
            roseExpression = new SgAsmBinaryMultiply(lhs, rhs);
        else if (binfunc->isLeftShift())
            roseExpression = new SgAsmBinaryLsl(lhs, rhs);
        else if (binfunc->isRightArithmeticShift())
            roseExpression = new SgAsmBinaryAsr(lhs, rhs);
        else if (binfunc->isRightLogicalShift())
            roseExpression = new SgAsmBinaryLsr(lhs, rhs);
        else if (binfunc->isRightRotate())
            roseExpression = new SgAsmBinaryRor(lhs, rhs);
        else roseExpression = NULL; // error
    }
    m_stack.push_front(roseExpression);
}
