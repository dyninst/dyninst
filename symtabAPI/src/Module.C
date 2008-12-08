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
 * License as published by the Free Software Foundation; either * version 2.1 of the License, or (at your option) any later version.
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

#include "Annotatable.h"
#include "Module.h"
#include "Symtab.h"
#include "Collections.h"

#include "common/h/pathName.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

static AnnotationClass<LineInformation> ModuleLineInfoAnno(std::string("ModuleLineInfoAnno"));
static AnnotationClass<typeCollection> ModuleTypeInfoAnno(std::string("ModuleTypeInfoAnno"));
static SymtabError serr;

bool Module::findSymbolByType(std::vector<Symbol *> &found, 
      const std::string name,
      Symbol::SymbolType sType, 
      bool isMangled,
      bool isRegex, 
      bool checkCase)
{
   unsigned orig_size = found.size();
   std::vector<Symbol *> obj_syms;

   if (!exec()->findSymbolByType(obj_syms, name, sType, isMangled, isRegex, checkCase))
   {
      //fprintf(stderr, "%s[%d]:  no symbols matching %s found\n", FILE__, __LINE__, name.c_str());
      return false;
   }

   for (unsigned i = 0; i < obj_syms.size(); i++)  
   {
      if (obj_syms[i]->getModule() == this)
         found.push_back(obj_syms[i]);
#if 0
      else 
      {
         if (obj_syms[i]->getName() == name) {
            fprintf(stderr, "%s[%d]:  found symbol %s in different module %s not %s\n", 
                  FILE__, __LINE__, name.c_str(), 
                  obj_syms[i]->getModule()->fileName().c_str(), 
                  this->fileName().c_str());
         }
      }
#endif
   }

   if (found.size() > orig_size) 
      return true;

   return false;        
}

const std::string &Module::fileName() const
{
   return fileName_;
}

const std::string &Module::fullName() const
{
   return fullName_;
}

 Symtab *Module::exec() const
{
   return exec_;
}

supportedLanguages Module::language() const
{
   return language_;
}

bool Module::hasLineInformation()
{
   LineInformation *li =  NULL;
   if (getAnnotation(li, ModuleLineInfoAnno)) 
   {
      if (!li) 
      {
         fprintf(stderr, "%s[%d]:  weird inconsistency with getAnnotation here\n", 
               FILE__, __LINE__);
         return false;
      }

      if (li->getSize())
      {
         return true;
      }
   }

   return false;
#if 0
   Annotatable<LineInformation *, module_line_info_a,  true> &liA = *this;
   return ( 0 != liA.size());
#endif
}

LineInformation *Module::getLineInformation()
{
   if (!exec_->isLineInfoValid_)
      exec_->parseLineInformation();

   if (!exec_->isLineInfoValid_) 
   {
      return NULL;
   }

   LineInformation *li =  NULL;
   if (getAnnotation(li, ModuleLineInfoAnno)) 
   {
      if (!li) 
      {
         fprintf(stderr, "%s[%d]:  weird inconsistency with getAnnotation here\n", 
               FILE__, __LINE__);
         return NULL;
      }

      if (!li->getSize())
      {
         return NULL;
      }
   }

   return li;

#if 0
   Annotatable<LineInformation *, module_line_info_a,  true> &mt = *this;

   if (exec_->isLineInfoValid_) {
      if (!mt.size()) {
          // There is no line info valid for this module
         return NULL;
      }
      if (mt.size() > 1) {
        // fprintf(stderr, "%s[%d]:  weird, multiple line info for %s: FIXME\n", 
        //       FILE__, __LINE__, fileName_.c_str());
      }
      if (!mt[0]) {
         fprintf(stderr, "%s[%d]:  FIXME:  Line info annotation is NULL!\n", FILE__, __LINE__);
      }
      return mt[0];
   }
#endif

   fprintf(stderr, "%s[%d]:  FIXME:  line info not valid after parse\n",
         FILE__, __LINE__);

   return NULL;
}

bool Module::getAddressRanges(std::vector<pair<Offset, Offset> >&ranges,
      std::string lineSource, unsigned int lineNo)
{
   unsigned int originalSize = ranges.size();

   LineInformation *lineInformation = getLineInformation();
   if (lineInformation)
      lineInformation->getAddressRanges( lineSource.c_str(), lineNo, ranges );

   if ( ranges.size() != originalSize )
      return true;

   return false;
}

bool Module::getSourceLines(std::vector<LineNoTuple> &lines, Offset addressInRange)
{
   unsigned int originalSize = lines.size();

   LineInformation *lineInformation = getLineInformation();
   if (lineInformation)
      lineInformation->getSourceLines( addressInRange, lines );

   if ( lines.size() != originalSize )
      return true;

   return false;
}

vector<Type *> *Module::getAllTypes()
{
   typeCollection *tc = NULL;
   if (!getAnnotation(tc, ModuleTypeInfoAnno))
   {
#if 0
      tc = new typeCollection();
      if (!addAnnotation(tc, ModuleTypeInfoAnno))
      {
         fprintf(stderr, "%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
         return NULL;
      }
#endif
      return NULL;
   }
   if (!tc)
   {
      fprintf(stderr, "%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
      return NULL;
   }

   return tc->getAllTypes();
}

vector<pair<string, Type *> > *Module::getAllGlobalVars()
{

   typeCollection *tc = NULL;
   if (!getAnnotation(tc, ModuleTypeInfoAnno))
   {
#if 0
      tc = new typeCollection();
      if (!addAnnotation(tc, ModuleTypeInfoAnno))
      {
         fprintf(stderr, "%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
         return NULL;
      }
#endif
      return NULL;
   }
   if (!tc)
   {
      fprintf(stderr, "%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
      return NULL;
   }

   return tc->getAllGlobalVariables();
}

typeCollection *Module::getModuleTypes()
{
   typeCollection *tc = NULL;
   if (!getAnnotation(tc, ModuleTypeInfoAnno))
   {
      //  add an empty type collection (to be filled in later)
      tc = new typeCollection();
      if (!addAnnotation(tc, ModuleTypeInfoAnno))
      {
         fprintf(stderr, "%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
         return NULL;
      }

      return tc;
   }

   if (!tc)
   {
      fprintf(stderr, "%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
      return NULL;
   }

   return tc;
}

bool Module::findType(Type *&type, std::string name)
{
   exec_->parseTypesNow();
   type = getModuleTypes()->findType(name);

   if (type == NULL)
      return false;

   return true;
}

bool Module::findVariableType(Type *&type, std::string name)
{
   exec_->parseTypesNow();
   type = getModuleTypes()->findVariableType(name);

   if (type == NULL)
      return false;

   return true;
}

void Symtab::parseTypesNow()
{
   if (isTypeInfoValid_)
      return;

   parseTypes();
}

bool Module::setLineInfo(LineInformation *lineInfo)
{
   LineInformation *li =  NULL;
   if (!getAnnotation(li, ModuleLineInfoAnno)) 
   {
      if (li) 
      {
         fprintf(stderr, "%s[%d]:  weird inconsistency with getAnnotation here\n", 
               FILE__, __LINE__);
         return false;
      }

      if (!addAnnotation(lineInfo, ModuleLineInfoAnno))
      {
         fprintf(stderr, "%s[%d]:  failed to add lineInfo annotation\n", FILE__, __LINE__);
         return false;
      }

      return true;
   }
   else 
   {
      if (li != lineInfo)
         delete li;

      //      fprintf(stderr, "%s[%d]:  weird, already have line info anno, check this\n", FILE__, __LINE__);
      if (!addAnnotation(lineInfo, ModuleLineInfoAnno))
      {
         fprintf(stderr, "%s[%d]:  failed to add lineInfo annotation\n", FILE__, __LINE__);
         return false;
      }
   }

   return false;

#if 0
   Annotatable<LineInformation *, module_line_info_a,  true> &mt = *this;

   if (mt.size()) 
   {
      // We need to remove the existing annotation and make sure there is only one annotation.
      mt.clearAnnotations();
      //fprintf(stderr, "%s[%d]:  WARNING, already have lineInfo set for module %s\n", FILE__, __LINE__, fileName_.c_str());
   }
   mt.addAnnotation(lineInfo);
   return true;
#endif
}

bool Module::findLocalVariable(std::vector<localVar *>&vars, std::string name)
{
   exec_->parseTypesNow();
   std::vector<Symbol *>mod_funcs;

   if (!getAllSymbolsByType(mod_funcs, Symbol::ST_FUNCTION))
   {
      return false;
   }

   unsigned origSize = vars.size();

   for (unsigned int i = 0; i < mod_funcs.size(); i++)
   {
      mod_funcs[i]->findLocalVariable(vars, name);
   }

   if (vars.size() > origSize)
      return true;

   return false;
}

Module::Module(supportedLanguages lang, Offset adr,
      std::string fullNm, Symtab *img) :
   fullName_(fullNm),
   language_(lang),
   addr_(adr),
   exec_(img)
{
   fileName_ = extract_pathname_tail(fullNm);
}

Module::Module() :
   fileName_(""),
   fullName_(""),
   language_(lang_Unknown),
   addr_(0),
   exec_(NULL)
{
}

Module::Module(const Module &mod) :
   LookupInterface(),
   Serializable(),
   AnnotatableSparse(),
   fileName_(mod.fileName_),
   fullName_(mod.fullName_),
   language_(mod.language_),
   addr_(mod.addr_),
   exec_(mod.exec_)
{
   //  Copy annotations here or no?
}

Module::~Module()
{

   //  individual deletion of annotations??

#if 0
   if (!clearAnnotations()) 
   {
      fprintf(stderr, "%s[%d]:  failed to clear annotations\n", FILE__, __LINE__);
   }
#endif
   fprintf(stderr, "%s[%d]:  FIXME:  need to clear annotations??\n", FILE__, __LINE__);

}

bool Module::isShared() const
{
   return !exec_->isExec();
}

bool Module::getAllSymbolsByType(std::vector<Symbol *> &found, Symbol::SymbolType sType)
{
   unsigned orig_size = found.size();
   std::vector<Symbol *> obj_syms;

   if (!exec()->getAllSymbolsByType(obj_syms, sType))
      return false;

   for (unsigned i = 0; i < obj_syms.size(); i++) 
   {
      if (obj_syms[i]->getModule() == this)
         found.push_back(obj_syms[i]);
   }

   if (found.size() > orig_size)
   {
      return true;
   }

   serr = No_Such_Symbol;
   return false;
}

bool Module::operator==(Module &mod) 
{
   LineInformation *li =  NULL;
   LineInformation *li_src =  NULL;
   bool get_anno_res = false, get_anno_res_src = false;
   get_anno_res = getAnnotation(li, ModuleLineInfoAnno);
   get_anno_res_src = mod.getAnnotation(li_src, ModuleLineInfoAnno);

   if (get_anno_res != get_anno_res_src)
   {
      fprintf(stderr, "%s[%d]:  weird inconsistency with getAnnotation here\n", 
            FILE__, __LINE__);
      return false;
   }

   if (li) 
   {
      if (!li_src) 
      {
         fprintf(stderr, "%s[%d]:  weird inconsistency with getAnnotation here\n", 
               FILE__, __LINE__);
         return false;
      }

      if (li->getSize() != li_src->getSize()) 
         return false;

      if ((li != li_src)) 
         return false;
   }
   else
   {
      if (li_src) 
      {
         fprintf(stderr, "%s[%d]:  weird inconsistency with getAnnotation here\n", 
               FILE__, __LINE__);
         return false;
      }
   }

   return (
         (language_==mod.language_)
         && (addr_==mod.addr_)
         && (fullName_==mod.fullName_)
         && (exec_==mod.exec_)
         );
}

bool Module::setName(std::string newName)
{
   fullName_ = newName;
   fileName_ = extract_pathname_tail(fullName_);
   return true;
}

void Module::setLanguage(supportedLanguages lang)
{
   language_ = lang;
}

Offset Module::addr() const
{
   return addr_;
}

bool Module::setDefaultNamespacePrefix(string str)
{
    return exec_->setDefaultNamespacePrefix(str);
}

void Module::serialize(SerializerBase *sb, const char *tag)
{
   ifxml_start_element(sb, tag);
   gtranslate(sb, fileName_, "fileName");
   gtranslate(sb, fullName_, "fullName");
   gtranslate(sb, addr_, "Address");
   gtranslate(sb, language_, supportedLanguages2Str, "Language");
   ifxml_end_element(sb, tag);

   //  Patch up exec_ (pointer to Symtab) at a higher level??
   //if (getSD().iomode() == sd_deserialize)
   //   param.exec_ = parent_symtab;
}

