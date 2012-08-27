
#if !defined(_dwarf_walker_h_)
#define _dwarf_walker_h_

#include "elf.h"
#include "libelf.h"
#include "libdwarf.h"
#include <stack>
#include <vector>
#include <string>
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

class dwarfWalker {

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
      Context() :
         func(NULL), commonBlock(NULL),
            enumType(NULL), enclosure(NULL),
            parseSibling(true), parseChild(true) {};
      };
      
      std::stack<Context> contexts;
      void push();
      void pop();
      Function *curFunc();
      typeCommon * curCommon();
      typeEnum *curEnum();
      fieldListType *curEnclosure();
      bool parseSibling();
      bool parseChild();
      Dwarf_Die entry();
      Dwarf_Die specEntry();

      void setFunc(Function *);
      void setCommon(typeCommon *);
      void setEnum(typeEnum *);
      void setEnclosure(fieldListType *);
      void setParseSibling(bool);
      void setParseChild(bool);
      void setEntry(Dwarf_Die);
      void setSpecEntry(Dwarf_Die);
   };

  public:
   typedef enum {
      NoError

   } Error;

   dwarfWalker(Dwarf_Debug &dbg,
               Module *mod,
               Symtab *symtab,
               std::vector<std::string> &srcFiles);
   ~dwarfWalker();

   bool parse(Dwarf_Die dieEntry, Address lowpc);

  private:
   bool setup(Dwarf_Die dieEntry,
              Dwarf_Off modOffset,
              Address lowpc);
   
   // Non-recursive version of parse
   // A Context must be provided as an _input_ to this function,
   // whereas parse creates a context.
   bool parse_int(Dwarf_Die entry, bool parseSiblings);

   bool parseSubprogram();
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
   bool nameDefined();

   // These are invariant across a parse
   Object *obj();
   Symtab *symtab() { return symtab_; }
   Module *mod() { return mod_; }
   std::vector<std::string> &srcFiles();
   Address lowaddr() { return lowaddr_; }
   typeCollection *tc() { return tc_; }

   bool parseSibling();
   bool parseChild();
   void setParseSibling(bool);
   void setParseChild(bool);

   Dwarf_Half tag();
   Dwarf_Off offset();
   Dwarf_Die &entry();
   // For functions and variables with a separate specification, a 
   // pointer to that spec. For everyone else, this points to entry
   Dwarf_Die &specEntry();
   // We might be able to fold this into specEntry and call it
   // "authoritativeEntry" or something. 
   Dwarf_Die &abstractEntry();
   Dwarf_Debug &dbg();

   void setEntry(Dwarf_Die entry);
   void setTag(Dwarf_Half tag);
   void setOffset(Dwarf_Off offset);


   bool findTag();
   bool findOffset();
   bool handleAbstractOrigin(bool &isAbstractOrigin, Dwarf_Die &abstractEntry);
   bool handleSpecification(bool &hasSpec, Dwarf_Die &specEntry);
   bool findFuncName();
   bool findFunction();
   bool findLowAddr();
   bool getFrameBase();
   bool getReturnType(bool hasSpecification);
   bool addFuncToContainer(fieldListType *dieEnclosure);
   bool findType(Type *&, bool defaultToVoid);
   bool getLineInformation(Dwarf_Unsigned &variableLineNo,
                           bool &hasLineNumber,
                           std::string &filename); 
   bool findName(std::string &);
   void removeFortranUnderscore(std::string &);
   bool getLocationList(std::vector<VariableLocation> &locs);
   bool findSize(unsigned &size);
   bool findVisibility(visibility_t &visibility);
   bool findValue(long &value, bool &valid);
   bool fixName(std::string &name, Type *type);
   bool fixBitFields(std::vector<VariableLocation> &locs, long &size);
   bool decodeLocationList(Dwarf_Half attr,
                           Address *initialVal,
                           std::vector<VariableLocation> &locs);
   typeArray *parseMultiDimensionalArray(Dwarf_Die firstRange,
                                         Type *elementType);
   bool decipherBound(Dwarf_Attribute boundAttribute, std::string &name);
   bool decodeLocationListForStaticOffsetOrAddress(Dwarf_Locdesc **locationList, 
                                                   Dwarf_Signed listLength, 
                                                   Symtab * objFile, 
                                                   vector<VariableLocation>& locs, 
                                                   Address lowpc,
                                                   Address * initialStackValue = NULL);
   void deallocateLocationList(Dwarf_Locdesc *locationList,
                               Dwarf_Signed listLength);
   void deallocateLocationList(Dwarf_Locdesc **locationList,
                               Dwarf_Signed listLength);

   // Track which enclosure (array, struct, class, etc.) contains the current
   // dwarf parsee
   std::map<Dwarf_Off, fieldListType *> enclosureMap;
   
   Contexts contexts_;

   Dwarf_Debug &dbg_;
   Module *mod_;
   Symtab *symtab_;
   std::vector<std::string> &srcFiles_;
   typeCollection *tc_;
   Address lowaddr_;

   std::string name_;
};

};
};

#endif
