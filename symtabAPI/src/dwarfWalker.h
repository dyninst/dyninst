
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
   class typeCommon;
   class typeEnum;
   class fieldListType;
   class typeCollection;
   class Type;

class DwarfWalker {

   struct Contexts {
      struct Context {
         Function *func;
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
         Address low;
         Address high;
      Context() :
         func(NULL), commonBlock(NULL),
            enumType(NULL), enclosure(NULL),
            parseSibling(true), parseChild(true), 
            base(0), low(0), high(0) {};
      };
      
      std::stack<Context> c;
      void push();
      void pop();
      Function *curFunc() { return c.top().func; }
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
      Address low() { return c.top().low; }
      Address high() { return c.top().high; }

      void setFunc(Function *f) { c.top().func = f; }
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
      void setLow(Address a) { c.top().low = a; }
      void setHigh(Address a) { c.top().high = a; }
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
   bool parseModule(Module *&fixUnknownMod);
   
   // Non-recursive version of parse
   // A Context must be provided as an _input_ to this function,
   // whereas parse creates a context.
   bool parse_int(Dwarf_Die entry, bool parseSiblings);

   bool parseSubprogram();
   bool parseLexicalBlock();
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

   // These vary as we parse the tree
   Function *curFunc() { return contexts_.curFunc(); }
   typeCommon *curCommon() { return contexts_.curCommon(); }
   typeEnum *curEnum() { return contexts_.curEnum(); }
   fieldListType *curEnclosure() { return contexts_.curEnclosure(); }

   void setFunc(Function *f) { contexts_.setFunc(f); }
   void setCommon(typeCommon *c) { contexts_.setCommon(c); }
   void setEnum(typeEnum *e) { contexts_.setEnum(e); }
   void setEnclosure(fieldListType *f) { contexts_.setEnclosure(f); }

   // This is a handy scratch space that is cleared for each parse. 
   std::string &curName() { return name_; }
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
   Address lowAddr() { return contexts_.low(); }
   Address highAddr() { return contexts_.high(); }

   // A printable ID for a particular entry
   unsigned long id() { return (unsigned long) (offset() - compile_offset); }

   void setEntry(Dwarf_Die entry);
   void setSpecEntry(Dwarf_Die se) { contexts_.setSpecEntry(se); }
   void setAbstractEntry(Dwarf_Die se) { contexts_.setAbstractEntry(se); }
   void setTag(Dwarf_Half tag) { contexts_.setTag(tag); }
   void setOffset(Dwarf_Off offset) { contexts_.setOffset(offset); }
   void setLow(Address low) { contexts_.setLow(low); }
   void setHigh(Address high) { contexts_.setHigh(high); }

   bool buildSrcFiles(Dwarf_Die entry);
   bool hasDeclaration(bool &decl);
   bool findTag();
   bool findOffset();
   bool handleAbstractOrigin(bool &isAbstractOrigin);
   bool handleSpecification(bool &hasSpec);
   bool findFuncName();
   bool findFunction(bool &found);
   bool findBaseAddr();
   bool getFrameBase();
   bool getReturnType(bool hasSpecification, Type *&returnType);
   bool addFuncToContainer(Type *returnType);
   bool findType(Type *&, bool defaultToVoid);
   bool getLineInformation(Dwarf_Unsigned &variableLineNo,
                           bool &hasLineNumber,
                           std::string &filename); 
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
   bool checkForConstant(Dwarf_Half attr,
                         Dwarf_Attribute &locationAttribute,
                         bool &constant,
                         Dwarf_Half &form);
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
   bool decipherBound(Dwarf_Attribute boundAttribute, std::string &name);
   bool decodeLocationListForStaticOffsetOrAddress(Dwarf_Locdesc **locationList, 
                                                   Dwarf_Signed listLength, 
                                                   std::vector<VariableLocation>& locs, 
                                                   Address * initialStackValue = NULL);
   void deallocateLocationList(Dwarf_Locdesc *locationList,
                               Dwarf_Signed listLength);
   void deallocateLocationList(Dwarf_Locdesc **locationList,
                               Dwarf_Signed listLength);

   // Track which enclosure (array, struct, class, etc.) contains the current
   // dwarf parsee
   std::map<Dwarf_Off, fieldListType *> enclosureMap;
   // Header-only functions get multiple parsed.
   std::set<Function *> parsedFuncs;
   
   Contexts contexts_;

   Dwarf_Debug &dbg_;
   Module *mod_;
   Symtab *symtab_;
   std::vector<std::string> srcFiles_;
   typeCollection *tc_;

   std::string name_;

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

};

};
};

#endif
