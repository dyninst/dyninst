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

#include "serialize.h"
#include "Collections.h"
#include "LineInformation.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

//template <class Dyninst::SymtabAPI::LineInformation *, module_line_info_a, SymtabTranslatorBase>
//DLLEXPORT bool init_anno_serialization(void);
//DLLEXPORT bool init_anno_serialization<Dyninst::SymtabAPI::LineInformation *, module_line_info_a, Dyninst::SymtabAPI::SymtabTranslatorBase>();

bool dummy_for_ser_instance(std::string file, SerializerBase *sb) 
{
   if (file == std::string("no_such_file")) {
      SymtabTranslatorBase *stb = dynamic_cast<SymtabTranslatorBase *>(sb);
      if (!stb) {
         fprintf(stderr, "%s[%d]:  really should not happen\n", FILE__, __LINE__);
         return false;
      }
      bool r = false;
      r = init_anno_serialization<Dyninst::SymtabAPI::localVarCollection, symbol_parameters_a, SymtabTranslatorBase>(stb);
      if (!r) {fprintf(stderr, "%s[%d]:  failed to init anno serialize for symbol_params\n", FILE__, __LINE__);}
      r = false;
      r = init_anno_serialization<Dyninst::SymtabAPI::localVarCollection, symbol_variables_a, SymtabTranslatorBase>(stb);
      if (!r) {fprintf(stderr, "%s[%d]:  failed to init anno serialize for symbol_vars\n", FILE__, __LINE__);}
      r = false;
      r = init_anno_serialization<Dyninst::SymtabAPI::LineInformation *, module_line_info_a, Dyninst::SymtabAPI::SymtabTranslatorBase>(stb);
      if (!r) {fprintf(stderr, "%s[%d]:  failed to init anno serialize for module_line_info\n", FILE__, __LINE__);}
      r = false;
      r = init_anno_serialization<Dyninst::SymtabAPI::typeCollection *, module_type_info_a, Dyninst::SymtabAPI::SymtabTranslatorBase>(stb);
      if (!r) {fprintf(stderr, "%s[%d]:  failed to init anno serialize for module_type_info\n", FILE__, __LINE__);}
      r = false;
   }
   return true;
}

bool init_annotation_types(SerializerBase *sb) 
{
   assert(sb);
   fprintf(stderr, "%s[%d]:  welcome to init_annotation_types...  serializer = %p\n", FILE__, __LINE__, sb);
   bool ret = true;
   int anno_id = -1;
   std::string anno_name;

   anno_name = typeid(user_funcs_a).name();
   if (-1 == (anno_id = AnnotatableBase::createAnnotationType(anno_name, NULL, sb))) {
      fprintf(stderr, "%s[%d]:  Failed to init annotation type %s\n", FILE__, __LINE__, 
            anno_name.c_str());
      ret = false;
   }
   anno_name = typeid(user_symbols_a).name();
   if (-1 == (anno_id = AnnotatableBase::createAnnotationType(anno_name, NULL, sb))) {
      fprintf(stderr, "%s[%d]:  Failed to init annotation type %s\n", FILE__, __LINE__, 
            anno_name.c_str());
      ret = false;
   }
   anno_name = typeid(module_type_info_a).name();
   if (-1 == (anno_id = AnnotatableBase::createAnnotationType(anno_name, NULL, sb))) {
      fprintf(stderr, "%s[%d]:  Failed to init annotation type %s\n", FILE__, __LINE__, 
            anno_name.c_str());
      ret = false;
   }
   anno_name = typeid(module_line_info_a).name();
   if (-1 == (anno_id = AnnotatableBase::createAnnotationType(anno_name, NULL, sb))) {
      fprintf(stderr, "%s[%d]:  Failed to init annotation type %s\n", FILE__, __LINE__, 
            anno_name.c_str());
      ret = false;
   }
   anno_name = typeid(symbol_variables_a).name();
   if (-1 == (anno_id = AnnotatableBase::createAnnotationType(anno_name, NULL, sb))) {
      fprintf(stderr, "%s[%d]:  Failed to init annotation type %s\n", FILE__, __LINE__, 
            anno_name.c_str());
      ret = false;
   }
   anno_name = typeid(symbol_parameters_a).name();
   if (-1 == (anno_id = AnnotatableBase::createAnnotationType(anno_name, NULL, sb))) {
      fprintf(stderr, "%s[%d]:  Failed to init annotation type %s\n", FILE__, __LINE__, 
            anno_name.c_str());
      ret = false;
   }
   return ret;
}

SymtabTranslatorBin::SymtabTranslatorBin(Symtab *st, string file, bool verbose) :
       SymtabTranslatorBase(st, file, sd_serialize, verbose),
#if 0
       sf(file, sd_serialize, verbose),
#endif
       default_module(NULL)
{
   fprintf(stderr, "%s[%d]:  welcome to SymtabTranslatorBin ctor: serializer = %p\n", 
         FILE__, __LINE__, getSF().getSD());
   init_annotation_types(this);
   dummy_for_ser_instance(file, this);
}

SymtabTranslatorBin::SymtabTranslatorBin(Symtab *st, string file, iomode_t iomode,
      bool verbose) :
       SymtabTranslatorBase(st, file, iomode, verbose),
#if 0
       sf(file, iomode, verbose),
#endif
       default_module(NULL)
{
   fprintf(stderr, "%s[%d]:  welcome to SymtabTranslatorBin ctor: serializer %p\n", 
         FILE__, __LINE__, getSF().getSD());
   init_annotation_types(this);
   dummy_for_ser_instance(file, this);
}

SymtabTranslatorBin::~SymtabTranslatorBin()
{
}

SymtabTranslatorBin *SymtabTranslatorBin::getTranslator(Symtab *st, std::string file, iomode_t iomode, bool verbose)
{
   SerializerBase *sb = SerializerBase::getSerializer(std::string("SymtabTranslatorBin"), file);
   if (!sb) {
      SymtabTranslatorBin *stb = new SymtabTranslatorBin(st, file, iomode, verbose);
      bool ok = SerializerBase::addSerializer(std::string("SymtabTranslatorBin"), file, stb);
      if (!ok) {
         fprintf(stderr, "%s[%d]:  failed to add new serializer\n", FILE__, __LINE__);
         delete stb;
         return NULL;
      }
      return stb;
   }
   else {
      //  sanity checks necessary?
      SymtabTranslatorBin *stb = dynamic_cast<SymtabTranslatorBin *>(sb);
      if (!stb) {
         fprintf(stderr, "%s[%d]:  invalid serializer pointer\n", FILE__, __LINE__);
         return NULL;
      }
      if (stb->iomode() != iomode) {
         fprintf(stderr, "%s[%d]:  invalid serializer iomode\n", FILE__, __LINE__);
      }
      return stb;
   }
   return NULL;
}

void SymtabTranslatorBin::translate(Symbol::SymbolType &param, const char *tag) 
{
  size_t sz = sizeof(Symbol::SymbolType);
  if (sz == sizeof(int)) {
      int &conv = (int &) param;
      getSD().translate(conv,tag); 
  } else if (sz == sizeof(char)) {
      char &conv = (char &) param;
      getSD().translate(conv,tag); 
  }
  else {
    assert(0 && "bad size for enum!!");
    //SER_ERR("bad size for enum!!!");
  }
}

void SymtabTranslatorBin::translate(Symbol::SymbolLinkage &param, const char *tag) 
{
  size_t sz = sizeof(Symbol::SymbolType);
  if (sz == sizeof(int)) {
      int &conv = (int &) param;
      getSD().translate(conv,tag); 
  } else if (sz == sizeof(char)) {
      char &conv = (char &) param;
      getSD().translate(conv,tag); 
  }
  else {
    assert(0 && "bad size for enum!!");
    //SER_ERR("bad size for enum!!!");
  }
}

void SymtabTranslatorBin::translate(Symbol::SymbolTag &param, const char * tag) 
{
  size_t sz = sizeof(Symbol::SymbolType);
  if (sz == sizeof(int)) {
      int &conv = (int &) param;
      getSD().translate(conv, tag); 
  } else if (sz == sizeof(char)) {
      char & conv = (char &) param;
      getSD().translate(conv,tag); 
  }
  else {
    assert(0 && "bad size for enum!!");
    //SER_ERR("bad size for enum!!!");
  }
}

void SymtabTranslatorBin::translate(supportedLanguages &param, const char *tag) 
{
  if (sizeof(supportedLanguages) == sizeof(int)) {
      int &conv = (int &) param;
      getSD().translate(conv, tag); 
  } else if (sizeof(supportedLanguages) == sizeof(char)) {
      char &conv = (char &) param;
      getSD().translate(conv,tag); 
  }
  else {
    assert(sizeof(supportedLanguages) == sizeof(char));
    //SER_ERR("bad size for enum!!!");
  }
}

void SymtabTranslatorBin::symtab_start(Symtab &param, const char *) 
{
#if 0
  sd.translate(param.name_);
#endif
//  sd.translate(param.codeValidStart_);
//  sd.translate(param.codeValidEnd_);
//  sd.translate(param.dataValidStart_);
//  sd.translate(param.dataValidEnd_);
  getSD().translate(param.main_call_addr_);
  getSD().translate(param.nativeCompiler);
  getSD().translate(param.no_of_symbols);
} 

void SymtabTranslatorBin::symtab_end(Symtab &param, const char *) 
{
  //  auxilliary accounting that does not generalize to the xml output case
  //  read extra data, rebuild indexes and hashes.

  translate_notype_syms(param);
  if (getSD().iomode() == sd_deserialize) {
     rebuild_section_hash(param);
     fprintf(stderr, "%s[%d]:  rebuilding symbol indexes for %s\n", FILE__, __LINE__, param.file().c_str());
     rebuild_symbol_indexes(param);
  }
}

void SymtabTranslatorBin::translate_notype_syms(Symtab &param)
{
  translate_vector<SymtabTranslatorBase, Symbol>(this, param.notypeSyms);
}

void SymtabTranslatorBin::symbol_end(Symbol &param, const char *)
{
   Module *mod = param.module_;
   Region *sec = param.sec_;
   Offset mod_off = 0;
   Offset region_entry = 0;

   if (sec)
      region_entry = sec->getRegionAddr();

   if (mod)
      mod_off = mod->addr();

   getSD().translate(region_entry);
   getSD().translate(mod_off);

   if (getSD().iomode() == sd_deserialize) {
#if 0
      if (!default_module) {
         default_module = parent_symtab->newModule(std::string("DEFAULT_MODULE"), 0, lang_Unknown);
      }
#endif
     //  lookup section and module by number and offset, respectively
      if (mod_off)
         param.module_ = parent_symtab->findModuleByOffset(mod_off);
      else {
         std::string dmname("DEFAULT_MODULE");
         if (!parent_symtab->findModule(default_module, dmname)) {
            fprintf(stderr, "%s[%d]:  WARNING:  no module found for symbol %s, offset %lx\n", FILE__, __LINE__, param.prettyNames[0].c_str(), mod_off);
            param.module_ = NULL;
         }
         else
            param.module_ = default_module;
      }
      if (region_entry) {
         //  findSectionByIndex just iterates thru the vector of sections,
         //  this is actually a good thing, since we haven't rebuilt the
         //  section<->offset map yet
         if (!parent_symtab->findRegionByEntry(param.sec_, region_entry)) {
            fprintf(stderr, "%s[%d]:  WARNING:  cannot find region with offset %p\n", FILE__, __LINE__, (void *)region_entry);
         }
      } else {
         fprintf(stderr, "%s[%d]:  WARNING:  no section offset for section %p\n", FILE__, __LINE__, (void *) region_entry);
         param.sec_ = NULL;
      }
   }
}

void SymtabTranslatorBin::rebuild_symbol_indexes(Symtab &param)
{
    int counter = 0;
    vector<Symbol *> &unique_funcs = param.everyUniqueFunction;

    for (unsigned int i = 0; i < unique_funcs.size(); ++i) {
#if 0 // SERIALIZE
      Symbol *f = unique_funcs[i];
      if (f->wasCreated()) {
         param.createdFunctions.push_back(f);
      }
#endif
#if 0
      if (f->isExported()) {
         param.exportedFunctions.push_back(f);
      }
#endif
    }

#if 0 // SERIALIZE
    fprintf(stderr, "%s[%d]:  restored symtab has %d created %d exported\n", 
            FILE__, __LINE__, param.createdFunctions.size(), param.exportedFunctions.size());
#endif
    for (unsigned int i = 0; i < unique_funcs.size(); ++i) {
      Symbol *f = unique_funcs[i];
      param.funcsByEntryAddr[f->getAddr()].push_back(f);
      if (f->getAddr() == 0x8050c78) 
         counter++;
    }

    fprintf(stderr, "%s[%d]:  %s %d symbols for address 0x8050c78\n", 
            FILE__, __LINE__, (getSD().iomode() == sd_serialize) ? "saved" : "restored", counter);

    
    for (unsigned int i = 0; i < unique_funcs.size(); ++i) {
      Symbol *f = unique_funcs[i];
      const std::vector<string> &fpnames = f->getAllPrettyNames();
      const std::vector<string> &fmnames = f->getAllMangledNames();
      //const std::vector<string> &ftnames = f->getAllTypedNames();

      //  add to funcsByPretty hash
      for (unsigned int j = 0; j < fpnames.size(); ++j) {
        dyn_hash_map<string, std::vector<Symbol *> *> &pnhash = param.funcsByPretty;
        std::vector<Symbol *> *namevecp = NULL;
        if (pnhash.find(fpnames[j]) == pnhash.end()) {
           namevecp = new std::vector <Symbol *>;
           pnhash[fpnames[j]] = namevecp;
        }
        else
           namevecp  = pnhash[fpnames[j]]; 
        namevecp->push_back(f);
      }

      //  add to funcsByMangled hash
      for (unsigned int j = 0; j < fmnames.size(); ++j) {
        dyn_hash_map<string, std::vector<Symbol *> *> &mnhash = param.funcsByMangled;
        std::vector<Symbol *> *namevecp = NULL;
        if (mnhash.find(fmnames[j]) == mnhash.end()) {
           namevecp = new std::vector <Symbol *>;
           mnhash[fmnames[j]] = namevecp;
        }
        else
           namevecp  = mnhash[fmnames[j]]; 
        namevecp->push_back(f);
      }

    }

    //  Now Variable hashes
    for (unsigned int i = 0; i < param.everyUniqueVariable.size(); ++i) {
       Symbol *v = param.everyUniqueVariable[i];
       assert(v);
       const std::vector<string> &vpnames = v->getAllPrettyNames();
       const std::vector<string> &vmnames = v->getAllMangledNames();

      //  add to varsByPretty
      for (unsigned int j = 0; j < vpnames.size(); ++j) {
        dyn_hash_map<string, std::vector<Symbol *> *> &pnhash = param.varsByPretty;
        std::vector<Symbol *> *namevecp = NULL;
        if (pnhash.find(vpnames[j]) == pnhash.end()) {
           namevecp = new std::vector <Symbol *>;
           pnhash[vpnames[j]] = namevecp;
        }
        else
           namevecp  = pnhash[vmnames[j]]; 

        bool found = false;
        std::vector<Symbol *> &namevec = *namevecp;
        for (unsigned int k = 0; k <namevec.size(); ++k) {
           assert(namevec[k]);
           if ((v == namevec[k]) || (*v == *(namevec[k]))) {
              found = true;
              break;
           }
        }
        if (!found) {
           namevecp->push_back(v);
#if 0
           if (namevecp->size() > 1) {
              fprintf(stderr, "%s[%d]:  have %d symbols matching name %s\n", FILE__, __LINE__, namevecp->size(), v->getName().c_str());
              for (unsigned int k = 0; k < namevecp->size(); ++k) {
                  cerr << *namevec[k] << endl;
              }
           }
#endif
        }
        else {
           fprintf(stderr, "%s[%d]:  FIXME:  dropping duplicate symbol:\n\t", FILE__, __LINE__);
           cerr << *v << endl;
        }
      }

      //  add to varsByMangled
      for (unsigned int j = 0; j < vmnames.size(); ++j) {
        dyn_hash_map<string, std::vector<Symbol *> *> &mnhash = param.varsByMangled;
        std::vector<Symbol *> *namevecp = NULL;
        if (mnhash.find(vmnames[j]) == mnhash.end()) {
           namevecp = new std::vector <Symbol *>;
           mnhash[vmnames[j]] = namevecp;
        }
        else
           namevecp  = mnhash[vmnames[j]]; 

        bool found = false;
        std::vector<Symbol *> &namevec = *namevecp;
        for (unsigned int k = 0; k <namevec.size(); ++k) {
           assert(namevec[k]);
           if ((v == namevec[k]) || (*v == *(namevec[k]))) {
              found = true;
              break;
           }
        }
        if (!found)
           namevecp->push_back(v);
        else {
           fprintf(stderr, "%s[%d]:  FIXME:  dropping duplicate symbol:\n\t", FILE__, __LINE__);
           cerr << *v << endl;
        }

        //namevecp->push_back(v);
      }

      //  add to varsByAddr
      dyn_hash_map<Offset, Symbol *>  &vba = param.varsByAddr;
      if (vba.find(v->getAddr()) == vba.end())
         vba[v->getAddr()] = v;
      else {
         fprintf(stderr, "%s[%d]:  collision! error restoring variable hash:\n", FILE__, __LINE__ ); 
         cerr << *v << endl;
         cerr << *(vba[v->getAddr()]) << endl;
      }
    }

    //  rebuild modsByName

    for (unsigned int i = 0; i < param.modSyms.size(); ++i) {
       Symbol *m = param.modSyms[i];
       assert(m);
       const std::vector<string> &mpnames = m->getAllPrettyNames();

       for (unsigned int j = 0; j < mpnames.size(); ++j) {
          dyn_hash_map<string, vector<Symbol *> *>  &mba = param.modsByName;
          std::vector<Symbol *> *namevecp = NULL;
          if (mba.find(mpnames[j]) == mba.end()) {
             namevecp = new std::vector <Symbol *>;
             mba[mpnames[j]] = namevecp;
          }
          else
             namevecp  = mba[mpnames[j]]; 
          namevecp->push_back(m);
       }
    }
}

void SymtabTranslatorBin::rebuild_section_hash(Symtab &/*param*/)
{
#if 0
   vector<Section *> &sections_ = param.sections_;
   for (unsigned index=0;index<sections_.size();index++)
      param.secsByEntryAddr[sections_[index]->getSecAddr()] = sections_[index];

   param.no_of_sections = sections_.size();
#endif
   fprintf(stderr, "%s[%d]:  WARNING:  rebuild section hash needs to be updated\n", FILE__, __LINE__);
}

void bogus_func()
{
   assert(0);
   //SerDes *sd_bin = NULL;
   //localVarCollection *lvC;
   //sd_translate(lvC, sd_bin);
}
