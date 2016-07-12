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

#ifndef __MODULE__H__
#define __MODULE__H__
 
#include "symutil.h"
#include "Symbol.h"

#include "Annotatable.h"
#include "Serialization.h"

namespace Dyninst{
namespace SymtabAPI{

class typeCollection;
class LineInformation;
class localVar;
class Symtab;

class SYMTAB_EXPORT Statement : public AnnotatableSparse, public Serializable
{
	friend class Module;
	friend class std::vector<Statement>;
	friend class LineInformation;

	Statement(const char *file, unsigned int line, unsigned int col = 0,
             Offset start_addr = (Offset) -1L, Offset end_addr = (Offset) -1L) :
      file_(file),
      line_(line),
      start_addr_(start_addr),
      end_addr_(end_addr),
      first(file_),
      second(line_),
      column(col)
      {
      }
	
	const char* file_; // Maybe this should be module?
	unsigned int line_;
	Offset start_addr_;
	Offset end_addr_;

	public:
	// DEPRECATED. First and second need to die, and column should become an accessor.
	// Duplication of data is both bad and stupid, okay?
	const char *first;
	unsigned int second;
	unsigned int column;

	Statement() : first(NULL), second(0) {}
	struct StatementLess {
		bool operator () ( const Statement &lhs, const Statement &rhs ) const;
	};

	typedef StatementLess LineNoTupleLess;

	bool operator==(const Statement &cmp) const;
	~Statement() {}

	Offset startAddr() { return start_addr_;}
	Offset endAddr() {return end_addr_;}
	std::string getFile() { return file_;}
	unsigned int getLine() {return line_;}
	unsigned int getColumn() {return column;}

	Serializable *serialize_impl(SerializerBase *sb, const char *tag = "Statement") THROW_SPEC (SerializerError);

	//  Does dyninst really need these?
	void setLine(unsigned int l) {line_ = l;}
	void setColumn(unsigned int l) {column = l;}
	void setFile(const char * l) {file_ = l; first = file_;}
	void setStartAddr(Offset l) {start_addr_ = l;}
	void setEndAddr(Offset l) {end_addr_ = l;}
};

typedef Statement LineNoTuple;
#define MODULE_ANNOTATABLE_CLASS AnnotatableSparse

 class SYMTAB_EXPORT Module : public LookupInterface
{
	friend class Symtab;

	public:

	Module();
	Module(supportedLanguages lang, Offset adr, std::string fullNm,
                        Symtab *img);
	Module(const Module &mod);
	bool operator==(Module &mod);

	const std::string &fileName() const;
	const std::string &fullName() const;
	bool setName(std::string newName);

	supportedLanguages language() const;
	void setLanguage(supportedLanguages lang);

	Offset addr() const;
	Symtab *exec() const;

	bool isShared() const;
	~Module();

	// Symbol output methods
	virtual bool findSymbol(std::vector<Symbol *> &ret, 
                                              const std::string& name,
                                              Symbol::SymbolType sType = Symbol::ST_UNKNOWN, 
                                              NameType nameType = anyName,
                                              bool isRegex = false, 
                                              bool checkCase = false,
                                              bool includeUndefined = false);
	virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, 
			Symbol::SymbolType sType);
	virtual bool getAllSymbols(std::vector<Symbol *> &ret);


	// Function based methods
	bool getAllFunctions(std::vector<Function *>&ret);
	bool findFunctionByEntryOffset(Function *&ret, const Offset offset);
	bool findFunctionsByName(std::vector<Function *> &ret, const std::string& name,
			NameType nameType = anyName, 
			bool isRegex = false,
			bool checkCase = true);

	// Variable based methods
	bool findVariableByOffset(Variable *&ret, const Offset offset);
	bool findVariablesByName(std::vector<Variable *> &ret, const std::string& name,
			NameType nameType = anyName, 
			bool isRegex = false, 
			bool checkCase = true);
   bool getAllVariables(std::vector<Variable *> &ret);


   // Type output methods
   virtual bool findType(Type *&type, std::string name);
   virtual bool findVariableType(Type *&type, std::string name);

   std::vector<Type *> *getAllTypes();
   std::vector<std::pair<std::string, Type *> > *getAllGlobalVars();

   typeCollection *getModuleTypes();

   /***** Local Variable Information *****/
   bool findLocalVariable(std::vector<localVar *>&vars, std::string name);

   /***** Line Number Information *****/
   bool getAddressRanges(std::vector<std::pair<Offset, Offset> >&ranges,
         std::string lineSource, unsigned int LineNo);
   bool getSourceLines(std::vector<Statement *> &lines,
         Offset addressInRange);
   bool getSourceLines(std::vector<LineNoTuple> &lines,
         Offset addressInRange);
   bool getStatements(std::vector<Statement *> &statements);
   LineInformation *getLineInformation();

   bool hasLineInformation();
   bool setDefaultNamespacePrefix(std::string str);


   //  Super secret private methods that aren't really private
   typeCollection *getModuleTypesPrivate();
   void setModuleTypes(typeCollection* tc) 
   {
     typeInfo_ = tc;
   }
   
   bool setLineInfo(Dyninst::SymtabAPI::LineInformation *lineInfo);
   private:
   Dyninst::SymtabAPI::LineInformation* lineInfo_;
   typeCollection* typeInfo_;
   

   std::string fileName_;                   // short file 
   std::string fullName_;                   // full path to file 
   supportedLanguages language_;
   Offset addr_;                      // starting address of module
   Symtab *exec_;
};
		template <typename OS>
		OS& operator<<(OS& os, const Module& m)
		{
			os << m.fileName() << ": " << m.addr();
			return os;
		}





}//namespace SymtabAPI

}//namespace Dyninst
#endif
