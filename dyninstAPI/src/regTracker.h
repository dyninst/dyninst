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

#ifndef DYNINST_DYNINSTAPI_REGTRACKER_H
#define DYNINST_DYNINSTAPI_REGTRACKER_H

#include "dyn_register.h"

#include <unordered_map>

class AstNode;
class codeGen;

class regTracker_t {
public:
  class commonExpressionTracker {
  public:
    Dyninst::Register keptRegister;
    int keptLevel;
    commonExpressionTracker()
        : keptRegister(Dyninst::Null_Register), keptLevel(-1) {}
  };

  int condLevel;

  regTracker_t() : condLevel(0) {}

  std::unordered_map<AstNode *, commonExpressionTracker> tracker;

  void addKeptRegister(codeGen &gen, AstNode *n, Dyninst::Register reg);
  void removeKeptRegister(codeGen &gen, AstNode *n);
  Dyninst::Register hasKeptRegister(AstNode *n);
  bool stealKeptRegister(Dyninst::Register reg);

  void reset();

  void increaseConditionalLevel();
  void decreaseAndClean(codeGen &gen);
  void cleanKeptRegisters(int level);
  void debugPrint();
};

#endif
