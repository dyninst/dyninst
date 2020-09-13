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

// $Id: Object.C,v 1.31 2008/11/03 15:19:25 jaw Exp $

#include "annotations.h"
#include "Annotatable.h"

AnnotationClass<localVarCollection> FunctionLocalVariablesAnno("FunctionLocalVariablesAnno");
AnnotationClass<localVarCollection> FunctionParametersAnno("FunctionParametersAnno");
AnnotationClass<std::vector<std::string> > SymbolVersionNamesAnno("SymbolVersionNamesAnno");
AnnotationClass<std::string> SymbolFileNameAnno("SymbolFileNameAnno");
AnnotationClass<std::vector<Function *> > UserFuncsAnno("UserFuncsAnno");
AnnotationClass<std::vector<Region *> > UserRegionsAnno("UserRegionsAnno");
AnnotationClass<std::vector<Type *> > UserTypesAnno("UserTypesAnno");
AnnotationClass<std::vector<Symbol *> > UserSymbolsAnno("UserSymbolsAnno");
AnnotationClass<LineInformation> ModuleLineInfoAnno("ModuleLineInfoAnno");
AnnotationClass<typeCollection> ModuleTypeInfoAnno("ModuleTypeInfoAnno");
AnnotationClass<dyn_hash_map<Address, Symbol *> > IdToSymAnno("IdToSymMap");
