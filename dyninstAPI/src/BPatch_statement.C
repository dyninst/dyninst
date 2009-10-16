/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

#include "BPatch_statement.h"
#include "BPatch_module.h"
#include "Module.h"
#include "mapped_object.h"
#include "mapped_module.h"
BPatch_statement::BPatch_statement(BPatch_module *mod, Dyninst::SymtabAPI::Statement *s) :
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
  return statement->line();
}

int BPatch_statement::lineOffset()
{
	assert(statement);
  return statement->column();
}

const char *BPatch_statement::fileName()
{
	assert(statement);
	return statement->file().c_str();
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
