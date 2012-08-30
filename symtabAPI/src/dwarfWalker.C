#include "dwarfWalker.h"
#include "common/h/headers.h"
#include "Module.h"
#include "Symtab.h"
#include "Collections.h"
#include "dwarf.h"
#include "Object.h"
#include "Object-elf.h"
#include "Function.h"
#include "common/h/dwarfExpr.h"
#include "common/h/pathName.h"

using namespace Dyninst;
using namespace SymtabAPI;

#define DWARF_FAIL_RET(x) {                                                 \
      int status = (x);                                                 \
      if (status != DW_DLV_OK) {                                        \
         fprintf(stderr, "[%s:%d]: libdwarf returned %d, ret false\n", FILE__, __LINE__, status); \
         return false;                                                  \
      }                                                                 \
   }

#define DWARF_ERROR_RET(x) {                                            \
      int status = (x);                                                 \
      if (status == DW_DLV_ERROR) {                                     \
         fprintf(stderr, "[%s:%d]: parsing failure, ret false\n", FILE__, __LINE__); \
         return false;                                                  \
      }                                                                 \
   }

#define DWARF_CHECK_RET(x) {                                            \
      if (x) {                                     \
         fprintf(stderr, "[%s:%d]: parsing failure, ret false\n", FILE__, __LINE__); \
         return false;                                                  \
      }                                                                 \
   }


// TODO
#define dwarf_printf printf

DwarfWalker::DwarfWalker(Symtab *symtab, Dwarf_Debug &dbg)
   :
   dbg_(dbg),
   mod_(NULL),
   symtab_(symtab),
   tc_(NULL),
   curBase(0) {
}   

DwarfWalker::~DwarfWalker() {}

bool DwarfWalker::parse() {
   dwarf_printf("Parsing DWARF for %s\n", symtab_->file().c_str());

   /* Start the dwarven debugging. */
   Module *fixUnknownMod = NULL;
   mod_ = NULL;
   
   /* Iterate over the compilation-unit headers. */
   while (dwarf_next_cu_header_c(dbg(),
                                 (Dwarf_Bool) 1, // Operate on .debug_info, not .debug_types
                                 &cu_header_length,
                                 &version,
                                 &abbrev_offset,
                                 &addr_size,
                                 &offset_size,
                                 &extension_size,
                                 &signature,
                                 &typeoffset,
                                 &next_cu_header, NULL) == DW_DLV_OK ) {
      if (!parseModule(fixUnknownMod)) return false;
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

bool DwarfWalker::parseModule(Module *&fixUnknownMod) {
   /* Obtain the module DIE. */
   Dwarf_Die moduleDIE;
   DWARF_FAIL_RET(dwarf_siblingof( dbg(), NULL, &moduleDIE, NULL ));
   
   /* Make sure we've got the right one. */
   Dwarf_Half moduleTag;
   DWARF_FAIL_RET(dwarf_tag( moduleDIE, & moduleTag, NULL ));

   if (moduleTag != DW_TAG_compile_unit) return false;
   
   /* Extract the name of this module. */
   std::string moduleName;
   char *tmp;
   int status = dwarf_diename( moduleDIE, &tmp, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);

   if ( status == DW_DLV_NO_ENTRY ) {
      moduleName = "{ANONYMOUS}";
   }
   else {
      moduleName = tmp;
      dwarf_dealloc( dbg(), tmp, DW_DLA_STRING );
   }

   dwarf_printf("Next DWARF module: %s with DIE %p and tag %d\n", moduleName.c_str(), moduleDIE, moduleTag);
   
   /* Set the language, if any. */
   Dwarf_Attribute languageAttribute;
   DWARF_ERROR_RET(dwarf_attr( moduleDIE, DW_AT_language, & languageAttribute, NULL ));
   
   /* And the base address for that module */
   Dwarf_Addr lowPCdwarf = 0;
   status = dwarf_lowpc( moduleDIE, &lowPCdwarf, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);   
   if (status == DW_DLV_OK)
   {
      curBase = (Address) lowPCdwarf;
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

   enclosureMap.clear();

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
   contexts_.push();

   setEntry(e);
   setParseSibling(p);


   // We escape the loop by checking parseSibling() after 
   // parsing this DIE and its children, if any
   while(1) {
      if (!findTag()) return false;
      if (!findOffset()) return false;
      curName() = std::string();

      dwarf_printf("(%lu) Parsing entry with tag 0x%x\n", id(), tag());

      // Insert only inserts the first time; we need that behavior
      enclosureMap.insert(std::make_pair(offset(), curEnclosure()));
      dwarf_printf("(%lu) Using enclosure pointer %p for offset\n",
                   id(), enclosureMap.find(offset())->second);
      
      bool ret = false;
      
      switch(tag()) {
         case DW_TAG_subprogram:
         case DW_TAG_entry_point:
            ret = parseSubprogram();
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
            // Parse child
            if (!parseChild()) return false;
            ret = true;
            break;
         default:
            dwarf_printf("(%lu) Warning: unparsed entry with tag %x\n",
                         id(), tag());
            return true;
      }
      
      if (ret && parseChild() ) {
         // Parse children
         Dwarf_Die childDwarf;
         int status = dwarf_child( entry(), & childDwarf, NULL );
         DWARF_CHECK_RET(status == DW_DLV_ERROR);
         if (status == DW_DLV_OK) {
            if (!parse_int(childDwarf, true)) return false;
         }
      }

      if (!parseSibling()) 
         break;

      Dwarf_Die siblingDwarf;
      int status =  dwarf_siblingof( dbg(), entry(), & siblingDwarf, NULL );
      DWARF_CHECK_RET(status == DW_DLV_ERROR);
      
      /* Deallocate the entry we just parsed. */
      dwarf_dealloc( dbg(), entry(), DW_DLA_DIE );

      if (status != DW_DLV_OK) {
         break;
      }

      setEntry(siblingDwarf);
   }
   contexts_.pop();
   return true;
}

bool DwarfWalker::parseSubprogram() {
   dwarf_printf("(%lu) parseSubprogram entry\n", id());

   // We may revisit dieEntries; look up the authoritative enclosure for this entry
   fieldListType *dieEnclosure = enclosureMap.find(offset())->second;
   dwarf_printf("(%lu) using enclosure pointer %p\n", id(), dieEnclosure);

   // We may be using a specification elsewhere in DWARF; dieEntry
   // represents our current information (e.g., address range)
   // and specEntry represents the specification (e.g., parameters),
   // which may be the same as dieEntry. 

   bool isAbstractOrigin = false;
   if (!handleAbstractOrigin(isAbstractOrigin)) return false;
 
   // This can actually be three-layered, so backchain again
   bool hasSpecification = false;
   if (!handleSpecification(hasSpecification)) return false;
   
   if (!findFuncName()) return false;

   if (!nameDefined()) return true; 

   dwarf_printf("(%lu) Identified function name as %s\n", id(), curName().c_str());

   // We're going to be parsing in the context of this function, so increase the
   // stack. 

   if (!findFunction()) return false;

   if (!parseSibling() && curFunc()) {
      // We do a localized parse to get info from the abstract origin
      // or specification. If we find a function for this, then we've already
      // parsed the information and we can return immediately.
      return true;
   }

   if (!curFunc()) return false;

   // Get the frame base if it exists
   if (!getFrameBase()) return false;

   // Get the return type
   if (!getReturnType(hasSpecification)) return false;
      
   if (dieEnclosure) {
      // It's a member function, so add it as a field to the container
      addFuncToContainer(dieEnclosure);
   }
   
   // Parse parent nodes and their children but not their sibling
   
   if ( isAbstractOrigin ) {
      dwarf_printf("(%lu) Parsing abstract parent", id());
      if (!parse_int(abstractEntry(), false)) return false;

      dwarf_dealloc(dbg(), abstractEntry(), DW_DLA_DIE); 
   } 
   else if ( hasSpecification ) {
      dwarf_printf("(%lu) Parsing specification entry", id());
      if (!parse_int(specEntry(), false)) return false;
   }
   return true;
}

bool DwarfWalker::parseCommonBlock() {
   dwarf_printf("(%lu) Parsing common block\n", id());

   char * commonBlockName_ptr;
   DWARF_FAIL_RET(dwarf_diename( entry(), & commonBlockName_ptr, NULL ));
   std::string commonBlockName = commonBlockName_ptr;
   
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
      dwarf_printf("(%lu) Couldn't find variable for common block %s\n", id(), commonBlockName.c_str());
      return false;
   }
   
   Symbol *commonBlockVar = commonBlockVars[0];
   assert(commonBlockVar);

   typeCommon *commonBlockType = NULL;

   commonBlockType = dynamic_cast<typeCommon *>(tc()->findVariableType(commonBlockName));
   if (commonBlockType == NULL) {
      commonBlockType = new typeCommon( (typeId_t) offset(), commonBlockName );
      assert( commonBlockType != NULL );
      tc()->addGlobalVariable( commonBlockName, commonBlockType );
   }	

   setCommon(commonBlockType);

   return true;
}

bool DwarfWalker::parseConstant() {
   // Right now we don't handle these
   dwarf_printf("(%lu) Skipping named constant/variable with constant value\n", id());
   return true;
}

bool DwarfWalker::parseVariable() {
   dwarf_printf("(%lu) ParseVariable entry\n", id());
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
   
   /* We'll start with the location, since that's most likely to
      require the _specification. */

   std::vector<VariableLocation> locs;
   if (!decodeLocationList(DW_AT_location, NULL, locs)) return false;
   
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
   
   /* If this DIE has a _specification, use that for the rest of our inquiries. */
   bool hasSpecification = false;
   if (!handleSpecification(hasSpecification)) return false;

   if (!findName(curName())) return false;

   removeFortranUnderscore(curName());

   Type *type = NULL;
   if (!findType(type, false)) return false;
   
   Dwarf_Unsigned variableLineNo;
   bool hasLineNumber = false;
   std::string fileName;

   if (curFunc() || curEnclosure()) {
      if (!getLineInformation(variableLineNo, hasLineNumber, fileName)) return false;
   }

   if (!curFunc() && !curEnclosure()) {
      /* The typeOffset forms a module-unique type identifier,
         so the Type look-ups by it rather than name. */
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
      dwarf_printf("(%lu) localVariable '%s', currentFunction %p\n", 
                   id(), curName().c_str(), curFunc());

      localVar * newVariable = new localVar(curName(),
                                            type,
                                            fileName, 
                                            (int) variableLineNo, 
                                            curFunc());
      for (unsigned int i = 0; i < locs.size(); ++i) {
         dwarf_printf("(%lu) Adding location %d of %d: (0x%lx - 0x%lx): %s, %s, %s, %ld\n",
                      id(), i+1, (int) locs.size(), locs[i].lowPC, locs[i].hiPC, 
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
      curEnclosure()->addField( curName(), type, locs[0].frameOffset);
   } /* end if this variable is not global */
   return true;
}

bool DwarfWalker::parseFormalParam() {
   dwarf_printf("(%lu) Parsing formal parameter\n", id());

   /* A formal parameter must occur in the context of a function.
      (That is, we can't do anything with a formal parameter to a
      function we don't know about.) */
   /* It's probably worth noting that a formal parameter may have a
      default value.  Since, AFAIK, Dyninst does nothing with this information,
      neither will we. */
   if (!curFunc()) return true;
   
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
   
   if(locs[0].stClass == storageAddr) {
      dwarf_printf("(%lu) Ignoring formal parameter that appears to be in memory, not on stack/in register\n",
                   id());
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
   if (!nameDefined()) return true; 
      
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
   if (findName(curName())) return false;
   if (!nameDefined()) return true;

   unsigned size = 0;
   if (!findSize(size)) return false;

            
   /* Generate the appropriate built-in type; since there's no
      reliable way to distinguish between a built-in and a scalar,
      we don't bother to try. */
   typeScalar * baseType = new typeScalar( (typeId_t) id(), (unsigned int) size, curName());
   assert( baseType != NULL );
   
   /* Add the basic type to our collection. */

   baseType = tc()->addOrUpdateType( baseType );
   
   return true;
}

bool DwarfWalker::parseTypedef() {
   if (!findName(curName())) return false;

   Type *referencedType = NULL;
   if (!findType(referencedType, true)) return false;

   if (!nameDefined()) {
      if (!fixName(curName(), referencedType)) return false;
   }

   typeTypedef * typedefType = new typeTypedef( (typeId_t) offset(), referencedType, curName());
   typedefType = tc()->addOrUpdateType( typedefType );

   return true;
}

bool DwarfWalker::parseArray() {
   dwarf_printf("(%lu) Parsing array\n", id());
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

   Type *elementType = NULL;
   if (!findType(elementType, false)) return false;
   if (!elementType) return false;

   /* Find the range(s) of the elements. */
   Dwarf_Die firstRange;

   DWARF_FAIL_RET(dwarf_child( entry(), & firstRange, NULL ));

   typeArray * baseArrayType = parseMultiDimensionalArray(firstRange, 
                                                          elementType);
   if (!baseArrayType) return false;

   dwarf_dealloc( dbg(), firstRange, DW_DLA_DIE );
   
   /* The baseArrayType is an anonymous type with its own typeID.  Extract
      the information and add an array type for this DIE. */

   typeArray *arrayType = new typeArray( (typeId_t) offset(),
                                         baseArrayType->getBaseType(), 
                                         baseArrayType->getLow(),
                                         baseArrayType->getHigh(), 
                                         curName());
   assert( arrayType != NULL );

   arrayType = tc()->addOrUpdateType( arrayType );
   
   /* Don't parse the children again. */
   setParseChild(false);
   
   return true;
}

bool DwarfWalker::parseSubrange() {
   std::string loBound;
   std::string hiBound;
   parseSubrangeAUX(entry(), loBound, hiBound);

  return true;
}

bool DwarfWalker::parseEnum() {
   if (!findName(curName())) return false;

   typeEnum* enumerationType = new typeEnum( (typeId_t) offset(), curName());
   assert( enumerationType != NULL );
   enumerationType = dynamic_cast<typeEnum *>(tc()->addOrUpdateType( enumerationType ));

   setEnum(enumerationType);
   return true;
}

bool DwarfWalker::parseInheritance() {
   /* Acquire the super class's type. */
   Type *superClass = NULL;
   if (!findType(superClass, false)) return false;
   if (!superClass) return false;

   visibility_t visibility;
   if (!findVisibility(visibility)) return false;

   /* Add a readily-recognizable 'bad' field to represent the superclass.
      Type::getComponents() will Do the Right Thing. */
   std::string fName = "{superclass}";
   curEnclosure()->addField( fName, superClass, -1, visibility );
   return true;
}

bool DwarfWalker::parseStructUnionClass() {
   assert(tag() == DW_TAG_structure_type ||
          tag() == DW_TAG_union_type ||
          tag() == DW_TAG_class_type);

   if (!findName(curName())) return false;

   unsigned size;
   if (!findSize(size)) return false;

   fieldListType * containingType = NULL;

   switch ( tag() ) {
      case DW_TAG_structure_type: 
      case DW_TAG_class_type: {
         typeStruct *ts = new typeStruct( (typeId_t) offset(), curName());
         containingType = dynamic_cast<fieldListType *>(tc()->addOrUpdateType(ts));
         break;
      }
      case DW_TAG_union_type: 
      {
         typeUnion *tu = new typeUnion( (typeId_t) offset(), curName());
         containingType = dynamic_cast<fieldListType *>(tc()->addOrUpdateType(tu));
         break;
      }
   }
   setEnclosure(containingType);
   return true;
}

bool DwarfWalker::parseEnumEntry() {
   if (!findName(curName())) return false;

   long value;
   bool valid;
   if (!findValue(value, valid)) return false;

   curEnum()->addConstant(curName(), value);
   return true;
}

bool DwarfWalker::parseMember() {
   if (!findName(curName())) return false;

   Type *memberType = NULL;
   if (!findType(memberType, false)) return false;
   if (!memberType) return false;

   std::vector<VariableLocation> locs;
   Address initialStackValue = 0;
   if (!decodeLocationList(DW_AT_data_member_location, &initialStackValue, locs)) return false;

   assert(locs[0].stClass == storageAddr);

   /* DWARF stores offsets in bytes unless the member is a bit field.
      Correct memberOffset as indicated.  Also, memberSize is in bytes
      from the underlying type, not the # of bits used from it, so
      correct that as necessary as well. */
   
   long int memberSize = memberType->getSize();

   // This code changes memberSize, which is then discarded. I'm not sure why...
   if (!fixBitFields(locs, memberSize)) return false;

   int offset_to_use = locs.size() ? locs[0].frameOffset : -1;

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
   if (!findName(curName())) return false;

   Type *type = NULL;
   if (!findType(type, true)) return false;

   if (!nameDefined()) {
      if (!fixName(curName(), type)) return false;
   }

   typeTypedef * modifierType = new typeTypedef((typeId_t) offset(), type, curName());
   assert( modifierType != NULL );
   modifierType = tc()->addOrUpdateType( modifierType );
   return true;
}

bool DwarfWalker::parseTypeReferences() {
   if (!findName(curName())) return false;

   Type *typePointedTo = NULL;
   if (!findType(typePointedTo, true)) return false;
   
   Type * indirectType = NULL;
   switch ( tag() ) {
      case DW_TAG_subroutine_type:
         indirectType = new typeFunction((typeId_t) offset(), typePointedTo, curName());
         indirectType = tc()->addOrUpdateType((typeFunction *) indirectType );
         break;
      case DW_TAG_ptr_to_member_type:
      case DW_TAG_pointer_type:
         indirectType = new typePointer((typeId_t) offset(), typePointedTo, curName());
         indirectType = tc()->addOrUpdateType((typePointer *) indirectType );
         break;
      case DW_TAG_reference_type:
         indirectType = new typeRef((typeId_t) offset(), typePointedTo, curName());
         indirectType = tc()->addOrUpdateType((typeRef *) indirectType );
         break;
      default:
         return false;
   }

   assert( indirectType != NULL );
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


bool DwarfWalker::handleAbstractOrigin(bool &isAbstract) {
   Dwarf_Die absE;

   dwarf_printf("(%lu) Checking for abstract origin\n", id());
   
   isAbstract = false;
   Dwarf_Bool isAbstractOrigin;

   DWARF_FAIL_RET(dwarf_hasattr(entry(),
                            DW_AT_abstract_origin, 
                            &isAbstractOrigin, NULL ));
   
   if (!isAbstractOrigin) return true;

   isAbstract = true;
   dwarf_printf("(%lu) abstract_origin is true, looking up reference\n", id());
   
   Dwarf_Attribute abstractAttribute;
   DWARF_FAIL_RET(dwarf_attr( entry(), DW_AT_abstract_origin, & abstractAttribute, NULL ));
   
   Dwarf_Off abstractOffset;
   DWARF_FAIL_RET(dwarf_global_formref( abstractAttribute, & abstractOffset, NULL ));
   
   DWARF_FAIL_RET(dwarf_offdie( dbg(), abstractOffset, & absE, NULL));
   
   dwarf_dealloc( dbg() , abstractAttribute, DW_DLA_ATTR );
   
   setAbstractEntry(absE);

   return true;
}

bool DwarfWalker::handleSpecification(bool &hasSpec) {
   Dwarf_Die specE; 

   dwarf_printf("(%lu) Checking for separate specification\n", id());
   hasSpec = false;

   Dwarf_Bool hasSpecification;
   DWARF_FAIL_RET(dwarf_hasattr( entry(), DW_AT_specification, & hasSpecification, NULL ));

   if (!hasSpecification) return true;

   hasSpec = true;
   
   dwarf_printf("(%lu) Entry has separate specification, retrieving\n", id());

   Dwarf_Attribute specAttribute;
   DWARF_FAIL_RET(dwarf_attr( entry(), DW_AT_specification, & specAttribute, NULL ));
   
   Dwarf_Off specOffset;
   DWARF_FAIL_RET(dwarf_global_formref( specAttribute, & specOffset, NULL ));
   
   DWARF_FAIL_RET(dwarf_offdie( dbg(), specOffset, & specE, NULL ));
   
   dwarf_dealloc( dbg(), specAttribute, DW_DLA_ATTR );

   setSpecEntry(specE);

   return true;
}

bool DwarfWalker::findName(std::string &name) {
   char *cname = NULL;

   int status = dwarf_diename( specEntry(), &cname, NULL );     
   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   if (status != DW_DLV_OK) {
      name = std::string();
      return true;
   }

   name = cname;
   return true;
}
   

bool DwarfWalker::findFuncName() {
   dwarf_printf("(%lu) Checking for function name\n", id());
   /* Prefer linkage names. */
   char *dwarfName = NULL;

   Dwarf_Attribute linkageNameAttr;

   int status = dwarf_attr( specEntry(), DW_AT_MIPS_linkage_name, & linkageNameAttr, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   if ( status == DW_DLV_OK )  {
      DWARF_FAIL_RET(dwarf_formstring( linkageNameAttr, &dwarfName, NULL ));
      curName() = dwarfName;
      dwarf_dealloc( dbg(), linkageNameAttr, DW_DLA_ATTR );
   } 
   else 
   {
      if (!findName(curName())) return false;
   } 

   return true;   
}

bool DwarfWalker::findFunction() {
   dwarf_printf("(%lu) Looking up function object matching %s\n", id(), curName().c_str());
   std::vector<Function *> ret_funcs;
   if (symtab()->findFunctionsByName(ret_funcs, curName())) {
      setFunc(ret_funcs[0]);
      dwarf_printf("(%lu) Found %ld possible matches, picking first: %p\n",
                   id(), ret_funcs.size(), curFunc());
      return true;
   }

   // Look it up by (entry) address
   Dwarf_Addr entryAddr;
   Offset lowpc;
   int status = dwarf_lowpc(entry(), &entryAddr, NULL);
   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   if (status == DW_DLV_OK 
       && obj()->convertDebugOffset(entryAddr, lowpc)) {
      Function *tmp;
      symtab()->findFuncByEntryOffset(tmp, lowpc);
      setFunc(tmp);
      dwarf_printf("(%lu) Lookup by offset 0x%lx identifies %p\n",
                   id(), lowpc, curFunc());
      return true;
   }
   dwarf_printf("(%lu) Failed to lookup by both name and address\n", id());
   return false;
}

bool DwarfWalker::getFrameBase() {
   dwarf_printf("(%lu) Checking for frame pointer information\n", id());

   std::vector<VariableLocation> *funlocs = new std::vector<VariableLocation>();
   if (!decodeLocationList(DW_AT_frame_base, NULL, *funlocs)) return false;

   if (funlocs->size()) {
      bool ret = curFunc()->setFramePtr(funlocs);
      DWARF_CHECK_RET(ret != true);
   }
   else {
      delete funlocs;
   }

   return true;
}

bool DwarfWalker::getReturnType(bool hasSpecification) {
   Dwarf_Attribute typeAttribute;
   int status = DW_DLV_OK;

   if (hasSpecification) {
      status = dwarf_attr( specEntry(), DW_AT_type, & typeAttribute, NULL );
   }   
   if (!hasSpecification || (status == DW_DLV_NO_ENTRY)) {
      status = dwarf_attr( entry(), DW_AT_type, & typeAttribute, NULL );
   }

   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   
   Type * returnType = NULL;
   Type *voidType = tc()->findType("void");
   
   if ( status == DW_DLV_NO_ENTRY ) {
      if (parseSibling()) {
         // If return type is void, specEntry or abstractOriginEntry would have set it
         dwarf_printf("(%lu) Return type void \n", id());
         curFunc()->setReturnType( voidType );
      }
   }
   else  {
      /* There's a return type attribute. */
      dwarf_printf("(%lu) Return type is not void\n", id());
      Dwarf_Off typeOffset;
      
      DWARF_FAIL_RET(dwarf_global_formref( typeAttribute, & typeOffset, NULL ));
      
      //parsing_printf("%s/%d: ret type %d\n",
      //			   __FILE__, __LINE__, typeOffset);
      
      returnType = tc()->findOrCreateType( (int) typeOffset );
      curFunc()->setReturnType( returnType );
      
      dwarf_dealloc( dbg(), typeAttribute, DW_DLA_ATTR );
   } /* end if not a void return type */
   return true;
}

// I'm not sure how the provided fieldListType is different from curEnclosure(),
// but that's the way the code was structured and it was working. 
bool DwarfWalker::addFuncToContainer(fieldListType *dieEnclosure) {
   /* Using the mangled name allows us to distinguish between overridden
      functions, but confuses the tests.  Since Type uses vectors
      to hold field names, however, duplicate -- demangled names -- are OK. */
   
   char * demangledName = P_cplus_demangle( curName().c_str(), symtab()->isNativeCompiler() );
   std::string toUse;

   if (!demangledName) {
      dwarf_printf("(%lu) Unable to demangle %s, using it as mangled\n", id(), curName().c_str());
      toUse = curName();
   }
   else {
      // Strip everything left of the rightmost ':' off to get rid of the class names
      toUse = demangledName;
      size_t offset = toUse.rfind(':');
      if (offset != toUse.npos) {
         toUse = toUse.substr(offset);
      }
   }

   typeFunction *funcType = new typeFunction( (typeId_t) offset(), curFunc()->getReturnType(), toUse);
   dieEnclosure->addField( toUse, funcType);
   free( demangledName );
   return true;
}

bool DwarfWalker::findType(Type *&type, bool defaultToVoid) {
   int status;

   /* Acquire the parameter's type. */
   Dwarf_Attribute typeAttribute;
   status = dwarf_attr( specEntry(), DW_AT_type, & typeAttribute, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);

   if (status == DW_DLV_NO_ENTRY) {
      if (defaultToVoid) {
         type = tc()->findType("void");
      }
      return true;
   }

   Dwarf_Off typeOffset;
   DWARF_FAIL_RET(dwarf_global_formref( typeAttribute, & typeOffset, NULL ));
   
   /* The typeOffset forms a module-unique type identifier,
      so the Type look-ups by it rather than name. */
   type = tc()->findOrCreateType( (int) typeOffset );
   dwarf_dealloc( dbg(), typeAttribute, DW_DLA_ATTR );
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
   locs.clear();
   dwarf_printf("(%lu) decodeLocationList for attr %d\n", id(), attr);

   Dwarf_Bool hasLocation = false;
   DWARF_FAIL_RET(dwarf_hasattr( specEntry(), attr, & hasLocation, NULL ));
   if (!hasLocation) {
      dwarf_printf("(%lu): no such attribute\n", id());
      return true;
   }
   
   /* Acquire the location of this formal parameter. */
   Dwarf_Attribute locationAttribute;
   DWARF_FAIL_RET(dwarf_attr( specEntry(), attr, & locationAttribute, NULL ));

   // Get the form (datatype) for this particular attribute
   Dwarf_Half retform;
   DWARF_FAIL_RET(dwarf_whatform(locationAttribute, &retform, NULL));

   // And see if it's a constant
   Dwarf_Form_Class form = dwarf_get_form_class(version, attr, offset_size, retform);
   if (form == DW_FORM_CLASS_CONSTANT) {
      Address value = 0;
      switch (retform) {
         case DW_FORM_sdata: {
            Dwarf_Signed tmp;
            DWARF_FAIL_RET(dwarf_formsdata(locationAttribute, &tmp, NULL));
            value = (Address) tmp;
            break;
         }
         default:
            return false;
      }
      

   }
   else {
      Dwarf_Locdesc **locationList;
      Dwarf_Signed listLength;
      int status = dwarf_loclist_n( locationAttribute, & locationList, & listLength, NULL );
      
      dwarf_dealloc( dbg(), locationAttribute, DW_DLA_ATTR );
      
      if (status != DW_DLV_OK) return true;
      
      dwarf_printf("(%lu) location list with %d entries found\n", id(), (int) listLength);
      
      if (!decodeLocationListForStaticOffsetOrAddress( locationList,
                                                       listLength, 
                                                       locs, 
                                                       initialStackValue)) return false;
      
      deallocateLocationList( locationList, listLength );
   }

   return true;
}

bool DwarfWalker::findSize(unsigned &size) {
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
   int status = dwarf_attr( entry, DW_AT_lower_bound, & lowerBoundAttribute, NULL );
   DWARF_CHECK_RET(status == DW_DLV_ERROR);
   
   if ( status == DW_DLV_OK ) {
      if (!decipherBound(lowerBoundAttribute, loBound )) return false;
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
      if (!decipherBound(upperBoundAttribute, hiBound )) return false;
      dwarf_dealloc( dbg(), upperBoundAttribute, DW_DLA_ATTR );
   } /* end if we found an upper bound or count. */
   
   /* Construct the range type. */
   if (!findName(curName())) return false;
   if (!nameDefined()) {
      curName() = "{anonymousRange}";
   }

   Dwarf_Off subrangeOffset;
   DWARF_ERROR_RET(dwarf_dieoffset( entry, & subrangeOffset, NULL ));
   
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
   
   typeSubrange * rangeType = new typeSubrange( (int) subrangeOffset, 
                                                0, low_conv, hi_conv, curName() );
   assert( rangeType != NULL );
   rangeType = tc()->addOrUpdateType( rangeType );
   return true;
}

typeArray *DwarfWalker::parseMultiDimensionalArray(Dwarf_Die range, 
                                                   Type * elementType)
{
  char buf[32];
  /* Get the (negative) typeID for this range/subarray. */
  Dwarf_Off dieOffset;
  DWARF_FAIL_RET(dwarf_dieoffset( range, & dieOffset, NULL ));

  /* Determine the range. */
  std::string loBound;
  std::string hiBound;
  parseSubrangeAUX(range, loBound, hiBound);

  /* Does the recursion continue? */
  Dwarf_Die nextSibling;
  int status = dwarf_siblingof( dbg(), range, & nextSibling, NULL );
  DWARF_CHECK_RET(status == DW_DLV_ERROR);

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

bool DwarfWalker::decipherBound(Dwarf_Attribute boundAttribute, std::string &boundString ) {
   Dwarf_Half boundForm;
   DWARF_FAIL_RET(dwarf_whatform( boundAttribute, & boundForm, NULL ));
   
   switch( boundForm ) {
      case DW_FORM_data1:
      case DW_FORM_data2:
      case DW_FORM_data4:
      case DW_FORM_data8:
      case DW_FORM_sdata:
      case DW_FORM_udata: 
      {
         Dwarf_Unsigned constantBound;
         DWARF_FAIL_RET(dwarf_formudata( boundAttribute, & constantBound, NULL ));
         char bString[40];
         sprintf(bString, "%lu", (unsigned long)constantBound);
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
         DWARF_FAIL_RET(dwarf_offdie( dbg(), boundOffset, & boundEntry, NULL ));
         
      /* Does it have a name? */
         char * boundName = NULL;
         int status = dwarf_diename( boundEntry, & boundName, NULL );
         DWARF_CHECK_RET(status == DW_DLV_ERROR);
         
         if ( status == DW_DLV_OK ) {
            boundString = boundName;
            
            dwarf_dealloc( dbg(), boundName, DW_DLA_STRING );
            return true;
         }
         
         /* Does it describe a nameless constant? */
         Dwarf_Attribute constBoundAttribute;
         status = dwarf_attr( boundEntry, DW_AT_const_value, & constBoundAttribute, NULL );
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
 
   for (unsigned locIndex = 0 ; locIndex < listLength; locIndex++) {
      dwarf_printf("(%lu) Decoding entry %d of %d over range 0x%lx - 0x%lx\n", 
                   id(), locIndex+1, (int) listLength,
                   (unsigned long) locationList[locIndex]->ld_lopc,
                   (unsigned long) locationList[locIndex]->ld_hipc);

                   
      bool isLocSet = false;
      VariableLocation loc;
      // Initialize location values.
      loc.stClass = storageAddr;
      loc.refClass = storageNoRef;
      loc.reg = -1;
      
      /* There is only one location. */
      Dwarf_Locdesc *location = locationList[locIndex];
      
      loc.lowPC = (Offset)location->ld_lopc;
      loc.hiPC = (Offset)location->ld_hipc;
      
      // if range of the variable is the entire address space, do not add lowpc
      if (loc.lowPC != (unsigned long) 0 || loc.hiPC != (unsigned long) ~0) 
      {
         loc.lowPC = (Offset)location->ld_lopc + curBase;
         loc.hiPC = (Offset)location->ld_hipc + curBase;
      }
      
      long int end_result = 0;
      long int *tmp = (long int *)initialStackValue;
      bool result = decodeDwarfExpression(location, tmp, &loc, isLocSet, NULL,
                                          symtab()->getArchitecture(), end_result);
      initialStackValue = (Address *)tmp;
      if (!result) {
         dwarf_printf("(%lu): decodeDwarfExpr failed\n", id());
         return false;
      }
      
      if (!isLocSet)
      {
         loc.frameOffset = end_result;
         locs.push_back(loc);
      }
      else
         locs.push_back(loc);
   }
   
   /* decode successful */
   return true;
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

