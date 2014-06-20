
#if !defined(_dwarf_walker_h_)
#define _dwarf_walker_h_

#include "elf.h"
#include "libelf.h"
#include "libdwarf.h"
#include <stack>
#include <vector>
#include <string>
#include <set>
#include "dyntypes.h"
#include "VariableLocation.h"
#include "Type.h"

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

class DwarfWalker {
   typedef std::vector<std::pair<Address, Address> > range_set_t;
   typedef boost::shared_ptr<std::vector<std::pair<Address, Address> > > range_set_ptr;

   struct Contexts {
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
         Dwarf_Half tag;
         Address base;
         range_set_ptr ranges;
         Context() :
            func(NULL), commonBlock(NULL),
            enumType(NULL), enclosure(NULL),
            parseSibling(true), parseChild(true), 
            entry(NULL), specEntry(NULL), abstractEntry(NULL),
            offset(0), tag(0), base(0) {};
      };
      
      std::stack<Context> c;
      void push();
      void pop();
      FunctionBase *curFunc() { return c.top().func; }
      typeCommon * curCommon() { return c.top().commonBlock; }
      typeEnum *curEnum() { return c.top().enumType; }
      fieldListType *curEnclosure() { return c.top().enclosure; }
      bool parseSibling() { return c.top().parseSibling; }
      bool parseChild() { return c.top().parseChild; }
      Dwarf_Die entry() { return c.top().entry; }
      Dwarf_Die specEntry() { return c.top().specEntry; }
      Dwarf_Die abstractEntry() { return c.top().abstractEntry; }
      Dwarf_Off offset() { return c.top().offset; }
      Dwarf_Half tag() { return c.top().tag; }
      Address base() { return c.top().base; }
      range_set_ptr ranges() { return c.top().ranges; }

      void setFunc(FunctionBase *f); 
      void setCommon(typeCommon *tc) { c.top().commonBlock = tc; }
      void setEnum(typeEnum *e) { c.top().enumType = e; }
      void setEnclosure(fieldListType *f) { c.top().enclosure = f; }
      void setParseSibling(bool p) { c.top().parseSibling = p; }
      void setParseChild(bool p) { c.top().parseChild = p; }
      void setEntry(Dwarf_Die e) { c.top().entry = e; }
      void setSpecEntry(Dwarf_Die e) { c.top().specEntry = e; }
      void setAbstractEntry(Dwarf_Die e) { c.top().abstractEntry = e; }
      void setOffset(Dwarf_Off o) { c.top().offset = o; }
      void setTag(Dwarf_Tag t) { c.top().tag = t; }
      void setBase(Address a) { c.top().base = a; }
      void setRange(std::pair<Address, Address> range) { 
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
   };

   struct ContextGuard {
     Contexts& c;
     ContextGuard(Contexts& c): c(c) { c.push(); }
     ~ContextGuard() { c.pop(); }
   };

  public:
   typedef enum {
      NoError

   } Error;

   DwarfWalker(Symtab *symtab, Dwarf_Debug &dbg);

   ~DwarfWalker();

   bool parse();

  private:
   bool setup(Dwarf_Die dieEntry,
              Dwarf_Off modOffset,
              Address lowpc);

   // Takes current debug state as represented by dbg_;
   bool parseModule(Dwarf_Bool is_info, Module *&fixUnknownMod);
   
   // Non-recursive version of parse
   // A Context must be provided as an _input_ to this function,
   // whereas parse creates a context.
   bool parse_int(Dwarf_Die entry, bool parseSiblings);

   enum inline_t {
      NormalFunc,
      InlinedFunc
   };

   bool parseSubprogram(inline_t func_type);
   bool parseLexicalBlock();
   bool parseRangeTypes();
   bool parseCommonBlock();
   bool parseConstant();
   bool parseVariable();
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
   bool parseHighPCLowPC();
   

   // These vary as we parse the tree
   FunctionBase *curFunc() { return contexts_.curFunc(); }
   typeCommon *curCommon() { return contexts_.curCommon(); }
   typeEnum *curEnum() { return contexts_.curEnum(); }
   fieldListType *curEnclosure() { return contexts_.curEnclosure(); }

   void setFunc(FunctionBase *f) { contexts_.setFunc(f); }
   void setCommon(typeCommon *c) { contexts_.setCommon(c); }
   void setEnum(typeEnum *e) { contexts_.setEnum(e); }
   void setEnclosure(fieldListType *f) { contexts_.setEnclosure(f); }

   // This is a handy scratch space that is cleared for each parse. 
   std::string &curName() { return name_; }
   bool isMangledName() { return is_mangled_name_; }
   void setMangledName(bool b) { is_mangled_name_ = b; }
   bool nameDefined() { return name_ != ""; }
   // These are invariant across a parse
   Object *obj(); 
   Symtab *symtab() { return symtab_; }
   Module *mod() { return mod_; }
   std::vector<std::string> &srcFiles() { return srcFiles_; }
   typeCollection *tc() { return tc_; }
   Dwarf_Debug &dbg() { return dbg_; }

   bool parseSibling() { return contexts_.parseSibling(); }
   bool parseChild() { return contexts_.parseChild(); }
   void setParseSibling(bool p) { return contexts_.setParseSibling(p); }
   void setParseChild(bool p) { return contexts_.setParseChild(p); }

   Dwarf_Half tag() { return contexts_.tag(); }
   Dwarf_Off offset() { return contexts_.offset(); }
   Dwarf_Die entry() { return contexts_.entry(); }
   // For functions and variables with a separate specification, a 
   // pointer to that spec. For everyone else, this points to entry
   Dwarf_Die specEntry() { return contexts_.specEntry(); }
   // We might be able to fold this into specEntry and call it
   // "authoritativeEntry" or something. 
   Dwarf_Die abstractEntry() { return contexts_.abstractEntry(); }
   void clearRanges() { contexts_.clearRanges(); }
   bool hasRanges() { return contexts_.ranges() != NULL; }
   size_t rangesSize() { return contexts_.ranges()->size(); }
   range_set_t::iterator ranges_begin() { return contexts_.ranges()->begin(); }
   range_set_t::iterator ranges_end() { return contexts_.ranges()->end(); }

   // A printable ID for a particular entry
   unsigned long id() { return (unsigned long) (offset() - compile_offset); }

   void setEntry(Dwarf_Die entry);
   void setSpecEntry(Dwarf_Die se) { contexts_.setSpecEntry(se); }
   void setAbstractEntry(Dwarf_Die se) { contexts_.setAbstractEntry(se); }
   void setTag(Dwarf_Half tag) { contexts_.setTag(tag); }
   void setOffset(Dwarf_Off offset) { contexts_.setOffset(offset); }
   void setRange(std::pair<Address, Address> range) { contexts_.setRange(range); }

   bool parseCallsite();
   bool buildSrcFiles(Dwarf_Die entry);
   bool hasDeclaration(bool &decl);
   bool findTag();
   bool findOffset();
   bool handleAbstractOrigin(bool &isAbstractOrigin);
   bool handleSpecification(bool &hasSpec);
   bool findFuncName();
   bool findBaseAddr();
   bool setFunctionFromRange(inline_t func_type);
   bool getFrameBase();
   bool getReturnType(bool hasSpecification, Type *&returnType);
   bool addFuncToContainer(Type *returnType);
   bool findType(Type *&, bool defaultToVoid);
   bool findAnyType(Dwarf_Attribute typeAttribute,
                    Dwarf_Bool is_info, Type *&type);
   bool findDieOffset(Dwarf_Attribute attr, Dwarf_Off &offset);
   bool getLineInformation(Dwarf_Unsigned &variableLineNo,
                           bool &hasLineNumber,
                           std::string &filename); 
   bool findDieName(Dwarf_Die die, std::string &);
   bool findName(std::string &);
   void removeFortranUnderscore(std::string &);
   bool findSize(unsigned &size);
   bool findVisibility(visibility_t &visibility);
   bool findValue(long &value, bool &valid);
   bool fixName(std::string &name, Type *type);
   bool fixBitFields(std::vector<VariableLocation> &locs, long &size);
   bool findEntryToUse(Dwarf_Half attr, bool &found, Dwarf_Die &entry);
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
   bool findConstant(Dwarf_Half attr, Address &value);
   bool findConstantWithForm(Dwarf_Attribute &attr,
                               Dwarf_Half form,
                               Address &value);
   bool decodeConstantLocation(Dwarf_Attribute &attr, Dwarf_Half form, 
                               std::vector<VariableLocation> &locs);
   bool constructConstantVariableLocation(Address value,
                                          std::vector<VariableLocation> &locs);
   typeArray *parseMultiDimensionalArray(Dwarf_Die firstRange,
                                         Type *elementType);
   bool decipherBound(Dwarf_Attribute boundAttribute, Dwarf_Bool is_info,
                      std::string &name);

   bool decodeExpression(Dwarf_Attribute &attr,
			 std::vector<VariableLocation> &locs);

   bool decodeLocationListForStaticOffsetOrAddress(Dwarf_Locdesc **locationList, 
                                                   Dwarf_Signed listLength, 
                                                   std::vector<VariableLocation>& locs, 
                                                   Address * initialStackValue = NULL);
   void deallocateLocationList(Dwarf_Locdesc *locationList,
                               Dwarf_Signed listLength);
   void deallocateLocationList(Dwarf_Locdesc **locationList,
                               Dwarf_Signed listLength);

   // A BG-Q XLC hack; clear any function-level enclosure from the context stack
   // to handle a bug where they don't finish off functions. 
   void clearFunc() { contexts_.clearFunc(); }

   // Header-only functions get multiple parsed.
   std::set<FunctionBase *> parsedFuncs;
   
   Contexts contexts_;

   Dwarf_Debug &dbg_;
   Module *mod_;
   Symtab *symtab_;
   std::vector<std::string> srcFiles_;
   typeCollection *tc_;

   std::string name_;
   bool is_mangled_name_;

   // Per-module info
   Address modLow;
   Address modHigh;
   Dwarf_Unsigned cu_header_length;
   Dwarf_Half version;
   Dwarf_Unsigned abbrev_offset;
   Dwarf_Half addr_size;
   Dwarf_Half offset_size;
   Dwarf_Half extension_size;
   Dwarf_Sig8 signature;
   Dwarf_Unsigned typeoffset;
   Dwarf_Unsigned next_cu_header;

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
   bool parseModuleSig8(Dwarf_Bool is_info);
   void findAllSig8Types();
   bool findSig8Type(Dwarf_Sig8 *signature, Type *&type);
};

};
};

#endif
