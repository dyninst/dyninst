#include "dwarfWalker.h"
#include "common/h/headers.h"
#include "Module.h"
#include "Symtab.h"
#include "Collections.h"
#include "dwarf.h"
#include "Object.h"
#include "Object-elf.h"
#include "Function.h"

using namespace Dyninst;
using namespace SymtabAPI;

#define DWARF_WRAP(x) {                                                 \
      int status = (x);                                                 \
      if (status != DW_DLV_OK) {                                        \
         fprintf(stderr, "[%s:%d]: libdwarf returned %d, ret false\n", FILE__, __LINE__, status); \
         return false;                                                  \
      }                                                                 \
   }

#define DWARF_ERROR_RET(x) {                                            \
      if (x) {                                                          \
         fprintf(stderr, "[%s:%d]: parsing failure, ret false\n", FILE__, __LINE__); \
         return false;                                                  \
      }                                                                 \
   }

// TODO
#define dwarf_printf printf

dwarfWalker::dwarfWalker(Dwarf_Debug &dbg,
                         Module *mod,
                         Symtab *symtab,
                         std::vector<std::string> &srcFiles) 
   :
   dbg_(dbg),
   mod_(mod),
   symtab_(symtab),
   srcFiles_(srcFiles) {
   
   }

dwarfWalker::~dwarfWalker() {}

Object *dwarfWalker::obj() { 
   return symtab_->getObject();
}

// Initialize for a particular walk through the dwarf subtree
bool dwarfWalker::parse(Dwarf_Die dieEntry, Address lowpc) {
   lowaddr_ = lowpc;

   return parse_int(dieEntry, true);
}

// As mentioned in the header, this is separate from parse()
// so we can have a non-Context-creating parse method that reuses
// the Context from the parent. This allows us to pass in current
// function, etc. without the Context stack exploding
bool dwarfWalker::parse_int(Dwarf_Die e, bool p) {
   contexts_.push();

   setEntry(e);
   setParseSibling(p);

   // We escape the loop by checking parseSibling() after 
   // parsing this DIE and its children, if any
   while(1) {
      if (!findTag()) return false;
      if (!findOffset()) return false;
      curName() = std::string();

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
         default:
            return true;
      }
      
      if (ret && parseChild() ) {
         // Parse children
         Dwarf_Die childDwarf;
         int status = dwarf_child( entry(), & childDwarf, NULL );
         DWARF_ERROR_RET(status == DW_DLV_ERROR);
         if (status == DW_DLV_OK) {
            if (!parse_int(childDwarf, true)) return false;
         }
      }

      if (!parseSibling()) 
         break;

      Dwarf_Die siblingDwarf;
      int status = dwarf_siblingof( dbg(), entry(), & siblingDwarf, NULL );
      DWARF_ERROR_RET(status == DW_DLV_ERROR);
      
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

bool dwarfWalker::parseSubprogram() {
   dwarf_printf("parseSubprogram entry\n");

   // We may revisit dieEntries; look up the authoritative enclosure for this entry
   fieldListType *dieEnclosure = enclosureMap.find(offset())->second;

   // We may be using a specification elsewhere in DWARF; dieEntry
   // represents our current information (e.g., address range)
   // and specEntry represents the specification (e.g., parameters),
   // which may be the same as dieEntry. 

   bool isAbstractOrigin = false;
   if (!handleAbstractOrigin(isAbstractOrigin, specEntry())) return false;

   // This can actually be three-layered, so backchain again
   bool hasSpecification = false;
   if (!handleSpecification(hasSpecification, specEntry())) return false;
   
   if (!findFuncName()) return false;

   if (!nameDefined()) return true; 

   dwarf_printf("Identified function name as %s\n", curName().c_str());

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
      dwarf_printf("Parsing abstract parent");
      if (!parse_int(abstractEntry(), false)) return false;

      dwarf_dealloc(dbg(), abstractEntry(), DW_DLA_DIE); 
   } 
   else if ( hasSpecification ) {
      dwarf_printf("Parsing specification");
      if (!parse_int(specEntry(), false)) return false;
   }
   return true;
}

bool dwarfWalker::parseCommonBlock() {
   dwarf_printf("Parsing common block\n");

   char * commonBlockName_ptr;
   DWARF_WRAP(dwarf_diename( entry(), & commonBlockName_ptr, NULL ));
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
      dwarf_printf("Couldn't find variable for common block %s\n", commonBlockName.c_str());
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

bool dwarfWalker::parseConstant() {
   // Right now we don't handle these
   dwarf_printf("Skipping named constant/variable with constant value\n");
   return true;
}

bool dwarfWalker::parseVariable() {
   dwarf_printf("ParseVariable entry\n");
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

   vector<VariableLocation> locs;
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
   if (!handleSpecification(hasSpecification, specEntry())) return false;

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
      assert(!curEnclosure());
      /* We now have the variable name, type, offset, and line number.
         Tell Dyninst about it. */
      if (!nameDefined()) return true;
      dwarf_printf( "localVariable '%s', currentFunction %p\n", 
                    curName().c_str(), curFunc());

      localVar * newVariable = new localVar(curName(),
                                            type,
                                            fileName, 
                                            (int) variableLineNo, 
                                            curFunc());
      for (unsigned int i = 0; i < locs.size(); ++i) {
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

bool dwarfWalker::parseFormalParam() {
   dwarf_printf("Parsing formal parameter\n");

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
   
   vector<VariableLocation> locs;
   if (!getLocationList(locs)) return false;
   
   if(locs[0].stClass == storageAddr) {
      dwarf_printf("Ignoring formal parameter that appears to be in memory, not on stack/in register\n");
      return true;
   }
   
   /* If the DIE has an _abstract_origin, we'll use that for the
      remainder of our inquiries. */
   Dwarf_Die abstractEntry;
   bool hasAbstractOrigin;
   if (!handleAbstractOrigin(hasAbstractOrigin, abstractEntry)) return false;
   specEntry() = abstractEntry;

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

bool dwarfWalker::parseBaseType() {
   if (findName(curName())) return false;
   if (!nameDefined()) return true;

   unsigned size = 0;
   if (!findSize(size)) return false;

            
   /* Generate the appropriate built-in type; since there's no
      reliable way to distinguish between a built-in and a scalar,
      we don't bother to try. */
   typeScalar * baseType = new typeScalar( (typeId_t) offset(), (unsigned int) size, curName());
   assert( baseType != NULL );
   
   /* Add the basic type to our collection. */

   baseType = tc()->addOrUpdateType( baseType );
   
   return true;
}

bool dwarfWalker::parseTypedef() {
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

bool dwarfWalker::parseArray() {
   dwarf_printf(" DW_TAG_array \n");
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

   DWARF_WRAP(dwarf_child( entry(), & firstRange, NULL ));

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

bool dwarfWalker::parseSubrange() {
   std::string loBound = "{unknown or default}";
   std::string hiBound = "{unknown or default}";

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
   int status = dwarf_attr( entry(), DW_AT_lower_bound, & lowerBoundAttribute, NULL );
   DWARF_ERROR_RET(status == DW_DLV_ERROR);
   
   if ( status == DW_DLV_OK ) {
      if (!decipherBound(lowerBoundAttribute, loBound )) return false;
      dwarf_dealloc( dbg(), lowerBoundAttribute, DW_DLA_ATTR );
   } /* end if we found a lower bound. */
   
   /* Look for the upper bound. */
   Dwarf_Attribute upperBoundAttribute;
   status = dwarf_attr( entry(), DW_AT_upper_bound, & upperBoundAttribute, NULL );
   DWARF_ERROR_RET(status == DW_DLV_ERROR);

   if ( status == DW_DLV_NO_ENTRY ) {
      status = dwarf_attr( entry(), DW_AT_count, & upperBoundAttribute, NULL );
      DWARF_ERROR_RET(status == DW_DLV_ERROR);
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
   status = dwarf_dieoffset( entry(), & subrangeOffset, NULL );
   DWARF_ERROR_RET(status == DW_DLV_ERROR);
   
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

bool dwarfWalker::parseEnum() {
   if (!findName(curName())) return false;

   typeEnum* enumerationType = new typeEnum( (typeId_t) offset(), curName());
   assert( enumerationType != NULL );
   enumerationType = dynamic_cast<typeEnum *>(tc()->addOrUpdateType( enumerationType ));

   setEnum(enumerationType);
   return true;
}

bool dwarfWalker::parseInheritance() {
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

bool dwarfWalker::parseStructUnionClass() {
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

bool dwarfWalker::parseEnumEntry() {
   if (!findName(curName())) return false;

   long value;
   bool valid;
   if (!findValue(value, valid)) return false;

   curEnum()->addConstant(curName(), value);
   return true;
}

bool dwarfWalker::parseMember() {
   if (!findName(curName())) return false;

   Type *memberType = NULL;
   if (!findType(memberType, false)) return false;
   if (!memberType) return false;

   std::vector<VariableLocation> locs;
   Address initialStackValue;
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

   if (curName()) {
      curEnclosure()->addField( curName(), memberType, offset_to_use);
   }
   else {
      curEnclosure()->addField("[anonymous union]", memberType, offset_to_use);
   }
   return true;
}

// TODO: this looks a lot like parseTypedef. Collapse?
bool dwarfWalker::parseConstPackedVolatile() {
   if (!findName(curName())) return false;

   Type *type = NULL;
   if (!findType(type, true)) return false;

   if (!curName()) {
      if (!fixName(curName(), type)) return false;
   }

   typeTypedef * modifierType = new typeTypedef((typeId_t) offset(), type, curName());
   assert( modifierType != NULL );
   modifierType = tc()->addOrUpdateType( modifierType );
   return true;
}

bool dwarfWalker::parseTypeReferences() {
   if (!findName(curName())) return false;

   Type *typePointedTo = NULL;
   if (!findType(typePointedTo, true)) return false;
   
   Type * indirectType = NULL;
   switch ( tag() ) {
      case DW_TAG_subroutine_type:
         indirectType = new typeFunction((typeId_t) dieOffset, typePointedTo, tName);
         indirectType = tc()->addOrUpdateType((typeFunction *) indirectType );
         break;
      case DW_TAG_ptr_to_member_type:
      case DW_TAG_pointer_type:
         indirectType = new typePointer((typeId_t) dieOffset, typePointedTo, tName);
         indirectType = tc()->addOrUpdateType((typePointer *) indirectType );
         break;
      case DW_TAG_reference_type:
         indirectType = new typeRef((typeId_t) dieOffset, typePointedTo, tName);
         indirectType = tc()->addOrUpdateType((typeRef *) indirectType );
         break;
      default:
         return false;
   }

   assert( indirectType != NULL );
   return true;
}

bool dwarfWalker::findTag() {
   Dwarf_Half dieTag;
   DWARF_WRAP(dwarf_tag( entry(), & dieTag, NULL ));
   setTag(dieTag);
   return true;
}


bool dwarfWalker::findOffset() {
   Dwarf_Off dieOffset;
   DWARF_WRAP(dwarf_dieoffset( entry(), & dieOffset, NULL ));
   
   setOffset(dieOffset);
   return true;
}


bool dwarfWalker::handleAbstractOrigin(bool &isAbstract, Dwarf_Die &abstractEntry) {
   dwarf_printf("Checking for abstract origin\n");
   
   isAbstract = false;
   Dwarf_Bool isAbstractOrigin;

   DWARF_WRAP(dwarf_hasattr(baseEntry(),
                            DW_AT_abstract_origin, 
                            &isAbstractOrigin, NULL ));
   
   if (!isAbstractOrigin) return true;

   isAbstract = true;
   dwarf_printf("abstract_origin is true, looking up reference\n");
   
   Dwarf_Attribute abstractAttribute;
   DWARF_WRAP(dwarf_attr( entry(), DW_AT_abstract_origin, & abstractAttribute, NULL ));
   
   Dwarf_Off abstractOffset;
   DWARF_WRAP(dwarf_global_formref( abstractAttribute, & abstractOffset, NULL ));
   
   DWARF_WRAP(dwarf_offdie( dbg, abstractOffset, & abstractEntry, NULL));
   
   dwarf_dealloc( dbg() , abstractAttribute, DW_DLA_ATTR );

   dwarf_printf("Abstract origin found: %d (0x%x)\n", abstractEntry, abstractEntry);
   return true;
}

bool dwarfWalker::handleSpecification(bool &hasSpec, Dwarf_Die &specEntry) {
   dwarf_printf("Checking for separate specification\n");
   hasSpec = false;

   Dwarf_Bool hasSpecification;
   DWARF_WRAP(dwarf_hasattr( specEntry, DW_AT_specification, & hasSpecification, NULL ));

   if (!hasSpecification) return true;

   hasSpec = true;
   
   Dwarf_Attribute specAttribute;
   DWARF_WRAP(dwarf_attr( specEntry, DW_AT_specification, & specAttribute, NULL ));
   
   Dwarf_Off specOffset;
   DWARF_WRAP(dwarf_global_formref( specAttribute, & specOffset, NULL ));
   
   DWARF_WRAP(dwarf_offdie( dbg, specOffset, & specEntry, NULL ));
   
   dwarf_dealloc( dbg(), specAttribute, DW_DLA_ATTR );
   dwarf_printf("Separate specification found: %d (0x%x)\n", specEntry);

   return true;
}

bool dwarfWalker::findName(std::string &name, Dwarf_Die entry) {
   char *cname = NULL;

   int status = dwarf_diename( entry, &cname, NULL );     
   DWARF_ERROR_RET(status == DW_DLV_ERROR);
   if (status != DW_DLV_OK) {
      name = std::string();
      return true;
   }

   name = cname;
   return true;
}
   

bool dwarfWalker::findFuncName() {
   dwarf_printf("Checking for function name\n");
   /* Prefer linkage names. */
   char *dwarfName = NULL;

   Dwarf_Attribute linkageNameAttr;

   int status = dwarf_attr( specEntry, DW_AT_MIPS_linkage_name, & linkageNameAttr, NULL );
   DWARF_ERROR_RET(status == DW_DLV_ERROR);
   else if ( status == DW_DLV_OK )  {
      DWARF_WRAP(dwarf_formstring( linkageNameAttr, &dwarfName, NULL ));
      curName() = dwarfName;
      dwarf_dealloc( dbg(), linkageNameAttr, DW_DLA_ATTR );
   } 
   else 
   {
      if (!findName(specEntry, curName())) return false;
   } 

   return true;   
}

bool dwarfWalker::findFunction() {
   dwarf_printf("Looking up function object matching %s\n", curName().c_str());
   std::vector<Function *> ret_funcs;
   if (symtab()->findFunctionsByName(curName(), ret_funcs)) {
      setCurFunc(ret_funcs[0]);
      dwarf_printf("Found %d possible matches, picking first: %p\n",
                   ret_funcs.size(), curFunc());
      return true;
   }

   // Look it up by (entry) address
   Dwarf_Addr entryAddr;
   Offset lowpc;
   int status = dwarf_lowpc(baseEntry(), &entryAddr, NULL);
   DWARF_ERROR_RET(status == DW_DLV_ERROR);
   if (status == DW_DLV_OK 
       && obj()->convertDebugOffset(baseAddr, lowpc)) {
      symtab()->findFuncByEntryOffset(curFunc(), lowpc);
      dwarf_printf("Lookup by offset 0x%lx identifies %p\n",
                   lowpc, curFunc());
      return true;
   }
   dwarf_printf("Failed to lookup by both name and address\n");
   return false;
}

bool dwarfWalker::getFrameBase() {
   dwarf_printf("Checking for frame pointer information\n");
   Dwarf_Attribute frameBaseAttribute;
   int status = dwarf_attr( entry(), DW_AT_frame_base, & frameBaseAttribute, NULL );
   
   DWARF_ERROR_RET(status == DW_DLV_ERROR);
   
   if ( status == DW_DLV_OK )  {
      Dwarf_Locdesc ** locationList;
      Dwarf_Signed listLength;
      
      status = dwarf_loclist_n( frameBaseAttribute, & locationList, & listLength, NULL );
      DWARF_ERROR_RET(status == DW_DLV_ERROR);
      if (status != DW_DLV_OK) return true;
      
      dwarf_dealloc( dbg(), frameBaseAttribute, DW_DLA_ATTR );
      dwarf_printf("Have frame pointer information, decoding\n");

      vector<VariableLocation> *funlocs = new vector<VariableLocation>();
      bool ret = decodeLocationListForStaticOffsetOrAddress(locationList, 
                                                            listLength,
                                                            symtab(),
                                                            *funlocs, 
                                                            lowaddr(),
                                                            NULL);
      DWARF_ERROR_RET(ret != true);
      
      ret = newFunction->setFramePtr(funlocs);
      DWARF_ERROR_RET(ret != true);
      
      deallocateLocationList( dbg, locationList, listLength );
   } /* end if this DIE has a frame base attribute */
   return true;
}

bool dwarfWalker::getReturnType(bool hasSpecification, Dwarf_Die specEntry) {
   Dwarf_Attribute typeAttribute;
   int status;

   if (hasSpecification) {
      status = dwarf_attr( specEntry(), DW_AT_type, & typeAttribute, NULL );
   }   
   if (status == DW_DLV_NO_ENTRY) {
      status = dwarf_attr( baseEntry(), DW_AT_type, & typeAttribute, NULL );
   }

   DWARF_ERROR_RET(status == DW_DLV_ERROR);
   
   Type * returnType = NULL;
   Type *voidType = tc()->findType("void");
   
   if ( status == DW_DLV_NO_ENTRY ) {
      if (parseSibling()) {
         // If return type is void, specEntry or abstractOriginEntry would have set it
         dwarf_printf(" Return type void \n");
         newFunction->setReturnType( voidType );
      }
   }
   else  {
      /* There's a return type attribute. */
      dwarf_printf(" Return type is not void \n");
      Dwarf_Off typeOffset;
      
      DWARF_WRAP(dwarf_global_formref( typeAttribute, & typeOffset, NULL ));
      
      //parsing_printf("%s/%d: ret type %d\n",
      //			   __FILE__, __LINE__, typeOffset);
      
      returnType = tc()->findOrCreateType( (int) typeOffset );
      newFunction->setReturnType( returnType );
      
      dwarf_dealloc( dbg(), typeAttribute, DW_DLA_ATTR );
   } /* end if not a void return type */
   return true;
}

// I'm not sure how the provided fieldListType is different from curEnclosure(),
// but that's the way the code was structured and it was working. 
void dwarfWalker::addFuncToContainer(fieldListType *dieEnclosure) {
   /* Using the mangled name allows us to distinguish between overridden
      functions, but confuses the tests.  Since Type uses vectors
      to hold field names, however, duplicate -- demangled names -- are OK. */
   
   char * demangledName = P_cplus_demangle( curName().c_str(), symtab()->isNativeCompiler() );
   std::string toUse;

   if (!demangledName) {
      dwarf_printf("Unable to demangle %s, using it as mangled\n", curName().c_str());
      toUse = curName();
   }
   else {
      // Strip everything left of the rightmost ':' off to get rid of the class names
      toUse = demangledName;
      size_t offset = toUse.rfind(':');
      if (offset != toUse::npos) {
         toUse = toUse.substr(offset);
      }
   }

   typeFunction *funcType = new typeFunction( (typeId_t) dieOffset, returnType, toUse);
   dieEnclosure->addField( fName, funcType);
   dwarf_printf(" Adding function %s to class \n", leftMost);
   free( demangledName );
}

bool dwarfWalker::findType(variableType *&type, bool defaultToVoid) {
   int status;

   /* Acquire the parameter's type. */
   Dwarf_Attribute typeAttribute;
   status = dwarf_attr( specEntry(), DW_AT_type, & typeAttribute, NULL );
   DWARF_ERROR_RET(status == DW_DLV_ERROR);

   if (status == DW_DLV_NO_ENTRY) {
      if (defaultToVoid) {
         type = tc()->findType("void");
      }
      return true;
   }

   Dwarf_Off typeOffset;
   DWARF_WRAP(dwarf_global_formref( typeAttribute, & typeOffset, NULL ));
   
   /* The typeOffset forms a module-unique type identifier,
      so the Type look-ups by it rather than name. */
   Type * type = tc()->findOrCreateType( (int) typeOffset );
   dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
   return true;
}

bool dwarfWalker::getLineInformation(Dwarf_Unsigned &variableLineNo,
                                     bool &hasLineNumber,
                                     std::string &fileName) {
   Dwarf_Attribute fileDeclAttribute;
   status = dwarf_attr( specEntry(), DW_AT_decl_file, & fileDeclAttribute, NULL );
   DWARF_ERROR_RET(status == DW_DLV_ERROR);

   if (status == DW_DLV_NO_ENTRY) {
      fileName = "";
   }
   else if (status == DW_DLV_OK) {
      Dwarf_Unsigned fileNameDeclVal;
      DWARF_WRAP(dwarf_formudata(fileDeclAttribute, &fileNameDeclVal, NULL));
      dwarf_dealloc( dbg, fileDeclAttribute, DW_DLA_ATTR );			
      fileName = srcFiles()[fileNameDeclVal-1];
   }
   else {
      return true;
   }

   Dwarf_Attribute lineNoAttribute;
   status = dwarf_attr( specEntry, DW_AT_decl_line, & lineNoAttribute, NULL );
   DWARF_ERROR_RET(status == DW_DLV_ERROR);
   if (status != DW_DLV_OK) return true;
   
   /* We don't need to tell Dyninst a line number for C++ static variables,
      so it's OK if there isn't one. */
   hasLineNumber = true;
   DWARF_WRAP(dwarf_formudata( lineNoAttribute, & variableLineNo, NULL ));
   dwarf_dealloc( dbg, lineNoAttribute, DW_DLA_ATTR );			

   return true;
}

bool dwarfWalker::decodeLocationList(Dwarf_Half attr, 
                                     long int *initialStackValue, 
                                     std::vector<VariableLocation> &locs) {
   locs.clear();

   Dwarf_Bool hasLocation = false;
   DWARF_WRAP(dwarf_hasattr( specEntry(), attr, & hasLocation, NULL ));
   if (!hasLocation) return true;
   
   /* Acquire the location of this formal parameter. */
   Dwarf_Attribute locationAttribute;
   DWARF_WRAP(dwarf_attr( specEntry(), attr, & locationAttribute, NULL ));
   
   Dwarf_Locdesc **locationList;
   Dwarf_Signed listLength;
   status = dwarf_loclist_n( locationAttribute, & locationList, & listLength, NULL );

   dwarf_dealloc( dbg, locationAttribute, DW_DLA_ATTR );

   if (status != DW_DLV_OK) return true;
   
   if (!decodeLocationListForStaticOffsetOrAddress( locationList,
                                                    listLength, 
                                                    symtab(), 
                                                    locs, 
                                                    lowaddr(), 
                                                    initialStackValue)) return false;

   deallocateLocationList( dbg, locationList, listLength );
   
   return true;
}

bool dwarfWalker::findSize(unsigned &size) {
   Dwarf_Attribute byteSizeAttr;
   Dwarf_Unsigned byteSize;
   DWARF_WRAP(dwarf_attr( specEntry(), DW_AT_byte_size, & byteSizeAttr, NULL ));

   DWARF_WRAP(dwarf_formudata( byteSizeAttr, & byteSize, NULL ));
   
   dwarf_dealloc( dbg, byteSizeAttr, DW_DLA_ATTR );
   size = (unsigned) byteSize;
   return true;
}

bool dwarfWalker::findVisibility(visibility_t &visibility) {
   /* Acquire the visibility, if any.  DWARF calls it accessibility
      to distinguish it from symbol table visibility. */
   Dwarf_Attribute visAttr;
   int status = dwarf_attr( entry(), DW_AT_accessibility, & visAttr, NULL );
   DWARF_ERROR_RET(status == DW_DLV_ERROR);
   if (status != DW_DLV_OK) {
      visibility = visPrivate;
      return true;
   }

   Dwarf_Unsigned visValue;
   DWARF_WRAP(dwarf_formudata( visAttr, & visValue, NULL ));
   
   switch( visValue ) {
      case DW_ACCESS_public: visibility = visPublic; break;
      case DW_ACCESS_protected: visibility = visProtected; break;
      case DW_ACCESS_private: visibility = visPrivate; break;
      default:
         //bperr ( "Uknown visibility, ignoring.\n" );
         break;
   } /* end visibility switch */
   
   dwarf_dealloc( dbg, visAttr, DW_DLA_ATTR );

   return true;
}

bool dwarfWalker::findValue(long &value, bool &valid) {
   Dwarf_Attribute valueAttr;
   int status = dwarf_attr( entry(), DW_AT_const_value, & valueAttr, NULL );
   DWARF_ERROR_RET(status == DW_DLV_ERROR);
   
   if (status != DW_DLV_OK) {
      valid = false;
      return true;
   }

   Dwarf_Signed enumValue;

   DWARF_WRAP(dwarf_formsdata(valueAttr, &enumValue, NULL));

   value = enumValue;
   valid = true;

   dwarf_dealloc( dbg, valueAttr, DW_DLA_ATTR );
   return true;
}

bool dwarfWalker::fixBitFields(std::vector<VariableLocation> &locs,
                               long &size) {
   Dwarf_Attribute bitOffset;
   int status = dwarf_attr( entry(), DW_AT_bit_offset, & bitOffset, NULL );
   DWARF_ERROR_RET(status == DW_DLV_ERROR);
   
   if ( status == DW_DLV_OK && locs.size() ) 
   {
      Dwarf_Unsigned memberOffset_du = locs[0].frameOffset;
      
      DWARF_WRAP(dwarf_formudata( bitOffset, &memberOffset_du, NULL ));
      
      dwarf_dealloc( dbg(), bitOffset, DW_DLA_ATTR );
      
      Dwarf_Attribute bitSize;
      DWARF_WRAP(dwarf_attr( entry(), DW_AT_bit_size, & bitSize, NULL ));
      
      Dwarf_Unsigned memberSize_du = memberSize;
      DWARF_WRAP(dwarf_formudata( bitSize, &memberSize_du, NULL ));

      dwarf_dealloc( dbg(), bitSize, DW_DLA_ATTR );
      
      /* If the DW_AT_byte_size field exists, there's some padding.
         FIXME?  We ignore padding for now.  (We also don't seem to handle
         bitfields right in getComponents() anyway...) */
   }
   else 
   {
      dwarf_printf("%s[%d]:  got status != OK\n", FILE__, __LINE__);
      if (locs.size())
         locs[0].frameOffset *= 8;
      memberSize *= 8;
   } /* end if not a bit field member. */
   return true;
}

bool dwarfWalker::fixName(std::string &name, Type *type) {
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

void dwarfWalker::removeFortranUnderscore(std::string &name) {
      /* If we're fortran, get rid of the trailing _ */
   if (!curFunc() && !curEnclosure()) return;

   supportedLanguages lang = mod()->language();
   if ((lang != lang_Fortran) &&
       (lang != lang_CMFortran) &&
       (land != lang_Fortran_with_pretty_debug)) return;
   
   if (name[name.length()-1] == '_') {
      name = name.substr(0, name.length()-1);
   }
}

void dwarfWalker::setEntry(Dwarf_Die entry) {
   contexts_.setEntry(entry);
   contexts_.setSpecEntry(entry);
}

Function *dwarfWalker::Contexts::curFunc() {
   assert(!contexts.empty());
   return contexts.top().func;
}

typeCommon *dwarfWalker::Contexts::curCommon() {
   assert(!contexts.empty());
   return contexts.top().commonBlock;
}

typeEnum *dwarfWalker::Contexts::curEnum() {
   assert(!contexts.empty());
   return contexts.top().enumType;
}

fieldListType *dwarfWalker::Contexts::curEnclosure() {
   assert(!contexts.empty());
   return contexts.top().enclosure;
}

void dwarfWalker::Contexts::push() {
   // I believe we want to preserve whatever else was at the
   // "top" until it is overridden. 
   if (contexts.empty()) {
      contexts.push(Context());
   }
   else {
      contexts.push(Context(contexts.top()));
   }
}

void dwarfWalker::Contexts::pop() {
   assert(!contexts.empty());
   contexts.pop();
}

void dwarfWalker::Contexts::setFunc(Function *f) {
   assert(!contexts.empty());
   contexts.top().func = f;
}

void dwarfWalker::Contexts::setCommon(typeCommon *c) {
   assert(!contexts.empty());
   contexts.top().commonBlock = c;
}

void dwarfWalker::Contexts::setEnum(typeEnum *e) {
   assert(!contexts.empty());
   contexts.top.enumType = e;
}

void dwarfWalker::Contexts::setEnclosure(fieldListType *f) {
   assert(!contexts.empty());
   contexts.top.enclosure = f;
}
