/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

// $Id: mapped_module.h,v 1.14 2008/08/01 17:55:13 roundy Exp $

#if !defined(mapped_module_h)
#define mapped_module_h

class mapped_module;
class mapped_object;

class int_function;
class int_variable;

class parse_image;
class pdmodule;
class image;

#include <string>
#include "common/h/Types.h"
#include "dyninstAPI/src/symtab.h"
#include "symtabAPI/h/Symtab.h"

#define CHECK_ALL_CALL_POINTS  // paradyn might need it


// pdmodule equivalent The internals tend to use images, while the
// BPatch layer uses modules. On the other hand, "module" means
// "compilation unit for the a.out, or the entire image for a
// library". At some point this will need to be fixed, which will be a
// major pain.

class mapped_module {
   public:
      static mapped_module *createMappedModule(mapped_object *obj,
            pdmodule *pdmod);

      mapped_object *obj() const;
      pdmodule *pmod() const;

      const string &fileName() const;
      const string &fullName() const;

      AddressSpace *proc() const;

      // A lot of stuff shared with the internal module
      // Were we compiled with the native compiler?
      bool isNativeCompiler() const;

      SymtabAPI::supportedLanguages language() const;

      const pdvector<int_function *> &getAllFunctions();
      const pdvector<int_variable *> &getAllVariables();

      bool findFuncVectorByPretty(const std::string &funcname,
            pdvector<int_function *> &funcs);

      // Yeah, we can have multiple mangled matches -- for libraries there
      // is a single module. Even if we went multiple, we might not have
      // module information, and so we can get collisions.
      bool findFuncVectorByMangled(const std::string &funcname,
            pdvector<int_function *> &funcs);

    bool findFuncsByAddr(const Address addr, std::set<int_function *> &funcs);
    bool findBlocksByAddr(const Address addr, std::set<int_block *> &blocks);


      void dumpMangled(std::string prefix) const;

      /////////////////////////////////////////////////////
      // Line information
      /////////////////////////////////////////////////////
      // Line info is something we _definitely_ don't want multiple copies
      // of. So instead we provide pass-through functions that handle
      // things like converting absolute addresses (external) into offsets
      // (internal).  Its all in SymtabAPI now

      std::string processDirectories(const std::string &fn) const;

      // Given a line in the module, get the set of addresses that it maps
      // to. Calls the internal getAddrFromLine and then adds the base
      // address to the returned list of offsets.
      bool getAddrFromLine(unsigned lineNum,
            pdvector<Address> &addresses,
            bool exactMatch);

      void addFunction(int_function *func);
      void addVariable(int_variable *var);
      int_variable* createVariable(std::string name, Address offset, int size);
      
      void removeFunction(int_function *func);

      static bool truncateLineFilenames;
      unsigned int getFuncVectorSize() { return everyUniqueFunction.size(); }

   private:

      pdmodule *internal_mod_;
      mapped_object *obj_;

      mapped_module();
      mapped_module(mapped_object *obj,
            pdmodule *pdmod);

      pdvector<int_function *> everyUniqueFunction;
      pdvector<int_variable *> everyUniqueVariable;
};

#endif
