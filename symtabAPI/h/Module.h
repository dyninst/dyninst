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

namespace Dyninst{
namespace SymtabAPI{

class typeCollection;
class LineInformation;
class localVar;
class Symtab;

class Module : public LookupInterface,
               public Serializable, 
               public AnnotatableSparse
{
   friend class Symtab;

   public:

   SYMTAB_EXPORT Module();
   SYMTAB_EXPORT Module(supportedLanguages lang, Offset adr, std::string fullNm,
         Symtab *img);
   SYMTAB_EXPORT Module(const Module &mod);
   SYMTAB_EXPORT bool operator==(Module &mod);

   SYMTAB_EXPORT void serialize(SerializerBase *sb, const char *tag = "Module") THROW_SPEC (SerializerError);

   SYMTAB_EXPORT const std::string &fileName() const;
   SYMTAB_EXPORT const std::string &fullName() const;
   SYMTAB_EXPORT bool setName(std::string newName);

   SYMTAB_EXPORT supportedLanguages language() const;
   SYMTAB_EXPORT void setLanguage(supportedLanguages lang);

   SYMTAB_EXPORT Offset addr() const;
   SYMTAB_EXPORT Symtab *exec() const;

   SYMTAB_EXPORT bool isShared() const;
   SYMTAB_EXPORT ~Module();

   // Symbol output methods
   SYMTAB_EXPORT virtual bool findSymbolByType(std::vector<Symbol *> &ret, 
                                               const std::string name,
                                               Symbol::SymbolType sType, 
                                               NameType nameType = anyName,
                                               bool isRegex = false, 
                                               bool checkCase = false);
   SYMTAB_EXPORT virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, 
                                                  Symbol::SymbolType sType);
   SYMTAB_EXPORT virtual bool getAllSymbols(std::vector<Symbol *> &ret);


   // Function based methods
   SYMTAB_EXPORT bool getAllFunctions(std::vector<Function *>&ret);
   SYMTAB_EXPORT bool findFunctionByEntryOffset(Function *&ret, const Offset offset);
   SYMTAB_EXPORT bool findFunctionsByName(std::vector<Function *> &ret, const std::string name,
                                      NameType nameType = anyName, 
                                      bool isRegex = false,
                                      bool checkCase = true);

   // Variable based methods
   SYMTAB_EXPORT bool findVariableByOffset(Variable *&ret, const Offset offset);
   SYMTAB_EXPORT bool findVariablesByName(std::vector<Variable *> &ret, const std::string name,
                                      NameType nameType = anyName, 
                                      bool isRegex = false, 
                                      bool checkCase = true);
   SYMTAB_EXPORT bool getAllVariables(std::vector<Variable *> &ret);


   // Type output methods
   SYMTAB_EXPORT virtual bool findType(Type *&type, std::string name);
   SYMTAB_EXPORT virtual bool findVariableType(Type *&type, std::string name);

   SYMTAB_EXPORT std::vector<Type *> *getAllTypes();
   SYMTAB_EXPORT std::vector<std::pair<std::string, Type *> > *getAllGlobalVars();

   SYMTAB_EXPORT typeCollection *getModuleTypes();

   /***** Local Variable Information *****/
   SYMTAB_EXPORT bool findLocalVariable(std::vector<localVar *>&vars, std::string name);

   /***** Line Number Information *****/
   SYMTAB_EXPORT bool getAddressRanges(std::vector<std::pair<Offset, Offset> >&ranges,
         std::string lineSource, unsigned int LineNo);
   SYMTAB_EXPORT bool getSourceLines(std::vector<LineNoTuple> &lines,
         Offset addressInRange);
   SYMTAB_EXPORT bool setLineInfo(LineInformation *lineInfo);
   SYMTAB_EXPORT LineInformation *getLineInformation();
   SYMTAB_EXPORT bool hasLineInformation();
   SYMTAB_EXPORT bool setDefaultNamespacePrefix(std::string str);

   // Deprecated methods
   SYMTAB_EXPORT virtual bool findSymbolByType(std::vector<Symbol *> &ret, 
                                               const std::string name,
                                               Symbol::SymbolType sType, 
                                               bool isMangled = false,
                                               bool isRegex = false, 
                                               bool checkCase = false);


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
