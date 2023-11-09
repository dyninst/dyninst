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

#include "common/src/vgannotations.h"
#include "compiler_diagnostics.h"
#include "dwarfWalker.h"
#include "headers.h"
#include "Module.h"
#include "Symtab.h"
#include "Collections.h"
#include "dwarf.h"
#include "Object.h"
#include "Object-elf.h"
#include "Function.h"
#include "debug.h"
#include "dwarfExprParser.h"
#include "pathName.h"
#include "debug_common.h"
#include "Type-mem.h"
#include <elfutils/libdw.h>
#include <dwarf/src/dwarf_subrange.h>
#include <dwarf_names.h>
#include <dwarf_cu_info.h>
#include <stack>

using namespace Dyninst;
using namespace SymtabAPI;
using namespace DwarfDyninst;
using namespace std;

#define DWARF_FAIL_RET_VAL(x, v) do  {                                  \
      int dwarf_fail_ret_val_status = (x);                              \
      if (dwarf_fail_ret_val_status != 0) {                             \
         types_printf("[%s:%d]: libdwarf returned %d, ret false\n",     \
                 FILE__, __LINE__, dwarf_fail_ret_val_status);          \
         return (v);                                                    \
      }                                                                 \
   } while (0)
#define DWARF_FAIL_RET(x) DWARF_FAIL_RET_VAL(x, false)

#define DWARF_ERROR_RET_VAL(x, v) do  {                                 \
      int dwarf_error_ret_val_status = (x);                             \
      if (dwarf_error_ret_val_status == 1 /*DW_DLV_ERROR*/) {           \
         types_printf("[%s:%d]: parsing failure, ret false\n",          \
                 FILE__, __LINE__);                                     \
         return (v);                                                    \
      }                                                                 \
   } while (0)
#define DWARF_ERROR_RET(x) DWARF_ERROR_RET_VAL(x, false)

#define DWARF_CHECK_RET_VAL(x, v) do  {                                 \
      if (x) {                                                          \
         types_printf("[%s:%d]: parsing failure, ret false\n",          \
                 FILE__, __LINE__);                                     \
         return (v);                                                    \
      }                                                                 \
   } while (0)
#define DWARF_CHECK_RET(x) DWARF_CHECK_RET_VAL(x, false)

DwarfWalker::DwarfWalker(Symtab *symtab, ::Dwarf *dbg, std::shared_ptr<ParsedFuncs> pf) :
   DwarfParseActions(symtab, dbg),
   parsedFuncs(pf),
   is_mangled_name_(false),
   modLow(0),
   modHigh(0),
   cu_header_length(0),
   abbrev_offset(0),
   addr_size(0),
   offset_size(0),
   extension_size(0),
   signature(),
   typeoffset(0),
   next_cu_header(0),
   compile_offset(0)
{
    if (!parsedFuncs)  {
        parsedFuncs = std::make_shared<ParsedFuncs>();
    }
}

DwarfWalker::~DwarfWalker() {
}

static inline void ompc_leftmost(Module* &out, Module* &in) {
    out = out == NULL ? out : in;
}
#pragma omp declare \
    reduction(leftmost : Module* : ompc_leftmost(omp_out, omp_in)) \
    initializer(omp_priv = NULL)

bool DwarfWalker::parse() {
    dwarf_printf("In DwarfWalker::parse() Parsing DWARF for %s, dgb():0x%p\n",filename().c_str(), (void*)dbg());

    /* Start the dwarven debugging. */
    Module *fixUnknownMod = NULL;
    mod() = NULL;

    /* Prepopulate type signatures for DW_FORM_ref_sig8 */
    findAllSig8Types();

    /* First .debug_types (0), then .debug_info (1).
     * In DWARF4, only .debug_types contains DW_TAG_type_unit,
     * but DWARF5 is considering them for .debug_info too.*/

    /* NB: parseModule used to compute compile_offset as 11 bytes before the
     * first die offset, to account for the header.  This would need 23 bytes
     * instead for 64-bit format DWARF, and even more for type units.
     * (See DWARF4 sections 7.4 & 7.5.1.)
     * But more directly, we know the first CU is just at 0x0, and each
     * following CU is already reported in next_cu_header.
     */
    compile_offset = next_cu_header = 0;

    /* Iterate over the compilation-unit headers for .debug_types. */
    uint64_t type_signaturep;
    std::vector<Dwarf_Die> module_dies;
    for(Dwarf_Off cu_off = 0;
            dwarf_next_unit(dbg(), cu_off, &next_cu_header, &cu_header_length,
                NULL, &abbrev_offset, &addr_size, &offset_size,
                &type_signaturep, NULL) == 0;
            cu_off = next_cu_header)
    {
        if(!dwarf_offdie_types(dbg(), cu_off + cu_header_length, &current_cu_die))
            continue;
        module_dies.push_back(current_cu_die);
        compile_offset = next_cu_header;
    }
    size_t total_from_unit = module_dies.size();
    dwarf_printf("Modules from dwarf_next_unit: %zu\n", total_from_unit);

    /* Iterate over the compilation-unit headers for .debug_info. */
    for(Dwarf_Off cu_off = 0;
            dwarf_nextcu(dbg(), cu_off, &next_cu_header, &cu_header_length,
                &abbrev_offset, &addr_size, &offset_size) == 0;
            cu_off = next_cu_header)
    {
        if(!dwarf_offdie(dbg(), cu_off + cu_header_length, &current_cu_die))
            continue;
        module_dies.push_back(current_cu_die);
        compile_offset = next_cu_header;
    }
    compile_offset = 0;
    dwarf_printf("Modules from dwarf_nextcu: %zu\n", module_dies.size() - total_from_unit);

    if (dwarf_getalt(dbg()) != NULL) {
        DwarfWalker w(symtab(), dbg());
        for (unsigned int i = 0; i < module_dies.size(); i++) {
            w.push();
            w.parseModule(module_dies[i],fixUnknownMod);
            w.pop();
        }
    } else {
#pragma omp parallel
    {
    DwarfWalker w(symtab(), dbg(), parsedFuncs);
#pragma omp for reduction(leftmost:fixUnknownMod) \
        schedule(dynamic) nowait
    for (unsigned int i = 0; i < module_dies.size(); i++) {
        w.push();
        w.parseModule(module_dies[i],fixUnknownMod);
        w.pop();
    }
    }
    }

    if (!fixUnknownMod)
        return true;

    dwarf_printf("Fixing types for final module %s\n", fixUnknownMod->fileName().c_str());

   /* Fix type list. */
   typeCollection *moduleTypes = typeCollection::getModTypeCollection(fixUnknownMod);
   if(!moduleTypes) return false;
   auto typeIter =  moduleTypes->typesByID.begin();
   for (;typeIter!=moduleTypes->typesByID.end();typeIter++)
   {
      typeIter->second->fixupUnknowns(fixUnknownMod);
   }

   /* Fix the types of variables. */
   std::string variableName;
   auto variableIter = moduleTypes->globalVarsByName.begin();
   for (;variableIter!=moduleTypes->globalVarsByName.end();variableIter++)
   {
      if (variableIter->second->getDataClass() == dataUnknownType &&
          moduleTypes->findType( variableIter->second->getID(), Type::share ) != NULL )
      {
         variableIter->second = moduleTypes->findType(variableIter->second->getID(), Type::share);
      }
   }

    moduleTypes->setDwarfParsed();
    return true;
}
bool DwarfWalker::parseModule(Dwarf_Die moduleDIE, Module *&fixUnknownMod) {

    // Make sure `moduleDIE` is actually a compilation unit
    if (!DwarfDyninst::is_parseable_unit(moduleDIE)) {
    	dwarf_printf("(0x%lx) Attempting to parse unit that isn't parseable.\n", id());
        return false;
    }

    std::string moduleName = DwarfDyninst::die_name(moduleDIE);

    auto moduleTag = dwarf_tag(&moduleDIE);
    if (moduleName.empty() && moduleTag == DW_TAG_type_unit) {
      uint64_t sig8 = * reinterpret_cast<uint64_t*>(&signature);
      char buf[20];
      snprintf(buf, sizeof(buf), "{%016llx}", (unsigned long long)sig8);
      moduleName = buf;
    }

    dwarf_printf("Next DWARF module: %s with DIE %p and tag %d\n", moduleName.c_str(), (void*)moduleDIE.addr, moduleTag);

    /* Set the language, if any. */
    Dwarf_Attribute languageAttribute;
    dwarf_attr(&moduleDIE, DW_AT_language, &languageAttribute);

    // Set low and high ranges; this can fail, so don't check return addr.
    setEntry(moduleDIE);

    // These may not be set.
    Address tempModLow, tempModHigh;
    modLow = modHigh = 0;
    Dwarf_Die e;
    bool found_entry = dwarf_offdie(dbg(),offset(), &e);
    if(!found_entry) return false;

    if (findConstant(DW_AT_low_pc, tempModLow, &e, dbg())) {
        modLow = convertDebugOffset(tempModLow);
    }
    if (findConstant(DW_AT_high_pc, tempModHigh, &e, dbg())) {
        modHigh = convertDebugOffset(tempModHigh);
    }

    // Find the Symtab Module corresponding to this CU
    mod() = [this]() {
      SymtabAPI::Module* m = symtab()->findModuleByOffset(offset());
      if(m) return m;
      return symtab()->getDefaultModule();
    }();

    dwarf_printf("Mapped to Symtab module %p from '%s' at offset %zx\n", (void*)mod(), mod()->fileName().c_str(), offset());

    if(!fixUnknownMod){
      fixUnknownMod = mod();
    }

    if (!parse_int(moduleDIE, true))
        return false;

    return true;
}

bool DwarfWalker::buildSrcFiles(::Dwarf * /*dbg*/, Dwarf_Die entry, StringTablePtr srcFiles) {
    size_t cnt = 0;
    Dwarf_Files * df;
    int ret = dwarf_getsrcfiles(&entry, &df, &cnt);
    if(ret==-1) return true;

    boost::unique_lock<dyn_mutex> l(srcFiles->lock);
    if(!srcFiles->empty()) {
        return true;
    } // already parsed, the module had better be right.

    // The CU name is always an absolute path
    auto comp_dir = Dyninst::DwarfDyninst::cu_dirname(entry);

    // store all file sources found by libdw
    for (unsigned i = 0; i < cnt; ++i) {
        auto filename = dwarf_filesrc(df, i, NULL, NULL);
        if(!filename) continue;
        srcFiles->emplace_back(DwarfDyninst::detail::absolute_path(filename, comp_dir),"");
    }
    return true;
}


// As mentioned in the header, this is separate from parse()
// so we can have a non-Context-creating parse method that reuses
// the Context from the parent. This allows us to pass in current
// function, etc. without the Context stack exploding
bool DwarfWalker::parse_int(Dwarf_Die e, bool parseSib, bool dissociate_context) {
    dwarf_printf("PARSE_INT entry, context size %d\n", stack_size());
    // We escape the loop by checking parseSibling() after
    // parsing this DIE and its children, if any
    while(1) {
        ContextGuard cg(*this, dissociate_context);

        setEntry(e);
        setParseSibling(parseSib);

        if (!findTag())
            return false;
        name_.clear();
        setMangledName(false);

        Dwarf * desc = dwarf_cu_getdwarf(e.cu);
        dwarf_printf("(0x%lx) Parsing entry with context size %d, func %s, encl %p, (%s), mod:%s, tag: %x\n",
                id(), stack_size(),
                curFunc()?curFunc()->getName().c_str():"(N/A)",
                (void*)curEnclosure().get(), (dbg()!=desc)?"sup":"not sup",
                mod()->fileName().c_str(), (unsigned int)dwarf_tag(&e));

        bool ret = false;
        switch(dwarf_tag(&e)) {
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
            case DW_TAG_try_block:
                ret = parseTryBlock();
                break;
            case DW_TAG_catch_block:
                ret = parseCatchBlock();
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
            case DW_TAG_rvalue_reference_type:
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
            case DW_TAG_imported_unit:
                {
                    dwarf_printf("(0x%lx) Imported unit, parsing imported\n", id());
                    // Parse child
                    ret = parseChild();
                    // parse imported
                    Dwarf_Attribute importAttribute;
                    dwarf_attr(&e, DW_AT_import, &importAttribute);
                    Dwarf_Die importedDIE;
                    auto die_p = dwarf_formref_die(&importAttribute, &importedDIE);
                    Dwarf * desc1 = dwarf_cu_getdwarf(importedDIE.cu);
                    dwarf_printf("(0x%lx) Imported DIE dwarf_desc: %p\n", id(), (void*)desc1);
                    if(!die_p) break;
                    if (!parse_int(importedDIE, true))
                        return false;
                    break;
                }
            default:
                dwarf_printf("(0x%lx) Warning: unparsed entry with tag 0x%x, dwarf_tag(): 0x%x\n",
                        id(), tag(), (unsigned int)dwarf_tag(&e));
                ret = true;
                break;
        }

        dwarf_printf("Finished parsing 0x%lx, ret %d, parseChild %d, parseSibling %d\n\n",
                id(), ret, parseChild(), parseSibling());

        if (ret && parseChild() ) {
            // Parse children
            Dwarf_Die childDwarf;
            int status = dwarf_child(&e, &childDwarf);
            if(status == -1)
                return false;
            if (status == 0) {
                dwarf_printf("(0x%lx) Parsing child\n\n", id());
                if (!parse_int(childDwarf, true))
                    return false;
            }
        }

        if (!parseSibling()) {
            dwarf_printf("(0x%lx) Skipping sibling parse\n", id());
            break;
        }

        dwarf_printf("(0x%lx) Asking for sibling\n", id());

        Dwarf_Die siblingDwarf;
        int status = dwarf_siblingof(&e, &siblingDwarf);
        DWARF_CHECK_RET(status == -1);

        if (status == 1) {
            break;
        }
        e = siblingDwarf;
    }

    dwarf_printf("PARSE_INT exit, context size %d\n", stack_size());
    return true;
}

bool DwarfWalker::parseCallsite()
{
    int has_line = 0, has_file = 0;
    Dwarf_Die e = entry();
    has_file = dwarf_hasattr_integrate(&e, DW_AT_call_file);
    if (!has_file)
        return true;
    has_line = dwarf_hasattr_integrate(&e, DW_AT_call_line);
    if (!has_line)
        return true;

    using opt_string = boost::optional<std::string>;
    opt_string inline_file = find_call_file();
    if (!inline_file)
        return false;

    Dyninst::Offset inline_line;
    bool result = findConstant(DW_AT_call_line, inline_line, &e, dbg());
    if (!result)
        return false;

    InlinedFunction *ifunc = static_cast<InlinedFunction *>(curFunc());
    ifunc->setFile(inline_file.get());
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
      return createInlineFunc();
   }

   //Try to associate the function with existing symbols
   setFuncFromLowest(lowest);


   return true;
}

void DwarfWalker::setFuncFromLowest(Address lowest) {
   Function *f = NULL;
   bool result = symtab()->findFuncByEntryOffset(f, lowest);
   if (result) {
      setFunc(f);
      dwarf_printf("(0x%lx) Lookup by offset 0x%lx identifies %s\n",
                   id(), lowest, curFunc()->getName().c_str());
   } else {
     dwarf_printf("(0x%lx) Lookup by offset 0x%lx failed\n", id(), lowest);
   }
}

bool DwarfWalker::createInlineFunc() {
   FunctionBase *parent = curFunc();
   if (parent) {
         InlinedFunction *ifunc = new InlinedFunction(parent);
         setFunc(ifunc);
         dwarf_printf("(0x%lx) Created new inline, parent is %s\n", id(), parent->getName().c_str());
         return true;
      } else {
         //InlinedSubroutine without containing subprogram.  Weird.
         dwarf_printf("(0x%lx) setFunctionFromRange found inline without parent, returning false\n", id());
         return false;
      }
}

void DwarfParseActions::addMangledFuncName(std::string name)
{
   curFunc()->addMangledName(name, true, true);
}
void DwarfParseActions::addPrettyFuncName(std::string name)
{
   curFunc()->addPrettyName(name, true, true);
}


// helper class to set and restore currentSubprogramFunction
namespace {
class SetAndRestoreFunction
{
    public:
        SetAndRestoreFunction(FunctionBase* &v, FunctionBase* newFunc) : var(v)
        {
            savedFunc = var;
            var = newFunc;
        }
        ~SetAndRestoreFunction()
        {
            var = savedFunc;
        }
    private:
        FunctionBase* &var;
        FunctionBase* savedFunc;
};
}


bool DwarfWalker::parseSubprogram(DwarfWalker::inline_t func_type) {
   bool name_result;

   dwarf_printf("(0x%lx) parseSubprogram entry\n", id());
   parseRangeTypes(entry());
   setFunctionFromRange(func_type);

   // Name first
   FunctionBase *func = curFunc();
   name_result = findFuncName();
   if (curEnclosure() && !func) {
      // This is a member function; create the type entry
      // Since curFunc is false, we're not going back to reparse this
      // entry with a function object.
      boost::shared_ptr<Type> ftype = NULL;
      getReturnType(ftype);
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

   // this function's accessor lock to check if parsed and/or parse it
   ParsedFuncs::accessor funcParsed;

   // recursive if parsing a specification or abstract origin,
   // accessor is already locked
   bool isRecursiveSubprogramParse = (func == currentSubprogramFunction);

   if (!isRecursiveSubprogramParse)  {
       // check if function is parsed, hold mutex until scope is left
       parsedFuncs->insert(funcParsed, func);
       if (funcParsed->second) {
          dwarf_printf("(0x%lx) parseSubprogram not parsing children b/c curFunc() marked parsed in parsedFuncs\n", id());
          if(name_result) {
              dwarf_printf("\tname is %s\n", curName().c_str());
          }
          setParseChild(false);
          return true;
       }
   }

   // set the current function being parsed and restore old value on return
   SetAndRestoreFunction setAndRestore(currentSubprogramFunction, func);

   if (name_result && !curName().empty()) {
      dwarf_printf("(0x%lx) Identified function name as %s\n", id(), curName().c_str());
      if (isMangledName()) {
	  func->addMangledName(curName(), true);
      }
      // Only keep pretty names around for inlines, which probably don't have mangled names
      else {
          dwarf_printf("(0x%lx) Adding as pretty name to inline\n", id());
          func->addPrettyName(curName(), true);
      }
   }

   //Collect callsite information for inlined functions.
   if (func_type == InlinedFunc) {
      parseCallsite();
   }

   // Get the return type
   setFuncReturnType();

   // Get range information
   if (hasRanges()) {
       setRanges(func);
   }
   // Dwarf outlines some function information. You have the base entry, which contains
   // address ranges, frame base information, and optionally a "abstract origin"
   // or "specification" entry that points to more information.
   // We want to skip parsing specification or abstract entries until we have
   // the base entry and can find/create the corresponding function object.


   // Get the frame base if it exists
   if (!getFrameBase())
       return false;

   // Parse parent nodes and their children but not their sibling
   bool hasAbstractOrigin = false;
   if (!handleAbstractOrigin(hasAbstractOrigin))
       return false;
   if (hasAbstractOrigin) {
      dwarf_printf("(0x%lx) Parsing abstract parent\n", id());
      if (!parse_int(abstractEntry(), false))
          return false;
   }
   // An abstract origin will point to a specification if it exists
   // This can actually be three-layered, so backchain again
   bool hasSpecification = false;
   if (!handleSpecification(hasSpecification))
       return false;
   if ( hasSpecification ) {
      dwarf_printf("(0x%lx) Parsing specification entry\n", id());
      if (!parse_int(specEntry(), false))
          return false;
   }

   if (!isRecursiveSubprogramParse)  {
       // the function is now parsed
       funcParsed->second = true;
   }

    if (func_type == InlinedFunc) {
        dwarf_printf("End parseSubprogram for inlined func at 0x%p\n", (void*)func->getOffset());
    }

   return true;
}

void DwarfWalker::setRanges(FunctionBase *func) {
   if(func->ranges.empty()) {
	   Address last_low = 0, last_high = 0;
	   for (auto i = ranges_begin(); i != ranges_end(); i++) {
	       Address low = i->first;
	       Address high = i->second;
	       if (last_low == low && last_high == high)
		   continue;
	       last_low = low;
	       last_high = high;

           func->ranges.push_back(FuncRange(low, high - low, curFunc()));
	   }
    }
}

bool DwarfWalker::parseRangeTypes(Dwarf_Die die) {
   dwarf_printf("(0x%lx) Parsing ranges\n", id());

   clearRanges();
    std::vector<AddressRange> newRanges = getDieRanges(die);
    for(auto r = newRanges.begin();
        r != newRanges.end();
        ++r)
    {
        setRange(*r);
    }
   return !newRanges.empty();
}

vector<AddressRange> DwarfWalker::getDieRanges(Dwarf_Die die)
{
    std::vector<AddressRange> newRanges;

    Dwarf_Addr base;
    Dwarf_Addr start, end;
    ptrdiff_t offset = 0;
    while(1)
    {
        offset = dwarf_ranges(&die, offset, &base, &start, &end);
        if(offset < 0) return std::vector<AddressRange>();
        if(offset == 0) break;
        dwarf_printf("Lexical block from 0x%lx to 0x%lx\n", start, end);
        newRanges.push_back(AddressRange(start, end));
    }
    return newRanges;
}

bool DwarfWalker::parseLexicalBlock() {
   dwarf_printf("(0x%lx) Parsing lexical block\n", id());
   return parseRangeTypes(entry());
}

bool DwarfWalker::parseTryBlock() {
   dwarf_printf("(0x%lx) Parsing try block ranges\n", id());
   return parseRangeTypes(entry());
}

bool DwarfWalker::parseCatchBlock() {
   dwarf_printf("(0x%lx) Parsing catch block ranges\n", id());
   return parseRangeTypes(entry());
}

bool DwarfWalker::parseCommonBlock() {
   dwarf_printf("(0x%lx) Parsing common block\n", id());

   std::string commonBlockName = die_name();
   Symbol* commonBlockVar = findSymbolForCommonBlock(commonBlockName);
   if(!commonBlockVar)
   {
     return false;
   }

   setCommon(getCommonBlockType(commonBlockName));

   return true;
}

Symbol *DwarfWalker::findSymbolForCommonBlock(const string &commonBlockName) {
   return findSymbolByName(commonBlockName, Symbol::ST_OBJECT);
}

boost::shared_ptr<Type> DwarfWalker::getCommonBlockType(string &commonBlockName) {
   auto commonBlockType = tc()->findVariableType(commonBlockName, Type::share);
   if(!commonBlockType || !commonBlockType->isCommonType()) {
     commonBlockType = Type::make_shared<typeCommon>( type_id(), commonBlockName );
     tc()->addGlobalVariable(commonBlockType);
   }
   return commonBlockType;
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

   boost::shared_ptr<Type> type;
   if (!findType(type, false))
       return false;
   if(!type)
       return false;

   /* If this DIE has a _specification, use that for the rest of our inquiries. */
   bool hasSpecification = false;
   if (!handleSpecification(hasSpecification))
       return false;

   curName() = die_name();
   removeFortranUnderscore(curName());

   /* We'll start with the location, since that's most likely to
      require the _specification. */

   std::vector<VariableLocation> locs;
   if (!decodeLocationList(DW_AT_location, NULL, locs))
       return false;
   if (locs.empty()) return true;

   for (unsigned i=0; i<locs.size(); i++) {
      if (locs[i].lowPC) {
         locs[i].lowPC = convertDebugOffset(locs[i].lowPC);
      }
      if (locs[i].hiPC) {
         locs[i].hiPC = convertDebugOffset(locs[i].hiPC);
      }
   }

   /* If the DIE has an _abstract_origin, we'll use that for the
            remainder of our inquiries. */
   bool hasAbstractOrigin;
   if (!handleAbstractOrigin(hasAbstractOrigin)) return false;
   if (hasAbstractOrigin) {
          // Clone to spec entry too
          setSpecEntry(abstractEntry());
   }

   Dwarf_Word variableLineNo;
   bool hasLineNumber = false;
   std::string fileName;

   if (!curFunc() && !curEnclosure()) {
      createGlobalVariable(locs, type);

   }
   else
   {
      if (!getLineInformation(variableLineNo, hasLineNumber, fileName))
          return false;
      if (!nameDefined()) return true;
      if (curFunc()) {
         /* We now have the variable name, type, offset, and line number.
            Tell Dyninst about it. */

         createLocalVariable(locs, type, variableLineNo, fileName);

      }
      else {
         auto ret = addStaticClassVariable(locs, type);
         return ret;
      }
   }
   return true;
}

bool DwarfWalker::addStaticClassVariable(const vector<VariableLocation> &locs, boost::shared_ptr<Type> type) {
   if( locs[0].stClass != storageRegOffset )
   {
      dwarf_printf("(0x%lx) Adding variable to an enclosure\n", id());
      curEnclosure()->asFieldListType().addField(curName(), type, locs[0].frameOffset);
      return true;
   }
   return false;
}

void DwarfWalker::createGlobalVariable(const vector<VariableLocation> &locs, boost::shared_ptr<Type> type) {
   /* The typeOffset forms a module-unique type identifier,
         so the Type look-ups by it rather than name. */
   dwarf_printf("(0x%lx) Adding global variable\n", id());

   Offset addr = 0;
   if (locs.size() && locs[0].stClass == storageAddr)
         addr = locs[0].frameOffset;
   std::vector<Variable *> vars;
   bool result = symtab()->findVariablesByOffset(vars, addr);
   if (result)  {
	for (auto v: vars)  {
	    v->setType(type);
	}
   }
   tc()->addGlobalVariable(type);
}

void DwarfWalker::createLocalVariable(const vector<VariableLocation> &locs, boost::shared_ptr<Type> type,
                                      Dwarf_Word variableLineNo,
                                      const string &fileName) {
   localVar * newVariable = new localVar(curName(),
                                         type,
                                         fileName,
                                         (int) variableLineNo,
                                         curFunc());
   dwarf_printf("(0x%lx) Created localVariable '%s' (%p), currentFunction %p\n",
                   id(), curName().c_str(), (void*)newVariable, (void*)curFunc());

   for (unsigned int i = 0; i < locs.size(); ++i) {
         dwarf_printf("(0x%lx) (%s) Adding location %u of %d: (0x%lx - 0x%lx): %s, %s, %s, %ld\n",
                      id(), newVariable->getName().c_str(), i + 1, (int) locs.size(), locs[i].lowPC, locs[i].hiPC,
                      storageClass2Str(locs[i].stClass),
                      storageRefClass2Str(locs[i].refClass),
                      locs[i].mr_reg.name().c_str(),
                      locs[i].frameOffset);
         newVariable->addLocation(locs[i]);
      }
   curFunc()->addLocalVar(newVariable);
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

   /* Acquire the parameter's type. */
   boost::shared_ptr<Type> paramType;
   if (!findType(paramType, false))
   {
       dwarf_printf("(0x%lx) param type not acquired\n", id());
       return false;
   }

   curName() = die_name();
   /* We can't do anything with anonymous parameters. */
   if (!nameDefined()) {
     dwarf_printf("(0x%lx) No name associated with formal, returning\n", id());
     return true;
   }

   Dwarf_Word lineNo = 0;
   bool hasLineNumber = false;
   std::string fileName;
   if (!getLineInformation(lineNo, hasLineNumber, fileName)) return false;
   createParameter(locs, paramType, lineNo, fileName);

   return true;
}

void DwarfWalker::createParameter(const vector<VariableLocation> &locs,
        boost::shared_ptr<Type> paramType, Dwarf_Word lineNo, const string &fileName)
{
   localVar * newParameter = new localVar(curName(),
                                          paramType,
                                          fileName, (int) lineNo,
                                          curFunc());
   dwarf_printf("(0x%lx) Creating new formal parameter %s/%p (%s) (%p)\n",
                id(),
                curName().c_str(),
                (void*)paramType.get(), paramType->getName().c_str(),
                (void*)curFunc());

   for (unsigned int i = 0; i < locs.size(); ++i)
   {
      newParameter->addLocation(locs[i]);
   }

   /* This is just brutally ugly.  Why don't we take care of this invariant automatically? */

   curFunc()->addParam(newParameter);
}

bool DwarfWalker::parseBaseType() {
   if(!tc()) return false;
   dwarf_printf("(0x%lx) parseBaseType entry\n", id());

   curName() = die_name();
   if (!nameDefined()) {
      dwarf_printf("(0x%lx) No name for type, returning early\n", id());
      return true;
   }

   unsigned size = 0;
   if (!findSize(size)) {
	   dwarf_printf("(0x%lx) No size for type '%s', returning early\n", id(), curName().c_str());
	   return false;
   }

   // Fetch the encoding
   Dwarf_Die e = entry();
   Dwarf_Attribute encoding_attr{};

   if(!dwarf_attr(&e, DW_AT_encoding, &encoding_attr)) {
	   dwarf_printf("(0x%lx) Unable to determine encoding for type '%s'\n", id(), curName().c_str());
       return false;
   }

   Dwarf_Sword encoding{};
   DWARF_FAIL_RET(dwarf_formsdata(&encoding_attr, &encoding));

   // linear machine address (for segmented addressing modes)
   const bool is_address = encoding == DW_ATE_address;

   // Integral types
   // NB: We explicitly don't support DW_ATE_[un]signed_fixed and DW_ATE_packed_decimal
   //     because we don't support languages that use those (e.g., COBOL)
   const bool is_boolean = encoding == DW_ATE_boolean;
   const bool is_signed_int = encoding == DW_ATE_signed;
   const bool is_signed_char = encoding == DW_ATE_signed_char;
   const bool is_unsigned_int = encoding == DW_ATE_unsigned;
   const bool is_unsigned_char = encoding == DW_ATE_unsigned_char;

   // Float types
   const bool is_complex_float = encoding == DW_ATE_complex_float;
   const bool is_float = encoding == DW_ATE_float;
   const bool is_imaginary_float = encoding == DW_ATE_imaginary_float;
   const bool is_decimal_float = encoding == DW_ATE_decimal_float;

   // Stringy types
   // NB: We explicitly don't support DW_ATE_edited and DW_ATE_numeric_string
   //     because we don't support languages that use those (e.g., COBOL)
   const bool is_UTF = encoding == DW_ATE_UTF;

   const bool is_signed = is_signed_int || is_signed_char;
   const bool is_unsigned = is_unsigned_int || is_unsigned_char;
   const bool is_integral = is_boolean || is_signed || is_unsigned;
   const bool is_floating_point = is_float || is_complex_float || is_imaginary_float || is_decimal_float;
   const bool is_string = is_UTF;

   typeScalar::properties_t p = {
	   is_integral, is_floating_point, is_string,
	   is_address, is_boolean,
	   is_complex_float, is_float, is_imaginary_float, is_decimal_float,
	   is_signed, is_signed_char, is_unsigned, is_unsigned_char,
	   is_UTF
   };

   /* Generate the appropriate built-in type; since there's no
      reliable way to distinguish between a built-in and a scalar,
      we don't bother to try. */
   auto baseType = Type::make_shared<typeScalar>( type_id(), size, curName(), p);

   /* Add the basic type to our collection. */
   typeScalar *debug = baseType.get();
   auto baseTy = tc()->addOrUpdateType( baseType );
   dwarf_printf("(0x%lx) Created type %p / %s (pre add %p / %s) for id %d, size %u, in TC %p\n", id(),
                (void*)baseTy.get(), baseTy->getName().c_str(),
                (void*)debug, debug->getName().c_str(),
                (int) offset(), size,
                (void*)tc());

   return true;
}

bool DwarfWalker::parseTypedef() {
    dwarf_printf("(0x%lx) parseTypedef entry\n", id());

    boost::shared_ptr<Type> referencedType;
    if (!findType(referencedType, true)) return false;

    curName() = die_name();
    if (!nameDefined()) {
        if (!fixName(curName(), referencedType)) return false;
    }

    auto typeDef = tc()->addOrUpdateType( Type::make_shared<typeTypedef>( type_id(), referencedType, curName()) );
    dwarf_printf("(0x%lx) Created type %p / %s for type_id %d, offset 0x%lx, size %u, in TC %p, mod:%s\n", id(),
            (void*)typeDef.get(), typeDef->getName().c_str(), type_id(),
            offset(), typeDef->getSize(), (void*)tc(), mod()->fileName().c_str());

    return true;
}

bool DwarfWalker::parseArray() {
  dwarf_printf("(0x%lx) Parsing array\n", id());

  boost::shared_ptr<Type> elementType;
  if (!findType(elementType, false))
    return false;
  if (!elementType)
    return false;

  curName() = die_name();

  Dwarf_Die e = entry();

  // The dimensions of an array are stored as subranges in the child DIE
  Dwarf_Die child;
  int result = dwarf_child(&e, &child);
  if (result < 0) {
    dwarf_printf("(0x%lx) Error calling dwarf_child\n", id());
    return false;
  }
  if (result == 1) {
    dwarf_printf("(0x%lx) dwarf_child found no subranges for array\n", id());
    return false;
  }

  // Find the subranges
  // A multidimensional array will have a subrange for each dimension
  std::stack<boost::shared_ptr<typeSubrange>> subranges;
  do {
    auto subrange = parseSubrange(&child);
    if (!subrange) {
      return false;
    }

    // Register the subrange with the type collection
    tc()->addOrUpdateType(subrange);

    subranges.push(std::move(subrange));
  } while (dwarf_siblingof(&child, &child) == 0);

  // This should never happen, but it's good to be paranoid
  if (subranges.size() == 0) {
    dwarf_printf("(0x%lx) No subranges found for array\n", id());
    return false;
  }

  std::string name{"__array" + std::to_string(offset())};

  auto convert = [this, &name](boost::shared_ptr<typeSubrange> t,
                               boost::shared_ptr<Type> base_t) {
    auto arr_t =
        Type::make_shared<typeArray>(base_t, t->getLow(), t->getHigh(), name);
    auto type = tc()->addOrUpdateType(arr_t);
    dwarf_printf("(0x%lx) Creating array dimension. ID: %d, %s[%lu:%lu]\n",
    			 id(), type->getID(), name.c_str(), t->getLow(), t->getHigh());
    return type;
  };

  auto make_base_t = [this, &name](boost::shared_ptr<typeSubrange> t,
          	  	  	  	  	boost::shared_ptr<Type> base) {
	    auto arr_t =
	        Type::make_shared<typeArray>(type_id(), base, t->getLow(),
	                                     t->getHigh(), name + "[]");
	    auto type = tc()->addOrUpdateType(arr_t);
	    dwarf_printf("(0x%lx) Creating array. ID: %d, %s[%lu:%lu]\n",
	    			 id(), type->getID(), arr_t->getName().c_str(), t->getLow(), t->getHigh());
	    return type;
  };

  // Convert the subranges to a Type sequence
  boost::shared_ptr<Type> base_t;
  if(subranges.size() == 1) {
	  auto cur_subrange = subranges.top();
	  subranges.pop();
	  base_t = make_base_t(cur_subrange, elementType);
  } else {
    // The last subrange (i.e., the highest dimension) gets the element type as
    // its base type
    auto cur_subrange = subranges.top();
    subranges.pop();
    auto base = convert(cur_subrange, elementType);

    // The rest get the subsequent dimension's type as their base types
    while (subranges.size() > 1) {
      cur_subrange = subranges.top();
      subranges.pop();
      base = convert(cur_subrange, base);
    }

    // The first subrange acts as the entry point for the array's type, so it
    // needs an explicit `type_id` and different name.
    cur_subrange = subranges.top();
    subranges.pop();
    base_t = make_base_t(cur_subrange, base);
  }

  /* Don't parse the children again. */
  setParseChild(false);

  return true;
}

bool DwarfWalker::parseSubrange() {
   Dwarf_Die e = entry();
   auto subrange = parseSubrange(&e);

   boost::shared_ptr<Type> rangeType = tc()->addOrUpdateType(subrange);
   dwarf_printf("(0x%lx) Created subrange type: ID 0x%d, pointer %p (tc %p)\n", id(),
                rangeType->getID(), (void *)rangeType.get(), (void *)tc());

   return true;
}

bool DwarfWalker::parseEnum() {
   if(!tc()) return false;
   dwarf_printf("(0x%lx) parseEnum entry\n", id());

   boost::shared_ptr<Type> underlying_type{};
   if (!findType(underlying_type, false)) {
       dwarf_printf("(0x%lx) type not found\n", id());
       return false;
   }

   curName() = die_name();

   // Handle C++ scoped enums (i.e., 'enum class')
   auto type = [this, &underlying_type]() {
	   Dwarf_Attribute valueAttr{};
	   Dwarf_Die e = entry();
	   if(dwarf_attr(&e, DW_AT_enum_class, & valueAttr)) {
		   return Type::make_shared<typeEnum>(underlying_type, curName(), type_id(), typeEnum::scoped_t{});
	   }
	   return Type::make_shared<typeEnum>(underlying_type, curName(), type_id());
   }();

   setEnum(tc()->addOrUpdateType(type));

   dwarf_printf("(0x%lx) end parseEnum\n", id());
   return true;
}

bool DwarfWalker::parseInheritance() {
   dwarf_printf("(0x%lx) parseInheritance entry\n", id());

   /* Acquire the super class's type. */
   boost::shared_ptr<Type> superClass = NULL;
   if (!findType(superClass, false)) return false;
   if (!superClass) return false;

   dwarf_printf("(0x%lx) Found %p as superclass\n", id(), (void*)superClass.get());

   visibility_t visibility = visUnknown;
   if (!findVisibility(visibility)) return false;

   /* Add a readily-recognizable 'bad' field to represent the superclass.
      Type::getComponents() will Do the Right Thing. */
   std::string fName = "{superclass}";
   curEnclosure()->asFieldListType().addField( fName, superClass, -1, visibility );
   dwarf_printf("(0x%lx) Added type %p as %s to %p\n", id(), (void*)superClass.get(), fName.c_str(), (void*)curEnclosure().get());
   return true;
}

bool DwarfWalker::parseStructUnionClass() {
    if(!tc()) return false;
    dwarf_printf("(0x%lx) parseStructUnionClass entry\n", id());

    if(tag() != DW_TAG_structure_type &&
            tag() != DW_TAG_union_type &&
            tag() != DW_TAG_class_type)
    {
        dwarf_printf("WARNING: parseStructUnionClass called on non-aggregate tagged entry\n");
        return false;
    }

    curName() = die_name();
    dwarf_printf("(0x%lx) Struct/Union/Class name from dwarf: %s\n", id(), curName().c_str());

    if (!nameDefined()) {
        /* anonymous unions, structs and classes are explicitely labelled */
        Dwarf_Word lineNumber;
        bool hasLineNumber = false;
        std::string fileName;
        if (!getLineInformation(lineNumber, hasLineNumber, fileName)) return false;
        stringstream ss;
        ss << "{anonymous ";
        switch (tag()) {
            case DW_TAG_structure_type:
                ss << "struct";
                break;
            case DW_TAG_union_type:
                ss << "union";
                break;
            case DW_TAG_class_type:
                ss << "class";
                break;
        }
        if (fileName.length() && hasLineNumber)
            ss << " at " << fileName << ":" << lineNumber;
        ss << "}";
        curName() = ss.str();
    }

    unsigned size=0;
    findSize(size);
    
    boost::shared_ptr<Type> containingType;
    switch ( tag() ) {
        case DW_TAG_structure_type:
        case DW_TAG_class_type:
            {
                auto ts = Type::make_shared<typeStruct>( type_id(), curName());
                ts->setSize(size);
                containingType = tc()->addOrUpdateType(ts);
                dwarf_printf("(0x%lx) Created type %p / %s for type_id %d, offset 0x%lx, size %u, in TC %p, mod:%s\n", id(),
                        (void*)containingType.get(), containingType->getName().c_str(), type_id(),
                        offset(), containingType->getSize(), (void*)tc(), mod()->fileName().c_str());
                break;
            }
        case DW_TAG_union_type:
            {
                auto tu = Type::make_shared<typeUnion>( type_id(), curName());
                tu->setSize(size);
                containingType = tc()->addOrUpdateType(tu);
                dwarf_printf("(0x%lx) Created type %p / %s for type_id %d, offset 0x%lx, size %u, in TC %p, mod:%s\n", id(),
                        (void*)containingType.get(), containingType->getName().c_str(), type_id(),
                        offset(), containingType->getSize(), (void*)tc(), mod()->fileName().c_str());
                break;
            }
        default:
            {
                Dwarf_Die e = entry();
                dwarf_printf("(0x%lx) Warning: type not created tag 0x%x, dwarf_tag(): 0x%x\n",
                        id(), tag(), (unsigned int)dwarf_tag(&e));
            }

    }
    setEnclosure(containingType);
    dwarf_printf("(0x%lx) Started class, union, or struct: %p\n",
            id(), (void*)containingType.get());
    return true;
}

bool DwarfWalker::parseEnumEntry() {
   dwarf_printf("(0x%lx) parseEnumEntry entry\n", id());

   std::string name = die_name();

   auto value = findConstValue();
   if(!value) { return false; }

   curEnum()->asEnumType().addConstant(name, *value);

   dwarf_printf("(0x%lx) end parseEnumEntry\n", id());
   return true;
}

bool DwarfWalker::parseMember() {
   dwarf_printf("(0x%lx) parseMember entry\n", id());
   if (!curEnclosure()) return false;

   boost::shared_ptr<Type> memberType = NULL;
   if (!findType(memberType, false)) return false;
   if (!memberType) return false;

   curName() = die_name();

   if (findConstValue()) {
      if(!nameDefined()) return false;
      dwarf_printf("(0x%lx) member is a named constant, forwarding to parseConstant\n", id());
      return parseConstant();
   }

   std::vector<VariableLocation> locs;
   Address initialStackValue = 0;
   if (!decodeLocationList(DW_AT_data_member_location, &initialStackValue, locs)) return false;
   if (locs.empty()) {
	if (curEnclosure()->getUnionType()) {
		/* GCC generates DW_TAG_union_type without DW_AT_data_member_location */
		if (!constructConstantVariableLocation((Address) 0, locs)) return false;
	} else {
		dwarf_printf("(0x%lx) Skipping member as no location is given.\n", id());
		return true;
	}
   }

   /* DWARF stores offsets in bytes unless the member is a bit field.
      Correct memberOffset as indicated.  Also, memberSize is in bytes
      from the underlying type, not the # of bits used from it, so
      correct that as necessary as well. */

   long int memberSize = memberType->getSize();

   // This code changes memberSize, which is then discarded. I'm not sure why...
   if (!fixBitFields(locs, memberSize)) return false;

   if(locs.empty()) return false;

   int offset_to_use = locs[0].frameOffset;

   dwarf_printf("(0x%lx) Using offset of 0x%x\n", id(), (unsigned int)offset_to_use);

   if (nameDefined()) {
      curEnclosure()->asFieldListType().addField(curName(), memberType, offset_to_use);
   }
   else {
      curEnclosure()->asFieldListType().addField("[anonymous union]", memberType, offset_to_use);
   }
   return true;
}

// TODO: this looks a lot like parseTypedef. Collapse?
bool DwarfWalker::parseConstPackedVolatile() {
   dwarf_printf("(0x%lx) parseConstPackedVolatile entry\n", id());

   boost::shared_ptr<Type> type = NULL;
   if (!findType(type, true)) return false;

   curName() = die_name();
   if (!nameDefined()) {
       dwarf_printf("(0x%lx) parseConstPackedVolatile fixName\n", id());
       if (!fixName(curName(), type)) return false;
   }
   tc()->addOrUpdateType( Type::make_shared<typeTypedef>(type_id(), type, curName()));

   return true;
}

bool DwarfWalker::parseTypeReferences() {
   dwarf_printf("(0x%lx) parseTypeReferences entry\n", id());

   boost::shared_ptr<Type> typePointedTo = NULL;
   if (!findType(typePointedTo, true))
   {
       dwarf_printf("(0x%lx) type not found\n", id());
       return false;
   }

   curName() = die_name();

   Dwarf_Die e = entry();
   boost::shared_ptr<Type> indirectType;
   switch ( tag() ) {
      case DW_TAG_subroutine_type:
         indirectType = tc()->addOrUpdateType(Type::make_shared<typeFunction>(
                            type_id(), typePointedTo, curName()));
         break;
      case DW_TAG_ptr_to_member_type:
      case DW_TAG_pointer_type:
         indirectType = tc()->addOrUpdateType(Type::make_shared<typePointer>(
                            type_id(), typePointedTo, curName()));
         dwarf_printf("(0x%lx) Created type %p / %s for type_id %d, offset 0x%lx, size %u, in TC %p, mod:%s\n", id(),
                 (void*)indirectType.get(), indirectType->getName().c_str(), type_id(),
                 offset(), indirectType->getSize(), (void*)tc(), mod()->fileName().c_str());
         break;
      case DW_TAG_reference_type:
    	 if(!nameDefined()){
    		 curName() = "&";
    	 }
         indirectType = tc()->addOrUpdateType(Type::make_shared<typeRef>(
                            type_id(), typePointedTo, curName()));
         dwarf_printf("(0x%lx) Created type %p / %s for type_id %d, offset 0x%lx, size %u, in TC %p\n", id(),
                 (void*)indirectType.get(), indirectType->getName().c_str(), type_id(),
                 offset(), indirectType->getSize(), (void*)tc());
         break;
      case DW_TAG_rvalue_reference_type:
    	  if(!nameDefined()) {
    		  curName() = "&&";
    	  }
          indirectType = tc()->addOrUpdateType(Type::make_shared<typeRef>(
                             type_id(), typePointedTo, curName(), typeRef::rvalue_t{}));
          dwarf_printf("(0x%lx) Created type %p / %s for type_id %d, offset 0x%lx, size %u, in TC %p\n", id(),
                  (void*)indirectType.get(), indirectType->getName().c_str(), type_id(),
                  offset(), indirectType->getSize(), (void*)tc());
          break;
      default:
         dwarf_printf("(0x%lx) Warning: nothing done for tag 0x%x, dwarf_tag(): 0x%x\n",
                 id(), tag(), (unsigned int)dwarf_tag(&e));
         return false;
   }

   dwarf_printf("(0x%lx) end parseTypeReferences\n", id());
   return indirectType ? true : false;
}

bool DwarfWalker::hasDeclaration(bool &isDecl) {
    Dwarf_Die e = entry();
    isDecl = dwarf_hasattr(&e, DW_AT_declaration);
    return true;
}

bool DwarfWalker::findTag() {
    Dwarf_Die e = entry();
    Dwarf_Half dieTag = dwarf_tag(&e);
    setTag(dieTag);
    return true;
}


bool DwarfWalker::handleAbstractOrigin(bool &isAbstract) {
    Dwarf_Die absE, e;
    e = entry();
    dwarf_printf("(0x%lx) Checking for abstract origin\n", id());
    isAbstract = false;

    // check if current DIE has abstract origin
    bool hasAbstractOrigin;
    hasAbstractOrigin = dwarf_hasattr(&e, DW_AT_abstract_origin);
    if (!hasAbstractOrigin) return true;

    // get abstract origin attribute
    Dwarf_Attribute abstractAttribute, *ret_p;
    ret_p = dwarf_attr(&e, DW_AT_abstract_origin, &abstractAttribute);
    if(ret_p == NULL) return false;

    // check if form of attribute DW_AT_abstract_origin is DW_FORM_GNU_ref_alt
    if(dwarf_whatform(&abstractAttribute)==DW_FORM_GNU_ref_alt)
        return true; //handled by libdw, meaning ignoring it here

    isAbstract = true;
    dwarf_printf("(0x%lx) abstract_origin is true, looking up reference\n", id());

    auto die_p = dwarf_formref_die(&abstractAttribute, &absE);
    if(!die_p) return false;

    setAbstractEntry(absE);

    return true;
}

bool DwarfWalker::handleSpecification(bool &hasSpec) {
    Dwarf_Die specE, e;
    e = entry();
    dwarf_printf("(0x%lx) Checking for separate specification\n", id());
    hasSpec = false;

    bool hasSpecification = dwarf_hasattr_integrate(&e, DW_AT_specification);

    if (!hasSpecification) return true;

    hasSpec = true;
    dwarf_printf("(0x%lx) Entry has separate specification, retrieving\n", id());

    Dwarf_Attribute specAttribute, *ret_p;
    ret_p = dwarf_attr_integrate(&e, DW_AT_specification, &specAttribute);
    if(ret_p == NULL) return false;

    auto die_p = dwarf_formref_die(&specAttribute, &specE);
    if(!die_p) return false;

    setSpecEntry(specE);

    return true;
}

std::string DwarfWalker::die_name() {
    auto name = DwarfDyninst::die_name(specEntry());
    dwarf_printf("(0x%lx) Found name %s.\n", id(), name.c_str());
    return name;
}


bool DwarfWalker::findFuncName() {
    dwarf_printf("(0x%lx) Checking for function name\n", id());

    Dwarf_Die e = entry();

    // Does this function have a linkage name?
    {
        Dwarf_Attribute linkageNameAttr{};
        Dwarf_Attribute *attr = dwarf_attr_integrate(&e, DW_AT_linkage_name, &linkageNameAttr);
        if (attr)  {
          char const* dwarfName = dwarf_formstring(attr);
          if(!dwarfName) {
            dwarf_printf("(0x%lx) Found 'DW_AT_linkage_name', but formstring is empty\n", id());
            return false;
          }
          curName() = dwarfName;
          setMangledName(true);
          dwarf_printf("(0x%lx) Found DW_AT_linkage_name of %s\n", id(), curName().c_str());
          return true;
        }
    }

    // Is this an inlined function?
    {
        if(dwarf_hasattr(&e, DW_AT_inline)) {
          // Find the 'DW_AT_name' for this DIE. Do not traverse this as an abstract
          // instance root (if it is one)- i.e., don't use dwarf_attr_integrate here.
          Dwarf_Attribute nameAttr{};
          Dwarf_Attribute *res = dwarf_attr(&e, DW_AT_name, &nameAttr);
          if(!res) {
        	  dwarf_printf("(0x%lx) Found an inlined subroutine, but has no 'DW_AT_name'\n", id());
        	  return false;
          }
          char const* dwarfName = dwarf_formstring(&nameAttr);
          if(!dwarfName) {
            dwarf_printf("(0x%lx) Found an inlined subroutine, but formstring is empty\n", id());
            return false;
          }
          curName() = dwarfName;

          // Section 2.15 of the DWARF5 spec suggests that DW_AT_name should be the name as it
          // appears in the source- not mangled.
          setMangledName(false);

          dwarf_printf("(0x%lx) Found inline DW_AT_name '%s'\n", id(), curName().c_str());
          return true;
        }
    }

    // Assume the name is the unmangled name associated with the current DIE, if any
    curName() = die_name();
    setMangledName(false);
    dwarf_printf("(0x%lx) No explicit function name found; using most-recently found name '%s'\n", id(), curName().c_str());
    return true;
}

std::vector<VariableLocation>& DwarfParseActions::getFramePtrRefForInit()
{
   return curFunc()->getFramePtrRefForInit();
}

// getFrameBase is a helper function to parseSubprogram
// inlined functions will return its parent's frame info
bool DwarfWalker::getFrameBase() {
    dwarf_printf("(0x%lx) Checking for frame pointer information\n", id());

    boost::unique_lock<dyn_mutex> l(curFunc()->getFramePtrLock());
    std::vector<VariableLocation> &funlocs = getFramePtrRefForInit();
    if (!funlocs.empty()) {
        DWARF_CHECK_RET(false);
    }

    if (!decodeLocationList(DW_AT_frame_base, NULL, funlocs))
        return true;
    dwarf_printf("(0x%lx) After frame base decode, %d entries\n", id(), (int) funlocs.size());

    return true || !funlocs.empty(); // johnmc added true
}

bool DwarfWalker::getReturnType(boost::shared_ptr<Type>&returnType) {
    dwarf_printf("(0x%lx) In getReturnType().\n", id());
    Dwarf_Attribute typeAttribute;
    Dwarf_Attribute* status = 0;

    bool is_info = true;
    Dwarf_Die e = entry();
    is_info = !dwarf_hasattr_integrate(&e, DW_TAG_type_unit);
    status = dwarf_attr(&e, DW_AT_type, &typeAttribute);

    if ( status == 0) {
        dwarf_printf("(0x%lx) Return type is void\n", id());
        return false;
    }

    /* There's a return type attribute. */
    dwarf_printf("(0x%lx) Return type is not void\n", id());

    bool ret = findAnyType( typeAttribute, is_info, returnType );
    return ret;
}

// I'm not sure how the provided fieldListType is different from curEnclosure(),
// but that's the way the code was structured and it was working.
bool DwarfWalker::addFuncToContainer(boost::shared_ptr<Type> returnType) {
   /* Using the mangled name allows us to distinguish between overridden
      functions, but confuses the tests.  Since Type uses vectors
      to hold field names, however, duplicate -- demangled names -- are OK. */

   std::string demangledName = P_cplus_demangle( curName() );

   // Strip everything left of the rightmost ':' off to get rid of the class names
   // rfind finds the last occurrence of ':'; add 1 to get past it.
   size_t offset = demangledName.rfind(':');
   if (offset != demangledName.npos) {
      demangledName.erase(0, offset+1);
   }

   curEnclosure()->asFieldListType().addField( demangledName, Type::make_shared<typeFunction>(
      type_id(), returnType, demangledName));

   return true;
}

bool DwarfWalker::isStaticStructMember(std::vector<VariableLocation> &locs, bool &isStatic) {
   isStatic = false;

   // if parsing a struct-member which is not a regular member (i.e. not with an offset)
   if (curEnclosure()->getDataClass() == dataStructure && locs.size() == 0) {
     // and it is not a constant, then it must be a static field member
     isStatic = !findConstValue();
   }

   return true;
}

bool DwarfWalker::findType(boost::shared_ptr<Type>&type, bool defaultToVoid) {
    if(!tc()) return false;
    // Do *not* return true unless type is actually usable.

    /* Acquire the parameter's type. */
    Dwarf_Attribute typeAttribute;
    Dwarf_Die e = specEntry();
    auto attr_p = dwarf_attr_integrate( &e, DW_AT_type, &typeAttribute);

    if (attr_p == 0) {
        if (defaultToVoid) {
            type = tc()->findType("void", Type::share);
            return type ? true : false;
        }
        return false;
    }

    bool is_info = !dwarf_hasattr_integrate(&e, DW_TAG_type_unit);

    bool ret = findAnyType( typeAttribute, is_info, type );
    return ret;
}

bool DwarfWalker::findDieOffset(Dwarf_Attribute attr, Dwarf_Off &offset) {
    Dwarf_Half form = dwarf_whatform(&attr);
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
        case DW_FORM_GNU_ref_alt:
            {
                //TODO debug to compare values of offset
                Dwarf_Die die;
                auto ret_p = dwarf_formref_die(&attr, &die);
                if(!ret_p) return false;
                offset = dwarf_dieoffset(&die);
                return true;
            }
            /* Then there's DW_FORM_sec_offset which refer other sections, or
             * DW_FORM_GNU_ref_alt that refers to a whole different file.  We can't
             * use such forms as a die offset, even if dwarf_global_formref is
             * willing to decode it. */
        default:
            dwarf_printf("(0x%lx) error Can't use form 0x%x as a die offset\n", id(), (int) form);
            return false;
    }
}

bool DwarfWalker::findAnyType(Dwarf_Attribute typeAttribute,
        bool is_info, boost::shared_ptr<Type>&type)
{
    /* If this is a ref_sig8, look for the type elsewhere. */
    Dwarf_Half form = dwarf_whatform(&typeAttribute);
    if (form == DW_FORM_ref_sig8) {
        Dwarf_Sig8 sig8;
        const char * sig = dwarf_formstring(&typeAttribute);
        if(!sig) return false;
        memcpy(sig8.signature, sig, 8);
        return findSig8Type(&sig8, type);
    }

    Dwarf_Off typeOffset;
    if (!findDieOffset( typeAttribute, typeOffset)) return false;

    /* NB: It's possible for an incomplete type to have a DW_AT_signature
     * reference to a complete definition.  For example, GCC may output just the
     * subprograms for a struct's methods in one CU, with the full struct
     * defined in a type unit.
     *
     * No DW_AT_type has been found referencing an incomplete type this way,
     * but if it did, here's the place to look for that DW_AT_signature and
     * recurse into findAnyType again.
     */

    Dwarf_Die dieType;
    auto ret_p = dwarf_formref_die(&typeAttribute, &dieType);
    if(!ret_p) return false;

    Dwarf * desc = dwarf_cu_getdwarf(dieType.cu);
    bool is_sup = dbg()!=desc;
    typeId_t type_id = get_type_id(typeOffset, is_info, is_sup);

    dwarf_printf("(0x%lx) type offset 0x%lx, is_supplemental=%s\n", id(), (unsigned long)typeOffset, is_sup?"true":"false");

    /* The typeOffset forms a module-unique type identifier,
       so the Type look-ups by it rather than name. */
    type = tc()->findOrCreateType( type_id, Type::share );

    // parse this referenced by type_id die type in case it hasn't yet
    if(type->getDataClass()==dataUnknownType){
        dwarf_printf("(0x%lx) type not parsed yet, calling parse_int() \n", id());
        // call parse_int creating new context by context_dissociation = true;
        parse_int(dieType, false, true);
    }

    dwarf_printf("(0x%lx) type pointer %p / name:%s, type_id %d, tc():%p, mod: %s, specificType:%s\n",
            id(), (void*)type.get(), type->getName().c_str(), type_id, (void*)tc(), mod()->fileName().c_str(), type->specificType().c_str());

    return true;
}

bool DwarfWalker::getLineInformation(Dwarf_Word &variableLineNo,
        bool &hasLineNumber,
        std::string &fileName)
{
    Dwarf_Attribute fileDeclAttribute;
    Dwarf_Die specE = specEntry();
    auto status = dwarf_attr(&specE, DW_AT_decl_file, &fileDeclAttribute);

    if (status != 0) {
        StringTablePtr files = srcFiles();
        boost::unique_lock<dyn_mutex> l(files->lock);
        Dwarf_Word fileNameDeclVal;
        DWARF_FAIL_RET(dwarf_formudata(&fileDeclAttribute, &fileNameDeclVal));
        if (fileNameDeclVal >= files->size() || fileNameDeclVal <= 0) {
            dwarf_printf("Dwarf error reading line index %lu from srcFiles of size %lu\n",
                    fileNameDeclVal, files->size());
            return false;
        }
        fileName = ((files->get<0>())[fileNameDeclVal]).str;
    }
    else {
        return true;
    }

    Dwarf_Attribute lineNoAttribute;
    status = dwarf_attr(&specE, DW_AT_decl_line, &lineNoAttribute);
    if (status == 0) return true;

    /* We don't need to tell Dyninst a line number for C++ static variables,
       so it's OK if there isn't one. */
    hasLineNumber = true;
    DWARF_FAIL_RET(dwarf_formudata(&lineNoAttribute, &variableLineNo));

    return true;
}

bool DwarfWalker::decodeLocationList(Dwarf_Half attr,
        Address *initialStackValue,
        std::vector<VariableLocation> &locs)
{
    Dwarf_Die e = entry();
    if (!dwarf_hasattr(&e, attr)) {
        dwarf_printf("(0x%lx): no such attribute 0x%x\n", id(), attr);
        return true;
    }

    locs.clear();

    /* Acquire the location of this formal parameter. */
    Dwarf_Attribute locationAttribute;
    auto ret_p = dwarf_attr(&e, attr, &locationAttribute);
    if(!ret_p)
        return false;

    bool isExpr = false;
    bool isConstant = false;
    Dwarf_Half form;
    if (!checkForConstantOrExpr(attr, locationAttribute, isConstant, isExpr, form))
        return false;
    dwarf_printf("(0x%lx) After checkForConstantOrExpr, form class is 0x%x\n",id(), form);

    if (isConstant) {
        dwarf_printf("(0x%lx) Decoding constant location\n", id());
        if (!decodeConstantLocation(locationAttribute, form, locs))
            return false;
    }
    else if (isExpr) {
        dwarf_printf("(0x%lx) Decoding expression without location list\n", id());
        if (!decodeExpression(locationAttribute, locs))
            return false;
    }
    else {
        dwarf_printf("(0x%lx) Decoding loclist location\n", id());

        Dwarf_Op * exprs = NULL;
        size_t exprlen = 0;
        std::vector<LocDesc> locDescs;
        ptrdiff_t offset = 0;
        Dwarf_Addr basep, start, end;

        do {
            offset = dwarf_getlocations(&locationAttribute, offset, &basep,
                    &start, &end, &exprs, &exprlen);
            if(offset==-1){
                dwarf_printf("err message: %s\n", dwarf_errmsg(dwarf_errno()));
                return false;
            }
            if(offset==0) break;

            LocDesc ld;
            ld.ld_lopc = start;
            ld.ld_hipc = end;
            ld.dwarfOp = exprs;
            ld.opLen = exprlen;
            locDescs.push_back(ld);
        }while(offset > 0);
        if (locDescs.size() <= 0) {
            dwarf_printf("(0x%lx) Failed loclist decode\n", id());
            return true;
        }

        if(!decodeLocationListForStaticOffsetOrAddress(locDescs, locDescs.size(),
                locs, initialStackValue))
        {
            return false;
        }
    }

    return true;

}

bool DwarfWalker::checkForConstantOrExpr(Dwarf_Half /*attr*/,
        Dwarf_Attribute &locationAttribute,
        bool &constant,
        bool & expr,
        Dwarf_Half &form)
{
    constant = false;
    // Get the form (datatype) for this particular attribute
    form = dwarf_whatform(&locationAttribute);
    constant = dwarf_hasform(&locationAttribute, DW_FORM_data1);
    expr = dwarf_hasform(&locationAttribute, DW_FORM_exprloc);
    return true;
}

boost::optional<std::string> DwarfWalker::find_call_file() {
    Dwarf_Die e = entry();
    unsigned long line_index;
    bool result = findConstant(DW_AT_call_file, line_index, &e, dbg());
    if (!result)
        return {};
    StringTablePtr strs = mod()->getStrings();
    boost::unique_lock<dyn_mutex> l(strs->lock);
    if (line_index >= strs->size()) {
        dwarf_printf("Dwarf error reading line index %lu from srcFiles(%p) of size %lu\n",
                line_index, (void*)strs.get(), strs->size());
        return {};
    }
    return (*srcFiles())[line_index].str;
}

bool DwarfWalker::findConstant(Dwarf_Half attr, Address &value, Dwarf_Die *entry, Dwarf * /*dbg*/) {
    bool has = dwarf_hasattr(entry, attr);
    if (!has) return false;

    // Get the attribute
    Dwarf_Attribute d_attr;
    auto ret_p = dwarf_attr(entry, attr, &d_attr);
    if(!ret_p) return false;

    // Get the form (datatype) for this particular attribute
    Dwarf_Half form = dwarf_whatform(&d_attr);

    bool ret = findConstantWithForm(d_attr, form, value);
    return ret;
}

bool DwarfWalker::findConstantWithForm(Dwarf_Attribute &locationAttribute,
        Dwarf_Half form, Address &value)
{
    value = 0;

    switch(form) {
        case DW_FORM_addr:
            Dwarf_Addr addr;
            DWARF_FAIL_RET(dwarf_formaddr(&locationAttribute, &addr));
            value = (Address) addr;
            return true;
        case DW_FORM_sdata:
            Dwarf_Sword s_tmp;
            DWARF_FAIL_RET(dwarf_formsdata(&locationAttribute, &s_tmp));
            value = (Address) s_tmp;
            dwarf_printf("Decoded data of form %x to 0x%lx\n",
                    form, value);
            return true;
        case DW_FORM_data1:
        case DW_FORM_data2:
        case DW_FORM_data4:
        case DW_FORM_data8:
        case DW_FORM_udata:
        case DW_FORM_sec_offset:
            Dwarf_Word u_tmp;
            DWARF_FAIL_RET(dwarf_formudata(&locationAttribute, &u_tmp));
            value = (Address) u_tmp;
            return true;
        default:
            dwarf_printf("Warning: Unhandled form 0x%x for constant decode\n", (unsigned) form);
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
    bool hasSize;
    Dwarf_Die e = entry();
    if(!e.addr) return false;
    hasSize = dwarf_hasattr(&e, DW_AT_byte_size);
    if (!hasSize) return false;

    Dwarf_Attribute byteSizeAttr;
    Dwarf_Word byteSize;
    e = specEntry();
    auto ret_p = dwarf_attr(&e, DW_AT_byte_size, &byteSizeAttr);
    if(!ret_p) return false;

    DWARF_FAIL_RET(dwarf_formudata(&byteSizeAttr, &byteSize));

    size = (unsigned) byteSize;
    return true;
}

bool DwarfWalker::findVisibility(visibility_t &visibility) {
    /* Acquire the visibility, if any.  DWARF calls it accessibility
       to distinguish it from symbol table visibility. */
    Dwarf_Attribute visAttr;
    Dwarf_Die e = entry();
    auto status = dwarf_attr(&e, DW_AT_accessibility, &visAttr);
    if (status != 0) {
        visibility = visPrivate;
        return true;
    }

    Dwarf_Word visValue;
    dwarf_formudata( &visAttr, &visValue);

    switch( visValue ) {
        case DW_ACCESS_public: visibility = visPublic; break;
        case DW_ACCESS_protected: visibility = visProtected; break;
        case DW_ACCESS_private: visibility = visPrivate; break;
        default:
                                break;
    }

    return true;
}

boost::optional<long> DwarfWalker::findConstValue() {
	dwarf_printf("(0x%lx) findConstValue entry\n", id());

    Dwarf_Attribute valueAttr{};
    Dwarf_Die e = entry();

    // This also applies to constexpr objects (i.e., DW_AT_const_expr)
    if(!dwarf_attr(&e, DW_AT_const_value, & valueAttr)) {
    	dwarf_printf("No const value found\n");
    	return {};
    }

    Dwarf_Sword enumValue;
    if(dwarf_formsdata(&valueAttr, &enumValue) != 0) {
    	dwarf_printf("ERROR: dwarf_formsdata error for entry %p\n", static_cast<void*>(&e));
    	return {};
    }

    dwarf_printf("Found value '%ld'\n", static_cast<long>(enumValue));

    dwarf_printf("(0x%lx) end findConstValue\n", id());

    return enumValue;
}

bool DwarfWalker::fixBitFields(std::vector<VariableLocation> &locs,
        long &size) {
    Dwarf_Attribute bitOffset;
    Dwarf_Die e(entry());
    auto status = dwarf_attr( &e, DW_AT_bit_offset, & bitOffset);

    if ( status != 0 && locs.size() )
    {
        Dwarf_Word memberOffset_du = locs[0].frameOffset;

        DWARF_FAIL_RET(dwarf_formudata( &bitOffset, &memberOffset_du));

        Dwarf_Attribute bitSize;
        auto ret_p = dwarf_attr( &e, DW_AT_bit_size, & bitSize);
        if(!ret_p) return false;

        Dwarf_Word memberSize_du = size;
        DWARF_FAIL_RET(dwarf_formudata( &bitSize, &memberSize_du));

        /* If the DW_AT_byte_size field exists, there's some padding.
           FIXME?  We ignore padding for now.  (We also don't seem to handle
           bitfields right in getComponents() anyway...) */
    }
    else
    {
        if (locs.size())
            locs[0].frameOffset *= 8;
        size *= 8;
    }
    return true;
}

bool DwarfWalker::fixName(std::string &name, boost::shared_ptr<Type> type) {
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
       (lang != lang_CMFortran)) return;

   if (name[name.length()-1] == '_') {
      name = name.substr(0, name.length()-1);
   }
}

boost::shared_ptr<typeSubrange> DwarfWalker::parseSubrange(Dwarf_Die *entry) {
  const auto subrange_id = dwarf_dieoffset(entry) - this->compile_offset;
  dwarf_printf("(0x%lx) parseSubrange entry for <0x%lx>\n", id(), subrange_id);

  boost::optional<Dwarf_Word> upper_bound, lower_bound;

  /* An array can have DW_TAG_subrange_type or DW_TAG_enumeration_type
   children instead that give the size of each dimension.  */
  switch (dwarf_tag(entry)) {
  case DW_TAG_subrange_type: {
    namespace dw = Dyninst::DwarfDyninst;
    auto bounds = dwarf_subrange_bounds(entry);
    if (!bounds.lower || !bounds.upper) {
      dwarf_printf("parseSubrange failed, error finding range bounds\n");
      return nullptr;
    }

    if (!bounds.lower.value) {
      // dwarf_subrange_bounds will try dwarf_default_lower_bound for the
      // current DIE. If we got here, that didn't work, so try using the one
      // Dyninst parsed.
      switch (mod()->language()) {
      case lang_Fortran:
      case lang_CMFortran:
        bounds.lower.value = 1;
        break;
      default:
        // Assume all non-Fortran languages use 0
        bounds.lower.value = 0;
        break;
      }
    }

    upper_bound = bounds.upper.value;
    lower_bound = bounds.lower.value;
  } break;
  case DW_TAG_enumeration_type: {
    // If there is an enum value, then it represents the total number of
    // elements in the array. We take the bounds to be [0, upper-1)
    auto upper = dwarf_subrange_length_from_enum(entry);
    if (!upper) {
      dwarf_printf("parseSubrange failed, error finding length from enum\n");
      return nullptr;
    }
    if (upper) {
      upper.value.get() -= 1;
    }
    lower_bound = 0;
    upper_bound = upper.value;
  }
  }

  // Don't set the name until we're guaranteed a subrange was found
  curName() = die_name();
  if (!nameDefined()) {
    curName() = "{anonymousRange}";
  }

  Dwarf_Off subrangeOffset = dwarf_dieoffset(entry);
  bool is_info = !dwarf_hasattr_integrate(entry, DW_TAG_type_unit);
  typeId_t type_id = get_type_id(subrangeOffset, is_info, false);

  // `typeSubrange` expects numeric values for the bounds
  DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_MAYBE_UNINITIALIZED
  auto range = Type::make_shared<typeSubrange>(
      type_id, 0, lower_bound.value_or(LONG_MIN),
      upper_bound.value_or(LONG_MAX), curName());
  DYNINST_DIAGNOSTIC_END_SUPPRESS_MAYBE_UNINITIALIZED

  dwarf_printf(
      "(0x%lx) Parsed subrange: id %d, low %lu, high %lu, named %s\n",
      id(), type_id, range->getLow(), range->getHigh(), curName().c_str());

  return range;
}

bool DwarfWalker::decodeExpression(Dwarf_Attribute &locationAttribute,
        std::vector<VariableLocation> &locs)
{
    Dwarf_Op * exprs = NULL;
    size_t exprlen = 0;

    std::vector<LocDesc> locDescs;
    ptrdiff_t offset = 0;
    Dwarf_Addr basep, start, end;
    do {
        offset = dwarf_getlocations(&locationAttribute, offset, &basep,
                &start, &end, &exprs, &exprlen);
        if(offset==-1) return false;
        LocDesc ld;
        ld.ld_lopc = start;
        ld.ld_hipc = end;
        ld.dwarfOp = exprs;
        ld.opLen = exprlen;
        locDescs.push_back(ld);
    }while(offset > 0);

    bool ret = decodeLocationListForStaticOffsetOrAddress(locDescs, 1, locs);
    return ret;
}


bool DwarfWalker::decodeLocationListForStaticOffsetOrAddress(
        std::vector<LocDesc>& locationList,
        Dwarf_Sword listLength,
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
        LocDesc * location = &locationList[locIndex];

        // If location == 0..-1, it's "unset" and we keep the big range unless
        // we're in a lexical block construct.
        //
        dwarf_printf("(0x%lx) Decoding entry %u of %d over range 0x%lx - 0x%lx, mod 0x%lx - 0x%lx, base 0x%lx\n",
                id(), locIndex+1, (int) listLength,
                location->ld_lopc,
                location->ld_hipc,
                modLow, modHigh, base);

        if (location->ld_lopc == max_addr) {
            //This is a base address selection entry, which changes the base address of
            // subsequent entries
            base = location->ld_hipc;
            continue;
        }

        VariableLocation loc;
        // Initialize location values.
        loc.stClass = storageAddr;
        loc.refClass = storageNoRef;

        long int *tmp = (long int *)initialStackValue;
        bool result = decodeDwarfExpression(location->dwarfOp, location->opLen, tmp, loc,
                symtab()->getArchitecture());
        if (!result) {
            dwarf_printf("(0x%lx): dwarf expression not decoded\n", id());
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

                    dwarf_printf("(0x%lx) valid over range 0x%lx to 0x%lx\n",
                            id(), loc.lowPC, loc.hiPC);
                    locs.push_back(loc);
                }
            }
            else {
                dwarf_printf("(0x%lx) Using open location range\n", id());
                loc.lowPC = location->ld_lopc;
                loc.hiPC = location->ld_hipc;

                dwarf_printf("(0x%lx) valid over range 0x%lx to 0x%lx\n",
                        id(), loc.lowPC, loc.hiPC);
                locs.push_back(loc);
            }
        }
        else {
            dwarf_printf("(0x%lx) Using lexical range\n", id());

            loc.lowPC = location->ld_lopc;
            loc.hiPC = location->ld_hipc;

            dwarf_printf("(0x%lx) valid over range 0x%lx to 0x%lx\n",
                    id(), loc.lowPC, loc.hiPC);
            locs.push_back(loc);
        }
    }

    /* decode successful */
    return !locs.empty();
}


void DwarfWalker::setEntry(Dwarf_Die entry) {
   DwarfParseActions::setEntry(entry);
   DwarfParseActions::setSpecEntry(entry);
   DwarfParseActions::setAbstractEntry(entry);
}
void DwarfParseActions::push(bool dissociate_context) {
   if (c.empty() || dissociate_context) {
      c.push(Context());
   }
   else {
      c.push(Context(c.top()));
   }
}

void DwarfParseActions::pop() {
   if(!c.empty())
   {
      c.pop();
   }
}

void DwarfParseActions::setFunc(FunctionBase *f) {
  // Bug workaround; if we're setting a function, ignore
  // any preceding lexical information since we probably
  // nested.
  c.top().func = f;
}

void DwarfParseActions::clearFunc() {
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

unsigned int DwarfWalker::getNextTypeId(){

  static boost::atomic<unsigned int> next_type_id(1);
  unsigned int val = next_type_id.fetch_add(1);
  return val;
}

typeId_t DwarfWalker::get_type_id(Dwarf_Off offset, bool is_info, bool is_sup)
{
  auto& type_ids = is_info ? info_type_ids_ : types_type_ids_;
  typeId_t type_id = 0;
  {
    type_map::const_accessor a;
    if(type_ids.find(a, {offset,is_sup,mod()})) type_id = a->second;
  }
  if(type_id) return type_id;

  unsigned int val = getNextTypeId();
  {
    type_map::accessor a;
    type_key tk{offset, is_sup, mod()};
    type_ids.insert(a, make_pair(tk, val));
    dwarf_printf("(0x%lx) type_id %u, key created {0x%lx,%s,mod: %s}\n", id(),val,offset,is_sup?"sup":"not sup", mod()->fileName().c_str());
  }

  return val;
}

typeId_t DwarfWalker::type_id()
{
    Dwarf_Die e(entry());
    bool is_info = !dwarf_hasattr_integrate(&e, DW_TAG_type_unit);
    Dwarf * desc = dwarf_cu_getdwarf(e.cu);
    return get_type_id(offset(), is_info, dbg()!=desc);
}

void DwarfWalker::findAllSig8Types()
{
    /* First .debug_types (0), then .debug_info (1).
     * In DWARF4, only .debug_types contains DW_TAG_type_unit,
     * but DWARF5 is considering them for .debug_info too.*/
    compile_offset = next_cu_header = 0;

    /* Iterate over the compilation-unit headers for .debug_types. */
    uint64_t type_signaturep;
    for(Dwarf_Off cu_off = 0;
            dwarf_next_unit(dbg(), cu_off, &next_cu_header, &cu_header_length,
                NULL, &abbrev_offset, &addr_size, &offset_size,
                &type_signaturep, NULL) == 0;
            cu_off = next_cu_header)
    {
        if(!dwarf_offdie_types(dbg(), cu_off + cu_header_length, &current_cu_die))
            continue;

        if (DwarfDyninst::is_partial_unit(current_cu_die)) {
            continue;
        }
        parseModuleSig8(false);
        compile_offset = next_cu_header;
    }

    /* Iterate over the compilation-unit headers for .debug_info. */
    for(Dwarf_Off cu_off = 0;
            dwarf_nextcu(dbg(), cu_off, &next_cu_header, &cu_header_length,
                &abbrev_offset, &addr_size, &offset_size) == 0;
            cu_off = next_cu_header)
    {
        if(!dwarf_offdie(dbg(), cu_off + cu_header_length, &current_cu_die))
            continue;
        if (DwarfDyninst::is_partial_unit(current_cu_die)) {
            continue;
        }
        parseModuleSig8(true);
        compile_offset = next_cu_header;
    }
}

bool DwarfWalker::parseModuleSig8(bool is_info)
{
    if (!DwarfDyninst::is_type_unit(current_cu_die))
        return false;
    uint64_t sig8 = * reinterpret_cast<uint64_t*>(&signature);
    typeId_t type_id = get_type_id(typeoffset, is_info, false);

    {
      dyn_c_hash_map<uint64_t, typeId_t>::accessor a;
      sig8_type_ids_.insert(a, std::make_pair(sig8, type_id));
    }
    dwarf_printf("Mapped Sig8 {%016llx} to type id 0x%x\n", (unsigned long long) sig8, (unsigned int)type_id);
    return true;
}

bool DwarfWalker::findSig8Type(Dwarf_Sig8 * s, boost::shared_ptr<Type>&returnType)
{
   uint64_t sig8 = * reinterpret_cast<uint64_t*>(s);
   typeId_t type_id = 0;
   {
     dyn_c_hash_map<uint64_t, typeId_t>::const_accessor a;
     if(sig8_type_ids_.find(a, sig8)) type_id = a->second;
   }

   if(type_id){
     returnType = tc()->findOrCreateType(type_id, Type::share);
     dwarf_printf("Found Sig8 {%016llx} as type id 0x%x\n", (unsigned long long) sig8, (unsigned int)type_id);
     return true;
   }

   dwarf_printf("Couldn't find Sig8 {%016llx}!\n", (unsigned long long) sig8);
   return false;
}

Object *DwarfParseActions::obj() const {
   return symtab()->getObject();
}

Offset DwarfParseActions::convertDebugOffset(Offset from) {
   Offset to;
   obj()->convertDebugOffset(from, to);
   return to;
}

void DwarfWalker::setFuncReturnType() {
    dwarf_printf("(0x%lx) In setFuncReturnType().\n", id());
   boost::shared_ptr<Type> returnType;
      getReturnType(returnType);
      if (returnType)
         curFunc()->setReturnType(returnType);
}

