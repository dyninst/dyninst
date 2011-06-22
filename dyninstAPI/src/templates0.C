/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

// $Id: templates0.C,v 1.66 2008/06/19 19:53:47 legendre Exp $
// Generate all the templates in one file.

/*
 * This file (and the other templatesXXX.C files) exist for a single purpose:
 * to explicitly instantiate code for all of the template classes we use.
 * Although the C++ standard dictates that compilers should automatically
 * instantiate all templates (and leaves the details to the implementation),
 * g++ 2.7.2, which we currently use, doesn't do that correctly.  So instead,
 * we use a special compiler switch (-fno-implicit-templates or 
 * -fexternal-templates) which tells g++ not to try and automatically
 * instantiate any templates.  We manually instantiate the templates in this
 * and the other templatesXXX.C files.  If you are porting Paradyn, and are
 * using a compiler that correctly and automatically instantiates its 
 * templates, then you don't need to use any of the templatesXXX.C files (so
 * remove their entries from the appropriate make.module.tmpl file).
 *
 */

#if defined(__XLC__) || defined(__xlC__)
#pragma implementation("Vector.h")
#else
#pragma implementation "Vector.h"
#endif
#include "common/h/Vector.h"

#include <string>
#include "dyninstAPI/src/inst.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
template class pdvector<callWhen>;

#include <set>

template class  pdvector<int>;
template class  pdvector<bool>;
template class  pdvector<std::string>;
template class  pdvector<pdvector<std::string> >;
template class  pdvector<unsigned>;
template class  pdvector<long>;

class dyn_thread;
template class  pdvector<dyn_thread *>;

#include "ast.h"
template class  pdvector<AstNodePtr>;

#include "frame.h"
template class  pdvector<Frame>;
template class  pdvector<pdvector<Frame> >;

class heapItem;
template class  pdvector<heapItem*>;

class image;
template class  pdvector<image*>;

class image_instPoint;
template class  pdvector<image_instPoint *>;

class instPoint;
template class  pdvector<instPoint *>;
template class  pdvector<const instPoint *>;

class instPointInstance;
template class  pdvector<instPointInstance *>;


class baseTrampInstance;
template class  pdvector<baseTrampInstance *>;

#include "common/h/arch.h"
template class  pdvector<instruction>;
template class  pdvector<instruction *>;

#include "symtab.h"
template class  pdvector< ExceptionBlock >;

class codeRange;
template class  pdvector<codeRange *>;

class pdmodule;
template class  pdvector<pdmodule *>;

class func_instance;
template class  pdvector<func_instance*>;
class int_variable;
template class  pdvector<int_variable*>;
class int_basicBlock;
template class  pdvector<int_basicBlock *>;
class bblInstance;
template class  pdvector<bblInstance *>;


class process;
template class  pdvector<process*>;

#include "infHeap.h"
template class  pdvector<disabledItem>;
template class  pdvector<addrVecType>;
template class pdvector<heapDescriptor>;

class miniTramp;
class miniTrampInstance;
template class  pdvector<miniTramp *>;
template class  pdvector<miniTrampInstance *>;

class generatedCodeObject;
template class  pdvector<generatedCodeObject *>;

class parse_func;
class parse_block;
class image_variable;
template class  pdvector<parse_func *>;
template class  pdvector<pdvector<parse_func *> *>;
template class  pdvector<parse_block *>;
template class  pdvector<image_variable *>;

#include "symtabAPI/h/Symtab.h"
template class  pdvector<relocationEntry>;

class sharedLibHook;
template class  pdvector<sharedLibHook *>;


class instMapping;
template class pdvector<instMapping *>;

class mapped_module;
template class pdvector<mapped_module *>;


template class std::vector<Address>;

template class pdvector<fileDescriptor>;

template class std::set<instPoint *>;

#if defined(arch_power)
class fileOpener;
template class pdvector<fileOpener *>;
#endif

class funcMod;
template class pdvector<funcMod *>;

