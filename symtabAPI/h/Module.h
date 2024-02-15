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

#include "Annotatable.h"
#include "RangeLookup.h"
#include "Statement.h"
#include "Symbol.h"

#include <set>
#include <stddef.h>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace Dyninst { namespace SymtabAPI {

  class typeCollection;
  class LineInformation;
  class localVar;
  class Symtab;

  typedef Dyninst::SimpleInterval<Offset, Module *> ModRange;

  class SYMTAB_EXPORT Module : public LookupInterface {
    friend class Symtab;

  public:
    Module();
    Module(supportedLanguages lang, Offset adr, std::string fullNm, Symtab *img);
    Module(const Module &mod);
    bool operator==(Module &mod);

    const std::string &fileName() const;
    const std::string &fullName() const;

    supportedLanguages language() const;
    void setLanguage(supportedLanguages lang);

    Offset addr() const;
    Symtab *exec() const;

    bool isShared() const;
    ~Module();

    // Symbol output methods
    virtual bool findSymbol(std::vector<Symbol *> &ret, const std::string &name,
                            Symbol::SymbolType sType = Symbol::ST_UNKNOWN,
                            NameType nameType = anyName, bool isRegex = false,
                            bool checkCase = false, bool includeUndefined = false);
    virtual bool getAllSymbolsByType(std::vector<Symbol *> &ret, Symbol::SymbolType sType);
    virtual bool getAllSymbols(std::vector<Symbol *> &ret);

    // Function based methods
    std::vector<Function*> getAllFunctions() const;

    // Variable based methods
    bool findVariablesByOffset(std::vector<Variable *> &ret, const Offset offset);
    bool findVariablesByName(std::vector<Variable *> &ret, const std::string &name,
                             NameType nameType = anyName, bool isRegex = false,
                             bool checkCase = true);

    // Type output methods
    virtual bool findType(boost::shared_ptr<Type> &type, std::string name);

    bool findType(Type *&t, std::string n) {
      boost::shared_ptr<Type> tp;
      auto r = findType(tp, n);
      t = tp.get();
      return r;
    }

    virtual bool findVariableType(boost::shared_ptr<Type> &type, std::string name);

    bool findVariableType(Type *&t, std::string n) {
      boost::shared_ptr<Type> tp;
      auto r = findVariableType(tp, n);
      t = tp.get();
      return r;
    }

    void getAllTypes(std::vector<boost::shared_ptr<Type>> &);

    std::vector<Type *> *getAllTypes() {
      std::vector<boost::shared_ptr<Type>> v;
      getAllTypes(v);
      auto r = new std::vector<Type *>(v.size());
      for (std::size_t i = 0; i < v.size(); i++)
        (*r)[i] = v[i].get();
      return r;
    }

    void getAllGlobalVars(std::vector<std::pair<std::string, boost::shared_ptr<Type>>> &);

    std::vector<std::pair<std::string, Type *>> *getAllGlobalVars() {
      std::vector<std::pair<std::string, boost::shared_ptr<Type>>> v;
      getAllGlobalVars(v);
      auto r = new std::vector<std::pair<std::string, Type *>>(v.size());
      for (std::size_t i = 0; i < v.size(); i++)
        (*r)[i] = {v[i].first, v[i].second.get()};
      return r;
    }

    typeCollection *getModuleTypes();

    /***** Local Variable Information *****/
    bool findLocalVariable(std::vector<localVar *> &vars, std::string name);

    /***** Line Number Information *****/
    bool getAddressRanges(std::vector<AddressRange> &ranges, std::string lineSource,
                          unsigned int LineNo);
    bool getSourceLines(std::vector<Statement::Ptr> &lines, Offset addressInRange);
    bool getSourceLines(std::vector<LineNoTuple> &lines, Offset addressInRange);
    bool getStatements(std::vector<Statement::Ptr> &statements);
    LineInformation *getLineInformation();
    LineInformation *parseLineInformation();

    bool setDefaultNamespacePrefix(std::string str);

    //  Super secret private methods that aren't really private
    typeCollection *getModuleTypesPrivate();

    void setModuleTypes(typeCollection *tc) { typeInfo_ = tc; }

    bool setLineInfo(Dyninst::SymtabAPI::LineInformation *lineInfo);
    void addRange(Dyninst::Address low, Dyninst::Address high);

    bool hasRanges() const { return !ranges.empty(); }

    StringTablePtr &getStrings();

  private:
    bool objectLevelLineInfo;
    Dyninst::SymtabAPI::LineInformation *lineInfo_;
    typeCollection *typeInfo_;

    std::string fileName_; // full path to file
    std::string compDir_;
    supportedLanguages language_;
    Offset addr_; // starting address of module
    Symtab *exec_;
    std::set<AddressRange> ranges;
    std::vector<ModRange *> finalizeRanges();

    StringTablePtr strings_;
  };

  template <typename OS> OS &operator<<(OS &os, const Module &m) {
    os << m.fileName() << ": " << m.addr();
    return os;
  }

  template <typename OS> OS &operator<<(OS &os, Module *m) {
    if (m)  {
	os << m->fileName() << ": " << m->addr();
    }  else  {
	os << "null";
    }
    return os;
  }

  inline bool operator==(Offset off, const ModRange &r) {
    return (r.low() <= off) && (off < r.high());
  }

  inline bool operator==(const ModRange &r, Offset off) { return off == r; }

  template <typename OS> OS &operator<<(OS &os, const ModRange &m) {
    os << m.id() << ": [" << m.low() << ", " << m.high() << ")";
    return os;
  }

}}

#endif
