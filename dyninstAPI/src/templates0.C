/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: templates0.C,v 1.55 2005/08/25 22:46:00 bernat Exp $
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

#if defined(__XLC__) || defined(__xlC__)
#pragma implementation("Symbol.h")
#else
#pragma implementation "Symbol.h"
#endif
#include "common/h/Symbol.h"

#include "common/h/String.h"

#if 0

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/Object.h"
#endif // if 0

#if defined(rs6000_ibm_aix4_1)
#include "LineInformation.h"
template class pdvector< IncludeFileInfo >;
#endif

// #include "dyninstAPI/src/dynamiclinking.h"

#include <set>

template class  pdvector<float>;
template class  pdvector<int>;
template class  pdvector<bool>;
template class  pdvector<pdstring>;
template class  pdvector<pdvector<pdstring> >;
template class  pdvector<unsigned>;
template class  pdvector<unsigned long>;
template class  pdvector<long>;
template class  pdvector<double>;

class dyn_thread;
template class  pdvector<dyn_thread *>;
template class  pdvector< pdvector<dyn_thread *> >;

class AstNode;
//template class  pdvector<AstNode>;
template class  pdvector<AstNode *>;

// Included above
template class  pdvector<Symbol>;
template class  pdvector<Symbol*>;

#include "frame.h"
template class  pdvector<Frame>;
template class  pdvector<Frame*>;
template class  pdvector<pdvector<Frame> >;
template class  pdvector<pdvector<Frame*> >;

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

class BPatch_basicBlock;
template class  pdvector<BPatch_basicBlock*>;

class instruction;
template class  pdvector<instruction *>;

class baseTramp;
class baseTrampInstance;
template class  pdvector<baseTramp *>;
template class  pdvector<baseTrampInstance *>;

#include "arch.h"
template class  pdvector<instruction>;

#include "symtab.h"
template class  pdvector< ExceptionBlock >;

class codeRange;
template class  pdvector<codeRange *>;

class module;
template class  pdvector<module *>;

class pdmodule;
template class  pdvector<pdmodule *>;

class int_function;
template class  pdvector<int_function*>;
class int_variable;
template class  pdvector<int_variable*>;
class int_basicBlock;
template class  pdvector<int_basicBlock *>;
class bblInstance;
template class  pdvector<bblInstance *>;

class BPatch_basicBlockLoop;
template class  pdvector<BPatch_basicBlockLoop*>;

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
template class  pdvector<const miniTramp *>;

class generatedCodeObject;
template class  pdvector<generatedCodeObject *>;

class image_func;
class image_basicBlock;
class image_variable;
template class  pdvector<image_func *>;
template class  pdvector<image_basicBlock *>;
template class  pdvector<image_variable *>;

#include "Object.h"
template class  pdvector<relocationEntry>;

class sharedLibHook;
template class  pdvector<sharedLibHook *>;

#include "imageUpdate.h"
template class pdvector<imageUpdate*>;//ccw 28 oct 2001
template class pdvector<dataUpdate*> ;//ccw 26 nov 2001

class instMapping;
template class pdvector<instMapping *>;

class mapped_module;
template class pdvector<mapped_module *>;

#include "mapped_object.h"
template class pdvector<mapped_object::foundHeapDesc>;

#include "InstrucIter.h"
template class pdvector<InstrucIter::previous>;

template class pdvector<fileDescriptor>;

template class std::set<instPoint *>;

#if defined(arch_power)
class fileOpener;
template class pdvector<fileOpener *>;
#endif

class funcMod;
template class pdvector<funcMod *>;
