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

#ifndef __MODULE__H__
#define __MODULE__H__
 
#include "symutil.h"
#include "Symbol.h"

#include "Annotatable.h"
#include "Serialization.h"
#include "LineInformation.h"

typedef struct {} module_line_info_a;
typedef struct {} module_type_info_a;

namespace Dyninst{
namespace SymtabAPI{

class typeCollection;
class LineInformation;

class Module : public LookupInterface,
               public Serializable, 
               public AnnotatableSparse
{
   friend class Symtab;

   public:

   SYMTABEXPORT Module();
   SYMTABEXPORT Module(supportedLanguages lang, Offset adr, std::string fullNm,
         Symtab *img);
   SYMTABEXPORT Module(const Module &mod);
   SYMTABEXPORT bool operator==(Module &mod);

   SYMTABEXPORT void serialize(SerializerBase *sb, const char *tag = "Module");

   SYMTABEXPORT const std::string &fileName() const;
   SYMTABEXPORT const std::string &fullName() const;
   SYMTABEXPORT bool setName(std::string newName);

   SYMTABEXPORT supportedLanguages language() const;
   SYMTABEXPORT void setLanguage(supportedLanguages lang);

   SYMTABEXPORT Offset addr() const;
   SYMTABEXPORT Symtab *exec() const;

   SYMTABEXPORT virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, 
         Symbol::SymbolType sType);

   SYMTABEXPORT bool getAllFunctions(std::vector<Function *>&ret);

   SYMTABEXPORT virtual bool findSymbolByType(std::vector<Symbol *> &ret, 
         const std::string name,
         Symbol::SymbolType sType, 
         bool isMangled = false,
         bool isRegex = false, 
         bool checkCase = false);

   SYMTABEXPORT bool isShared() const;
   SYMTABEXPORT ~Module();

   /***** Type Information *****/
   SYMTABEXPORT virtual bool findType(Type *&type, std::string name);
   SYMTABEXPORT virtual bool findVariableType(Type *&type, std::string name);

   SYMTABEXPORT std::vector<Type *> *getAllTypes();
   SYMTABEXPORT std::vector<std::pair<std::string, Type *> > *getAllGlobalVars();
   SYMTABEXPORT typeCollection *getModuleTypes();

   /***** Local Variable Information *****/
   SYMTABEXPORT bool findLocalVariable(std::vector<localVar *>&vars, std::string name);

   /***** Line Number Information *****/
   SYMTABEXPORT bool getAddressRanges(std::vector<std::pair<Offset, Offset> >&ranges,
         std::string lineSource, unsigned int LineNo);
   SYMTABEXPORT bool getSourceLines(std::vector<LineInformationImpl::LineNoTuple> &lines,
         Offset addressInRange);
   SYMTABEXPORT bool setLineInfo(LineInformation *lineInfo);
   SYMTABEXPORT LineInformation *getLineInformation();
   SYMTABEXPORT bool hasLineInformation();
   SYMTABEXPORT bool setDefaultNamespacePrefix(std::string str);

   private:

   std::string fileName_;                   // short file 
   std::string fullName_;                   // full path to file 
   supportedLanguages language_;
   Offset addr_;                      // starting address of module
   Symtab *exec_;
};





}//namespace SymtabAPI

}//namespace Dyninst
#endif
