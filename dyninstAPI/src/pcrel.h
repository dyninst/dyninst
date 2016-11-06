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

#if !defined(_PCREL_H_)
#define _PCREL_H_

class pcRelRegion {
 public:
  friend class codeGen;
  codeGen *gen;
  instruction orig_instruc;
  unsigned cur_offset;
  unsigned cur_size;
  pcRelRegion(const instruction &i);
  virtual unsigned apply(Address addr) = 0;
  virtual unsigned maxSize() = 0;
  virtual bool canPreApply();
  virtual ~pcRelRegion();
};

class pcRelJump : public pcRelRegion {
 private:
  Address addr_targ;
  patchTarget *targ;
  bool copy_prefixes_;

  Address get_target();

 public:
  pcRelJump(patchTarget *t, const instruction &i, bool copyPrefixes = true);
  pcRelJump(Address target, const instruction &i, bool copyPrefixes = true);
  virtual unsigned apply(Address addr);
  virtual unsigned maxSize();
  virtual bool canPreApply();
  virtual ~pcRelJump();
};

class pcRelJCC : public pcRelRegion {
 private:
  Address addr_targ;
  patchTarget *targ;

  Address get_target();

 public:
  pcRelJCC(patchTarget *t, const instruction &i);
  pcRelJCC(Address target, const instruction &i);
  virtual unsigned apply(Address addr);
  virtual unsigned maxSize();
  virtual bool canPreApply();
  virtual ~pcRelJCC();
};

class pcRelCall : public pcRelRegion {
 private:
  Address targ_addr;
  patchTarget *targ;

  Address get_target();

 public:
  pcRelCall(patchTarget *t, const instruction &i);
  pcRelCall(Address targ_addr, const instruction &i);

  virtual unsigned apply(Address addr);
  virtual unsigned maxSize();
  virtual bool canPreApply();
  ~pcRelCall();
};

class pcRelData : public pcRelRegion {
 private:
  Address data_addr;

 public:
  pcRelData(Address a, const instruction &i);
  virtual unsigned apply(Address addr);
  virtual unsigned maxSize();
  virtual bool canPreApply();
};

#endif
