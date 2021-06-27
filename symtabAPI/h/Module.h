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
#include "IBSTree.h"
#include "IBSTree-fast.h"
#if defined(cap_dwarf)
#include "elfutils/libdw.h"
#endif
#include <boost/shared_ptr.hpp>
#include "RangeLookup.h"

#include "StringTable.h"

namespace Dyninst{
	namespace SymtabAPI{

		class typeCollection;
		class LineInformation;
		class localVar;
		class Symtab;


		class SYMTAB_EXPORT Statement : public AddressRange
		{
			friend class Module;
			friend class LineInformation;
			Statement(int file_index, unsigned int line, unsigned int col = 0,
					  Offset start_addr = (Offset) -1L, Offset end_addr = (Offset) -1L) :
					AddressRange(start_addr, end_addr),
					file_index_(file_index),
					line_(line),
					column_(col)
			{
			}

			unsigned int file_index_; // Maybe this should be module?
			unsigned int line_;
			unsigned int column_;
			StringTablePtr strings_;
		public:
			StringTablePtr getStrings_() const;

			void setStrings_(StringTablePtr strings_);

		public:

			Statement() : AddressRange(0,0), file_index_(0), line_(0), column_(0)  {}
			struct StatementLess {
				bool operator () ( const Statement &lhs, const Statement &rhs ) const;
			};

			typedef StatementLess LineNoTupleLess;
			bool operator==(const Statement &cmp) const;
			bool operator==(Offset addr) const {
				return AddressRange::contains(addr);
			}
			bool operator<(Offset addr) const {
				return startAddr() <= addr;
			}
			bool operator>(Offset addr) const {
				return !((*this) < addr || (*this == addr));
			}
			~Statement() {}

			Offset startAddr() const { return first;}
			Offset endAddr() const {return second;}
			const std::string& getFile() const;
			unsigned int getFileIndex() const { return file_index_; }
			unsigned int getLine()const {return line_;}
			unsigned int getColumn() const { return column_; }
			struct addr_range {};
			struct line_info {};
			struct upper_bound {};

			typedef Statement* Ptr;
			typedef const Statement* ConstPtr;

		};
		template <typename OS>
		OS& operator<<(OS& os, const Statement& s)
		{
			os << "<statement>: [" << std::hex << s.startAddr() << ", " << s.endAddr() << std::dec << ") @ " << s.getFile()
			   << " (" << s.getFileIndex() << "): " << s.getLine();
			return os;
		}
		template <typename OS>
		OS& operator<<(OS& os, Statement* s)
		{
			os << "<statement>: [" << std::hex << s->startAddr() << ", " << s->endAddr() << std::dec << ") @ " << s->getFile()
			   << " (" << s->getFileIndex() << "): " << s->getLine();
			return os;
		}


		typedef Statement LineNoTuple;
#define MODULE_ANNOTATABLE_CLASS AnnotatableSparse

		class SYMTAB_EXPORT Module : public LookupInterface
		{
			friend class Symtab;

		public:
#if defined(cap_dwarf)
			typedef Dwarf_Die DebugInfoT;
#else
			typedef void* DebugInfoT;
#endif

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

            std::string getCompDir();
            std::string getCompDir(Module::DebugInfoT&); // For internal use

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
			bool findVariablesByOffset(std::vector<Variable *> &ret, const Offset offset);
			bool findVariablesByName(std::vector<Variable *> &ret, const std::string& name,
									 NameType nameType = anyName,
									 bool isRegex = false,
									 bool checkCase = true);

			// Type output methods
			virtual bool findType(boost::shared_ptr<Type>& type, std::string name);
            bool findType(Type*& t, std::string n) {
              boost::shared_ptr<Type> tp;
              auto r = findType(tp, n);
              t = tp.get();
              return r;
            }
			virtual bool findVariableType(boost::shared_ptr<Type>& type, std::string name);
            bool findVariableType(Type*& t, std::string n) {
              boost::shared_ptr<Type> tp;
              auto r = findVariableType(tp, n);
              t = tp.get();
              return r;
            }

			void getAllTypes(std::vector<boost::shared_ptr<Type>>&);
            std::vector<Type*>* getAllTypes() {
              std::vector<boost::shared_ptr<Type>> v;
              getAllTypes(v);
              auto r = new std::vector<Type*>(v.size());
              for(std::size_t i = 0; i < v.size(); i++) (*r)[i] = v[i].get();
              return r;
            }
			void getAllGlobalVars(std::vector<std::pair<std::string, boost::shared_ptr<Type>>>&);
            std::vector<std::pair<std::string, Type*>>* getAllGlobalVars() {
              std::vector<std::pair<std::string, boost::shared_ptr<Type>>> v;
              getAllGlobalVars(v);
              auto r = new std::vector<std::pair<std::string, Type*>>(v.size());
              for(std::size_t i = 0; i < v.size(); i++)
                (*r)[i] = {v[i].first, v[i].second.get()};
              return r;
            }

			typeCollection *getModuleTypes();

			/***** Local Variable Information *****/
			bool findLocalVariable(std::vector<localVar *>&vars, std::string name);

			/***** Line Number Information *****/
			bool getAddressRanges(std::vector<AddressRange >&ranges,
								  std::string lineSource, unsigned int LineNo);
			bool getSourceLines(std::vector<Statement::Ptr> &lines,
								Offset addressInRange);
			bool getSourceLines(std::vector<LineNoTuple> &lines,
								Offset addressInRange);
			bool getStatements(std::vector<Statement::Ptr> &statements);
			LineInformation *getLineInformation();
			LineInformation* parseLineInformation();

			bool setDefaultNamespacePrefix(std::string str);


			//  Super secret private methods that aren't really private
			typeCollection *getModuleTypesPrivate();
			void setModuleTypes(typeCollection* tc)
			{
				typeInfo_ = tc;
			}

			bool setLineInfo(Dyninst::SymtabAPI::LineInformation *lineInfo);
			void addRange(Dyninst::Address low, Dyninst::Address high);
			bool hasRanges() const { return !ranges.empty() || ranges_finalized; }
			void addDebugInfo(Module::DebugInfoT info);

			void finalizeRanges();

		private:
            bool objectLevelLineInfo;
			Dyninst::SymtabAPI::LineInformation* lineInfo_;
			typeCollection* typeInfo_;
			dyn_c_queue<Module::DebugInfoT> info_;


			std::string fileName_;                   // short file
			std::string fullName_;                   // full path to file
			std::string compDir_;
			supportedLanguages language_;
			Offset addr_;                      // starting address of module
			Symtab *exec_;
			std::set<AddressRange > ranges;

			StringTablePtr strings_;
		public:
			StringTablePtr & getStrings() ;

		private:
			bool ranges_finalized;

			void finalizeOneRange(Address ext_s, Address ext_e) const;
		};
		template <typename OS>
		OS& operator<<(OS& os, const Module& m)
		{
			os << m.fileName() << ": " << m.addr();
			return os;
		}
		template <typename OS>
		OS& operator<<(OS& os, Module* m)
		{
			os << m->fileName() << ": " << m->addr();
			return os;
		}

		typedef Dyninst::SimpleInterval<Offset, Module*> ModRange;

		inline bool operator==(Offset off, const ModRange& r) {
			return (r.low() <= off) && (off < r.high());
		}
		inline bool operator==(const ModRange& r, Offset off) {
			return off == r;
		}
		template<typename OS>
		OS& operator<<(OS& os, const ModRange& m)
		{
			os << m.id() << ": [" << m.low() << ", " << m.high() << ")";
			return os;
		}


	}//namespace SymtabAPI

}//namespace Dyninst


#endif
