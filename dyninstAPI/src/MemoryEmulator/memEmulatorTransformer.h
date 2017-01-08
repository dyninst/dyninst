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

#if !defined(cap_mem_emulation)
#error
#endif

#if !defined(_R_T_EMULATE_MEMORY_H_)
#define _R_T_EMULATE_MEMORY_H_

#include "dyninstAPI/src/Relocation/Transformers/Transformer.h"
#include "dyninstAPI/src/Relocation/Transformers/Modification.h"
#include "dataflowAPI/h/Absloc.h" // MemEmulator analysis
#include "dataflowAPI/h/AbslocInterface.h" // And more of the same

class func_instance;

namespace Dyninst {
namespace Relocation {

class InsnWidget;

class MemEmulatorTransformer : public Transformer {
   typedef boost::shared_ptr<InsnWidget> InsnWidgetPtr;

 public:
  typedef std::map<Register, Widget::RelocBlockPtr> TranslatorMap;

  virtual bool process(RelocBlock *, RelocGraph *);

 MemEmulatorTransformer() :
  aConverter(false, false) {};

  virtual ~MemEmulatorTransformer() {};

 private:

  WidgetPtr createReplacement(InsnWidgetPtr reloc,
			       func_instance *func, block_instance *);

  bool canRewriteMemInsn(InsnWidgetPtr reloc,
			 func_instance *func);

  bool isSensitive(InsnWidgetPtr reloc, 
		   func_instance *func,
		   block_instance *block);

  void createTranslator(Register r);

  bool override(InsnWidgetPtr reloc);

  TranslatorMap translators_;

  AssignmentConverter aConverter;

};
};
};

#endif
