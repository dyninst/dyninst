
#include <common/src/debug_common.h>

#if !defined(_dwarf_walker_h_)
#define _dwarf_walker_h_


#include "elf.h"
#include "libelf.h"
#include "elfutils/libdw.h"
#include <stack>
#include <vector>
#include <string>
#include <set>
#include "dyntypes.h"
#include "VariableLocation.h"
#include "Type.h"
#include "Object.h"
#include <boost/shared_ptr.hpp>
#include <Collections.h>

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

class DwarfParseActions {

protected:
    ::Dwarf* dbg() { return dbg_; } 

    Module *& mod() { return mod_; } 

    typeCollection *tc() { return typeCollection::getModTypeCollection(mod()); }

private:
    Module *mod_;
    ::Dwarf* dbg_;
public:
    DwarfParseActions(Symtab* s, ::Dwarf* d) :
        mod_(NULL),
        dbg_(d),
        symtab_(s)
{}
    typedef std::vector<std::pair<Address, Address> > range_set_t;
    typedef boost::shared_ptr<std::vector<std::pair<Address, Address> > > range_set_ptr;
private:
    struct Context {
        FunctionBase *func;
        typeCommon *commonBlock;
        typeEnum *enumType;
        fieldListType *enclosure;
        bool parseSibling;
        bool parseChild;
        Dwarf_Die entry;
        Dwarf_Die specEntry;
        Dwarf_Die abstractEntry;
        Dwarf_Off offset;
        unsigned int tag;
        Address base;
        range_set_ptr ranges;
        Context() :
            func(NULL), commonBlock(NULL),
            enumType(NULL), enclosure(NULL),
            parseSibling(true), parseChild(true),
            /*entry(NULL), specEntry(NULL), abstractEntry(NULL),
            offset(0),*/ tag(0), base(0) {};
    };

    std::stack<Context> c;
public:
    void push();
    void pop();
    int stack_size() const {return c.size(); }
    FunctionBase *curFunc() { return c.top().func; }
    virtual std::vector<VariableLocation>& getFramePtrRefForInit();
    virtual void addMangledFuncName(std::string);
    virtual void addPrettyFuncName(std::string);
    typeCommon * curCommon() { return c.top().commonBlock; }
    typeEnum *curEnum() { return c.top().enumType; }
    fieldListType *curEnclosure() { return c.top().enclosure; }
    bool parseSibling() { return c.top().parseSibling; }
    bool parseChild() { return c.top().parseChild; }
    Dwarf_Die entry() { return c.top().entry; }
    Dwarf_Die specEntry() { return c.top().specEntry; }
    Dwarf_Die abstractEntry() { return c.top().abstractEntry; }
    Dwarf_Off offset() { return c.top().offset; }
    unsigned int tag() { return c.top().tag; }
    Address base() { return c.top().base; }
    range_set_ptr ranges() { return c.top().ranges; }

    void setFunc(FunctionBase *f);
    void setCommon(typeCommon *tc) { c.top().commonBlock = tc; }
    void setEnum(typeEnum *e) { c.top().enumType = e; }
    void setEnclosure(fieldListType *f) { c.top().enclosure = f; }
    void setParseSibling(bool p) { c.top().parseSibling = p; }
    void setParseChild(bool p) { c.top().parseChild = p; }
    virtual void setEntry(Dwarf_Die e) { c.top().entry = e; }
    void setSpecEntry(Dwarf_Die e) { c.top().specEntry = e; }
    void setAbstractEntry(Dwarf_Die e) { c.top().abstractEntry = e; }
    void setOffset(Dwarf_Off o) { c.top().offset = o; }
    void setTag(unsigned int t) { c.top().tag = t; }
    void setBase(Address a) { c.top().base = a; }
    virtual void setRange(std::pair<Address, Address> range) {
        if (range.first >= range.second)
            return;
        if (!c.top().ranges)
            c.top().ranges = range_set_ptr(new std::vector<std::pair<Address, Address> >);
        c.top().ranges->push_back(range);
    }
    void clearRanges() {
        c.top().ranges = range_set_ptr();
    }
    void clearFunc();
    virtual bool isNativeCompiler() const
    {
        return symtab()->isNativeCompiler();
    }
    virtual std::string filename() const
    {
        return symtab()->file();
    }
    virtual void setModuleFromName(std::string moduleName);
    virtual Dyninst::Architecture getArchitecture() const
    {
        return symtab()->getArchitecture();
    }
    virtual Offset convertDebugOffset(Offset from);
    virtual void setFuncReturnType() {};
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

}; // class DwarfParseActions 

struct ContextGuard {
    DwarfParseActions& c;
    ContextGuard(DwarfParseActions& c): c(c) { c.push(); }
    ~ContextGuard() { c.pop(); }
};

class DwarfWalker : public DwarfParseActions {

public:
    typedef enum {
        NoError

    } Error;

    DwarfWalker(Symtab *symtab, ::Dwarf* dbg);

    virtual ~DwarfWalker();

    bool parse();

    // Takes current debug state as represented by dbg_;
    bool parseModule(bool is_info, Module *&fixUnknownMod);

    // Non-recursive version of parse
    // A Context must be provided as an _input_ to this function,
    // whereas parse creates a context.
    bool parse_int(Dwarf_Die entry, bool parseSiblings);
    
    // FIXME: is this function needed?
    //static std::pair<std::vector<Dyninst::SymtabAPI::AddressRange>::iterator, std::vector<Dyninst::SymtabAPI::AddressRange>::iterator>
    //    parseRangeList(Dwarf_Ranges *ranges, Dwarf_Sword num_ranges, Offset initial_base);
private:
    enum inline_t {
        NormalFunc,
        InlinedFunc
    };

    bool parseSubprogram(inline_t func_type);
    bool parseLexicalBlock();
    bool parseRangeTypes(::Dwarf* dbg, Dwarf_Die die);
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
    static std::pair<AddressRange, bool> parseHighPCLowPC(::Dwarf* dbg, Dwarf_Die entry);


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
    static bool buildSrcFiles(::Dwarf* dbg, Dwarf_Die entry, StringTablePtr strings);
private:

    bool parseCallsite();
    bool hasDeclaration(bool &decl);
    bool findTag();
    bool findOffset();
    bool handleAbstractOrigin(bool &isAbstractOrigin);
    bool handleSpecification(bool &hasSpec);
    bool findFuncName();
    bool setFunctionFromRange(inline_t func_type);
    virtual void setEntry(Dwarf_Die e);
    bool getFrameBase();
    bool getReturnType(bool hasSpecification, Type *&returnType);
    bool addFuncToContainer(Type *returnType);
    bool isStaticStructMember(std::vector<VariableLocation> &locs, bool &isStatic);
    virtual bool findType(Type *&, bool defaultToVoid);
    bool findAnyType(Dwarf_Attribute typeAttribute,
            bool is_info, Type *&type);
    bool findDieOffset(Dwarf_Attribute attr, Dwarf_Off &offset);
    bool getLineInformation(Dwarf_Word &variableLineNo,
            bool &hasLineNumber,
            std::string &filename);
public:
    static bool findDieName(::Dwarf* dbg, Dwarf_Die die, std::string &);
private:
    bool findName(std::string &);
    void removeFortranUnderscore(std::string &);
    bool findSize(unsigned &size);
    bool findVisibility(visibility_t &visibility);
    bool findValue(long &value, bool &valid);
    bool fixName(std::string &name, Type *type);
    bool fixBitFields(std::vector<VariableLocation> &locs, long &size);

    bool parseSubrangeAUX(Dwarf_Die entry,
            std::string &lobound,
            std::string &hibound);
    bool decodeLocationList(Dwarf_Half attr,
            Address *initialVal,
            std::vector<VariableLocation> &locs);
    bool checkForConstantOrExpr(Dwarf_Half attr,
            Dwarf_Attribute &locationAttribute,
            bool &constant,
            bool &expr,
            Dwarf_Half &form);
    bool findString(Dwarf_Half attr, std::string &str);
public:
    static bool findConstant(Dwarf_Half attr, Address &value, Dwarf_Die entry, ::Dwarf* dbg);
    static bool findConstantWithForm(Dwarf_Attribute &attr,
            Dwarf_Half form,
            Address &value);
    static std::vector<AddressRange> getDieRanges(::Dwarf* dbg, Dwarf_Die die, Offset base);
private:
    bool decodeConstantLocation(Dwarf_Attribute &attr, Dwarf_Half form,
            std::vector<VariableLocation> &locs);
    bool constructConstantVariableLocation(Address value,
            std::vector<VariableLocation> &locs);
    typeArray *parseMultiDimensionalArray(Dwarf_Die firstRange,
            Type *elementType);
    bool decipherBound(Dwarf_Attribute boundAttribute, bool is_info,
            std::string &name);

    bool decodeExpression(Dwarf_Attribute &attr,
            std::vector<VariableLocation> &locs);

    bool decodeLocationListForStaticOffsetOrAddress(Dwarf_Op* **locationList,
            Dwarf_Sword listLength,
            std::vector<VariableLocation>& locs,
            Address * initialStackValue = NULL);
    void deallocateLocationList(Dwarf_Op* **locationList,
            Dwarf_Sword listLength);


    // Header-only functions get multiple parsed.
    std::set<FunctionBase *> parsedFuncs;
private:
    std::vector<const char*> srcFiles_;
    char** srcFileList_;
    std::string name_;
    bool is_mangled_name_;

    // Per-module info
    Address modLow;
    Address modHigh;
    size_t  cu_header_length;
    Dwarf_Half version;
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

    // Type IDs are just int, but Dwarf_Off is 64-bit and may be relative to
    // either .debug_info or .debug_types.
    dyn_hash_map<Dwarf_Off, typeId_t> info_type_ids_; // .debug_info offset -> id
    dyn_hash_map<Dwarf_Off, typeId_t> types_type_ids_; // .debug_types offset -> id
    typeId_t get_type_id(Dwarf_Off offset, bool is_info);
    typeId_t type_id(); // get_type_id() for the current entry

    // Map to connect DW_FORM_ref_sig8 to type IDs.
    dyn_hash_map<uint64_t, typeId_t> sig8_type_ids_;
    bool parseModuleSig8(bool is_info);
    void findAllSig8Types();
    bool findSig8Type(Dwarf_Sig8 * signature, Type *&type);

protected:
    virtual void setFuncReturnType();

    virtual void createLocalVariable(const std::vector<VariableLocation> &locs, Type *type,
            Dwarf_Word variableLineNo,
            const std::string &fileName);

    virtual bool createInlineFunc();

    virtual void setFuncFromLowest(Address lowest);

    virtual void createParameter(const std::vector<VariableLocation> &locs, 
            Type *paramType, Dwarf_Word lineNo, const std::string &fileName);

    virtual void setRanges(FunctionBase *func);

    virtual void createGlobalVariable(const std::vector<VariableLocation> &locs, Type *type);

    virtual bool addStaticClassVariable(const std::vector<VariableLocation> &locs, Type *type);

    virtual typeCommon *getCommonBlockType(std::string &commonBlockName);

    virtual Symbol *findSymbolForCommonBlock(const std::string &commonBlockName);

}; // class DwarfWalker

}; // namespace SymtabAPI 
}; // namespace Dyninst 

#endif
