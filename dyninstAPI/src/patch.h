/*
 * Copyright (c) 1996-2008 Barton P. Miller
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

#if !defined(patch_h)
#define patch_h

#include <string>

class codeGen;

#define SIZE_16BIT 2
#define SIZE_32BIT 4
#define SIZE_64BIT 8

class patchTarget {
 public:
   virtual Address get_address() const = 0;
   virtual unsigned get_size() const = 0;
   virtual std::string get_name() const;
   virtual ~patchTarget();
};

class toAddressPatch : public patchTarget {
 private:
   Address addr;
 public:
   toAddressPatch(Address a) : addr(a) {};
   virtual ~toAddressPatch();

   virtual Address get_address() const;
   virtual unsigned get_size() const;
   void set_address(Address a);
};

class relocPatch {
public:
   typedef enum {
      abs,         //Patch the absolute address of the source into dest
      pcrel,       //Patch a PC relative address from codeGen start + offset
      abs_lo,      //Patch lower half of source's bytes into dest
      abs_hi,      //Patch upper half of source's bytes into dest
      abs_quad1,   //Patch the first quarter of source's bytes into dest
      abs_quad2,   //Patch the second quarter of source's bytes into dest
      abs_quad3,   //Patch the third quarter of source's bytes into dest
      abs_quad4    //Patch the forth quarter of source's bytes into dest
   } patch_type_t;


   relocPatch(void *d, patchTarget *s, relocPatch::patch_type_t ptype, 
              codeGen *gen, Dyninst::Offset off, unsigned size);

   void applyPatch();
   bool isApplied();

 private:
   void *dest_;
   patchTarget *source_;
   unsigned size_;
   patch_type_t ptype_;
   codeGen *gen_;
   Dyninst::Offset offset_;
   bool applied_;
};

#endif
