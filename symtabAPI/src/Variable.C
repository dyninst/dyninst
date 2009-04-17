/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

// $Id: Object.C,v 1.31 2008/11/03 15:19:25 jaw Exp $

#include "Annotatable.h"
#include "common/h/serialize.h"

#include "Symtab.h"
#include "symutil.h"
#include "Module.h"
#include "Collections.h"
#include "Variable.h"
#include "Aggregate.h"

#include "symtabAPI/src/Object.h"

#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;


Variable::Variable(Symbol *sym) :
	Aggregate(sym),
	type_(NULL)
{
}

Variable::Variable() :
	Aggregate(),
	type_(NULL)
{
}
void Variable::setType(Type *type)
{
	//fprintf(stderr, "%s[%d]:  setting variable %s to type id %d\n", FILE__, __LINE__, prettyName.c_str(), type ? type->getID() : 0xdeadbeef);
	type_ = type;
}

Type* Variable::getType()
{
	module_->exec()->parseTypesNow();
	return type_;
}

void Variable::serialize(SerializerBase *sb, const char *tag) THROW_SPEC (SerializerError)
{
	//fprintf(stderr, "%s[%d]:  welcome to Variable::serialize\n", FILE__, __LINE__);
	if (!sb)
	{
		SER_ERR("bad paramater sb");
	}

	//  Use typeID as unique identifier
	//  magic numbers stink, but we use both positive and negative numbers for type ids
	unsigned int t_id = type_ ? type_->getID() : (unsigned int) 0xdeadbeef; 

	try 
	{
		ifxml_start_element(sb, tag);
		gtranslate(sb, t_id, "typeID");
		Aggregate::serialize_aggregate(sb);
		ifxml_end_element(sb, tag);
		if (sb->isInput())
		{
		   if (t_id == 0xdeadbeef)
		   {
			   type_ = NULL;
		   }
		   else
		   {
			   restore_type_by_id(sb, type_, t_id);
		   }
		} 
		else
		{
			ScopedSerializerBase<Symtab> *ssb = dynamic_cast<ScopedSerializerBase<Symtab> *>(sb);

			if (!ssb)
			{
				fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
				SER_ERR("FIXME");
			}

			Symtab *st = ssb->getScope();

			//  remove this check
			if ((t_id != 0xdeadbeef) && !st->findType(t_id))
			{
				fprintf(stderr, "%s[%d]:  ERROR:  serialize bad type %s\n", FILE__, __LINE__, type_->getName().c_str());
			}
		}
	}
	SER_CATCH(tag);
}

std::ostream &operator<<(std::ostream &os, const Dyninst::SymtabAPI::Variable &v)
{
	std::string tname(v.type_ ? v.type_->getName() : "no_type");
	const Aggregate *ag = dynamic_cast<const Aggregate *>(&v);
	assert(ag);

	os  << "Variable{"        
		<< " type=" 
		<< tname
	    << " ";						   
	os  << 	*ag;					   
	os  << 	"}";
	return os;	

}
bool Variable::operator==(const Variable &v)
{
	if (type_ && !v.type_)
		return false;
	if (!type_ && v.type_)
		return false;
	if (type_)
		if (type_->getID() != v.type_->getID())
		{
			return false;
		}
	return ((Aggregate &)(*this)) == ((Aggregate &)v);
}

bool Variable::removeSymbol(Symbol *sym) 
{
    removeSymbolInt(sym);
    if (symbols_.empty()) {
        module_->exec()->deleteVariable(this);
    }
    return true;
}

namespace Dyninst {
	namespace SymtabAPI {
const char *storageClass2Str(Dyninst::SymtabAPI::storageClass sc) 
{
	switch(sc) {
		CASE_RETURN_STR(storageAddr);
		CASE_RETURN_STR(storageReg);
		CASE_RETURN_STR(storageRegOffset);
	};
	return "bad_storage_class";
}

const char *storageRefClass2Str(Dyninst::SymtabAPI::storageRefClass sc) 
{
	switch(sc) {
		CASE_RETURN_STR(storageRef);
		CASE_RETURN_STR(storageNoRef);
	};
	return "bad_storage_class";
}
}
}

bool VariableLocation::operator==(const VariableLocation &f)
{
	if (stClass != f.stClass) return false;
	if (refClass != f.refClass) return false;
	if (reg != f.reg) return false;
	if (frameOffset != f.frameOffset) return false;
	if (hiPC != f.hiPC) return false;
	if (lowPC != f.lowPC) return false;
	return true;
}
void VariableLocation::serialize(SerializerBase *sb, const char *tag) THROW_SPEC (SerializerError)
{
	ifxml_start_element(sb, tag);
	gtranslate(sb, (int &)stClass, "StorageClass");
	gtranslate(sb, (int &)refClass, "StorageRefClass");
	gtranslate(sb, reg, "register");
	gtranslate(sb, frameOffset, "frameOffset");
	gtranslate(sb, hiPC, "hiPC");
	gtranslate(sb, lowPC, "lowPC");
	ifxml_end_element(sb, tag);
}

localVar::localVar(std::string name,  Type *typ, std::string fileName, 
		int lineNum, std::vector<VariableLocation> *locs) :
	Serializable(),
	name_(name), 
	type_(typ), 
	fileName_(fileName), 
	lineNum_(lineNum) 
{
	type_->incrRefCount();

	if (locs)
	{
		for (unsigned int i = 0; i < locs->size(); ++i)
		{
			locs_.push_back((*locs)[i]);
		}
	}
}

localVar::localVar(localVar &lvar) :
	Serializable()
{
	name_ = lvar.name_;
	type_ = lvar.type_;
	fileName_ = lvar.fileName_;
	lineNum_ = lvar.lineNum_;

	for (unsigned int i = 0; i < lvar.locs_.size(); ++i)
	{
		locs_.push_back(lvar.locs_[i]);
	}

	if (type_ != NULL)
	{
		type_->incrRefCount();
	}
}

bool localVar::addLocation(VariableLocation &location)
{
	locs_.push_back(location);

	return true;
}

localVar::~localVar()
{
	//XXX jdd 5/25/99 More to do later
	type_->decrRefCount();
}

void localVar::fixupUnknown(Module *module) 
{
	if (type_->getDataClass() == dataUnknownType) 
	{
		Type *otype = type_;
		type_ = module->getModuleTypesPrivate()->findType(type_->getID());

		if (type_)
		{
			type_->incrRefCount();
			otype->decrRefCount();
		}
		else
			type_ = otype;
	}
}

std::string &localVar::getName() 
{
	return name_;
}

Type *localVar::getType() 
{
	return type_;
}

bool localVar::setType(Type *newType) 
{
	type_ = newType;
	return true;
}

int localVar::getLineNum() 
{
	return lineNum_;
}

std::string &localVar::getFileName() 
{
	return fileName_;
}

std::vector<Dyninst::SymtabAPI::VariableLocation> &localVar::getLocationLists() 
{
	return locs_;
}

bool localVar::operator==(const localVar &l)
{
	if (type_ && !l.type_) return false;
	if (!type_ && l.type_) return false;

	if (type_)
	{
		if (type_->getID() != l.type_->getID())
			return false;
	}

	if (name_ != l.name_) return false;
	if (fileName_ != l.fileName_) return false;
	if (lineNum_ != l.lineNum_) return false;

	if (locs_.size() != l.locs_.size()) return false;

	for (unsigned int i = 0; i < locs_.size(); ++i)
	{
		if ( !(locs_[i] == l.locs_[i]) ) return false;
	}

	return true;
}

void localVar::serialize(SerializerBase *sb, const char *tag) THROW_SPEC(SerializerError)
{
	//  Use typeID as unique identifier
	//  magic numbers stink, but we use both positive and negative numbers for type ids
	unsigned int t_id = type_ ? type_->getID() : (unsigned int) 0xdeadbeef; 

	ifxml_start_element(sb, tag);
	gtranslate(sb, name_, "Name");
	gtranslate(sb, fileName_, "FileName");
	gtranslate(sb, lineNum_, "LineNumber");
	gtranslate(sb, t_id, "TypeID");
	gtranslate(sb, locs_, "Locations", "Location");
	ifxml_end_element(sb, tag);

	if (sb->isInput())
	{
		if (t_id == 0xdeadbeef)
		{
			type_ = NULL;
		}
		else 
		{
			ScopedSerializerBase<Symtab> *ssb = dynamic_cast<ScopedSerializerBase<Symtab> *>(sb);

			if (!ssb)
			{
				fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
				SER_ERR("FIXME");
			}

			Symtab *st = ssb->getScope();

			if (!st)
			{
				fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
				SER_ERR("FIXME");
			}

			type_ = st->findType(t_id);

			if (!type_)
			{
				//  This should probably throw, but let's play nice for now
				fprintf(stderr, "%s[%d]:  FIXME: cannot find type with id %d\n", FILE__, __LINE__, t_id);
			}
		}

	}
}

