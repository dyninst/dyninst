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

#if !defined(patch_h)
#define patch_h

#include <string>
#include "dyntypes.h"

class codeGen;

#define SIZE_16BIT 2
#define SIZE_32BIT 4
#define SIZE_64BIT 8

class patchTarget {
 public:
   virtual Dyninst::Address get_address() const = 0;
   virtual unsigned get_size() const = 0;
   virtual std::string get_name() const;
   patchTarget() = default;
   patchTarget(const patchTarget&) = default;
   virtual ~patchTarget() = default;
};

class toAddressPatch : public patchTarget {
 private:
   Dyninst::Address addr;
 public:
   toAddressPatch(Dyninst::Address a) : addr(a) {}
   virtual ~toAddressPatch();

   virtual Dyninst::Address get_address() const;
   virtual unsigned get_size() const;
   void set_address(Dyninst::Address a);
};

class relocPatch {
public:
   enum class patch_type_t {
      abs,         //Patch the absolute address of the source into dest
      pcrel,       //Patch a PC relative address from codeGen start + offset
      abs_lo,      //Patch lower half of source's bytes into dest
      abs_hi,      //Patch upper half of source's bytes into dest
      abs_quad1,   //Patch the first quarter of source's bytes into dest
      abs_quad2,   //Patch the second quarter of source's bytes into dest
      abs_quad3,   //Patch the third quarter of source's bytes into dest
      abs_quad4    //Patch the forth quarter of source's bytes into dest
   };


   relocPatch(unsigned d, patchTarget *s, relocPatch::patch_type_t ptype, 
              codeGen *gen, Dyninst::Offset off, unsigned size);

   void applyPatch();
   bool isApplied();
   void setTarget(patchTarget *s) { source_ = s; }

 private:
   unsigned dest_;
   patchTarget *source_;
   unsigned size_;
   patch_type_t ptype_;
   codeGen *gen_;
   Dyninst::Offset offset_;
   bool applied_;
};

class ifTargetPatch : public patchTarget
{
 private:
   signed int targetOffset;
 public:
   ifTargetPatch(signed int o) { targetOffset = o; }
   virtual Dyninst::Address get_address() const { return (Dyninst::Address) targetOffset; }
   virtual unsigned get_size() const { return 0; }
   virtual std::string get_name() const { return std::string("ifTarget"); }
   virtual ~ifTargetPatch() { }
};


#endif
