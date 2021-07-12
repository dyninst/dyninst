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
/* PatchAPI uses the ParseAPI CFG to construct its own CFG, which 
   means that we need to be notified of any changes in the underlying
   CFG. These changes can come from a number of sources, including
   the PatchAPI modification interface or a self-modifying binary. 

   The PatchAPI modification chain looks like the following:
   1) User requests PatchAPI to modify the CFG;
   2) PatchAPI makes the corresponding request to ParseAPI;
   3) ParseAPI modifies its CFG, triggering "modified CFG" callbacks;
   4) PatchAPI hooks these callbacks and updates its structures.
   
   This is much easier than PatchAPI modifying them a priori, because
   it allows for self-modifying code (which skips step 2) to work
   with the exact same chain of events.
*/

#if !defined(_PATCHAPI_CALLBACK_H_)
#define _PATCHAPI_CALLBACK_H_

#include "parseAPI/h/ParseCallback.h"

namespace Dyninst {
namespace PatchAPI {

class PatchObject;

class PatchParseCallback : public ParseAPI::ParseCallback {
  public:
  PatchParseCallback(PatchObject *obj) : ParseAPI::ParseCallback(), _obj(obj) {}
   ~PatchParseCallback() {}
   
  protected:
   // Callbacks we want to know about: CFG mangling
   virtual void split_block_cb(ParseAPI::Block *, ParseAPI::Block *);
   virtual void destroy_cb(ParseAPI::Block *);
   virtual void destroy_cb(ParseAPI::Edge *);
   virtual void destroy_cb(ParseAPI::Function *);

   virtual void modify_edge_cb(ParseAPI::Edge *, ParseAPI::Block *, edge_type_t);
   
   virtual void remove_edge_cb(ParseAPI::Block *, ParseAPI::Edge *, edge_type_t);
   virtual void add_edge_cb(ParseAPI::Block *, ParseAPI::Edge *, edge_type_t);
   
   virtual void remove_block_cb(ParseAPI::Function *, ParseAPI::Block *);
   virtual void add_block_cb(ParseAPI::Function *, ParseAPI::Block *);

  // returns the load address of the code object containing an absolute address
  virtual bool absAddr(Address absolute, 
                       Address & loadAddr, 
                       ParseAPI::CodeObject *& containerObject);

  private:
   PatchObject *_obj;
};

}
}

#endif // _PATCHAPI_CALLBACK_H_
