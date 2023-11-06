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

// $Id: mapped_module.h,v 1.14 2008/08/01 17:55:13 roundy Exp $

#if !defined(mapped_module_h)
#define mapped_module_h

class mapped_module;
class mapped_object;

class func_instance;
class int_variable;

class parse_image;
class pdmodule;
class image;

#include <string>
#include <set>
#include <vector>
#include "dyninstAPI/src/image.h"
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

      AddressSpace *proc() const;

      // A lot of stuff shared with the internal module

      SymtabAPI::supportedLanguages language() const;

      const std::vector<func_instance *> &getAllFunctions();
      const std::vector<int_variable *> &getAllVariables();

      bool findFuncVectorByPretty(const std::string &funcname,
            std::vector<func_instance *> &funcs);

      // Yeah, we can have multiple mangled matches -- for libraries there
      // is a single module. Even if we went multiple, we might not have
      // module information, and so we can get collisions.
      bool findFuncVectorByMangled(const std::string &funcname,
            std::vector<func_instance *> &funcs);

    bool findFuncsByAddr(const Address addr, std::set<func_instance *> &funcs);
    bool findBlocksByAddr(const Address addr, std::set<block_instance *> &blocks);
    void getAnalyzedCodePages(std::set<Address> & pages);


      void dumpMangled(std::string prefix) const;

      /////////////////////////////////////////////////////
      // Line information
      /////////////////////////////////////////////////////
      // Line info is something we _definitely_ don't want multiple copies
      // of. So instead we provide pass-through functions that handle
      // things like converting absolute addresses (external) into offsets
      // (internal).  Its all in SymtabAPI now

      std::string processDirectories(const std::string &fn) const;

      void addFunction(func_instance *func);
      void addVariable(int_variable *var);
      int_variable* createVariable(std::string name, Address offset, int size);
      
      void remove(func_instance *func);

      unsigned int getFuncVectorSize() { return everyUniqueFunction.size(); }

   private:

      pdmodule *internal_mod_;
      mapped_object *obj_;

      mapped_module();
      mapped_module(mapped_object *obj,
            pdmodule *pdmod);

      std::vector<func_instance *> everyUniqueFunction;
      std::vector<int_variable *> everyUniqueVariable;
};

#endif
