/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __ST_SERIALIZE__H__
#define __ST_SERIALIZE__H__

#if defined(os_windows)
#include <libxml/xmlversion.h>
#undef LIBXML_ICONV_ENABLED
#endif

#include <libxml/xmlwriter.h>
#include "common/h/headers.h"
#include "common/h/serialize.h"
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/LineInformation.h"
#include "symtabAPI/h/RangeLookup.h"
#include "symtabAPI/src/Collections.h"

namespace Dyninst {
namespace SymtabAPI {
class SymtabTranslatorBase;

bool deserialize(Symtab &, SymtabTranslatorBase &);
bool serialize(Symtab &, SymtabTranslatorBase &);

class ExceptionBlock;
class relocationEntry;

class SymtabTranslatorBase : public SerializerBase {
   public:
      virtual void vector_start(unsigned int &size, const char *tag = NULL) {
         getSD().vector_start(size, tag);
      }

      virtual void vector_end() {
         getSD().vector_end();
      }

      virtual void hash_map_start(unsigned int &size, const char *tag = NULL) {
         getSD().hash_map_start(size, tag);
      }

      virtual void hash_map_end() {
         getSD().hash_map_end();
      }

      virtual void translate_base(int &v, const char *t){
         getSD().translate(v, t);
      }

      virtual void translate_base(std::string &v, const char *t){
         getSD().translate(v, t);
      }

      iomode_t iomode() {return getSD().iomode();}

      virtual void annotation_start(const char * /*string_id*/, const char * /*tag*/ = NULL) {}
      virtual void annotation_end() {}
   protected:
      Symtab *parent_symtab;

#if 0
      virtual SerDes &getSD() = 0;
#endif
   private:

      virtual void symbol_start(Symbol &, const char * = NULL) {}
      virtual void symbol_end(Symbol &, const char * = NULL) {}
      virtual void symtab_start(Symtab &, const char * = NULL) {}
      virtual void symtab_end(Symtab &, const char * = NULL) {}
      virtual void region_end(Region &, const char * = NULL) {}
      virtual void region_start(Region &, const char * = NULL) {}
      virtual void module_start(Module &, const char * = NULL) {}
      virtual void module_end(Module &, const char * = NULL) {}
      virtual void exception_start(ExceptionBlock &, const char * = NULL) {}
      virtual void exception_end(ExceptionBlock &, const char * = NULL){}
      virtual void relocation_start(relocationEntry &, const char * = NULL) {}
      virtual void relocation_end(relocationEntry &, const char * = NULL) {}
      virtual void line_information_start(LineInformation &, const char * = NULL) {}
      virtual void line_information_end(LineInformation &, const char * = NULL) {}
      virtual void type_collection_start(typeCollection &, const char * = NULL) {}
      virtual void type_collection_end(typeCollection &, const char * = NULL) {}
      virtual void local_var_collection_start(localVarCollection &, const char * = NULL) {}
      virtual void local_var_collection_end(localVarCollection &, const char * = NULL) {}

   public:
      SymtabTranslatorBase(Symtab *parent_symtab_, std::string fname, iomode_t dir, bool verbose = false) :
         SerializerBase("SymtabTranslator", fname, dir, verbose),
         parent_symtab(parent_symtab_)
      {}
#if 0
         SymtabTranslatorBase(Symtab *parent_symtab_, ) :
         SerializerBase("SymtabTranslator"),
         parent_symtab(parent_symtab_)
      {}
#endif
      virtual ~SymtabTranslatorBase() {}

      virtual bool translate_annotation(void * /*anno*/, const char * /*name*/) 
      {
#if 0
         annotation_start(name);
         annotation_end();
#endif
         return true;
      }

      virtual void translate(Symbol::SymbolType &param, const char *tag = NULL) = 0;
      virtual void translate(Symbol::SymbolLinkage &param, const char *tag = NULL) = 0;
      virtual void translate(Symbol::SymbolTag &param, const char *tag = NULL) = 0;
      virtual void translate(supportedLanguages & param, const char *tag = NULL) = 0;

      virtual void translate_base(std::vector<std::string> &param, const char * tag= NULL) {
         try {
            //type_start(param);
            //type_end(param);
            getSD().translate(param, tag);
         } SER_CATCH("vector<string>")
      }
      virtual void translate_base(Type &param, const char * = NULL) {
         try {
            //type_start(param);
            //type_end(param);
         } SER_CATCH("Type")
      }
      virtual void translate_base(Type *param, const char * tag= NULL) {
         assert(param);
         try {
            translate_base(*param, tag);
         } SER_CATCH("Type")
      }

      virtual void translate_base(localVar &param, const char * = NULL) {
         try {
            //type_start(param);
            //type_end(param);
         } SER_CATCH("localVar")
      }

      virtual void translate_base(LineInformation &param, const char * = NULL) {
         try {
            line_information_start(param);
#if 0
            translate_hash_set<SymtabTranslatorBase, std::string>(this, param.sourceLineNames, "sourceLineNames");
#endif
            line_information_end(param);
         } SER_CATCH("LineInformation")
      }

      virtual void translate_base(typeCollection &param, const char * = NULL) {
         try {
            type_collection_start(param);
            fprintf(stderr, "%s[%d]:  WARNING:  gutted type collection serialization\n", FILE__, __LINE__);
#if 0
            translate_hash_map<SymtabTranslatorBase, int, Type>(this, param.typesByID, "typesByID");
            translate_hash_map<SymtabTranslatorBase, std::string, Type>(this, param.globalVarsByName, "globalVarsByName");
            getSD().translate(param.dwarfParsed_, "dwarfParsed");
#endif
            type_collection_end(param);
         } SER_CATCH("typeCollection")
      }

      virtual void translate_base(localVarCollection &param, const char * = NULL) {
         try {
          //  local_var_collection_start(param);
         //   translate_vector<SymtabTranslatorBase, localVar>(this, param.localVars, "localVars");
           // local_var_collection_end(param);
         } SER_CATCH("localVarCollection")
      }

      virtual void translate_base(Symbol &param, const char * = NULL) {
         try {
            symbol_start(param);
            translate(param.type_, "type");
            translate(param.linkage_, "linkage");
            translate(param.tag_, "tag");
            getSD().translate(param.addr_, "addr");
            getSD().translate(param.size_, "size");
            getSD().translate(param.isInDynsymtab_, "isInDynsymtab");
            getSD().translate(param.isInSymtab_, "isInSymtab");
            getSD().translate(param.prettyNames, "prettyNames", "prettyName");
            getSD().translate(param.mangledNames, "mangledNames", "mangledName");
            getSD().translate(param.typedNames, "typedNames", "typedName");
            getSD().translate(param.framePtrRegNum_, "framePtrRegNum");
            //  Note:  have to deal with retType_ here?? Probably use type id.
            getSD().translate(param.moduleName_, "moduleName");
            symbol_end(param);
         } SER_CATCH("Symbol")
      }

      virtual void translate_base(Symtab &param, const char * = NULL) {
         try {
            symtab_start(param);
#if 0
            getSD().translate(param.filename_, "file");

            fprintf(stderr, "%s[%d]:  %sserialize: file = %s\n", FILE__, __LINE__, 
                  getSD().iomode() == sd_serialize ? "" : "de", 
                  param.filename_.c_str());
#endif

#if 0
            getSD().translate(param.codeOffset_, "codeOff");
            getSD().translate(param.codePtrOffset_, "codePtrOff");
            getSD().translate(param.codeLen_, "codeLen");
            fprintf(stderr, "%s[%d]:  %sserialize: codeLen = %d\n", FILE__, __LINE__, 
                  getSD().iomode() == sd_serialize ? "" : "de", 
                  param.codeLen_);
#endif
            //getSD().translate(param.codePtrOffset_, "codePtrOff");
            getSD().translate(param.imageOffset_, "imageOffset");
            getSD().translate(param.imageLen_, "imageLen");
            getSD().translate(param.dataOffset_, "dataOff");
            //getSD().translate(param.dataPtrOffset_, "dataPtrOff");
            getSD().translate(param.dataLen_, "dataLen");
            getSD().translate(param.is_a_out, "isExec");

            fprintf(stderr, "%s[%d]:  %sserializing %d modules\n", FILE__, __LINE__, getSD().iomode() == sd_serialize ? "" : "de", param._mods.size());
            translate_vector<SymtabTranslatorBase, Module>(this, param._mods, 
                  "Modules");
            fprintf(stderr, "%s[%d]:  %sserialized %d modules\n", FILE__, __LINE__, getSD().iomode() == sd_serialize ? "" : "de", param._mods.size());
            //translate_vector<SymtabTranslatorBase, Section>(this, param.sections_, 
             //     "Sections");
            translate_vector<SymtabTranslatorBase, Symbol>(this, param.everyUniqueFunction, 
                  "EveryUniqueFunction");
            translate_vector<SymtabTranslatorBase, Symbol>(this, param.everyUniqueVariable, 
                  "EveryUniqueVariable");
            translate_vector<SymtabTranslatorBase, Symbol>(this, param.modSyms, 
                  "ModuleSymbols");
            translate_vector<SymtabTranslatorBase, ExceptionBlock>(this, param.excpBlocks, 
                  "ExceptionBlocks");
            //translate_vector<SymtabTranslatorBase, relocationEntry>(this, param.relocation_table, 
            //      "RelocationEntries");

            symtab_end(param);
         } SER_CATCH("Symtab")
      }

      virtual void translate_base(Region &param, const char * = NULL) {
         try {
            region_start(param);
            fprintf(stderr, "%s[%D]:  IMPLEMENT ME\n", FILE__, __LINE__);
#if 0
            getSD().translate(param.sidnumber_, "id");
            getSD().translate(param.sname_, "name");
            getSD().translate(param.saddr_, "addr");
            getSD().translate(param.ssize_, "size");
#if 0
            //  I think this was obsoleted??
            getSD().translate(param.rawDataOffset_, "offset");
#endif
            getSD().translate(param.sflags_, "flags");
#endif
            region_end(param);
         } SER_CATCH("Section")
      }

      virtual void translate_base(Module &param, const char * = NULL) {
         try {
            module_start(param);
            getSD().translate(param.fileName_, "fileName");
            getSD().translate(param.fullName_, "fullName");
            getSD().translate(param.addr_, "Address");
            translate(param.language_, "Language");
            if (getSD().iomode() == sd_deserialize)
               param.exec_ = parent_symtab;
            module_end(param);
         } SER_CATCH("Module")
      }

      virtual void translate_base(ExceptionBlock &param, const char * = NULL) {
         try {
            exception_start(param);
            getSD().translate(param.tryStart_, "tryAddr");
            getSD().translate(param.trySize_, "trySize");
            getSD().translate(param.catchStart_, "catchAddr");
            getSD().translate(param.hasTry_, "hasTry");
            exception_end(param);
         }SER_CATCH("ExceptionBlock")
      }

      virtual void translate_base(relocationEntry &param, const char * = NULL) {
         try {
            relocation_start(param);
            getSD().translate(param.target_addr_, "targetAddr");
            getSD().translate(param.rel_addr_, "relocationAddr");
            getSD().translate(param.addend_, "addend");
            //getSD().translate(param.rtype_, "regionType");
            getSD().translate(param.name_, "name");
            relocation_end(param);
         } SER_CATCH("relocationEntry")
      }
};

class SymtabTranslatorXML : public SymtabTranslatorBase {
   //SerFile sf;
   xmlTextWriterPtr writer;
   public:
#if 0
   SerDesXML &getSD() {
      SerDes *sd = getSF().getSD();
      if (!sd) assert(0);
      SerDesXML *sdx = dynamic_cast<SerDesXML *> (sd);
      if (!sdx) assert(0);
      return *sdx;
   }
#endif
   SerDesXML &getSDXML() {
      SerDes *sd = getSF().getSD();
      SerDesXML *sdx = dynamic_cast<SerDesXML *> (sd);
      if (!sdx) assert(0);
      return *sdx;
   }

   SymtabTranslatorXML(Symtab *st, string file);
   virtual ~SymtabTranslatorXML() {}

   private:
   virtual void symtab_start(Symtab &, const char * = NULL);
   virtual void symtab_end(Symtab &, const char * = NULL); 
   virtual void symbol_start(Symbol &, const char * = NULL);
   virtual void symbol_end(Symbol &, const char * = NULL);
   virtual void exception_start(ExceptionBlock &, const char * = NULL);
   virtual void exception_end(ExceptionBlock &, const char * = NULL);
   virtual void relocation_start(relocationEntry &, const char * = NULL);
   virtual void relocation_end(relocationEntry &, const char * = NULL);
   virtual void module_start(Module &, const char * = NULL);
   virtual void module_end(Module &, const char * = NULL);
#if 0
   virtual void region_start(Section &, const char * = NULL);
   virtual void region_end(Section &, const char * = NULL);
#endif
   virtual void line_information_start(LineInformation &, const char * = NULL);
   virtual void line_information_end(LineInformation &, const char * = NULL);
   virtual void type_collection_start(typeCollection &, const char * = NULL);
   virtual void type_collection_end(typeCollection &, const char * = NULL);

   virtual void translate(Symbol::SymbolType &param, const char *tag = NULL);
   virtual void translate(Symbol::SymbolLinkage &param, const char *tag = NULL);
   virtual void translate(Symbol::SymbolTag &param, const char *tag = NULL);
   virtual void translate(supportedLanguages &param, const char *tag = NULL);
};

class SymtabTranslatorBin : public SymtabTranslatorBase {
   //SerFile sf;

   //  catch-all for modules without a home
   //  (they should all have offset=NULL
   Module *default_module;

   SymtabTranslatorBin(Symtab *st, string file, bool verbose = false); 
   SymtabTranslatorBin(Symtab *st, string file, iomode_t iomode, bool verbose = false); 

   public:
#if 0
   SerDes &getSD() {
      SerDes *sd = getSF().getSD();
      if (!sd) assert(0);
      return *sd;
   }
#endif
   virtual ~SymtabTranslatorBin();

   static SymtabTranslatorBin *getTranslator(Symtab *st, string file, iomode_t iomode = sd_serialize, bool verbose = false);
   private:
   virtual void translate(Symbol::SymbolType &param, const char *tag = NULL); 
   virtual void translate(Symbol::SymbolLinkage &param, const char *tag = NULL); 
   virtual void translate(Symbol::SymbolTag &param, const char *tag = NULL); 
   virtual void translate(supportedLanguages &param, const char *tag = NULL); 

   virtual void symtab_start(Symtab &param, const char * = NULL);
   virtual void symtab_end(Symtab &param, const char * = NULL);
   virtual void symbol_end(Symbol &param, const char * = NULL); 

#if 0
   virtual void line_information_start(LineInformation &, const char * = NULL);
   virtual void line_information_end(LineInformation &, const char * = NULL);
   virtual void type_collection_start(typeCollection &, const char * = NULL);
   virtual void type_collection_end(typeCollection &, const char * = NULL);
#endif

   //  Convenience functions called by symtab_end()...
   //  For deserialization, once we have created all the basic arrays
   //  of objects, we then need to do some extra work to re-create
   //  hashes, indexes, auxilliary data, etc.  That's what these do.

   void rebuild_section_hash(Symtab &param);
   void rebuild_symbol_indexes(Symtab &param);
   void translate_notype_syms(Symtab &param);

};

template<class S, class Value, class ValueLess>
void translate_range_lookup(S *ser, RangeLookup<Value, ValueLess> &rl, 
      const char *tag, const char *key_tag, const char *value_tag)
{
   ser->range_lookup_start(tag);
   translate_multimap(rl.valuesByAddressRangeMap, "ValuesByAddress", "Address", "Value");
   translate_multimap(rl.addressRangesByValueMap, "AddressRangesByValue", "Value", "AddressRange");
   ser->range_lookup_end();
}

} // namespace symtabAPI
} // namespace Dyninst
#endif
