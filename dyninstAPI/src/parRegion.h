/*
 * Copyright (c) 1996-2006 Barton P. Miller
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
 

#ifndef PARREGION_H
#define PARREGION_H

#include <string>
#include "common/h/Vector.h"
#include "common/h/Types.h"
#include "common/h/Pair.h"
#include "codeRange.h"
#include "arch.h" // instruction
#include "dyninstAPI/h/BPatch_Set.h"
#include "dyninstAPI/h/BPatch_parRegion.h"
#include "common/h/Dictionary.h"
#include <map>

class process;
class mapped_module;
class mapped_object;

class pdmodule;
class InstrucIter;
class image_func;

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
  image_parRegion(image_func * imageFunc);
  image_parRegion(Address firstOffset, image_func * imageFunc);
 
  Address firstInsnOffset() const { return firstInsnOffset_; }

  void setLastInsn(Address last) { lastInsnOffset_ = last;}
  Address lastInsnOffset() const { return lastInsnOffset_; }
  Address getSize() const { return lastInsnOffset_ - firstInsnOffset_; }

  Address get_address() const {return firstInsnOffset_; }
  unsigned int get_size() const {return 0;}

  parRegType getRegionType(){return regionType;}

  void setRegionType(parRegType rt); 

  image_func * getAssociatedFunc();

  void setParentFunc(image_func * parentFunc){parentIf_ = parentFunc;}
  image_func * getParentFunc(){return parentIf_;}

  void setClause(const char * key, int value);
  int getClause(const char * key);

  void setClauseLoc(const char * key, Address value);
  Address getClauseLoc(const char * key);
  
  void printDetails();

  void decodeClauses(int bitmap);

 private:
  image_func *regionIf_;
  image_func *parentIf_;
  Address firstInsnOffset_;
  Address lastInsnOffset_;
  parRegType regionType;
  std::map<const char*, int, ltstr> clauses;
  std::map<const char*, Address, ltstr> clause_locations;
};


class int_parRegion {
 public:
  int_parRegion(image_parRegion *ip, Address baseAddr, int_function * );
  ~int_parRegion();

  Address firstInsnAddr() {return addr_;}
  Address endAddr() {return endAddr_;}

  const image_parRegion * imagePar() const { return ip_; }
  
  void printDetails() { ip_->printDetails(); }

  const int_function * intFunc() { return intFunc_;}

  int getClause(const char * key);
  Address getClauseLoc(const char * key);

  int replaceOMPParameter(const char * key, int value);

  Address addr_; /* Absolute address of start of region */
  Address endAddr_; /* Address of end of region */

  int_function * intFunc_;

  image_parRegion *ip_;
};




#endif /*PARREGION_H */
