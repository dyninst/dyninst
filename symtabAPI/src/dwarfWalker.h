#include "common/src/vgannotations.h"
#include <common/src/debug_common.h>

#if !defined(_dwarf_walker_h_)
#define _dwarf_walker_h_


#include "elf.h"
#include "libelf.h"
#include "elfutils/libdw.h"
#include <stddef.h>
#include <stdint.h>
#include <utility>
#include <stack>
#include <vector>
#include <string>
#include <set>
#include "dyntypes.h"
#include "VariableLocation.h"
#include "Type.h"
#include "Object.h"
#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <Collections.h>

//Concurrent Hash Map
#include "concurrent.h"

namespace Dyninst {
	namespace SymtabAPI {
		typedef struct {
			Dwarf_Off off;
			bool file;
			Module * m;
		} type_key;
		inline bool operator==(type_key const& k1, type_key const& k2) {
			return k1.off==k2.off && k1.file==k2.file && k1.m==k2.m;
		}
	}
	namespace concurrent {
		template<>
		struct hasher<SymtabAPI::type_key> {
			size_t operator()(const SymtabAPI::type_key& k) const {
				size_t seed = 0;
				boost::hash_combine(seed, k.off);
				boost::hash_combine(seed, k.file);
				boost::hash_combine(seed, static_cast<void *>(k.m));
				return seed;
			}
		};
	}
}

namespace Dyninst {
namespace SymtabAPI {

// A restructuring of walkDwarvenTree

class Symtab;
class Module;
class Object;
class Function;
class FunctionBase;
class typeCommon;
class typeEnum;
class fieldListType;
class typeCollection;
class Type;

using namespace std;

class DwarfParseActions {

protected:
    Dwarf* dbg() { return dbg_; }

    Module *& mod() { return mod_; } 
    typeCollection *tc() { return typeCollection::getModTypeCollection(mod()); }

private:
    Module *mod_;
    Dwarf* dbg_;
public:
    DwarfParseActions(Symtab* s, Dwarf* d) :
        mod_(NULL),
        dbg_(d),
        symtab_(s)
{}
    DwarfParseActions(const DwarfParseActions& o) :
            mod_(o.mod_),
            dbg_(o.dbg_),
            c(o.c), symtab_(o.symtab_)
            {
            }
    virtual ~DwarfParseActions() = default;
    typedef std::vector<std::pair<Address, Address> > range_set_t;
    typedef boost::shared_ptr<std::vector<std::pair<Address, Address> > > range_set_ptr;
private:
    struct Context {
        FunctionBase *func{};
        boost::shared_ptr<Type> commonBlock{};
        boost::shared_ptr<Type> enumType{};
        boost::shared_ptr<Type> enclosure{};
        bool parseSibling{true};
        bool parseChild{true};
        Dwarf_Die offset{};
        Dwarf_Die specEntry{};
        Dwarf_Die abstractEntry{};
        unsigned int tag{};
        Address base{};
        range_set_ptr ranges{};
    };

    std::stack<Context> c;

public:
    void push(bool dissociate_context=false);
    void pop();
    int stack_size() const {return c.size(); }
    FunctionBase *curFunc() { return c.top().func; }
    virtual std::vector<VariableLocation>& getFramePtrRefForInit();
    virtual void addMangledFuncName(std::string);
    virtual void addPrettyFuncName(std::string);
    boost::shared_ptr<Type> curCommon() { return c.top().commonBlock; }
    boost::shared_ptr<Type> curEnum() { return c.top().enumType; }
    boost::shared_ptr<Type> curEnclosure() { return c.top().enclosure; }
    bool parseSibling() { return c.top().parseSibling; }
    bool parseChild() { return c.top().parseChild; }
    Dwarf_Die entry() {
        return c.top().offset;
    }
    Dwarf_Die specEntry() {
        return c.top().specEntry;
    }
    Dwarf_Die abstractEntry() {
        return c.top().abstractEntry;
    }
    Dwarf_Off offset() { return dwarf_dieoffset(&(c.top().offset)); }
    unsigned int tag() { return c.top().tag; }
    Address base() { return c.top().base; }
    range_set_ptr ranges() { return c.top().ranges; }

    void setFunc(FunctionBase *f);
    void setCommon(boost::shared_ptr<Type> tc) { c.top().commonBlock = tc; }
    void setEnum(boost::shared_ptr<Type> e) { c.top().enumType = e; }
    void setEnclosure(boost::shared_ptr<Type> f) { c.top().enclosure = f; }
    void setParseSibling(bool p) { c.top().parseSibling = p; }
    void setParseChild(bool p) { c.top().parseChild = p; }
    virtual void setEntry(Dwarf_Die e) { c.top().offset = e; }
    void setSpecEntry(Dwarf_Die e) { c.top().specEntry = e; }
    void setAbstractEntry(Dwarf_Die e) { c.top().abstractEntry = e; }
    void setTag(unsigned int t) { c.top().tag = t; }
    void setBase(Address a) { c.top().base = a; }
    virtual void setRange(const AddressRange& range) {
        if (range.first >= range.second) {
//            std:: cerr << "Discarding invalid range: "
//                       << std::hex << range.first << " - " << range.second << std::dec << std::endl;
            return;
        }
        if (!c.top().ranges)
            c.top().ranges = range_set_ptr(new std::vector<std::pair<Address, Address> >);
        c.top().ranges->push_back(range);
    }
    void clearRanges() {
        c.top().ranges = range_set_ptr();
    }
    void clearFunc();
    virtual std::string filename() const
    {
        return symtab()->file();
    }
    virtual Dyninst::Architecture getArchitecture() const
    {
        return symtab()->getArchitecture();
    }
    virtual Offset convertDebugOffset(Offset from);
    virtual void setFuncReturnType() {}
    virtual Symbol* findSymbolByName(std::string name, Symbol::SymbolType type)
    {
        std::vector<Symbol *> syms;
        if (!symtab()->findSymbol(syms,
                    name,
                    type,
                    anyName)) {

            return NULL;
        }
        return syms[0];
    }
protected:
    Symtab* symtab() const  { return symtab_; }
protected:
    Symtab *symtab_;
    virtual Object * obj() const ;

    // Function object of current subprogram being parsed; used to detect
    // parseSubprogram recursion
    FunctionBase *currentSubprogramFunction = nullptr;

}; // class DwarfParseActions 

struct ContextGuard {
    DwarfParseActions& walker;
    ContextGuard(DwarfParseActions& w, bool dissociate_context): walker(w)
    {
        walker.push(dissociate_context);
    }
    ~ContextGuard() { walker.pop(); }
};

class DwarfWalker : public DwarfParseActions {

public:
    typedef enum {
        NoError

    } Error;

    using ParsedFuncs = Dyninst::dyn_c_hash_map<FunctionBase *, bool>;

    DwarfWalker(Symtab *symtab, Dwarf* dbg, std::shared_ptr<ParsedFuncs> pf = nullptr);

    DwarfWalker(const DwarfWalker& o) :
            DwarfParseActions(o),
            current_cu_die(o.current_cu_die),
            parsedFuncs(o.parsedFuncs),
            name_(o.name_),
            is_mangled_name_(o.is_mangled_name_),
            modLow(o.modLow), modHigh(o.modHigh),
            cu_header_length(o.cu_header_length),
            abbrev_offset(o.abbrev_offset),
            addr_size(o.addr_size),
            offset_size(o.offset_size),
            extension_size(o.extension_size),
            signature(o.signature),
            typeoffset(o.typeoffset),
            next_cu_header(o.next_cu_header),
            compile_offset(o.compile_offset),
            info_type_ids_(o.info_type_ids_),
            types_type_ids_(o.types_type_ids_),
            sig8_type_ids_(o.sig8_type_ids_)
            {
                if (!parsedFuncs)  {
                    parsedFuncs = std::make_shared<ParsedFuncs>();
                }
            }

    virtual ~DwarfWalker();

    bool parse();

    // Takes current debug state as represented by dbg_;
    bool parseModule(Dwarf_Die is_info, Module *&fixUnknownMod);

    // Non-recursive version of parse
    // A Context must be provided as an _input_ to this function,
    // whereas parse creates a context.
    bool parse_int(Dwarf_Die entry, bool parseSiblings,
            bool dissociate_context=false);
    
private:
    Dwarf_Die current_cu_die;

    enum inline_t {
        NormalFunc,
        InlinedFunc
    };

    bool parseSubprogram(inline_t func_type);
    bool parseLexicalBlock();
    bool parseTryBlock();
    bool parseCatchBlock();
    bool parseRangeTypes(Dwarf_Die die);
    bool parseCommonBlock();
    bool parseConstant();
    virtual bool parseVariable();
    bool parseFormalParam();
    bool parseBaseType();
    bool parseTypedef();
    bool parseArray();
    bool parseSubrange();
    bool parseEnum();
    bool parseInheritance();
    bool parseStructUnionClass();
    bool parseEnumEntry();
    bool parseMember();
    bool parseConstPackedVolatile();
    bool parseTypeReferences();

    // These vary as we parse the tree


    // This is a handy scratch space that is cleared for each parse.
    std::string &curName() { return name_; }
    bool isMangledName() { return is_mangled_name_; }
    void setMangledName(bool b) { is_mangled_name_ = b; }
    bool nameDefined() { return name_ != ""; }
    // These are invariant across a parse

    StringTablePtr srcFiles() { return mod()->getStrings(); }

    // For functions and variables with a separate specification, a
    // pointer to that spec. For everyone else, this points to entry
    // We might be able to fold this into specEntry and call it
    // "authoritativeEntry" or something.
    bool hasRanges() { return ranges() != NULL; }
    size_t rangesSize() { return ranges()->size(); }
    range_set_t::iterator ranges_begin() { return ranges()->begin(); }
    range_set_t::iterator ranges_end() { return ranges()->end(); }

    // A printable ID for a particular entry
    unsigned long id() { return (unsigned long) (offset() - compile_offset); }
public:
    static bool buildSrcFiles(Dwarf* dbg, Dwarf_Die entry, StringTablePtr strings);
private:

    bool parseCallsite();
    bool hasDeclaration(bool &decl);
    bool findTag();
    bool handleAbstractOrigin(bool &isAbstractOrigin);
    bool handleSpecification(bool &hasSpec);
    bool findFuncName();
    bool setFunctionFromRange(inline_t func_type);
    virtual void setEntry(Dwarf_Die e);
    bool getFrameBase();
    bool getReturnType(boost::shared_ptr<Type>&returnType);
    bool addFuncToContainer(boost::shared_ptr<Type> returnType);
    bool isStaticStructMember(std::vector<VariableLocation> &locs, bool &isStatic);
    virtual bool findType(boost::shared_ptr<Type>&, bool defaultToVoid);
    bool findAnyType(Dwarf_Attribute typeAttribute,
            bool is_info, boost::shared_ptr<Type>&type);
    bool findDieOffset(Dwarf_Attribute attr, Dwarf_Off &offset);
    bool getLineInformation(Dwarf_Word &variableLineNo,
            bool &hasLineNumber,
            std::string &filename);
private:
    std::string die_name();
    void removeFortranUnderscore(std::string &);
    bool findSize(unsigned &size);
    bool findVisibility(visibility_t &visibility);
    boost::optional<long> findConstValue();
    bool fixName(std::string &name, boost::shared_ptr<Type> type);
    bool fixBitFields(std::vector<VariableLocation> &locs, long &size);

    boost::shared_ptr<typeSubrange> parseSubrange(Dwarf_Die *entry);
    bool decodeLocationList(Dwarf_Half attr,
            Address *initialVal,
            std::vector<VariableLocation> &locs);
    bool checkForConstantOrExpr(Dwarf_Half attr,
            Dwarf_Attribute &locationAttribute,
            bool &constant,
            bool &expr,
            Dwarf_Half &form);
    boost::optional<std::string> find_call_file();
public:
    static bool findConstant(Dwarf_Half attr, Address &value, Dwarf_Die *entry, Dwarf *dbg);
    static bool findConstantWithForm(Dwarf_Attribute &attr, Dwarf_Half form,
            Address &value);
    static std::vector<AddressRange> getDieRanges(Dwarf_Die die);
private:
    bool decodeConstantLocation(Dwarf_Attribute &attr, Dwarf_Half form,
            std::vector<VariableLocation> &locs);
    bool constructConstantVariableLocation(Address value,
            std::vector<VariableLocation> &locs);
    boost::shared_ptr<typeArray> parseMultiDimensionalArray(Dwarf_Die *firstRange,
                                          boost::shared_ptr<Type> elementType);

    bool decodeExpression(Dwarf_Attribute &attr,
            std::vector<VariableLocation> &locs);

    typedef struct {
        Dwarf_Addr ld_lopc, ld_hipc;
        Dwarf_Op * dwarfOp;
        size_t opLen;
    }LocDesc;

    bool decodeLocationListForStaticOffsetOrAddress(
            std::vector<LocDesc>& locationList,
            Dwarf_Sword listLength,
            std::vector<VariableLocation>& locs,
            Address * initialStackValue = NULL);


    // Map of Function* to bool (indicates function parsed)
    std::shared_ptr<ParsedFuncs> parsedFuncs;
private:
    std::string name_;
    bool is_mangled_name_;

    // Per-module info
    Address modLow;
    Address modHigh;
    size_t  cu_header_length;
    Dwarf_Word abbrev_offset;
    uint8_t /*Dwarf_Half*/ addr_size;
    uint8_t /*Dwarf_Half*/ offset_size;
    Dwarf_Half extension_size;

    typedef struct{
        char signature[8];
    } Dwarf_Sig8;

    Dwarf_Sig8 signature;

    Dwarf_Word typeoffset;
    Dwarf_Word next_cu_header;

    // For debugging purposes; to match dwarfdump's output,
    // we need to subtract a "header overall offset".
    Dwarf_Off compile_offset;

    typedef dyn_c_hash_map<type_key, typeId_t> type_map;
    // Type IDs are just int, but Dwarf_Off is 64-bit and may be relative to
    // either .debug_info or .debug_types.
    type_map info_type_ids_; // .debug_info offset -> id
    type_map types_type_ids_; // .debug_types offset -> id

    // is_sup indicates it's in the dwarf supplemental file
    typeId_t get_type_id(Dwarf_Off offset, bool is_info, bool is_sup);
    typeId_t type_id(); // get_type_id() for the current entry

    // Map to connect DW_FORM_ref_sig8 to type IDs.
    dyn_c_hash_map<uint64_t, typeId_t> sig8_type_ids_;

    bool parseModuleSig8(bool is_info);
    void findAllSig8Types();
    bool findSig8Type(Dwarf_Sig8 * signature, boost::shared_ptr<Type>&type);
    unsigned int getNextTypeId();
protected:
    virtual void setFuncReturnType();

    virtual void createLocalVariable(const std::vector<VariableLocation> &locs, boost::shared_ptr<Type> type,
            Dwarf_Word variableLineNo,
            const std::string &fileName);

    virtual bool createInlineFunc();

    virtual void setFuncFromLowest(Address lowest);

    virtual void createParameter(const std::vector<VariableLocation> &locs, 
            boost::shared_ptr<Type> paramType, Dwarf_Word lineNo, const std::string &fileName);

    virtual void setRanges(FunctionBase *func);

    virtual void createGlobalVariable(const std::vector<VariableLocation> &locs, boost::shared_ptr<Type> type);

    virtual bool addStaticClassVariable(const std::vector<VariableLocation> &locs, boost::shared_ptr<Type> type);

    virtual boost::shared_ptr<Type> getCommonBlockType(std::string &commonBlockName);

    virtual Symbol *findSymbolForCommonBlock(const std::string &commonBlockName);

}; // class DwarfWalker

} // namespace SymtabAPI 
} // namespace Dyninst 

#endif
