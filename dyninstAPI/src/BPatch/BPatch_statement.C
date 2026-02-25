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

#include "BPatch_statement.h"
#include "BPatch_module.h"
#include "Statement.h"
#include "mapped_object.h"
#include "mapped_module.h"
BPatch_statement::BPatch_statement(BPatch_module *mod, Dyninst::SymtabAPI::Statement::ConstPtr s) :
        module_(mod),
        statement(s)
{
}

BPatch_module *BPatch_statement::module()
{
  return module_;
}

int BPatch_statement::lineNumber()
{
	assert(statement);
  return statement->getLine();
}

int BPatch_statement::lineOffset()
{
	assert(statement);
  return statement->getColumn();
}

const char *BPatch_statement::fileName()
{
	assert(statement);
	return statement->getFile().c_str();
}

void *BPatch_statement::startAddr()
{
	assert(statement);
	assert(module_);
	mapped_object *mmod = module_->mod->obj();
	assert(mmod);
	return (void *)(mmod->codeBase() + statement->startAddr());
}

void *BPatch_statement::endAddr()
{
	assert(module_);
	assert(statement);
	mapped_object *mmod = module_->mod->obj();
	assert(mmod);
	return (void *)(mmod->codeBase() + statement->endAddr());
}
