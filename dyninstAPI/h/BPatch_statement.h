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

#ifndef _BPATCH_STATEMENT_H_
#define _BPATCH_STATEMENT_H_

#include "BPatch_dll.h"
#include "Statement.h"
class BPatch_module;


class BPATCH_DLL_EXPORT BPatch_statement
{
  friend class BPatch_module;
  friend class BPatch_image;

  public:
    //  BPatch_module * getModule()
    //  Return the BPatch_module that contains this statement
    BPatch_module * module(); 
    
    //  int getLineNumber()
    //  return the line number of this statement
    int lineNumber();

    //  int getLineOffset()
    //  return the line offset of this statement (its start column in the source file)
    //  This may not be supported on all platforms.
    //  Returns -1 if not supported.
    int lineOffset();

    //  const char * fileName()
    //  return the name of the file that contains this statement
    const char * fileName();

    //  void * startAddr()
    //  return the starting address of this statement
    void *startAddr();

    //  void * endAddr()
    //  return the last address associated with this statement
    //  (do we guarantee contiguity of addresses here?  not sure)
    void *endAddr();

  private:

    //  Full parameter ctor -- can only built by friend classes
    BPatch_statement(BPatch_module *mod,  Dyninst::SymtabAPI::Statement::ConstPtr s);

    BPatch_module *module_;
	Dyninst::SymtabAPI::Statement::ConstPtr statement;
};

#endif 

