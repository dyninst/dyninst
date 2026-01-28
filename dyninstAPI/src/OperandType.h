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

#ifndef DYNINST_DYNINSTAPI_OPERANDTYPE_H
#define DYNINST_DYNINSTAPI_OPERANDTYPE_H

#include <string>

enum class operandType {
  Constant,
  ConstantString,
  DataIndir,
  Param,
  ParamAtCall,
  ParamAtEntry,
  ReturnVal,
  ReturnAddr, // address of a return instruction
  DataAddr,   // Used to represent a variable in memory
  FrameAddr,  // Calculate FP
  RegOffset,  // Calculate *reg + offset; oValue is reg, loperand->oValue is offset.
  // PreviousStackFrameDataReg,
  // RegValue, // A possibly spilled, possibly saved register.
  //  Both the above are now: origRegister
  origRegister,
  variableAddr,
  variableValue,
  undefOperandType,
  // Specific to AMDGPU. This represents an address in the form of (PlaceholderReg + offset).
  // Codegen may assing the same or a different register in different contexts.
  // Offset must be a constant.
  AddressAsPlaceholderRegAndOffset
};

// clang-format off
inline std::string format_operand(operandType type) {
  switch(type) {
    case operandType::Constant: return "Constant";
    case operandType::ConstantString: return "ConstantString";
    case operandType::DataIndir: return "DataIndir";
    case operandType::Param: return "Param";
    case operandType::ParamAtCall: return "ParamAtCall";
    case operandType::ParamAtEntry: return "ParamAtEntry";
    case operandType::ReturnVal: return "ReturnVal";
    case operandType::ReturnAddr: return "ReturnAddr";
    case operandType::DataAddr: return "DataAddr";
    case operandType::FrameAddr: return "FrameAddr";
    case operandType::RegOffset: return "RegOffset";
    case operandType::origRegister: return "OrigRegister";
    case operandType::variableAddr: return "variableAddr";
    case operandType::variableValue: return "variableValue";
    case operandType::undefOperandType: return "undefOperandType";
    case operandType::AddressAsPlaceholderRegAndOffset: return "AddressAsPlaceholderRegAndOffset";
  }
  return "UnknownOperand";
}
// clang-format on

#endif
