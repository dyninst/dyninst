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

#include "dwarfResult.h"
#include "VariableLocation.h"
#include "dwarfFrameParser.h"
#include "ProcReader.h"
#include "dyntypes.h"
#include "registers/MachRegister.h"
#include "registers/abstract_regs.h"
#include "debug_common.h"
#include <iostream>
#include "debug_common.h"

using namespace Dyninst;
using namespace DwarfDyninst;
using namespace std;

#define CHECK_OPER(n) do { if (operands.size() < (n)) { error = true; return; } } while (0)

void SymbolicDwarfResult::pushReg(MachRegister reg) {
  dwarf_printf("\t\tPush %s\n", reg.name().c_str());
  
   if (var.stClass != storageUnset) { error = true; }
   var.stClass = storageReg;
   var.refClass = storageNoRef;
   var.frameOffset = 0;
   var.mr_reg = reg;
}

void SymbolicDwarfResult::readReg(MachRegister reg) {
  dwarf_printf("\t\t Read %s\n", reg.name().c_str());
  
   if (var.stClass != storageUnset) { error = true; }
   var.stClass = storageRegOffset;
   var.refClass = storageNoRef;
   // frameOffset will be set with an add operation
   //var.frameOffset = 0;
   var.mr_reg = reg;
}

void SymbolicDwarfResult::pushUnsignedVal(MachRegisterVal val) {
  dwarf_printf("\t\t Push 0x%lx\n", val);
  
   if (var.stClass == storageUnset) {
      // No register, so default to StorageAddr
      var.stClass = storageAddr;
   }
   operands.push(val);
}

void SymbolicDwarfResult::pushSignedVal(MachRegisterVal val) {
  dwarf_printf("\t\t Push 0x%lx\n", val);
  
   operands.push(val);
}

void SymbolicDwarfResult::pushOp(Operator op) {
   // This is "fill in as we see examples" code. 
   // Right now, the only use I know of is add. 
  dwarf_printf("Push op %d\n", op);
  
   switch (op) {
      case Add:
         CHECK_OPER(1);
         if (var.stClass == storageUnset) { error = true; }
         var.frameOffset += operands.top(); 
         operands.pop();
         break;
      default:
         error = true;
   }         
}

void SymbolicDwarfResult::pushOp(Operator op, long long u)
{
  dwarf_printf("Push op pair %d,%lld\n", op, u);
  
   switch(op) {
      case Add:
         var.frameOffset += u;
         break;
      default:
         error = true;
   }
}

void SymbolicDwarfResult::pushFrameBase() {
  dwarf_printf("Push frame base\n");
  
   readReg(FrameBase);
}

void SymbolicDwarfResult::pushCFA() {
  dwarf_printf("Push CFA\n");
  
   readReg(CFA);
}

VariableLocation &SymbolicDwarfResult::val() {
   if (!operands.empty()) {
      var.frameOffset += operands.top(); 
      operands.pop();
   }
   return var;
}

void ConcreteDwarfResult::pushReg(MachRegister) {
   // I don't believe this is legal
   error = true;
}

void ConcreteDwarfResult::readReg(MachRegister reg) {
   Dyninst::MachRegisterVal v;
   if (!reader->GetReg(reg, v)) error = true;
   push(v);
   dwarf_printf("readReg %s, got 0x%lx, queue size %lu\n",
                reg.name().c_str(), v, operands.size());
}

void ConcreteDwarfResult::pushUnsignedVal(MachRegisterVal v) {
   // Someday this will matter...
   push(v);
   dwarf_printf("pushUnsigned 0x%lx, queue size %lu\n", v, operands.size());
}

void ConcreteDwarfResult::pushSignedVal(MachRegisterVal v) {
   // Someday this will matter...
   push(v);
   dwarf_printf("pushSigned 0x%lx, queue size %lu\n", v, operands.size());
}


void ConcreteDwarfResult::pushOp(Operator op) {
   MachRegisterVal v;
   MachRegisterVal first;
   MachRegisterVal second;

   switch (op) {
      case Add:
         CHECK_OPER(2);
         v = peek(1) + peek(0);
         dwarf_printf("AddOp: 0x%lx + 0x%lx = 0x%lx\n", peek(1), peek(0), v);
         popRange(0, 1);
         push(v);
         break;
      case Sub:
         CHECK_OPER(2);
         v = peek(1) - peek(0);
         dwarf_printf("SubOp: 0x%lx + 0x%lx = 0x%lx\n", peek(1), peek(0), v);
         popRange(0, 1);
         push(v);
         break;
      case Mul:
         CHECK_OPER(2);
         v = peek(1) * peek(0);
         popRange(0, 1);
         push(v);
         break;
      case Div:
         CHECK_OPER(2);
         if (peek(0) == 0) { error = true; break; }
         v = peek(1) / peek(0);
         popRange(0, 1);
         push(v);
         break;
      case Mod:
         CHECK_OPER(2);
         if (peek(0) == 0) { error = true; break; }
         v = peek(1) % peek(0);
         popRange(0, 1);
         push(v);
         break;
      case And:
         CHECK_OPER(2);
         v = peek(1) & peek(0);
         popRange(0, 1);
         push(v);
         break;
      case Or:
         CHECK_OPER(2);
         v = peek(1) | peek(0);
         popRange(0, 1);
         push(v);
         break; 
      case Not:
         CHECK_OPER(1);
         v = ~peek(0);
         pop(0);
         push(v);
         break;
      case Xor:
         CHECK_OPER(2);
         v = peek(0) ^ peek(0);
         popRange(0, 1);
         push(v);
         break;
      case Abs:
         CHECK_OPER(1);
         v = std::abs((long) peek(0));
         pop(0);
         push(v);
         break;
      case Shl:
         CHECK_OPER(2);
         v = peek(1) << peek(0);
         popRange(0, 1);
         push(v);
         break;
      case Shr:
         CHECK_OPER(2);
         v = ((unsigned long) peek(1)) >> ((unsigned long) peek(0));
         popRange(0, 1);
         push(v);
         break;
      case ShrArith:
         CHECK_OPER(2);
         v = ((long) peek(1)) + ((long) peek(0));
         popRange(0, 1);
         push(v);
         break;
      case GE:
         CHECK_OPER(2);
         second = peek(1);
         first = peek(0);
         popRange(0, 1);
         push((second >= first) ? 1 : 0);
         break;
      case LE:
         CHECK_OPER(2);
         second = peek(1);
         first = peek(0);
         popRange(0, 1);
         push((second <= first) ? 1 : 0);
         break;
      case GT:
         CHECK_OPER(2);
         second = peek(1);
         first = peek(0);
         popRange(0, 1);
         push((second > first) ? 1 : 0);
         break;
      case LT:
         CHECK_OPER(2);
         second = peek(1);
         first = peek(0);
         popRange(0, 1);
         push((second < first) ? 1 : 0);
         break;
      case Eq:
         CHECK_OPER(2);
         second = peek(1);
         first = peek(0);
         popRange(0, 1);
         push((second == first) ? 1 : 0);
         break;
      case Neq:
         CHECK_OPER(2);
         second = peek(1);
         first = peek(0);
         popRange(0, 1);
         push((second != first) ? 1 : 0);
         break;
      case Deref:         
      case Pick:
      case Drop:
      default:
         // 2 argument
         error = true;
         break;
   }
   dwarf_printf("\t After queue manipulation, size %lu\n", operands.size());
}

void ConcreteDwarfResult::pushOp(Operator op, long long ref) {
   switch (op) {
      case Add: 
         pushUnsignedVal(ref);
         pushOp(Add);
         break;
      case Deref: {
         CHECK_OPER(1);
         MachRegisterVal v;
         switch(ref) {
            case 1: {
               unsigned char c;
               if (!reader->ReadMem(peek(0), &c, sizeof(c))) error = true;
               v = c;
               break;
            }
            case 2: {
               unsigned short s;
               if (!reader->ReadMem(peek(0), &s, sizeof(s))) error = true;
               v = s;
               break;
            }
            case 4: {
               uint32_t u;
               if (!reader->ReadMem(peek(0), &u, sizeof(u))) error = true;
               v = u;
               break;
            }
            case 8: {
               uint64_t u;
               if (!reader->ReadMem(peek(0), &u, sizeof(u))) error = true;
               v = u;
               dwarf_printf("Memory read from 0x%lx: 0x%lx\n", peek(0), u);
               break;
            }
            default:
               error = true;
               v = 0;
               break;
         }
         push(v);
         break;
      }
      case Pick:
         assert(ref>=0);
         CHECK_OPER((unsigned long long) ref);
         push(peek(ref));
         break;
      case Drop:
         assert(ref>=0);
         CHECK_OPER((unsigned long long) ref);
         pop(ref);
         break;
      default:
         error = true;
         break;
   }
}

void ConcreteDwarfResult::pushFrameBase() {
   error = true;
}

void ConcreteDwarfResult::pushCFA() {
   DwarfFrameParser::Ptr cfaParser = DwarfFrameParser::create(dbg, dbg_eh_frame, arch);
   if(!cfaParser) return; 
   MachRegisterVal cfa;
   FrameErrors_t err;
   dwarf_printf("Getting CFA value...\n");
   if (!cfaParser->getRegValueAtFrame(pc, CFA, cfa, reader, err))
       error = true;
   dwarf_printf("Got CFA value 0x%lx\n", cfa);
   pushUnsignedVal(cfa);
}

MachRegisterVal ConcreteDwarfResult::peek(int index) {
   dwarf_printf("peek @ %d, returning index %lu of size %lu\n",
                index, operands.size() - (index + 1), operands.size());
   return operands[operands.size() - (index + 1)];
}

void ConcreteDwarfResult::pop(int num) {
   dwarf_printf("pop @ %d, deleting index %lu of size %lu\n",
                num, operands.size() - (num + 1), operands.size());
   operands.erase(operands.begin() + (operands.size() - (num + 1)));
}

void ConcreteDwarfResult::popRange(int start, int end) {
   dwarf_printf("popRange %d .. %d of %lu\n", start, end, operands.size());
   std::vector<MachRegisterVal>::iterator b, e;
   if (start > end) {
      b = operands.begin() + (operands.size() - (start + 1));
      e = operands.begin() + (operands.size() - end);
   }
   else {
      b = operands.begin() + (operands.size() - (end + 1));
      e = operands.begin() + (operands.size() - start);
   }
   operands.erase(b, e);
   dwarf_printf("\t After popRange, size %lu\n", operands.size());
}

void ConcreteDwarfResult::push(MachRegisterVal v) {
   operands.push_back(v);
}

bool ConcreteDwarfResult::eval(MachRegisterVal &v) {
   if (err()) return false;
   v = val();
   return true;
}

MachRegisterVal ConcreteDwarfResult::val() {
   dwarf_printf("Eval: returning top value 0x%lx, stack size %lu\n",
                operands.back(), operands.size());
   return operands.back();
}
