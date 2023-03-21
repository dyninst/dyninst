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

#if !defined(IAPI_VISITOR_H)
#define IAPI_VISITOR_H
namespace Dyninst
{
namespace InstructionAPI
{
    class BinaryFunction;
    class Immediate;
    class RegisterAST;
    class Dereference;
    
    class Visitor
    {
        /// A %Visitor performs postfix-order traversal of the AST represented by
        /// an %Expression.  The %Visitor class specifies the interface from which
        /// users may derive %Visitors that perform specific tasks.
        ///
        /// The %visit method must be overridden for each type of %Expression node
        /// (%BinaryFunction, %Immediate, %RegisterAST, and %Dereference).  Any state that
        /// the %Visitor needs to preserve between nodes must be kept within the class.
        /// %Visitors are invoked by calling %Expression::apply(%Visitor* v); the %visit method
        /// should not be invoked by user code ordinarily.
       
        public:
            virtual ~Visitor() = default;
            Visitor& operator=(const Visitor&) = default;
            virtual void visit(BinaryFunction* b) = 0;
            virtual void visit(Immediate* i) = 0;
            virtual void visit(RegisterAST* r) = 0;
            virtual void visit(Dereference* d) = 0;
    };

    // class ATTVisitor : public Visitor
    // {
        // public:
            // std::string visitSection(void);
            // void getOffset(Immediate* imm);
            // void visitBase(Register* reg);
            // // void visitIndex(Immediate* imm);
            // void visitScale(Immediate* imm);
    // };

}
}

#endif //!defined(IAPI_VISITOR_H)
