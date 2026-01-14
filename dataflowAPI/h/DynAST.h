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

#if !defined(AST_H)
#define AST_H

#include <assert.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include "dyninst_visibility.h"

#include "AST.h"
#include "ASTVisitor.h"
#include "BottomAST.h"
#include "ConstantAST.h"
#include "RoseAST.h"
#include "VariableAST.h"

namespace Dyninst {

// We fully template the three types of nodes we have so that
// users can specify their own. This basically makes the AST
// a fully generic class. 
//
// TODO: do we want Variable and Constant to be different classes?
// I'm using the Absloc case as the basis here; EAX and '5' are
// very different things...
//
// Possible fourth template type: Type
// though I'm currently arguing that Type is an artifact of the
// Eval method you apply here. 
// ... and are Eval methods independent of Operation/Variable/Constant?
// I think they are...x

class ASTVisitor;  

 // For this to work, the ASTVisitor has to have a virtual
 // visit() method for every instantiation of an AST-typed
 // class. Yes, this means that if you add an AST class
 // somewhere else you have to come back and put it in here. 
 // Well, if you want to run a visitor over it, that is.

 // SymEval...
 namespace DataflowAPI {
 class BottomAST;
 class ConstantAST;
 class VariableAST;
 class RoseAST;
 }
 // Stack analysis...
 class StackAST;

 // InsnAPI...

 // Codegen...

}
#endif // AST_H

