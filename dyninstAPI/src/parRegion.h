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
 

#ifndef PARREGION_H
#define PARREGION_H

#include <string.h>
#include <string>
#include "codeRange.h"
#include "common/src/arch.h" // instruction
#include "dyninstAPI/h/BPatch_parRegion.h"
#include <unordered_map>
#include <map>
#include "dyntypes.h"

class mapped_module;
class mapped_object;

class pdmodule;
class parse_func;

/*
typedef enum{
  OMP_NONE, OMP_PARALLEL, OMP_DO_FOR,OMP_DO_FOR_LOOP_BODY, OMP_SECTIONS, OMP_SINGLE, 
    OMP_PAR_DO, OMP_PAR_SECTIONS, OMP_MASTER, OMP_CRITICAL,
    OMP_BARRIER, OMP_ATOMIC, OMP_FLUSH, OMP_ORDERED
    } parRegType;
*/

struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};


class image_parRegion : public codeRange {
 public:
  image_parRegion(parse_func * imageFunc);
  image_parRegion(Dyninst::Address firstOffset, parse_func * imageFunc);
 
  Dyninst::Address firstInsnOffset() const { return firstInsnOffset_; }

  void setLastInsn(Dyninst::Address last) { lastInsnOffset_ = last;}
  Dyninst::Address lastInsnOffset() const { return lastInsnOffset_; }
  Dyninst::Address getSize() const { return lastInsnOffset_ - firstInsnOffset_; }

  Dyninst::Address get_address() const {return firstInsnOffset_; }
  unsigned int get_size() const {return 0;}

  parRegType getRegionType(){return regionType;}

  void setRegionType(parRegType rt); 

  const parse_func * getAssociatedFunc() const;

  void setParentFunc(parse_func * parentFunc){parentIf_ = parentFunc;}
  parse_func * getParentFunc(){return parentIf_;}

  void setClause(const char * key, int value);
  int getClause(const char * key);

  void setClauseLoc(const char * key, Dyninst::Address value);
  Dyninst::Address getClauseLoc(const char * key);
  
  void printDetails();

  void decodeClauses(int bitmap);

 private:
  parse_func *regionIf_;
  parse_func *parentIf_;
  Dyninst::Address firstInsnOffset_;
  Dyninst::Address lastInsnOffset_;
  parRegType regionType;
  std::map<const char*, int, ltstr> clauses;
  std::map<const char*, Dyninst::Address, ltstr> clause_locations;
};


class int_parRegion {
 public:
  int_parRegion(image_parRegion *ip, Dyninst::Address baseAddr, func_instance * );
  ~int_parRegion();

  Dyninst::Address firstInsnAddr() {return addr_;}
  Dyninst::Address endAddr() {return endAddr_;}

  const image_parRegion * imagePar() const { return ip_; }
  
  void printDetails() { ip_->printDetails(); }

  const func_instance * intFunc() { return intFunc_;}

  int getClause(const char * key);
  Dyninst::Address getClauseLoc(const char * key);

  int replaceOMPParameter(const char * key, int value);

  Dyninst::Address addr_; /* Absolute address of start of region */
  Dyninst::Address endAddr_; /* Dyninst::Address of end of region */

  func_instance * intFunc_;

  image_parRegion *ip_;
};




#endif /*PARREGION_H */
