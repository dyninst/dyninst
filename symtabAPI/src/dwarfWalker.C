#include "dwarfWalker.h"
#include "common/src/headers.h"
#include "Module.h"
#include "Symtab.h"
#include "Collections.h"
#include "dwarf.h"
#include "Object.h"
#include "Object-elf.h"
#include "Function.h"
#include "dwarf/h/dwarfExprParser.h"
#include "common/src/pathName.h"
#include "common/src/debug_common.h"

using namespace Dyninst;
using namespace SymtabAPI;
using namespace Dwarf;
using namespace std;

#define DWARF_FAIL_RET_VAL(x, v) {                                      \
      int status = (x);                                                 \
      if (status != DW_DLV_OK) {                                        \
         fprintf(stderr, "[%s:%d]: libdwarf returned %d, ret false\n", FILE__, __LINE__, status); \
         return (v);                                                    \
      }                                                                 \
   }
#define DWARF_FAIL_RET(x) DWARF_FAIL_RET_VAL(x, false)

#define DWARF_ERROR_RET_VAL(x, v) {                                     \
      int status = (x);                                                 \
      if (status == DW_DLV_ERROR) {                                     \
         fprintf(stderr, "[%s:%d]: parsing failure, ret false\n", FILE__, __LINE__); \
         return (v);                                                    \
      }                                                                 \
   }
#define DWARF_ERROR_RET(x) DWARF_ERROR_RET_VAL(x, false)

#define DWARF_CHECK_RET_VAL(x, v) {                                     \
      if (x) {                                                          \
         fprintf(stderr, "[%s:%d]: parsing failure, ret false\n", FILE__, __LINE__); \
         return (v);                                                    \
      }                                                                 \
   }
#define DWARF_CHECK_RET(x) DWARF_CHECK_RET_VAL(x, false)

DwarfWalker::DwarfWalker(Symtab *symtab, Dwarf_Debug &dbg)
   :
   dbg_(dbg),
   mod_(NULL),
   symtab_(symtab),
   tc_(NULL),
   is_mangled_name_(false),
   modLow(0),
   modHigh(0),
   cu_header_length(0),
   version(0),
   abbrev_offset(0),
   addr_size(0),
   offset_size(0),
   extension_size(0),
   signature(),
   typeoffset(0),
   next_cu_header(0),
   compile_offset(0)
{
}

DwarfWalker::~DwarfWalker() {}

bool DwarfWalker::parse() {
   dwarf_printf("Parsing DWARF for %s\n", symtab_->file().c_str());

   /* Start the dwarven debugging. */
   Module *fixUnknownMod = NULL;
   mod_ = NULL;

   /* Prepopulate type signatures for DW_FORM_ref_sig8 */
   findAllSig8Types();

   /* First .debug_types (0), then .debug_info (1) */
   for (int i = 0; i < 2; ++i) {
      Dwarf_Bool is_info = i;

      /* NB: parseModule used to compute compile_offset as 11 bytes before the
       * first die offset, to account for the header.  This would need 23 bytes
       * instead for 64-bit format DWARF, and even more for type units.
       * (See DWARF4 sections 7.4 & 7.5.1.)
       * But more directly, we know the first CU is just at 0x0, and each
       * following CU is already reported in next_cu_header.
       */
      compile_offset = next_cu_header = 0;

      /* Iterate over the compilation-unit headers. */
      while (dwarf_next_cu_header_c(dbg(), is_info,
                                    &cu_header_length,
                                    &version,
                                    &abbrev_offset,
                                    &addr_size,
                                    &offset_size,
                                    &extension_size,
                                    &signature,
                                    &typeoffset,
                                    &next_cu_header, NULL) == DW_DLV_OK ) {
         contexts_.push();
         bool ret = parseModule(is_info, fixUnknownMod);
         contexts_.pop();
         if (!ret) return false;
         compile_offset = next_cu_header;
      }
   }
   
   if (!fixUnknownMod)
      return true;

   dwarf_printf("Fixing types for final module %s\n", fixUnknownMod->fileName().c_str());
   
   /* Fix type list. */
   typeCollection *moduleTypes = typeCollection::getModTypeCollection(fixUnknownMod);
   assert(moduleTypes);
   dyn_hash_map< int, Type * >::iterator typeIter =  moduleTypes->typesByID.begin();
   for (;typeIter!=moduleTypes->typesByID.end();typeIter++)
   {
      typeIter->second->fixupUnknowns(fixUnknownMod);
   } /* end iteration over types. */
   
   /* Fix the types of variables. */   
   std::string variableName;
   dyn_hash_map< std::string, Type * >::iterator variableIter = moduleTypes->globalVarsByName.begin();
   for (;variableIter!=moduleTypes->globalVarsByName.end();variableIter++)
   { 
      if (variableIter->second->getDataClass() == dataUnknownType && 
          moduleTypes->findType( variableIter->second->getID() ) != NULL ) 
      {
         moduleTypes->globalVarsByName[ variableIter->first ] 
            = moduleTypes->findType( variableIter->second->getID() );
      } /* end if data class is unknown but the type exists. */
   } /* end iteration over variables. */
      
   moduleTypes->setDwarfParsed();
   return true;
}

bool DwarfWalker::parseModule(Dwarf_Bool is_info, Module *&fixUnknownMod) {
   /* Obtain the module DIE. */
   Dwarf_Die moduleDIE;
   DWARF_FAIL_RET(dwarf_siblingof_b( dbg(), NULL, is_info, &moduleDIE, NULL ));
   
   /* Make sure we've got the right one. */
   Dwarf_Half moduleTag;
   DWARF_FAIL_RET(dwarf_tag( moduleDIE, & moduleTag, NULL ));

   if (moduleTag != DW_TAG_compile_unit
         && moduleTag != DW_TAG_partial_unit
         && moduleTag != DW_TAG_type_unit)
      return false;
   
   /* Extract the name of this module. */
   std::string moduleName;
   if (!findDieName( moduleDIE, moduleName )) return false;

   if (moduleName.empty() && moduleTag == DW_TAG_type_unit) {
      uint64_t sig8 = * reinterpret_cast<uint64_t*>(&signature);
      char buf[20];
      snprintf(buf, sizeof(buf), "{%016llx}", (long long) sig8);
      moduleName = buf;
   }

   if (moduleName.empty()) {
      moduleName = "{ANONYMOUS}";
   }

   dwarf_printf("Next DWARF module: %s with DIE %p and tag %d\n", moduleName.c_str(), moduleDIE, moduleTag);
   
   /* Set the language, if any. */
   Dwarf_Attribute languageAttribute;
   DWARF_ERROR_RET(dwarf_attr( moduleDIE, DW_AT_language, & languageAttribute, NULL ));
   
   // Set low and high ranges; this can fail, so don't check return addr.
   setEntry(moduleDIE);

   // These may not be set.
   Address tempModLow, tempModHigh;
   modLow = modHigh = 0;
   if (findConstant(DW_AT_low_pc, tempModLow)) {
      obj()->convertDebugOffset(tempModLow, modLow);
   }
   if (findConstant(DW_AT_high_pc, tempModHigh)) {
      obj()->convertDebugOffset(tempModHigh, modHigh);
   }

   if (!symtab()->findModuleByName(mod_, moduleName))
   {
      std::string fName = extract_pathname_tail(moduleName);
      if (!symtab()->findModuleByName(mod_, fName)) {
         moduleName = symtab()->file();
         if (!symtab()->findModuleByName(mod_, moduleName)) {
            mod_ = symtab()->getDefaultModule();
         }
      }
   }

   dwarf_printf("Mapped to Symtab module %s\n", mod_->fileName().c_str());
   
   if (!fixUnknownMod)
      fixUnknownMod = mod_;

   if (!buildSrcFiles(moduleDIE)) return false;
   
   tc_ = typeCollection::getModTypeCollection(mod_);

   if (!parse_int(moduleDIE, true)) return false;

   return true;

} 

       
bool DwarfWalker::buildSrcFiles(Dwarf_Die entry) {
   srcFiles_.clear();

   Dwarf_Signed cnt = 0;
   char **srcfiletmp = NULL;
   DWARF_ERROR_RET(dwarf_srcfiles(entry, &srcfiletmp, &cnt, NULL));

   for (unsigned i = 0; i < cnt; ++i) {
      srcFiles_.push_back(srcfiletmp[i]);
      dwarf_dealloc(dbg(), srcfiletmp[i], DW_DLA_STRING);
   }
   dwarf_dealloc(dbg(), srcfiletmp, DW_DLA_LIST);
   return true;
}

Object *DwarfWalker::obj() { 
   return symtab_->getObject();
}

// As mentioned in the header, this is separate from parse()
// so we can have a non-Context-creating parse method that reuses
// the Context from the parent. This allows us to pass in current
// function, etc. without the Context stack exploding
bool DwarfWalker::parse_int(Dwarf_Die e, bool p) {
   dwarf_printf("PARSE_INT entry, context size %d\n", (int) contexts_.c.size());
   // We escape the loop by checking parseSibling() after 
   // parsing this DIE and its children, if any
   while(1) {
      ContextGuard cg(contexts_);

      setEntry(e);
      setParseSibling(p);

      if (!findTag()) return false;
      if (!findOffset()) return false;
      curName() = std::string();
      setMangledName(false);

      dwarf_printf("(0x%lx) Parsing entry %p with context size %d, func %p (%s), encl %p\n",
                   id(),
                   e,
                   (int) contexts_.c.size(), 
                   curFunc(),
                   (curFunc() && !curFunc()->getAllMangledNames().empty()) ? 
                   curFunc()->getAllMangledNames()[0].c_str() : "<null>",
                   curEnclosure());

      bool ret = false;
      
   // BLUEGENE BUG HACK
#if defined(os_bg)
      if (tag() == DW_TAG_base_type ||
          tag() == DW_TAG_const_type ||
          tag() == DW_TAG_pointer_type) {
         // XLC compilers nest a bunch of stuff under an invented function; however,
         // this is broken (they don't close the function properly). If we see a 
         // tag like this, close off the previous function immediately
         clearFunc();
      }
#endif



      switch(tag()) {
         case DW_TAG_subprogram:
         case DW_TAG_entry_point:
            ret = parseSubprogram(NormalFunc);
            break;
         case DW_TAG_inlined_subroutine:
            ret = parseSubprogram(InlinedFunc);
            break;
         case DW_TAG_lexical_block:
            ret = parseLexicalBlock();
            break;
         case DW_TAG_common_block:
            ret = parseCommonBlock();
            break;
         case DW_TAG_constant:
            ret = parseConstant();
            break;
         case DW_TAG_variable:
            ret = parseVariable();
            break;
         case DW_TAG_formal_parameter:
            ret = parseFormalParam();
            break;
         case DW_TAG_base_type:
            ret = parseBaseType();
            break;
         case DW_TAG_typedef:
            ret = parseTypedef();
            break;
         case DW_TAG_array_type:
            ret = parseArray();
            break;
         case DW_TAG_subrange_type:
            ret = parseSubrange();
            break;
         case DW_TAG_enumeration_type:
            ret = parseEnum();
            break;
         case DW_TAG_inheritance:
            ret = parseInheritance();
            break;
         case DW_TAG_structure_type:
         case DW_TAG_union_type:
         case DW_TAG_class_type: 
            ret = parseStructUnionClass();
            break;
         case DW_TAG_enumerator:
            ret = parseEnumEntry();
            break;
         case DW_TAG_member:
            ret = parseMember();
            break;
         case DW_TAG_const_type:
         case DW_TAG_packed_type:
         case DW_TAG_volatile_type: 
            ret = parseConstPackedVolatile();
            break;
         case DW_TAG_subroutine_type:
            /* If the pointer specifies argument types, this DIE has
               children of those types. */
         case DW_TAG_ptr_to_member_type:
         case DW_TAG_pointer_type:
         case DW_TAG_reference_type: 
            ret = parseTypeReferences();
            break;
         case DW_TAG_compile_unit:
            dwarf_printf("(0x%lx) Compilation unit, parsing children\n", id());
            // Parse child
            ret = parseChild();
            break;
         case DW_TAG_partial_unit:
            dwarf_printf("(0x%lx) Partial unit, parsing children\n", id());
            // Parse child
            ret = parseChild();
            break;
         case DW_TAG_type_unit:
            dwarf_printf("(0x%lx) Type unit, parsing children\n", id());
            // Parse child
            ret = parseChild();
            break;
         default:
            dwarf_printf("(0x%lx) Warning: unparsed entry with tag %x\n",
                         id(), tag());
            ret = true;
            break;
      }
      
      dwarf_printf("Finished parsing 0x%lx, ret %d, parseChild %d, parseSibling %d\n",
                   id(), ret, parseChild(), parseSibling());

      if (ret && parseChild() ) {
         // Parse children
         Dwarf_Die childDwarf;
         int status = dwarf_child( entry(), & childDwarf, NULL );
         DWARF_CHECK_RET(status == DW_DLV_ERROR);
         if (status == DW_DLV_OK) {
            if (!parse_int(childDwarf, true)) return false;
         }
      }

      if (!parseSibling()) {
         dwarf_printf("(0x%lx) Skipping sibling parse\n", id());
         break;
      }

      dwarf_printf("(0x%lx) Asking for sibling\n", id());
      Dwarf_Die siblingDwarf;
      Dwarf_Bool is_info = dwarf_get_die_infotypes_flag(entry());
      int status =  dwarf_siblingof_b( dbg(), entry(), is_info, & siblingDwarf, NULL );

      DWARF_CHECK_RET(status == DW_DLV_ERROR);
      
      /* Deallocate the entry we just parsed. */
      dwarf_dealloc( dbg(), entry(), DW_DLA_DIE );

      if (status != DW_DLV_OK) {
         break;
      }

      e = siblingDwarf;
   }
   dwarf_printf("PARSE_INT exit, context size %d\n", (int) contexts_.c.size());
   return true;
}

bool DwarfWalker::parseCallsite()
{
   Dwarf_Bool has_line = false, has_file = false;
   DWARF_FAIL_RET(dwarf_hasattr(entry(), DW_AT_call_file, &has_file, NULL));
   if (!has_file)
      return true;
   DWARF_FAIL_RET(dwarf_hasattr(entry(), DW_AT_call_line, &has_line, NULL));
   if (!has_line)
      return true;

   std::string inline_file;
   bool result = findString(DW_AT_call_file, inline_file);
   if (!result)
      return false;

   Dyninst::Offset inline_line;
   result = findConstant(DW_AT_call_line, inline_line);
   if (!result)
      return false;

   InlinedFunction *ifunc = static_cast<InlinedFunction *>(curFunc());
   ifunc->callsite_file = inline_file;
   ifunc->callsite_line = inline_line;
   return true;
}

bool DwarfWalker::setFunctionFromRange(inline_t func_type)
{
   //Use the lowest range as an entry for symbol matching
   Address lowest = 0x0;
   bool set_lowest = false;
   if (!hasRanges()) {
     dwarf_printf("(0x%lx) setFunctionFromRange has no ranges, returning false\n", id());
     
      return false;
   }
   for (range_set_t::iterator i = ranges_begin(); i != ranges_end(); i++) {
      if (!set_lowest) {
         lowest = i->first;
         set_lowest = true;
         continue;
      }
      if (lowest > i->first)
         lowest = i->first;
   }
   if (!set_lowest) {
      //No ranges.  Don't panic, this is probably an abstract origin or specification
      // we'll get to it latter if it's really used.
     dwarf_printf("(0x%lx) setFunctionFromRange has ranges, but no lowest, returning false\n", id());
      return false;
   }

   if (func_type == InlinedFunc) {
      FunctionBase *parent = curFunc();
      if (!parent) {
         //InlinedSubroutine without containing subprogram.  Weird.
	dwarf_printf("(0x%lx) setFunctionFromRange found inline without parent, returning false\n", id());
         return false;
      }
      InlinedFunction *ifunc = new InlinedFunction(parent);
      setFunc(ifunc);
      return true;
   }

   //Try to associate the function with existing symbols
   Function *f = NULL;
   bool result = symtab()->findFuncByEntryOffset(f, lowest);
   if (result) {
      dwarf_printf("(0x%lx) Lookup by offset 0x%lx identifies %p\n",
                   id(), lowest, curFunc());
      setFunc(f);
      return true;
   } else 
   {
     dwarf_printf("(0x%lx) Lookup by offset 0x%lx failed\n", id(), lowest);
   }
   

   
   return true;
}

bool DwarfWalker::parseSubprogram(DwarfWalker::inline_t func_type) {
   bool name_result;
   FunctionBase *func = NULL;

   dwarf_printf("(0x%lx) parseSubprogram entry\n", id());

   parseRangeTypes();
   setFunctionFromRange(func_type);
   func = curFunc();

   // Name first
   name_result = findFuncName();

   if (curEnclosure() && !func) {
      // This is a member function; create the type entry
      // Since curFunc is false, we're not going back to reparse this
      // entry with a function object.
      Type *ftype = NULL;
      getReturnType(false, ftype);
      addFuncToContainer(ftype);
      dwarf_printf("(0x%lx) parseSubprogram not parsing member function's children\n", id());
      
      setParseChild(false);
   }

   //curFunc will be set if we're parsing a defined object, or
   // if we're recursively parsing a specification or abstract
   // entry under a defined object.  It'll be unset if we're
   // parsing a specification or abstract entry at the top-level
   //This keeps us from parsing abstracts or specifications until
   // we need them.
   if (!func) {
      dwarf_printf("(0x%lx) parseSubprogram not parsing children b/c curFunc() NULL\n", id());
      setParseChild(false);
      return true;
   }

   if (parsedFuncs.find(func) != parsedFuncs.end()) {
      dwarf_printf("(0x%lx) parseSubprogram not parsing children b/c curFunc() not in parsedFuncs\n", id());
      setParseChild(false);
      return true;
   }

   if (name_result && !curName().empty()) {
      dwarf_printf("(0x%lx) Identified function name as %s\n", id(), curName().c_str());
      if (isMangledName())
         func->addMangledNameInternal(curName(), true, true);
      else
         func->addPrettyName(curName(), true);
   }

   //Collect callsite information for inlined functions.
   if (func_type == InlinedFunc) {
      parseCallsite();
   }
   
   // Get the return type
   Type *returnType = NULL;
   if (!func->getReturnType()) {
      getReturnType(false, returnType);
      if (returnType)
         func->setReturnType(returnType);
   }

   // Get range information
   if (hasRanges() && func->ranges.empty()) {
      Address last_low = 0, last_high = 0;
      func->ranges.reserve(rangesSize());
      for (range_set_t::iterator i = ranges_begin(); i != ranges_end(); i++) {
         Address low = i->first;
         Address high = i->second;
         if (last_low == low && last_high == high)
            continue;
         last_low = low;
         last_high = high;

         func->ranges.push_back(FuncRange(low, high - low, func));         
      }
   }

   // Dwarf outlines some function information. You have the base entry, which contains
   // address ranges, frame base information, and optionally a "abstract origin"
   // or "specification" entry that points to more information. 
   // We want to skip parsing specification or abstract entries until we have
   // the base entry and can find/create the corresponding function object. 


   // Get the frame base if it exists
   if (!getFrameBase()) return false;

   // Parse parent nodes and their children but not their sibling
   bool hasAbstractOrigin = false;
   if (!handleAbstractOrigin(hasAbstractOrigin)) return false;
   if (hasAbstractOrigin) {
      dwarf_printf("(0x%lx) Parsing abstract parent\n", id());
      if (!parse_int(abstractEntry(), false)) return false;
      dwarf_dealloc(dbg(), abstractEntry(), DW_DLA_DIE); 
   } 
   // An abstract origin will point to a specification if it exists
   // This can actually be three-layered, so backchain again
   bool hasSpecification = false;
   if (!handleSpecification(hasSpecification)) return false;
   if ( hasSpecification ) {
      dwarf_printf("(0x%lx) Parsing specification entry\n", id());
      if (!parse_int(specEntry(), false)) return false;
   }

   parsedFuncs.insert(func);

   return true;
}

bool DwarfWalker::parseHighPCLowPC()
{
   Dwarf_Attribute hasLow;

   Dwarf_Attribute hasHigh;
   if(dwarf_attr(entry(), DW_AT_low_pc, &hasLow, NULL) != DW_DLV_OK) return false;
   if(dwarf_attr(entry(), DW_AT_high_pc, &hasHigh, NULL) != DW_DLV_OK) return false;

   Address low, high;
   Address tempLow, tempHigh;
   if (!findConstant(DW_AT_low_pc, tempLow)) return false;
   if (!findConstant(DW_AT_high_pc, tempHigh)) return false;
   obj()->convertDebugOffset(tempLow, low);
   obj()->convertDebugOffset(tempHigh, high);
   Dwarf_Half form;
   DWARF_FAIL_RET(dwarf_whatform(hasHigh, &form, NULL));
   
   if(form != DW_FORM_addr)
   {
     high += low;
   }
   dwarf_printf("(0x%lx) Lexical block from 0x%lx to 0x%lx\n", id(), low, high);
   setRange(make_pair(low, high));
   return true;
   
}


bool DwarfWalker::parseRangeTypes() {
   dwarf_printf("(0x%lx) Parsing ranges\n", id());

   clearRanges();
   parseHighPCLowPC();
   

   Dwarf_Bool hasRanges = false;
   DWARF_FAIL_RET(dwarf_hasattr(entry(), DW_AT_ranges, &hasRanges, NULL));
   if (hasRanges) {
      Address range_offset;
      if (!findConstant(DW_AT_ranges, range_offset)) return false;
      
      Dwarf_Ranges *ranges = NULL;
      Dwarf_Signed ranges_length = 0;
      DWARF_FAIL_RET(dwarf_get_ranges_a(dbg(), (Dwarf_Off) range_offset, entry(),
                                        &ranges, &ranges_length, NULL, NULL));

      bool done = false;
      for (unsigned i = 0; i < ranges_length && !done; i++) {
         Dwarf_Ranges *cur = ranges + i;
         Address cur_base = modLow;
         switch (cur->dwr_type) {
            case DW_RANGES_ENTRY: {
               Address low = cur->dwr_addr1 + cur_base;
               Address high = cur->dwr_addr2 + cur_base;
               dwarf_printf("(0x%lx) Lexical block from 0x%lx to 0x%lx\n", id(), low, high);
               setRange(make_pair(low, high));
               break;
            }
            case DW_RANGES_ADDRESS_SELECTION:
               cur_base = cur->dwr_addr2;
               break;
            case DW_RANGES_END:
               done = true;
               break;
         }
      }
   }   
   return true;
}

bool DwarfWalker::parseLexicalBlock() {
   dwarf_printf("(0x%lx) Parsing lexical block\n", id());
   return parseRangeTypes();
}

bool DwarfWalker::parseCommonBlock() {
   dwarf_printf("(0x%lx) Parsing common block\n", id());

   std::string commonBlockName;
   if (!findDieName( entry(), commonBlockName )) return false;
   
   std::vector<Symbol *> commonBlockVars;
   if (!symtab()->findSymbol(commonBlockVars,
                             commonBlockName,
                             Symbol::ST_OBJECT,
                             SymtabAPI::anyName)) {
      //pgcc 6 is naming common blocks with a trailing underscore
      std::string cbvname = commonBlockName + std::string("_");
      symtab()->findSymbol(commonBlockVars,
                           cbvname, 
                           Symbol::ST_OBJECT,
                           SymtabAPI::anyName);
   }
   if (commonBlockVars.empty()) {
      dwarf_printf("(0x%lx) Couldn't find variable for common block %s\n", id(), commonBlockName.c_str());
      return false;
   }
   
   Symbol *commonBlockVar = commonBlockVars[0];
   if(!commonBlockVar)
   {
     assert(!"No common block variables to parse!");
     return false;
   }


   typeCommon *commonBlockType = NULL;

   commonBlockType = dynamic_cast<typeCommon *>(tc()->findVariableType(commonBlockName));
   if (commonBlockType == NULL) {
      commonBlockType = new typeCommon( type_id(), commonBlockName );
      assert( commonBlockType != NULL );
      tc()->addGlobalVariable( commonBlockName, commonBlockType );
   }	

   setCommon(commonBlockType);

   return true;
}

bool DwarfWalker::parseConstant() {
   // Right now we don't handle these
   dwarf_printf("(0x%lx) Skipping named constant/variable with constant value\n", id());
   return true;
}

bool DwarfWalker::parseVariable() {
   dwarf_printf("(0x%lx) ParseVariable entry\n", id());
   /* Acquire the name, type, and line number. */
   /* A variable may occur inside a function, as either static or local.
      A variable may occur inside a container, as C++ static member.
      A variable may not occur in either, as a global. 
      
      For the first two cases, we need the variable's name, its type,
      its line number, and its offset or address in order to tell
      Dyninst about it.  Dyninst only needs to know the name and type
      of a global.  (Actually, it already knows the names of the globals;
      we're really just telling it the types.)
      
      Variables may have two entries, the second, with a _specification,
      being the only one with the location. */
      
   /* If this DIE has a _specification, use that for the rest of our inquiries. */
   bool hasSpecification = false;
   if (!handleSpecification(hasSpecification)) return false;

   if (!findName(curName())) return false;

   removeFortranUnderscore(curName());

   /* We'll start with the location, since that's most likely to
      require the _specification. */

   std::vector<VariableLocation> locs;
   if (!decodeLocationList(DW_AT_location, NULL, locs)) return false;
   if (locs.empty()) return true;
   
   for (unsigned i=0; i<locs.size(); i++) {
      //if (locs[i].stClass != storageAddr) 
      //continue;
      if (locs[i].lowPC) {
         Offset newlowpc = locs[i].lowPC;
         bool result = obj()->convertDebugOffset(locs[i].lowPC, newlowpc);
         if (result)
            locs[i].lowPC = newlowpc;
      }
      if (locs[i].hiPC) {
         Offset newhipc = locs[i].hiPC;
         bool result = obj()->convertDebugOffset(locs[i].hiPC, newhipc);
         if (result)
            locs[i].hiPC = newhipc;
      }
   }

   Type *type = NULL;
   if (!findType(type, false)) return false;
   assert(type);
   
   Dwarf_Unsigned variableLineNo;
   bool hasLineNumber = false;
   std::string fileName;

   if (curFunc() || curEnclosure()) {
      if (!getLineInformation(variableLineNo, hasLineNumber, fileName)) return false;
   }

   if (!curFunc() && !curEnclosure()) {
      /* The typeOffset forms a module-unique type identifier,
         so the Type look-ups by it rather than name. */
      dwarf_printf("(0x%lx) Adding global variable\n", id());

      Dyninst::Offset addr = 0;
      if (locs.size() && locs[0].stClass == storageAddr)
         addr = locs[0].frameOffset;
      Variable *var;
      bool result = symtab()->findVariableByOffset(var, addr);
      if (result) {
         var->setType(type);
      }      
      tc()->addGlobalVariable( curName(), type);
   } /* end if this variable is a global */
   else if (curFunc()) {
      /* We now have the variable name, type, offset, and line number.
         Tell Dyninst about it. */
      if (!nameDefined()) return true;

      localVar * newVariable = new localVar(curName(),
                                            type,
                                            fileName, 
                                            (int) variableLineNo, 
                                            curFunc());
      dwarf_printf("(0x%lx) localVariable '%s' (%p), currentFunction %p\n", 
                   id(), curName().c_str(), newVariable, curFunc());

      for (unsigned int i = 0; i < locs.size(); ++i) {
         dwarf_printf("(0x%lx) (%s) Adding location %d of %d: (0x%lx - 0x%lx): %s, %s, %s, %ld\n",
                      id(), newVariable->getName().c_str(), i+1, (int) locs.size(), locs[i].lowPC, locs[i].hiPC, 
                      storageClass2Str(locs[i].stClass),
                      storageRefClass2Str(locs[i].refClass),
                      locs[i].mr_reg.name().c_str(),
                      locs[i].frameOffset);
         newVariable->addLocation(locs[i]);
      }
      curFunc()->addLocalVar(newVariable);
   } /* end if a local or static variable. */
   else if (curEnclosure()) {
      if (!nameDefined()) return true;
      assert( locs[0].stClass != storageRegOffset );
      dwarf_printf("(0x%lx) Adding variable to an enclosure\n", id());

      curEnclosure()->addField( curName(), type, locs[0].frameOffset);
   } /* end if this variable is not global */
   return true;
}

bool DwarfWalker::parseFormalParam() {
   dwarf_printf("(0x%lx) Parsing formal parameter\n", id());

   /* A formal parameter must occur in the context of a function.
      (That is, we can't do anything with a formal parameter to a
      function we don't know about.) */
   /* It's probably worth noting that a formal parameter may have a
      default value.  Since, AFAIK, Dyninst does nothing with this information,
      neither will we. */
   if (!curFunc()) {
     dwarf_printf("(0x%lx) No function defined, returning\n", id());
     return true;
   }

   /* We need the formal parameter's name, its type, its line number,
      and its offset from the frame base in order to tell the 
      rest of Dyninst about it.  A single _formal_parameter
      DIE may not have all of this information; if it does not,
      we will ignore it, hoping to catch it when it is later
      referenced as an _abstract_origin from another _formal_parameter
      DIE.  If we never find such a DIE, than there is not enough
      information to introduce it to Dyninst. */
   
   /* We begin with the location, since this is the attribute
      most likely to require using the _abstract_origin. */
   
   std::vector<VariableLocation> locs;
   if (!decodeLocationList(DW_AT_location, NULL, locs)) return false;
   if (locs.empty()) {
     dwarf_printf("(0x%lx) No locations associated with formal, returning\n", id());
     return true;
   }

   /* If the DIE has an _abstract_origin, we'll use that for the
      remainder of our inquiries. */
   bool hasAbstractOrigin;
   if (!handleAbstractOrigin(hasAbstractOrigin)) return false;
   if (hasAbstractOrigin) {
      // Clone to spec entry too
      setSpecEntry(abstractEntry());
   }

   if (!findName(curName())) return false;
   /* We can't do anything with anonymous parameters. */   
   if (!nameDefined()) {
     dwarf_printf("(0x%lx) No name associated with formal, returning\n", id());
     return true; 
   }
      
   /* Acquire the parameter's type. */
   Type *paramType = NULL;
   if (!findType(paramType, false)) return false;

   Dwarf_Unsigned lineNo = 0;
   bool hasLineNumber = false;
   std::string fileName;
   if (!getLineInformation(lineNo, hasLineNumber, fileName)) return false;
   localVar * newParameter = new localVar(curName(), 
                                          paramType, 
                                          fileName, (int) lineNo, 
                                          curFunc());
   dwarf_printf("(0x%lx) Creating new formal parameter %s/%p (%s)/%s (%p)\n",
                id(),
                curName().c_str(), 
                paramType, paramType->getName().c_str(),
                ((curFunc() && !curFunc()->getAllMangledNames().empty()) ? 
                 curFunc()->getAllMangledNames()[0].c_str() : ""),
                curFunc());

   assert( newParameter != NULL );
   for (unsigned int i = 0; i < locs.size(); ++i)
   {
      newParameter->addLocation(locs[i]);
   }
   
   /* This is just brutally ugly.  Why don't we take care of this invariant automatically? */
   
   curFunc()->addParam(newParameter);
   return true;
}

bool DwarfWalker::parseBaseType() {
   dwarf_printf("(0x%lx) parseBaseType entry\n", id());

   if (!findName(curName())) return false;
   if (!nameDefined()) {
      dwarf_printf("(0x%lx) No name for type, returning early\n", id());
      return true;
   }

   unsigned size = 0;
   if (!findSize(size)) return false;

   /* Generate the appropriate built-in type; since there's no
      reliable way to distinguish between a built-in and a scalar,
      we don't bother to try. */
   typeScalar * baseType = new typeScalar( type_id(), (unsigned int) size, curName());
   assert( baseType != NULL );
   
   /* Add the basic type to our collection. */
   typeScalar *debug = baseType;
   baseType = tc()->addOrUpdateType( baseType );
   dwarf_printf("(0x%lx) Created type %p / %s (pre add %p / %s) for id %d, size %d, in TC %p\n", id(), 
                baseType, baseType->getName().c_str(),
                debug, debug->getName().c_str(),
                (int) offset(), size,
                tc());
   
   return true;
}

bool DwarfWalker::parseTypedef() {
   dwarf_printf("(0x%lx) parseTypedef entry\n", id());

   if (!findName(curName())) return false;

   Type *referencedType = NULL;
   if (!findType(referencedType, true)) return false;

   if (!nameDefined()) {
      if (!fixName(curName(), referencedType)) return false;
   }

   typeTypedef * typedefType = new typeTypedef( type_id(), referencedType, curName());
   typedefType = tc()->addOrUpdateType( typedefType );

   return true;
}

bool DwarfWalker::parseArray() {
   dwarf_printf("(0x%lx) Parsing array\n", id());
   /* Two words about pgf90 arrays.
      
      Primus: the PGI extensions to DWARF are documented in 
      '/p/paradyn/doc/External/manuals/pgf90-dwarf-arrays.txt'.
      
      Secundus: we ignore DW_AT_PGI_lbase, DW_AT_PGI_loffset, and DW_AT_PGI_lstride,
      even though libdwarf recognizes them, because our type modelling doesn't allow
      us to make use of this information.  Similarly, in virtually every place where
      the Portland Group extends DWARF to allow _form_block attributes encoding location
      lists, we ignore them.  We should, however, recognize these cases and ignore them
      gracefully, that is, without an error. :)
   */

   if (!findName(curName())) return false;

   // curName may get overridden by the subrange parsing code. 
   // TODO: make this part of the context stack. 
   std::string nameToUse = curName();

   Type *elementType = NULL;
   if (!findType(elementType, false)) return false;
   if (!elementType) return false;

   /* Find the range(s) of the elements. */
   Dwarf_Die firstRange;

   DWARF_FAIL_RET(dwarf_child( entry(), & firstRange, NULL ));

   contexts_.push();

   typeArray * baseArrayType = parseMultiDimensionalArray(firstRange, 
                                                          elementType);

   contexts_.pop();

   if (!baseArrayType) return false;
   
   dwarf_dealloc( dbg(), firstRange, DW_DLA_DIE );
   
   /* The baseArrayType is an anonymous type with its own typeID.  Extract
      the information and add an array type for this DIE. */

   dwarf_printf("(0x%lx) Creating array with base type %s, low bound %ld, high bound %ld, named %s\n",
                id(), baseArrayType->getBaseType()->getName().c_str(),
                baseArrayType->getLow(), 
                baseArrayType->getHigh(),
                curName().c_str());
   typeArray *arrayType = new typeArray( type_id(),
                                         baseArrayType->getBaseType(), 
                                         baseArrayType->getLow(),
                                         baseArrayType->getHigh(), 
                                         nameToUse); 
   assert( arrayType != NULL );

   arrayType = tc()->addOrUpdateType( arrayType );
   
   /* Don't parse the children again. */
   setParseChild(false);
   
   return true;
}

bool DwarfWalker::parseSubrange() {
   dwarf_printf("(0x%lx) parseSubrange entry\n", id());

   std::string loBound;
   std::string hiBound;
   parseSubrangeAUX(entry(), loBound, hiBound);

  return true;
}

bool DwarfWalker::parseEnum() {
   dwarf_printf("(0x%lx) parseEnum entry\n", id());
   if (!findName(curName())) return false;

   typeEnum* enumerationType = new typeEnum( type_id(), curName());
   assert( enumerationType != NULL );
   enumerationType = dynamic_cast<typeEnum *>(tc()->addOrUpdateType( enumerationType ));

   setEnum(enumerationType);
   return true;
}

bool DwarfWalker::parseInheritance() {
   dwarf_printf("(0x%lx) parseInheritance entry\n", id());

   /* Acquire the super class's type. */
   Type *superClass = NULL;
   if (!findType(superClass, false)) return false;
   if (!superClass) return false;

   dwarf_printf("(0x%lx) Found %p as superclass\n", id(), superClass);

   visibility_t visibility;
   if (!findVisibility(visibility)) return false;

   /* Add a readily-recognizable 'bad' field to represent the superclass.
      Type::getComponents() will Do the Right Thing. */
   std::string fName = "{superclass}";
   curEnclosure()->addField( fName, superClass, -1, visibility );
   dwarf_printf("(0x%lx) Added type %p as %s to %p\n", id(), superClass, fName.c_str(), curEnclosure());
   return true;
}

bool DwarfWalker::parseStructUnionClass() {
   dwarf_printf("(0x%lx) parseStructUnionClass entry\n", id());

   assert(tag() == DW_TAG_structure_type ||
          tag() == DW_TAG_union_type ||
          tag() == DW_TAG_class_type);

   if (!findName(curName())) return false;

   bool isDeclaration = false;
   if (!hasDeclaration(isDeclaration)) return false;

   unsigned size;
   if (!findSize(size)) {
      if (isDeclaration) return true; 
      else return false;
   }

   fieldListType * containingType = NULL;

   switch ( tag() ) {
      case DW_TAG_structure_type: 
      case DW_TAG_class_type: {
         typeStruct *ts = new typeStruct( type_id(), curName());
         ts->setSize(size);
         containingType = dynamic_cast<fieldListType *>(tc()->addOrUpdateType(ts));
         break;
      }
      case DW_TAG_union_type: 
      {
         typeUnion *tu = new typeUnion( type_id(), curName());
         tu->setSize(size);
         containingType = dynamic_cast<fieldListType *>(tc()->addOrUpdateType(tu));
         break;
      }
   }
   setEnclosure(containingType);
   dwarf_printf("(0x%lx) Started class, union, or struct: %p\n",
                id(), containingType);
   return true;
}

bool DwarfWalker::parseEnumEntry() {
   dwarf_printf("(0x%lx) parseEnumEntry entry\n", id());

   if (!findName(curName())) return false;

   long value;
   bool valid;
   if (!findValue(value, valid)) return false;

   curEnum()->addConstant(curName(), value);
   return true;
}

bool DwarfWalker::parseMember() {
   dwarf_printf("(0x%lx) parseMember entry\n", id());
   if (!curEnclosure()) return false;

   if (!findName(curName())) return false;

   Type *memberType = NULL;
   if (!findType(memberType, false)) return false;
   if (!memberType) return false;

   std::vector<VariableLocation> locs;
   Address initialStackValue = 0;
   if (!decodeLocationList(DW_AT_data_member_location, &initialStackValue, locs)) return false;

   /* DWARF stores offsets in bytes unless the member is a bit field.
      Correct memberOffset as indicated.  Also, memberSize is in bytes
      from the underlying type, not the # of bits used from it, so
      correct that as necessary as well. */
   
   long int memberSize = memberType->getSize();

   // This code changes memberSize, which is then discarded. I'm not sure why...
   if (!fixBitFields(locs, memberSize)) return false;

   int offset_to_use = locs.size() ? locs[0].frameOffset : -1;

   dwarf_printf("(0x%lx) Using offset of 0x%lx\n", id(), offset_to_use);

   if (nameDefined()) {
      curEnclosure()->addField( curName(), memberType, offset_to_use);
   }
   else {
      curEnclosure()->addField("[anonymous union]", memberType, offset_to_use);
   }
   return true;
}

// TODO: this looks a lot like parseTypedef. Collapse?
bool DwarfWalker::parseConstPackedVolatile() {
   dwarf_printf("(0x%lx) parseConstPackedVolatile entry\n", id());

   if (!findName(curName())) return false;

   Type *type = NULL;
   if (!findType(type, true)) return false;

   if (!nameDefined()) {
      if (!fixName(curName(), type)) return false;
   }

   typeTypedef * modifierType = new typeTypedef(type_id(), type, curName());
   assert( modifierType != NULL );
   modifierType = tc()->addOrUpdateType( modifierType );
   return true;
}

bool DwarfWalker::parseTypeReferences() {
   dwarf_printf("(0x%lx) parseTypeReferences entry\n", id());

   if (!findName(curName())) return false;

   Type *typePointedTo = NULL;
   if (!findType(typePointedTo, true)) return false;
   
   Type * indirectType = NULL;
   switch ( tag() ) {
      case DW_TAG_subroutine_type:
         indirectType = new typeFunction(type_id(), typePointedTo, curName());
         indirectType = tc()->addOrUpdateType((typeFunction *) indirectType );
         break;
      case DW_TAG_ptr_to_member_type:
      case DW_TAG_pointer_type:
         indirectType = new typePointer(type_id(), typePointedTo, curName());
         indirectType = tc()->addOrUpdateType((typePointer *) indirectType );
         break;
      case DW_TAG_reference_type:
         indirectType = new typeRef(type_id(), typePointedTo, curName());
         indirectType = tc()->addOrUpdateType((typeRef *) indirectType );
         break;
      default:
         return false;
   }

   assert( indirectType != NULL );
   return true;
}

bool DwarfWalker::hasDeclaration(bool &isDecl) {
   Dwarf_Bool tmp;
   DWARF_FAIL_RET(dwarf_hasattr(entry(),
                                DW_AT_declaration,
                                &tmp, NULL ));
   isDecl = tmp;
   return true;
}

bool DwarfWalker::findTag() {
   Dwarf_Half dieTag;
   DWARF_FAIL_RET(dwarf_tag( entry(), & dieTag, NULL ));
   setTag(dieTag);
   return true;
}


bool DwarfWalker::findOffset() {
   Dwarf_Off dieOffset;
   DWARF_FAIL_RET(dwarf_dieoffset( entry(), & dieOffset, NULL ));
   
   setOffset(dieOffset);
   return true;
}

bool DwarfWalker::findEntryToUse(Dwarf_Half attr, bool &found, Dwarf_Die &e) {
   Dwarf_Bool hasAttr;
   DWARF_FAIL_RET(dwarf_hasattr(entry(),
                                attr,
                                &hasAttr, NULL ));
   if (hasAttr) {
      e = entry();
      found = true;
      return true;
   }

   DWARF_FAIL_RET(dwarf_hasattr(specEntry(),
                                attr,
                                &hasAttr, NULL ));
   if (hasAttr) {
      e = specEntry();
      found = true;
      return true;
   }

   DWARF_FAIL_RET(dwarf_hasattr(abstractEntry(),
                                attr,
                                &hasAttr, NULL ));
   if (hasAttr) {
      e = abstractEntry();
      found = true;
      return true;
   }
   found = false;
   return true;
}

bool DwarfWalker::handleAbstractOrigin(bool &isAbstract) {
   Dwarf_Die absE;

   dwarf_printf("(0x%lx) Checking for abstract origin\n", id());
   
   isAbstract = false;
   Dwarf_Bool isAbstractOrigin;

   DWARF_FAIL_RET(dwarf_hasattr(entry(),
                            DW_AT_abstract_origin, 
                            &isAbstractOrigin, NULL ));
   
   if (!isAbstractOrigin) return true;

   isAbstract = true;
   dwarf_printf("(0x%lx) abstract_origin is true, looking up reference\n", id());
   
   Dwarf_Attribute abstractAttribute;
   DWARF_FAIL_RET(dwarf_attr( entry(), DW_AT_abstract_origin, & abstractAttribute, NULL ));
   
   Dwarf_Off abstractOffset;
   if (!findDieOffset( abstractAttribute, abstractOffset )) return false;
   
   Dwarf_Bool is_info = dwarf_get_die_infotypes_flag(entry());
   DWARF_FAIL_RET(dwarf_offdie_b( dbg(), abstractOffset, is_info, & absE, NULL));
   
   dwarf_dealloc( dbg() , abstractAttribute, DW_DLA_ATTR );
   
   setAbstractEntry(absE);

   return true;
}

bool DwarfWalker::handleSpecification(bool &hasSpec) {
   Dwarf_Die specE; 

   dwarf_printf("(0x%lx) Checking for separate specification\n", id());
   hasSpec = false;

   Dwarf_Bool hasSpecification;
   DWARF_FAIL_RET(dwarf_hasattr( entry(), DW_AT_specification, & hasSpecification, NULL ));

   if (!hasSpecification) return true;

   hasSpec = true;
   
   dwarf_printf("(0x%lx) Entry has separate specification, retrieving\n", id());

   Dwarf_Attribute specAttribute;
   DWARF_FAIL_RET(dwarf_attr( entry(), DW_AT_specification, & specAttribute, NULL ));
   
   Dwarf_Off specOffset;
   if (!findDieOffset( specAttribute, specOffset )) return false;
   
   Dwarf_Bool is_info = dwarf_get_die_infotypes_flag(entry());
   DWARF_FAIL_RET(dwarf_offdie_b( dbg(), specOffset, is_info, & specE, NULL ));
   
   dwarf_dealloc( dbg(), specAttribute, DW_DLA_ATTR );

   setSpecEntry(specE);

   return true;
}

bool DwarfWalker::findDieName(Dwarf_Die die, std::string &name) {
   char *cname = NULL;

   Dwarf_Error error;
   int status = dwarf_diename( die, &cname, &error );     

   /* Squash errors from unsupported forms, like DW_FORM_GNU_strp_alt. */
   if (status == DW_DLV_ERROR) {
      if (dwarf_errno(error) == DW_DLE_ATTR_FORM_BAD) {
         status = DW_DLV_NO_ENTRY;
      }
      dwarf_dealloc( dbg(), error, DW_DLA_ERROR );
   }

   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   if (status != DW_DLV_OK) {
      name = std::string();
      return true;
   }

   name = cname;
   dwarf_dealloc( dbg(), cname, DW_DLA_STRING );
   return true;
}

bool DwarfWalker::findName(std::string &name) {
   if (!findDieName( specEntry(), name)) return false;
   dwarf_printf("(0x%lx) Found name %s\n", id(), name.c_str());
   return true;
}
   

bool DwarfWalker::findFuncName() {
   dwarf_printf("(0x%lx) Checking for function name\n", id());
   /* Prefer linkage names. */
   char *dwarfName = NULL;

   Dwarf_Attribute linkageNameAttr;

   int status = dwarf_attr(entry(), DW_AT_MIPS_linkage_name, &linkageNameAttr, NULL);
   if (status != DW_DLV_OK)
      status = dwarf_attr(entry(), DW_AT_linkage_name, &linkageNameAttr, NULL);
   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   if ( status == DW_DLV_OK )  {
      DWARF_FAIL_RET(dwarf_formstring( linkageNameAttr, &dwarfName, NULL ));
      curName() = dwarfName;
      setMangledName(true);
      dwarf_printf("(0x%lx) Found mangled name of %s, using\n", id(), curName().c_str());
      dwarf_dealloc( dbg(), linkageNameAttr, DW_DLA_ATTR );
      return true;
   } 

   findDieName( entry(), curName() );
   setMangledName(false);
   return true;
}

bool DwarfWalker::getFrameBase() {
   dwarf_printf("(0x%lx) Checking for frame pointer information\n", id());

   std::vector<VariableLocation> &funlocs = curFunc()->getFramePtrRefForInit();
   if (!funlocs.empty()) {
      DWARF_CHECK_RET(false);
   }

   if (!decodeLocationList(DW_AT_frame_base, NULL, funlocs)) return false;
   dwarf_printf("(0x%lx) After frame base decode, %d entries\n", id(), (int) funlocs.size());

   return true;
}

bool DwarfWalker::getReturnType(bool hasSpecification, Type *&returnType) {
   Dwarf_Attribute typeAttribute;
   int status = DW_DLV_OK;

   Dwarf_Bool is_info = true;
   if (hasSpecification) {
      is_info = dwarf_get_die_infotypes_flag(specEntry());
      status = dwarf_attr( specEntry(), DW_AT_type, & typeAttribute, NULL );
   }   
   if (!hasSpecification || (status == DW_DLV_NO_ENTRY)) {
      is_info = dwarf_get_die_infotypes_flag(entry());
      status = dwarf_attr( entry(), DW_AT_type, & typeAttribute, NULL );
   }


   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   if ( status == DW_DLV_NO_ENTRY ) {
     dwarf_printf("(0x%lx) Return type is void\n", id());
      return false;
   }

   /* There's a return type attribute. */
   dwarf_printf("(0x%lx) Return type is not void\n", id());

   bool ret = findAnyType( typeAttribute, is_info, returnType );
   dwarf_dealloc( dbg(), typeAttribute, DW_DLA_ATTR );
   return ret;
}

// I'm not sure how the provided fieldListType is different from curEnclosure(),
// but that's the way the code was structured and it was working. 
bool DwarfWalker::addFuncToContainer(Type *returnType) {
   /* Using the mangled name allows us to distinguish between overridden
      functions, but confuses the tests.  Since Type uses vectors
      to hold field names, however, duplicate -- demangled names -- are OK. */
   
   char * demangledName = P_cplus_demangle( curName().c_str(), symtab()->isNativeCompiler() );
   std::string toUse;

   if (!demangledName) {
      dwarf_printf("(0x%lx) Unable to demangle %s, using it as mangled\n", id(), curName().c_str());
      toUse = curName();
   }
   else {
      // Strip everything left of the rightmost ':' off to get rid of the class names
      toUse = demangledName;
      // rfind finds the last occurrence of ':'; add 1 to get past it. 
      size_t offset = toUse.rfind(':');
      if (offset != toUse.npos) {
         toUse = toUse.substr(offset+1);
      }
   }

   typeFunction *funcType = new typeFunction( type_id(), returnType, toUse);
   curEnclosure()->addField( toUse, funcType);
   free( demangledName );
   return true;
}

bool DwarfWalker::findType(Type *&type, bool defaultToVoid) {
  // Do *not* return true unless type is actually usable.
   int status;

   /* Acquire the parameter's type. */
   Dwarf_Attribute typeAttribute;
   status = dwarf_attr( specEntry(), DW_AT_type, & typeAttribute, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);

   if (status == DW_DLV_NO_ENTRY) {
      if (defaultToVoid) {
         type = tc()->findType("void");
	 return (type != NULL);
      }
      return false;
   }

   Dwarf_Bool is_info = dwarf_get_die_infotypes_flag(specEntry());

   bool ret = findAnyType( typeAttribute, is_info, type );
   dwarf_dealloc( dbg(), typeAttribute, DW_DLA_ATTR );
   return ret;
}

bool DwarfWalker::findDieOffset(Dwarf_Attribute attr, Dwarf_Off &offset) {
   Dwarf_Half form;
   DWARF_FAIL_RET(dwarf_whatform( attr, &form, NULL ));
   switch (form) {
      /* These forms are suitable as direct DIE offsets */
      case DW_FORM_ref1:
      case DW_FORM_ref2:
      case DW_FORM_ref4:
      case DW_FORM_ref8:
      case DW_FORM_ref_udata:
      case DW_FORM_ref_addr:
      case DW_FORM_data4:
      case DW_FORM_data8:
         DWARF_FAIL_RET(dwarf_global_formref( attr, &offset, NULL ));
         return true;

      /* Then there's DW_FORM_sec_offset which refer other sections, or
       * DW_FORM_GNU_ref_alt that refers to a whole different file.  We can't
       * use such forms as a die offset, even if dwarf_global_formref is
       * willing to decode it. */
      default:
         dwarf_printf("(0x%lx) Can't use form 0x%x as a die offset\n", id(), (int) form);
         return false;
   }
}

bool DwarfWalker::findAnyType(Dwarf_Attribute typeAttribute,
                              Dwarf_Bool is_info, Type *&type) {
   /* If this is a ref_sig8, look for the type elsewhere. */
   Dwarf_Half form;
   DWARF_FAIL_RET(dwarf_whatform(typeAttribute, &form, NULL));
   if (form == DW_FORM_ref_sig8) {
      Dwarf_Sig8 signature;
      DWARF_FAIL_RET(dwarf_formsig8(typeAttribute, &signature, NULL));
      return findSig8Type(&signature, type);
   }

   Dwarf_Off typeOffset;
   if (!findDieOffset( typeAttribute, typeOffset )) return false;

   /* NB: It's possible for an incomplete type to have a DW_AT_signature
    * reference to a complete definition.  For example, GCC may output just the
    * subprograms for a struct's methods in one CU, with the full struct
    * defined in a type unit.
    *
    * No DW_AT_type has been found referencing an incomplete type this way,
    * but if it did, here's the place to look for that DW_AT_signature and
    * recurse into findAnyType again.
    */

   typeId_t type_id = get_type_id(typeOffset, is_info);

   dwarf_printf("(0x%lx) Returned type offset 0x%x\n", id(), (int) typeOffset);
   /* The typeOffset forms a module-unique type identifier,
      so the Type look-ups by it rather than name. */
   type = tc()->findOrCreateType( type_id );
   dwarf_printf("(0x%lx) Returning type %p / %s for id 0x%x\n", 
                id(), 
                type, type->getName().c_str(),
                type_id);
   return true;
}

bool DwarfWalker::getLineInformation(Dwarf_Unsigned &variableLineNo,
                                     bool &hasLineNumber,
                                     std::string &fileName) {
   Dwarf_Attribute fileDeclAttribute;
   int status = dwarf_attr( specEntry(), DW_AT_decl_file, & fileDeclAttribute, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);

   if (status == DW_DLV_NO_ENTRY) {
      fileName = "";
   }
   else if (status == DW_DLV_OK) {
      Dwarf_Unsigned fileNameDeclVal;
      DWARF_FAIL_RET(dwarf_formudata(fileDeclAttribute, &fileNameDeclVal, NULL));
      dwarf_dealloc( dbg(), fileDeclAttribute, DW_DLA_ATTR );			
      if (fileNameDeclVal > srcFiles().size()) {
         dwarf_printf("Dwarf error reading line index %d from srcFiles of size %lu\n",
                      fileNameDeclVal, srcFiles().size());
         return false;
      }
      fileName = srcFiles()[fileNameDeclVal-1];
   }
   else {
      return true;
   }

   Dwarf_Attribute lineNoAttribute;
   status = dwarf_attr( specEntry(), DW_AT_decl_line, & lineNoAttribute, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   if (status != DW_DLV_OK) return true;
   
   /* We don't need to tell Dyninst a line number for C++ static variables,
      so it's OK if there isn't one. */
   hasLineNumber = true;
   DWARF_FAIL_RET(dwarf_formudata( lineNoAttribute, & variableLineNo, NULL ));
   dwarf_dealloc( dbg(), lineNoAttribute, DW_DLA_ATTR );			

   return true;
}

bool DwarfWalker::decodeLocationList(Dwarf_Half attr, 
                                     Address *initialStackValue, 
                                     std::vector<VariableLocation> &locs) {
   dwarf_printf("(0x%lx) decodeLocationList for attr %d\n", id(), attr);

   Dwarf_Bool hasAttr = false;
   DWARF_FAIL_RET(dwarf_hasattr(entry(), attr, &hasAttr, NULL));

   if (!hasAttr) {
      dwarf_printf("(0x%lx): no such attribute\n", id());
      return true;
   }

   locs.clear();
   
   /* Acquire the location of this formal parameter. */
   Dwarf_Attribute locationAttribute;
   DWARF_FAIL_RET(dwarf_attr( entry(), attr, & locationAttribute, NULL ));

   bool isExpr = false;
   bool isConstant = false;
   Dwarf_Half form;
   if (!checkForConstantOrExpr(attr, locationAttribute, isConstant, isExpr, form))
     return false;
   dwarf_printf("(0x%lx) After checkForConstantOrExpr, form class is 0x%x\n",id(), form);
   if (isConstant) {
     dwarf_printf("(0x%lx) Decoding constant location\n", id());
      if (!decodeConstantLocation(locationAttribute, form, locs)) return false;
   }
   else if (isExpr) {
     dwarf_printf("(0x%lx) Decoding expression without location list\n", id());
     if (!decodeExpression(locationAttribute, locs)) return false;
   }   
   else {
     dwarf_printf("(0x%lx) Decoding loclist location\n", id());
      Dwarf_Locdesc **locationList;
      Dwarf_Signed listLength;
      int status = dwarf_loclist_n( locationAttribute, & locationList, & listLength, NULL );
      
      dwarf_dealloc( dbg(), locationAttribute, DW_DLA_ATTR );
      
      if (status != DW_DLV_OK) {
	

	dwarf_printf("(0x%lx) Failed loclist decode: %d\n", id(), status);
	return true;
      }
      
      dwarf_printf("(0x%lx) location list with %d entries found\n", id(), (int) listLength);
      
      if (!decodeLocationListForStaticOffsetOrAddress( locationList,
                                                       listLength, 
                                                       locs, 
                                                       initialStackValue)) return false;
      
      deallocateLocationList( locationList, listLength );
   }

   return true;
}

bool DwarfWalker::checkForConstantOrExpr(Dwarf_Half attr,
					 Dwarf_Attribute &locationAttribute,
					 bool &constant,
					 bool &expr,
					 Dwarf_Half &form) {
   constant = false;
   // Get the form (datatype) for this particular attribute
   DWARF_FAIL_RET(dwarf_whatform(locationAttribute, &form, NULL));
   
   // And see if it's a constant
   Dwarf_Form_Class formtype = dwarf_get_form_class(version, attr, offset_size, form);
   dwarf_printf("(0x%lx) Checking for constant, formtype is 0x%x looking for 0x%x\n", id(), formtype, DW_FORM_CLASS_CONSTANT);

   if (formtype == DW_FORM_CLASS_CONSTANT) {
      constant = true;
   }
   else if (formtype == DW_FORM_CLASS_EXPRLOC) {
     expr = true;
   }
   return true;
}

bool DwarfWalker::findString(Dwarf_Half attr,
                             std::string &str)
{
   Dwarf_Half form;
   Dwarf_Attribute strattr;

   if (attr == DW_AT_call_file || attr == DW_AT_decl_file) {
      unsigned long line_index;
      bool result = findConstant(attr, line_index);
      if (!result)
         return false;
      if (line_index == 0) {
         str = string("");
         return true;
      }
      line_index--;
      if (line_index >= srcFiles().size()) {
         dwarf_printf("Dwarf error reading line index %d from srcFiles of size %lu\n",
                      line_index, srcFiles().size());
         return false;
      }
      str = srcFiles()[line_index];
      return true;
   }

   DWARF_FAIL_RET(dwarf_attr(entry(), attr, &strattr, NULL)); 
   int status = dwarf_whatform(strattr, &form, NULL);
   if (status != DW_DLV_OK) {
      dwarf_dealloc(dbg(), strattr, DW_DLA_ATTR);
      return false;
   }
   
   bool result;
   switch (form) {
      case DW_FORM_string: {
         char *s = NULL;
         DWARF_FAIL_RET(dwarf_formstring(strattr, &s, NULL));
         str  = s;
         result = true;
         break;
      }
      case DW_FORM_block:
      case DW_FORM_block1: 
      case DW_FORM_block2:
      case DW_FORM_block4: {
         Dwarf_Block *block = NULL;
         DWARF_FAIL_RET(dwarf_formblock(strattr, &block, NULL));
         str = string((char *) block->bl_data);
         dwarf_dealloc(dbg(), block, DW_DLA_BLOCK);
         result = !str.empty();
         break;
      }
      default:
         result = false;
         break;
   }
   dwarf_dealloc(dbg(), strattr, DW_DLA_ATTR);
   return result;
}

bool DwarfWalker::findConstant(Dwarf_Half attr,
                               Address &value) {
   Dwarf_Bool has = false;
   DWARF_FAIL_RET(dwarf_hasattr(entry(), attr, &has, NULL));
   if (!has) return false;

   // Get the attribute
   Dwarf_Attribute d_attr;
   DWARF_FAIL_RET(dwarf_attr(entry(), attr, &d_attr, NULL));

   Dwarf_Half form;
   // Get the form (datatype) for this particular attribute
   DWARF_FAIL_RET(dwarf_whatform(d_attr, &form, NULL));

   return findConstantWithForm(d_attr, form, value);
}

bool DwarfWalker::findConstantWithForm(Dwarf_Attribute &locationAttribute,
                                       Dwarf_Half form,
                                       Address &value) {
   value = 0;

   switch(form) {
      case DW_FORM_addr:
         Dwarf_Addr addr;
         DWARF_FAIL_RET(dwarf_formaddr(locationAttribute, &addr, NULL));
         value = (Address) addr;
         return true;
      case DW_FORM_sdata:
         Dwarf_Signed s_tmp;
         DWARF_FAIL_RET(dwarf_formsdata(locationAttribute, &s_tmp, NULL));
         value = (Address) s_tmp;
         dwarf_printf("(0x%lx) Decoded data of form %x to 0x%lx\n", 
                      id(), form, value);
         return true;
      case DW_FORM_data1:
      case DW_FORM_data2:
      case DW_FORM_data4:
      case DW_FORM_data8:
      case DW_FORM_udata:
         Dwarf_Unsigned u_tmp;
         DWARF_FAIL_RET(dwarf_formudata(locationAttribute, &u_tmp, NULL));
         value = (Address) u_tmp;
         return true;
   case DW_FORM_sec_offset:
     DWARF_FAIL_RET(dwarf_global_formref(locationAttribute, &u_tmp, NULL));
     value = (Address)(u_tmp);
     return true;
      default:
         dwarf_printf("(0x%lx) Unhandled form 0x%x for constant decode\n", id(), (unsigned) form);
         return false;
   }
}

bool DwarfWalker::decodeConstantLocation(Dwarf_Attribute &attr, Dwarf_Half form,
                                         std::vector<VariableLocation> &locs) {
   Address value;
   if (!findConstantWithForm(attr, form, value)) return false;
   if (!constructConstantVariableLocation(value, locs)) return false;
   return true;
}

bool DwarfWalker::constructConstantVariableLocation(Address value,
                                                    std::vector<VariableLocation> &locs) {
   
   VariableLocation loc;
   loc.stClass = storageAddr;
   loc.refClass = storageNoRef;
   loc.frameOffset = value;


   if (hasRanges()) {
      for (range_set_t::iterator i = ranges_begin(); i != ranges_end(); i++) {
         pair<Address, Address> range = *i;
         loc.lowPC = range.first;
         loc.hiPC = range.second;
         locs.push_back(loc);
      }
   }
   else {
      loc.lowPC = (Address) 0;
      loc.hiPC = (Address) -1;
      locs.push_back(loc);
   }
   
   return true;
}

bool DwarfWalker::findSize(unsigned &size) {
   Dwarf_Bool hasSize;
   DWARF_FAIL_RET(dwarf_hasattr(entry(), DW_AT_byte_size, &hasSize, NULL));
   if (!hasSize) return false;

   Dwarf_Attribute byteSizeAttr;
   Dwarf_Unsigned byteSize;
   DWARF_FAIL_RET(dwarf_attr( specEntry(), DW_AT_byte_size, & byteSizeAttr, NULL ));

   DWARF_FAIL_RET(dwarf_formudata( byteSizeAttr, & byteSize, NULL ));
   
   dwarf_dealloc( dbg(), byteSizeAttr, DW_DLA_ATTR );
   size = (unsigned) byteSize;
   return true;
}

bool DwarfWalker::findVisibility(visibility_t &visibility) {
   /* Acquire the visibility, if any.  DWARF calls it accessibility
      to distinguish it from symbol table visibility. */
   Dwarf_Attribute visAttr;
   int status = dwarf_attr( entry(), DW_AT_accessibility, & visAttr, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   if (status != DW_DLV_OK) {
      visibility = visPrivate;
      return true;
   }

   Dwarf_Unsigned visValue;
   DWARF_FAIL_RET(dwarf_formudata( visAttr, & visValue, NULL ));
   
   switch( visValue ) {
      case DW_ACCESS_public: visibility = visPublic; break;
      case DW_ACCESS_protected: visibility = visProtected; break;
      case DW_ACCESS_private: visibility = visPrivate; break;
      default:
         //bperr ( "Uknown visibility, ignoring.\n" );
         break;
   } /* end visibility switch */
   
   dwarf_dealloc( dbg(), visAttr, DW_DLA_ATTR );

   return true;
}

bool DwarfWalker::findValue(long &value, bool &valid) {
   Dwarf_Attribute valueAttr;
   int status = dwarf_attr( entry(), DW_AT_const_value, & valueAttr, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   
   if (status != DW_DLV_OK) {
      valid = false;
      return true;
   }

   Dwarf_Signed enumValue;

   DWARF_FAIL_RET(dwarf_formsdata(valueAttr, &enumValue, NULL));

   value = enumValue;
   valid = true;

   dwarf_dealloc( dbg(), valueAttr, DW_DLA_ATTR );
   return true;
}

bool DwarfWalker::fixBitFields(std::vector<VariableLocation> &locs,
                               long &size) {
   Dwarf_Attribute bitOffset;
   int status = dwarf_attr( entry(), DW_AT_bit_offset, & bitOffset, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   
   if ( status == DW_DLV_OK && locs.size() ) 
   {
      Dwarf_Unsigned memberOffset_du = locs[0].frameOffset;
      
      DWARF_FAIL_RET(dwarf_formudata( bitOffset, &memberOffset_du, NULL ));
      
      dwarf_dealloc( dbg(), bitOffset, DW_DLA_ATTR );
      
      Dwarf_Attribute bitSize;
      DWARF_FAIL_RET(dwarf_attr( entry(), DW_AT_bit_size, & bitSize, NULL ));
      
      Dwarf_Unsigned memberSize_du = size;
      DWARF_FAIL_RET(dwarf_formudata( bitSize, &memberSize_du, NULL ));

      dwarf_dealloc( dbg(), bitSize, DW_DLA_ATTR );
      
      /* If the DW_AT_byte_size field exists, there's some padding.
         FIXME?  We ignore padding for now.  (We also don't seem to handle
         bitfields right in getComponents() anyway...) */
   }
   else 
   {
      if (locs.size())
         locs[0].frameOffset *= 8;
      size *= 8;
   } /* end if not a bit field member. */
   return true;
}

bool DwarfWalker::fixName(std::string &name, Type *type) {
   switch(tag()){
      case DW_TAG_const_type:
         name = std::string("const ") + type->getName();
         break;
      case DW_TAG_packed_type:
         name = std::string("packed ") + type->getName();
         break;
      case DW_TAG_volatile_type:
         name = std::string("volatile ") + type->getName();
         break;
      default:
         return false;
   }
   return true;
}

void DwarfWalker::removeFortranUnderscore(std::string &name) {
      /* If we're fortran, get rid of the trailing _ */
   if (!curFunc() && !curEnclosure()) return;

   supportedLanguages lang = mod()->language();
   if ((lang != lang_Fortran) &&
       (lang != lang_CMFortran) &&
       (lang != lang_Fortran_with_pretty_debug)) return;
   
   if (name[name.length()-1] == '_') {
      name = name.substr(0, name.length()-1);
   }
}

bool DwarfWalker::parseSubrangeAUX(Dwarf_Die entry,
                                   std::string &loBound,
                                   std::string &hiBound) {
   dwarf_printf("(0x%lx) Parsing subrange /w/ auxiliary function\n", id());
   loBound = "{unknown or default}";
   hiBound = "{unknown or default}";

   /* Set the default lower bound, if we know it. */
   switch ( mod()->language() ) {
      case lang_Fortran:
      case lang_Fortran_with_pretty_debug:
      case lang_CMFortran:
         loBound = "1";
         break;
      case lang_C:
      case lang_CPlusPlus:
         loBound = "0";
         break;
      default:
         break;
   } /* end default lower bound switch */
   
   /* Look for the lower bound. */
   Dwarf_Attribute lowerBoundAttribute;
   Dwarf_Bool is_info = dwarf_get_die_infotypes_flag(entry);
   int status = dwarf_attr( entry, DW_AT_lower_bound, & lowerBoundAttribute, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   
   if ( status == DW_DLV_OK ) {
      if (!decipherBound(lowerBoundAttribute, is_info, loBound )) return false;
      dwarf_dealloc( dbg(), lowerBoundAttribute, DW_DLA_ATTR );
   } /* end if we found a lower bound. */
   
   /* Look for the upper bound. */
   Dwarf_Attribute upperBoundAttribute;
   status = dwarf_attr( entry, DW_AT_upper_bound, & upperBoundAttribute, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);

   if ( status == DW_DLV_NO_ENTRY ) {
      status = dwarf_attr( entry, DW_AT_count, & upperBoundAttribute, NULL );
      DWARF_CHECK_RET(status == DW_DLV_ERROR);
   }

   if ( status == DW_DLV_OK ) {
      if (!decipherBound(upperBoundAttribute, is_info, hiBound )) return false;
      dwarf_dealloc( dbg(), upperBoundAttribute, DW_DLA_ATTR );
   } /* end if we found an upper bound or count. */
   
   /* Construct the range type. */
   if (!findName(curName())) return false;
   if (!nameDefined()) {
      curName() = "{anonymousRange}";
   }

   Dwarf_Off subrangeOffset;
   DWARF_ERROR_RET(dwarf_dieoffset( entry, & subrangeOffset, NULL ));
   typeId_t type_id = get_type_id(subrangeOffset, is_info);

   errno = 0;
   unsigned long low_conv = strtoul(loBound.c_str(), NULL, 10);
   if (errno) {
      low_conv = LONG_MIN;
   }
   
   errno = 0;
   unsigned long hi_conv = strtoul(hiBound.c_str(), NULL, 10);
   if (errno)  {
      hi_conv = LONG_MAX;
   }  
   dwarf_printf("(0x%lx) Adding subrange type: id %d, low %ld, high %ld, named %s\n",
                id(), type_id, 
                low_conv, hi_conv, curName().c_str());
   typeSubrange * rangeType = new typeSubrange( type_id, 
                                                0, low_conv, hi_conv, curName() );
   assert( rangeType != NULL );
   rangeType = tc()->addOrUpdateType( rangeType );
   dwarf_printf("(0x%lx) Subrange has pointer %p (tc %p)\n", id(), rangeType, tc());
   return true;
}

typeArray *DwarfWalker::parseMultiDimensionalArray(Dwarf_Die range, 
                                                   Type * elementType)
{
  char buf[32];
  /* Get the (negative) typeID for this range/subarray. */
  Dwarf_Off dieOffset;
  DWARF_FAIL_RET_VAL(dwarf_dieoffset( range, & dieOffset, NULL ), NULL);

  /* Determine the range. */
  std::string loBound;
  std::string hiBound;
  parseSubrangeAUX(range, loBound, hiBound);

  /* Does the recursion continue? */
  Dwarf_Die nextSibling;
  Dwarf_Bool is_info = dwarf_get_die_infotypes_flag(range);
  int status = dwarf_siblingof_b( dbg(), range, is_info, & nextSibling, NULL );
  DWARF_CHECK_RET_VAL(status == DW_DLV_ERROR, NULL);

  snprintf(buf, 31, "__array%d", (int) offset());

  if ( status == DW_DLV_NO_ENTRY ) {
    /* Terminate the recursion by building an array type out of the elemental type.
       Use the negative dieOffset to avoid conflicts with the range type created
       by parseSubRangeDIE(). */
    // N.B.  I'm going to ignore the type id, and just create an anonymous type here
     std::string aName = buf;
     typeArray* innermostType = new typeArray( elementType, 
                                               atoi( loBound.c_str() ), 
                                               atoi( hiBound.c_str() ), 
                                               aName );
     assert( innermostType != NULL );
     Type * typ = tc()->addOrUpdateType( innermostType );
    innermostType = dynamic_cast<typeArray *>(typ);
    return innermostType;
  } /* end base-case of recursion. */

  /* If it does, build this array type out of the array type returned from the next recusion. */
  typeArray * innerType = parseMultiDimensionalArray( nextSibling, elementType);
  assert( innerType != NULL );
  // same here - type id ignored    jmo
  std::string aName = buf;
  typeArray * outerType = new typeArray( innerType, atoi(loBound.c_str()), atoi(hiBound.c_str()), aName);
  assert( outerType != NULL );
  Type *typ = tc()->addOrUpdateType( outerType );
  outerType = static_cast<typeArray *>(typ);

  dwarf_dealloc( dbg(), nextSibling, DW_DLA_DIE );
  return outerType;
} /* end parseMultiDimensionalArray() */

bool DwarfWalker::decipherBound(Dwarf_Attribute boundAttribute, Dwarf_Bool is_info,
                                std::string &boundString )
{
   Dwarf_Half boundForm;
   DWARF_FAIL_RET(dwarf_whatform( boundAttribute, & boundForm, NULL ));
   
   switch( boundForm ) {
      case DW_FORM_data1:
      case DW_FORM_data2:
      case DW_FORM_data4:
      case DW_FORM_data8:
      case DW_FORM_udata: 
      {
         dwarf_printf("(0x%lx) Decoding form %d with formudata\n",
                      id(), boundForm);
                     
         Dwarf_Unsigned constantBound;
         DWARF_FAIL_RET(dwarf_formudata( boundAttribute, & constantBound, NULL ));
         char bString[40];
         sprintf(bString, "%llu", (unsigned long long)constantBound);
         boundString = bString;
         return true;
      } break;

      case DW_FORM_sdata:
      {
         dwarf_printf("(0x%lx) Decoding form %d with formsdata\n",
                      id(), boundForm);
                     
         Dwarf_Signed constantBound;
         DWARF_FAIL_RET(dwarf_formsdata( boundAttribute, & constantBound, NULL ));
         char bString[40];
         sprintf(bString, "%lld", (long long)constantBound);
         boundString = bString;
         return true;
      } break;
      
      case DW_FORM_ref_addr:
      case DW_FORM_ref1:
      case DW_FORM_ref2:
      case DW_FORM_ref4:
      case DW_FORM_ref8:
      case DW_FORM_ref_udata: 
      {
         /* Acquire the referenced DIE. */
         Dwarf_Off boundOffset;
         DWARF_FAIL_RET(dwarf_global_formref( boundAttribute, & boundOffset, NULL ));
         
         Dwarf_Die boundEntry;
         DWARF_FAIL_RET(dwarf_offdie_b( dbg(), boundOffset, is_info, & boundEntry, NULL ));
         
         /* Does it have a name? */
         if (findDieName( boundEntry, boundString )
               && !boundString.empty())
            return true;
         
         /* Does it describe a nameless constant? */
         Dwarf_Attribute constBoundAttribute;
         int status = dwarf_attr( boundEntry, DW_AT_const_value, & constBoundAttribute, NULL );
         DWARF_CHECK_RET(status == DW_DLV_ERROR);
         
         if ( status == DW_DLV_OK ) {
            Dwarf_Unsigned constBoundValue;
            DWARF_FAIL_RET(dwarf_formudata( constBoundAttribute, & constBoundValue, NULL ));
            
            char bString[40];
            sprintf(bString, "%lu", (unsigned long)constBoundValue);
            boundString = bString;
            
            dwarf_dealloc( dbg(), boundEntry, DW_DLA_DIE );
            dwarf_dealloc( dbg(), constBoundAttribute, DW_DLA_ATTR );
            return true;
         }
         
         return false;
      } break;
      case DW_FORM_block:
      case DW_FORM_block1:
      {
         /* PGI extends DWARF to allow some bounds to be location lists.  Since we can't
            do anything sane with them, ignore them. */
         // Dwarf_Locdesc * locationList;
         // Dwarf_Signed listLength;
         // status = dwarf_loclist( boundAttribute, & locationList, & listLength, NULL );
         boundString = "{PGI extension}";
         return false;
      } break;
      
      default:
         //bperr ( "Invalid bound form 0x%x\n", boundForm );
         boundString = "{invalid bound form}";
         return false;
         break;
   } /* end boundForm switch */
   return true;
}

bool DwarfWalker::decodeExpression(Dwarf_Attribute &attr,
				   std::vector<VariableLocation> &locs) {
  Dwarf_Unsigned expr_len;
  Dwarf_Ptr expr_ptr;
  DWARF_FAIL_RET(dwarf_formexprloc(attr, &expr_len, &expr_ptr, NULL));
  unsigned char *bitstream = (unsigned char *) expr_ptr;

  // expr_ptr is a pointer to a bytestream. Try to turn it into a Dwarf_Locdesc so
  // we can use decodeDwarfExpression. 

  dwarf_printf("(0x%lx) bitstream for expr has len %d\n", id(), expr_len);
  for (unsigned i = 0; i < expr_len; ++i) {
    dwarf_printf("(0x%lx) \t %#hhx\n", id(), bitstream[i]);
  }

  Dwarf_Signed cnt;
  Dwarf_Locdesc *descs;

  DWARF_FAIL_RET(dwarf_loclist_from_expr_a(dbg(), expr_ptr, expr_len, addr_size, 
					   &descs, &cnt, NULL));
  assert(cnt == 1);

  bool ret = decodeLocationListForStaticOffsetOrAddress(&descs, cnt, locs, NULL);
  //deallocateLocationList(&descs, cnt);
  return ret;
}

bool DwarfWalker::decodeLocationListForStaticOffsetOrAddress( Dwarf_Locdesc **locationList, 
                                                              Dwarf_Signed listLength, 
                                                              std::vector<VariableLocation>& locs,
                                                              Address * initialStackValue)
{
   locs.clear();

  /* We make a few heroic assumptions about locations in this decoder.

  We assume that all locations are either frame base-relative offsets,
  encoded with DW_OP_fbreg, or are absolute addresses.  We assume these
  locations are invariant with respect to the PC, which implies that all
  location lists have a single entry.  We assume that no location is
  calculated at run-time.

  We make these assumptions to match the assumptions of the rest of
  Dyninst, which makes no provision for pc-variant or run-time calculated
  locations, aside from the frame pointer.  However, it assumes that a frame
  pointer is readily available, which, on IA-64, it is not.  For that reason,
  when we encounter a function with a DW_AT_frame_base (effectively all of them),
  we do NOT use this decoder; we decode the location into an AST, which we
  will use to calculate the frame pointer when asked to do frame-relative operations.
  (These calculations will be invalid until the frame pointer is established,
  which may require some fiddling with the location of the 'entry' instpoint.) */

  /* We now parse the complete location list for variables and parameters within a
   * function. We still ignore the location list defined for DW_AT_frame_base of the
   * function as the frame pointer is readily available on all platforms(except for IA64)
   * May be we would need to parse the location list for IA64 functions to store the 
   * register numbers and offsets and use it based on the pc value. 
   */

   uint64_t max_addr = (addr_size == 4) ? 0xffffffff : 0xffffffffffffffff;
   Address base = modLow;
 
   for (unsigned locIndex = 0 ; locIndex < listLength; locIndex++) {
                   
      /* There is only one location. */
      Dwarf_Locdesc *location = locationList[locIndex];

      VariableLocation loc;
      // Initialize location values.
      loc.stClass = storageAddr;
      loc.refClass = storageNoRef;

      // If location == 0..-1, it's "unset" and we keep the big range unless
      // we're in a lexical block construct. 
      // 
      dwarf_printf("(0x%lx) Decoding entry %d of %d over range 0x%lx - 0x%lx, mod 0x%lx - 0x%lx, base 0x%lx\n", 
                   id(), locIndex+1, (int) listLength,
                   (long) location->ld_lopc,
                   (long) location->ld_hipc,
                   modLow, modHigh, base);

      if (location->ld_lopc == max_addr) {
         //This is a base address selection entry, which changes the base address of 
         // subsequent entries
         base = location->ld_hipc;
         continue;
      }

      long int *tmp = (long int *)initialStackValue;
      bool result = decodeDwarfExpression(location, tmp, loc,
                                          symtab()->getArchitecture());
      if (!result) {
         dwarf_printf("(0x%lx): decodeDwarfExpr failed\n", id());
         continue;
      }

      if (location->ld_lopc == 0 &&
          location->ld_hipc == (Dwarf_Addr) ~0) {
         // Unset low and high. Use the lexical block info if present, otherwise
         // pass through. 
         if (hasRanges()) {
            dwarf_printf("(0x%lx) Using lexical range\n", id());
            for (range_set_t::iterator i = ranges_begin(); i != ranges_end(); i++) {
               pair<Address, Address> range = *i;
               loc.lowPC = range.first;
               loc.hiPC = range.second;

               dwarf_printf("(0x%lx) Variable valid over range 0x%lx to 0x%lx\n", 
                            id(), loc.lowPC, loc.hiPC);
               locs.push_back(loc);
            }
         }
         else {
            dwarf_printf("(0x%lx) Using open location range\n", id());            
            loc.lowPC = location->ld_lopc;
            loc.hiPC = location->ld_hipc;

            dwarf_printf("(0x%lx) Variable valid over range 0x%lx to 0x%lx\n", 
                         id(), loc.lowPC, loc.hiPC);
            locs.push_back(loc);
         }
      }
      else {
         dwarf_printf("(0x%lx) Using lexical range, shifted by module low\n", id());
         loc.lowPC = location->ld_lopc + base;
         loc.hiPC = location->ld_hipc + base;

         dwarf_printf("(0x%lx) Variable valid over range 0x%lx to 0x%lx\n", 
                      id(), loc.lowPC, loc.hiPC);
         locs.push_back(loc);
      }
   }
   
   /* decode successful */
   return !locs.empty();
} /* end decodeLocationListForStaticOffsetOrAddress() */

void DwarfWalker::deallocateLocationList( Dwarf_Locdesc * locationList, 
                                          Dwarf_Signed listLength ) 
{
  for( int i = 0; i < listLength; i++ ) {
     dwarf_dealloc( dbg(), locationList[i].ld_s, DW_DLA_LOC_BLOCK );
  }
  dwarf_dealloc( dbg(), locationList, DW_DLA_LOCDESC );
} /* end deallocateLocationList() */

void DwarfWalker::deallocateLocationList( Dwarf_Locdesc ** locationList, 
                                          Dwarf_Signed listLength ) 
{
  for( int i = 0; i < listLength; i++ ) {
     dwarf_dealloc( dbg(), locationList[i]->ld_s, DW_DLA_LOC_BLOCK );
     dwarf_dealloc( dbg(), locationList[i], DW_DLA_LOCDESC );
  }
  dwarf_dealloc( dbg(), locationList, DW_DLA_LIST );
} /* end deallocateLocationList() */

void DwarfWalker::setEntry(Dwarf_Die entry) {
   contexts_.setEntry(entry);
   contexts_.setSpecEntry(entry);
   contexts_.setAbstractEntry(entry);
}
void DwarfWalker::Contexts::push() {
   if (c.empty()) {
      c.push(Context());
   }
   else {
      c.push(Context(c.top()));
   }
}

void DwarfWalker::Contexts::pop() {
   assert(!c.empty());
   c.pop();
}

void DwarfWalker::Contexts::setFunc(FunctionBase *f) {
  // Bug workaround; if we're setting a function, ignore
  // any preceding lexical information since we probably 
  // nested. 
  c.top().func = f;
}

void DwarfWalker::Contexts::clearFunc() {
  // We can't edit in the middle of the stack...

  std::stack<Context> repl;
  while (!c.empty()) {
    repl.push(c.top());
    c.pop();
  }

  while (!repl.empty()) {
    c.push(repl.top());
    c.top().func = NULL;
    repl.pop();
  }
}

typeId_t DwarfWalker::get_type_id(Dwarf_Off offset, bool is_info)
{
  auto& type_ids = is_info ? info_type_ids_ : types_type_ids_;
  auto it = type_ids.find(offset);
  if (it != type_ids.end())
    return it->second;

  size_t size = info_type_ids_.size() + types_type_ids_.size();
  typeId_t id = (typeId_t) size + 1;
  type_ids[offset] = id;
  return id;
}

typeId_t DwarfWalker::type_id()
{
  Dwarf_Bool is_info = dwarf_get_die_infotypes_flag(entry());
  return get_type_id(offset(), is_info);
}

void DwarfWalker::findAllSig8Types()
{
   /* First .debug_types (0), then .debug_info (1).
    * In DWARF4, only .debug_types contains DW_TAG_type_unit,
    * but DWARF5 is considering them for .debug_info too.*/
   for (int i = 0; i < 2; ++i) {
      Dwarf_Bool is_info = i;
      compile_offset = next_cu_header = 0;

      /* Iterate over the compilation-unit headers. */
      while (dwarf_next_cu_header_c(dbg(), is_info,
                                    &cu_header_length,
                                    &version,
                                    &abbrev_offset,
                                    &addr_size,
                                    &offset_size,
                                    &extension_size,
                                    &signature,
                                    &typeoffset,
                                    &next_cu_header, NULL) == DW_DLV_OK ) {
         parseModuleSig8(is_info);
         compile_offset = next_cu_header;
      }
   }
}

bool DwarfWalker::parseModuleSig8(Dwarf_Bool is_info)
{
   /* Obtain the type DIE. */
   Dwarf_Die typeDIE;
   DWARF_FAIL_RET(dwarf_siblingof_b( dbg(), NULL, is_info, &typeDIE, NULL ));

   /* Make sure we've got the right one. */
   Dwarf_Half typeTag;
   DWARF_FAIL_RET(dwarf_tag( typeDIE, & typeTag, NULL ));

   if (typeTag != DW_TAG_type_unit)
      return false;

   /* typeoffset is relative to the type unit; we want the global offset. */
   Dwarf_Off cu_off, cu_length;
   DWARF_FAIL_RET(dwarf_die_CU_offset_range( typeDIE, &cu_off, &cu_length, NULL ));

   uint64_t sig8 = * reinterpret_cast<uint64_t*>(&signature);
   typeId_t type_id = get_type_id(cu_off + typeoffset, is_info);
   sig8_type_ids_[sig8] = type_id;

   dwarf_printf("Mapped Sig8 {%016llx} to type id 0x%x\n", (long long) sig8, type_id);
   return true;
}

bool DwarfWalker::findSig8Type(Dwarf_Sig8 *signature, Type *&returnType)
{
   uint64_t sig8 = * reinterpret_cast<uint64_t*>(signature);
   auto it = sig8_type_ids_.find(sig8);
   if (it != sig8_type_ids_.end()) {
      typeId_t type_id = it->second;
      returnType = tc()->findOrCreateType( type_id );
      dwarf_printf("Found Sig8 {%016llx} as type id 0x%x\n", (long long) sig8, type_id);
      return true;
   }

   dwarf_printf("Couldn't find Sig8 {%016llx}!\n", (long long) sig8);
   return false;
}
