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
               public Annotatable<LineInformation *, module_line_info_a, true >,
               public Annotatable<typeCollection *, module_type_info_a, true> {
                  friend class Symtab;
 public:
    DLLEXPORT Module();
    DLLEXPORT Module(supportedLanguages lang, Offset adr, std::string fullNm,
                            Symtab *img);
    DLLEXPORT Module(const Module &mod);
    DLLEXPORT bool operator==(const Module &mod) const;

    DLLEXPORT void serialize(SerializerBase *sb, const char *tag = "Module");
    
    DLLEXPORT const std::string &fileName() const;
    DLLEXPORT const std::string &fullName() const;
    DLLEXPORT bool setName(std::string newName);
    
    DLLEXPORT supportedLanguages language() const;
    DLLEXPORT void setLanguage(supportedLanguages lang);
    
    DLLEXPORT Offset addr() const;
    DLLEXPORT Symtab *exec() const;
    
    DLLEXPORT virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, 
                                            Symbol::SymbolType sType);
    DLLEXPORT virtual bool findSymbolByType(std::vector<Symbol *> &ret, 
                                           const std::string name,
                                           Symbol::SymbolType sType, 
                                           bool isMangled = false,
                                           bool isRegex = false, 
                                           bool checkCase = false);
	
    DLLEXPORT bool isShared() const;
    DLLEXPORT ~Module();
    
    /***** Type Information *****/
    DLLEXPORT virtual bool findType(Type *&type, std::string name);
    DLLEXPORT virtual bool findVariableType(Type *&type, std::string name);

    DLLEXPORT std::vector<Type *> *getAllTypes();
    DLLEXPORT std::vector<std::pair<std::string, Type *> > *getAllGlobalVars();
    DLLEXPORT typeCollection *getModuleTypes();

    /***** Local Variable Information *****/
    DLLEXPORT bool findLocalVariable(std::vector<localVar *>&vars, std::string name);

    /***** Line Number Information *****/
	DLLEXPORT bool getAddressRanges(std::vector<std::pair<Offset, Offset> >&ranges,
					std::string lineSource, unsigned int LineNo);
    DLLEXPORT bool getSourceLines(std::vector<LineInformationImpl::LineNoTuple> &lines,
          Offset addressInRange);
    DLLEXPORT bool setLineInfo(LineInformation *lineInfo);
    DLLEXPORT LineInformation *getLineInformation();
    DLLEXPORT bool hasLineInformation();
    DLLEXPORT bool setDefaultNamespacePrefix(std::string str);

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
