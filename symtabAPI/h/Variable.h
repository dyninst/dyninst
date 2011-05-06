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

#if !defined(_Variable_h_)
#define _Variable_h_

#include "Annotatable.h"
#include "Serialization.h"
#include "Symtab.h"
#include "Aggregate.h"
#include "dyn_regs.h"

//class Dyninst::SymtabAPI::Variable;
SYMTAB_EXPORT std::ostream &operator<<(std::ostream &os, const Dyninst::SymtabAPI::Variable &);

namespace Dyninst {
namespace SymtabAPI {

/*
 * storageClass: Encodes how a variable is stored.
 *
 * storageAddr           - Absolute address of variable.
 * storageReg            - Register which holds variable value.
 * storageRegOffset      - Address of variable = $reg + address.
 */

typedef enum {
	storageAddr,
	storageReg,
	storageRegOffset
} storageClass;

const char *storageClass2Str(storageClass sc);

/*
 * storageRefClass: Encodes if a variable can be accessed through a register/address.
 *
 * storageRef        - There is a pointer to variable.
 * storageNoRef      - No reference. Value can be obtained using storageClass.
 */
typedef enum {
	storageRef,
	storageNoRef
} storageRefClass;

const char *storageRefClass2Str(storageRefClass sc);

//location for a variable
//Use mr_reg instead of reg for new code.  reg left in for backwards
// compatibility.
class VariableLocation : public Serializable {
	public:
	storageClass stClass;
	storageRefClass refClass;
	int reg;
   MachRegister mr_reg;
	long frameOffset;
	Address lowPC;
	Address hiPC;
	SYMTAB_EXPORT bool operator==(const VariableLocation &);
	SYMTAB_EXPORT Serializable * serialize_impl(SerializerBase *, 
			const char *tag = "VariableLocation") THROW_SPEC (SerializerError);
};


class Symbol;
class Symtab;
class Aggregate;
class Function;

class Variable : public Aggregate, public Serializable, public AnnotatableSparse {
	friend class Symtab;
	friend std::ostream &::operator<<(std::ostream &os, const Dyninst::SymtabAPI::Variable &);

	private:
   SYMTAB_EXPORT Variable(Symbol *sym);
   SYMTAB_EXPORT static Variable *createVariable(Symbol *sym);
   
 public:
   SYMTAB_EXPORT Variable();
    /* Symbol management */
    SYMTAB_EXPORT bool removeSymbol(Symbol *sym);      

   SYMTAB_EXPORT void setType(Type *type);
   SYMTAB_EXPORT Type *getType();

   SYMTAB_EXPORT Serializable *serialize_impl(SerializerBase *sb, 
		   const char *tag = "Variable") THROW_SPEC (SerializerError);
   SYMTAB_EXPORT bool operator==(const Variable &v);

 private:

   Type *type_;
};

class localVar : public Serializable, public AnnotatableSparse
{
	friend class typeCommon;
	friend class localVarCollection;

	std::string name_;
	Type *type_;
	std::string fileName_;
	int lineNum_;
   Function *func_;
	std::vector<VariableLocation> locs_;

	// scope_t scope;

	public:
	SYMTAB_EXPORT localVar() {}
	//  Internal use only
	localVar(std::string name,  Type *typ, std::string fileName, 
            int lineNum, Function *f, 
            std::vector<VariableLocation> *locs = NULL);
            
	// Copy constructor
	localVar(localVar &lvar);
	bool addLocation(VariableLocation &location);
	SYMTAB_EXPORT ~localVar();
	SYMTAB_EXPORT void fixupUnknown(Module *);

	public:
	//  end of functions for internal use only
	SYMTAB_EXPORT std::string &getName();
	SYMTAB_EXPORT Type *getType();
	SYMTAB_EXPORT bool setType(Type *newType);
	SYMTAB_EXPORT int  getLineNum();
	SYMTAB_EXPORT std::string &getFileName();
	SYMTAB_EXPORT std::vector<VariableLocation> &getLocationLists();
	SYMTAB_EXPORT bool operator==(const localVar &l);
	SYMTAB_EXPORT Serializable *serialize_impl(SerializerBase *, 
			const char * = "localVar") THROW_SPEC(SerializerError);
};

}
}

#endif
