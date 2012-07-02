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

// $Id: Object.C,v 1.31 2008/11/03 15:19:25 jaw Exp $

#include "Annotatable.h"
#include "common/h/serialize.h"

#include "Symtab.h"
#include "symutil.h"
#include "Module.h"
#include "Collections.h"
#include "Variable.h"
#include "Aggregate.h"
#include "Function.h"

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

#if !defined(SERIALIZATION_DISABLED)
Serializable *Variable::serialize_impl(SerializerBase *sb, const char *tag) THROW_SPEC (SerializerError)
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

		serialize_printf("%s[%d]:  %sSERIALIZED VARIABLE %s, %lu syms\n", 
				FILE__, __LINE__, 
				sb->isInput() ? "DE" : "", 
				getAllPrettyNames().size() ? getAllPrettyNames()[0].c_str() : "no_name",
				symbols_.size()); 

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
#if 0
			Dyninst::ScopedSerializerBase<Dyninst::SymtabAPI::Symtab> *ssb = dynamic_cast<Dyninst::ScopedSerializerBase<Dyninst::SymtabAPI::Symtab> *>(sb);

			if (!ssb)
			{
				fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME, sb is_bin = %s, sb = %p\n", FILE__, __LINE__, sb->isBin() ? "true" : "false", sb);
				SerializerBin<Symtab> *sbst = dynamic_cast<SerializerBin<Symtab> *> (sb);
				if (NULL == sbst)
				{
					fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
				}
				SER_ERR("FIXME");
			}

			Symtab *st = ssb->getScope();
#endif
			SerContextBase *scb = sb->getContext();
			if (!scb)
			{
				fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
				SER_ERR("FIXME");
			}

			SerContext<Symtab> *scs = dynamic_cast<SerContext<Symtab> *>(scb);

			if (!scs)
			{
				fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
				SER_ERR("FIXME");
			}

			Symtab *st = scs->getScope();


			//  remove this check
			if ((t_id != 0xdeadbeef) && !st->findType(t_id))
			{
				fprintf(stderr, "%s[%d]:  ERROR:  serialize bad type %s\n", FILE__, __LINE__, type_->getName().c_str());
			}
		}
	}
	SER_CATCH(tag);

	return NULL;
}
#else
Serializable *Variable::serialize_impl(SerializerBase *, const char *) THROW_SPEC (SerializerError)
{
   return NULL;
}
#endif

std::ostream &operator<<(std::ostream &os, const Dyninst::SymtabAPI::Variable &v)
{
	Type *var_t = (const_cast<Variable &>(v)).getType();
	std::string tname(var_t ? var_t->getName() : "no_type");
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

localVar::localVar(std::string name,  Type *typ, std::string fileName, 
		int lineNum, Function *f, std::vector<VariableLocation> *locs) :
	Serializable(),
	name_(name), 
	type_(typ), 
	fileName_(fileName), 
	lineNum_(lineNum),
   func_(f)
{
	type_->incrRefCount();

	if (locs)
	{
		for (unsigned int i = 0; i < locs->size(); ++i)
		{
         addLocation((*locs)[i]);
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
   func_ = lvar.func_;

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
   if (!func_) {
	locs_.push_back(location);
      return true;
   }
   std::vector<VariableLocation> &func_fp = func_->getFramePtr();
   if (!func_fp.size() || location.stClass != storageRegOffset || location.reg != -1) {
      locs_.push_back(location);
      return true;
   }

   // Merge variable and function frame pointer's location lists
   std::vector<VariableLocation>::iterator i;
   for (i = func_fp.begin(); i != func_fp.end(); i++) 
   {
      Offset fplowPC = i->lowPC;
      Offset fphiPC = i->hiPC;
      Offset varlowPC = location.lowPC;
      Offset varhiPC = location.hiPC;
      
      /* Combine fplocs->frameOffset to loc->frameOffset
         
      6 cases
      1) varlowPC > fphiPC > fplowPC - no overlap - no push
      2) fphiPC > varlowPC > fplowPC - one push
      3) fphiPC > fplowPC > varlowPC - one push
      4) fphiPC > varhiPC > fplowPC - one push
      5) fphiPC > fplowPC > varhiPC - no overlap - no push
      6) fphiPC > varhiPC > varlowPC> fpilowPC - one push 
      
      */

      VariableLocation newloc;
      newloc.stClass = location.stClass;
      newloc.refClass = location.refClass;
      newloc.reg = location.reg;
      newloc.frameOffset =location.frameOffset +  i->frameOffset;

      if ( (varlowPC > fplowPC && varlowPC >= fphiPC) || (varhiPC <= fplowPC && varhiPC < fplowPC) ) {
         //nothing
      } 
      else if ( varlowPC >= fplowPC && fphiPC >= varhiPC) 
      {
         newloc.lowPC = varlowPC;
         newloc.hiPC = varhiPC;
      } 
      else if (varlowPC >= fplowPC && varlowPC < fphiPC) 
      {
         newloc.lowPC = varlowPC;
         newloc.hiPC = fphiPC;
      } 
      else if (varlowPC < fplowPC && varlowPC < fphiPC ) 
      { // varhiPC > fplowPC && varhiPC > fphiPC
         newloc.lowPC = fplowPC;
         newloc.hiPC = fphiPC;
      } 
      else if (varhiPC > fplowPC && varhiPC < fphiPC) 
      {
         newloc.lowPC = fplowPC;
         newloc.hiPC = varhiPC;
      }
      locs_.push_back(newloc);
   } // fploc iteration

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
		typeCollection *tc = typeCollection::getModTypeCollection(module);
		assert(tc);
		type_ = tc->findType(type_->getID());

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

std::vector<Dyninst::VariableLocation> &localVar::getLocationLists() 
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

#if !defined(SERIALIZATION_DISABLED)
Serializable *localVar::serialize_impl(SerializerBase *sb, const char *tag) THROW_SPEC(SerializerError)
{
	serialize_printf("%s[%d]:  welcome to localVar::serialize_impl\n", FILE__, __LINE__);
	//  Use typeID as unique identifier
	//  magic numbers stink, but we use both positive and negative numbers for type ids
	unsigned int t_id = (unsigned int) 0xdeadbeef;
	if (sb->isOutput()) t_id = type_ ? type_->getID() : (unsigned int) 0xdeadbeef; 

	ifxml_start_element(sb, tag);
	gtranslate(sb, name_, "Name");
	gtranslate(sb, fileName_, "FileName");
	gtranslate(sb, lineNum_, "LineNumber");
	gtranslate(sb, t_id, "TypeID");
	gtranslate(sb, locs_, "Locations", "Location");
	ifxml_end_element(sb, tag);

	serialize_printf("%s[%d]:  %sserialize localVar %s\n", FILE__, __LINE__, sb->isInput() ? "de" : "", name_.c_str());

	if (sb->isInput())
	{
		if (t_id == 0xdeadbeef)
		{
			type_ = NULL;
		}
		else 
		{
			SerContextBase *scb = sb->getContext();
			if (!scb)
			{
				fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
				SER_ERR("FIXME");
			}

			SerContext<Symtab> *scs = dynamic_cast<SerContext<Symtab> *>(scb);

			if (!scs)
			{
				fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
				SER_ERR("FIXME");
			}

			Symtab *st = scs->getScope();

			if (!st)
			{
				fprintf(stderr, "%s[%d]:  SERIOUS:  FIXME\n", FILE__, __LINE__);
				SER_ERR("FIXME");
			}

			type_ = st->findType(t_id);

			if (!type_)
			{
				//  This should probably throw, but let's play nice for now
				serialize_printf("%s[%d]:  FIXME: cannot find type with id %d\n", FILE__, __LINE__, t_id);
			}
		}

	}
	serialize_printf("%s[%d]:  %sserialized localVar %s, done\n", FILE__, __LINE__, sb->isInput() ? "de" : "", name_.c_str());
	return NULL;
}
#else
Serializable *localVar::serialize_impl(SerializerBase *, const char *) THROW_SPEC(SerializerError)
{
   return NULL;
}
#endif
