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

namespace Dyninst
{
  namespace InstructionAPI
  {
    
    /*! \mainpage
     * 
     * When analyzing and modifying binary code, it is necessary to translate
     * between raw binary instructions and an abstract form that describes the semantics
     * of the instructions.
     * As a part of the %Dyninst project, we have developed the
     * %Instruction API, an API and library for decoding and representing
     * machine instructions in a platform-independent manner. The
     * %Instruction API includes methods for decoding machine language,
     * convenient abstractions for its analysis, and methods to produce disassembly from those
     * abstractions. The current implementation supports the IA32,
     * AMD-64, POWER, and PowerPC instruction sets.  The %Instruction API
     * has the following basic capabilities:
     * - Decoding: interpreting a sequence of bytes as a machine instruction in a given machine language.
     * - Abstract representation: representing the behavior of that instruction as an abstract syntax tree.
     * - Disassembly: translating an abstract representation of a machine instruction into a string 
     * representation of the corresponding assembly language instruction.
     *
     * Our goal in designing the %Instruction API is to provide a representation of machine
     * instructions that can be manipulated by higher-level algorithms with minimal knowledge
     * of platform-specific details.  In addition, users who need platform-specific information
     * should be able to access it.  To do so, we provide an interface that disassembles a machine
     * instruction, extracts an operation and its operands, converts the operands to abstract syntax trees,
     * and presents this to the user.  A user of the %Instruction API can work at a level of abstraction slightly
     * higher than assembly language,
     * rather than working directly with machine language.  Additionally, by converting the operands to abstract
     * syntax trees, we make it possible to analyze the operands in a uniform manner, regardless of the
     * complexity involved in the operand's actual computation.
     * \htmlonly
<h2><a class="anchor" name="abstractions">
Abstractions</a></h2>
<ul>
<li><a class="el" href="group__instruction.html">Instruction</a></li>
<li><a class="el" href="group__instructiondecoder.html">Instruction Decoder</a></li>
<li><a class="el" href="group__instructionASTModule.html">Instruction AST hierarchy</a> </li>
</ul>
<h2><a class="anchor" name="Examples">
Examples</a></h2>
<ul>
<li><a class="el" href="BasicCfgExample.html">Basic Control Flow Graph Construction</a> </li>
</ul>
The PDF version of this manual may be found here: <a class="el" href="../latex/refman.pdf">PDF Manual</a>
\endhtmlonly
     */
  };
};




