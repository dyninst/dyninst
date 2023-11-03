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
#include "DynPointMaker.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/instPoint.h"
#include "instructionAPI/h/Instruction.h"

Point *DynPointMaker::mkFuncPoint(Point::Type t, PatchMgrPtr m, PatchFunction *f) {
   return new instPoint(t, m, SCAST_FI(f));
}

Point *DynPointMaker::mkFuncSitePoint(Point::Type t, PatchMgrPtr m, PatchFunction *f, PatchBlock *b) {
   return new instPoint(t, m, SCAST_FI(f), SCAST_BI(b));
}

Point *DynPointMaker::mkBlockPoint(Point::Type t, PatchMgrPtr m, PatchBlock *b, PatchFunction *context) {
   return new instPoint(t, m, SCAST_BI(b), SCAST_FI(context));
}

Point *DynPointMaker::mkInsnPoint(Point::Type t, PatchMgrPtr m, PatchBlock *b, Address a,
                                  InstructionAPI::Instruction i, PatchFunction *context) {
   return new instPoint(t, m, SCAST_BI(b), a, i, SCAST_FI(context));
}

Point *DynPointMaker::mkEdgePoint(Point::Type t, PatchMgrPtr m, PatchEdge *e, PatchFunction *f) {
   return new instPoint(t, m, SCAST_EI(e), SCAST_FI(f));
}
