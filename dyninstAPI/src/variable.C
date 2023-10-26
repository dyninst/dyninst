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
 
// $Id: variable.C,v 1.12 2008/11/03 15:19:24 jaw Exp $

// Variable.C

#include <string>
#include "common/h/Annotatable.h"
#include "mapped_object.h"

image_variable::image_variable(SymtabAPI::Variable *var, pdmodule *mod) :
    var_(var),			       
    pdmod_(mod) 
{
}								    

Address image_variable::getOffset() const 
{
   return var_->getOffset();
}

bool image_variable::addSymTabName(const std::string &name, bool isPrimary) 
{
   if (var_->addMangledName(name.c_str(), isPrimary)){
      return true;
   }
   // Bool: true if the name is new; AKA !found
   return false;
}

bool image_variable::addPrettyName(const std::string &name, bool isPrimary) 
{
    if (var_->addPrettyName(name.c_str(), isPrimary)){
      return true;
   }
   // Bool: true if the name is new; AKA !found
   return false;
}       

SymtabAPI::Aggregate::name_iter image_variable::symtab_names_begin() const
{
  return var_->mangled_names_begin();
}

SymtabAPI::Aggregate::name_iter image_variable::symtab_names_end() const
{
  return var_->mangled_names_end();
}
SymtabAPI::Aggregate::name_iter image_variable::pretty_names_begin() const
{
  return var_->pretty_names_begin();
}

SymtabAPI::Aggregate::name_iter image_variable::pretty_names_end() const
{
  return var_->pretty_names_end();
}



int_variable::int_variable(image_variable *var, 
      Address base,
      mapped_module *mod) :
   addr_(base + var->getOffset()),
   size_(0),
   ivar_(var),
   mod_(mod)
{
}

int_variable::int_variable(int_variable *parVar,
      mapped_module *childMod) :
   addr_(parVar->addr_),
   size_(parVar->size_),
   ivar_(parVar->ivar_),
   mod_(childMod)
{
   // Mmm forkage
}

SymtabAPI::Aggregate::name_iter int_variable::symtab_names_begin() const
{
  return ivar_->symtab_names_begin();
}

SymtabAPI::Aggregate::name_iter int_variable::symtab_names_end() const
{
  return ivar_->symtab_names_end();
}
SymtabAPI::Aggregate::name_iter int_variable::pretty_names_begin() const
{
  return ivar_->pretty_names_begin();
}

SymtabAPI::Aggregate::name_iter int_variable::pretty_names_end() const
{
  return ivar_->pretty_names_end();
}

string int_variable::symTabName() const 
{
   return ivar_->symTabName();
}
