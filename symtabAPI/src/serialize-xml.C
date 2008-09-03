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
#if !defined(os_windows)
#include "common/h/pathName.h"
#include <dlfcn.h>
#else
#include "windows.h"
#include <libxml/xmlversion.h>
#undef LIBXML_ICONV_ENABLED
#endif

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

#if !defined(os_windows)
    //libxml2 functions
extern void *hXML;
#else
extern HINSTANCE hXML;
#endif

namespace Dyninst {
   namespace SymtabAPI {
#if 0
bool serialize(Symtab &st, SymtabTranslatorBase &trans)
{
  try 
  {
    trans.translate_base(st);
  } 
  catch(const SerializerError &err)
  {
    int line = err.line();
    std::string file = err.file();
    fprintf(stderr, "%s[%d]: serialization error: %s\n", __FILE__, __LINE__, err.what());
    fprintf(stderr, "\t original location is -- %s[%d]\n", file.c_str(), line);
    return false;
  }
  return true;
}

bool deserialize(Symtab &st, SymtabTranslatorBase &trans)
{
  try 
  {
    trans.translate_base(st);
  } 
  catch(const SerializerError &err)
  {
    fprintf(stderr, "%s[%d]: serialization error: %s\n", __FILE__, __LINE__, err.what());
    return false;
  }
  return true;
}
#endif

}
}

extern SymtabError serr;
extern string errMsg;
SymtabTranslatorXML::SymtabTranslatorXML(Symtab *st, string file) : 
   SymtabTranslatorBase(st, file, sd_serialize)
#if 0
   sf(file, sd_serialize)
#endif
{
}


void SymtabTranslatorXML::symtab_start(Symtab &, const char *)
{
  getSDXML().start_element("Symtab");
}

void SymtabTranslatorXML::symbol_start(Symbol &, const char *)
{
  getSDXML().start_element("Symbol");
}

void SymtabTranslatorXML::symbol_end(Symbol &, const char *)
{
  getSDXML().end_element();
}

void SymtabTranslatorXML::exception_start(ExceptionBlock &, const char *)
{
  getSDXML().start_element("ExceptionBlock");
}

void SymtabTranslatorXML::exception_end(ExceptionBlock &, const char *)
{
  getSDXML().end_element();
}

void SymtabTranslatorXML::relocation_start(relocationEntry &, const char *)
{
  getSDXML().start_element("Dyn_Relocation");
}

void SymtabTranslatorXML::relocation_end(relocationEntry &, const char *)
{
  getSDXML().end_element();
}
void SymtabTranslatorXML::module_start(Module &, const char *)
{
  getSDXML().start_element("Module");
}

void SymtabTranslatorXML::module_end(Module &, const char *)
{
  getSDXML().end_element();
}

#if 0
void SymtabTranslatorXML::section_start(Section &, const char *)
{
  sd.start_element("Section");
}

void SymtabTranslatorXML::section_end(Section &, const char *)
{
  sd.end_element();
}
#endif

void SymtabTranslatorXML::line_information_start(LineInformation &, const char *)
{
  getSDXML().start_element("ModuleLineInformation");
}

void SymtabTranslatorXML::line_information_end(LineInformation &, const char *)
{
  getSDXML().end_element();
}

void SymtabTranslatorXML::type_collection_start(typeCollection &, const char *)
{
  getSDXML().start_element("ModuleTypeCollection");
}

void SymtabTranslatorXML::type_collection_end(typeCollection &, const char *)
{
  getSDXML().end_element();
}

void SymtabTranslatorXML::translate(Symbol::SymbolType &t, const char *tag)
{
    getSDXML().xml_value(Symbol::symbolType2Str(t), tag);
}

void SymtabTranslatorXML::translate(Symbol::SymbolLinkage &t, const char *tag)
{
  getSDXML().xml_value(Symbol::symbolLinkage2Str(t), tag);
}
void SymtabTranslatorXML::translate(Symbol::SymbolTag &t, const char *tag)
{
  getSDXML().xml_value(Symbol::symbolTag2Str(t), tag);
}

void SymtabTranslatorXML::translate(supportedLanguages &l, const char *tag) 
{
    const char *lstr = NULL;
    switch (l) {
      case lang_Unknown: lstr = "lang_Unknown";
         break;
      case lang_Assembly: lstr = "lang_Assembly";
         break;
      case lang_C: lstr = "lang_C";
         break;
      case lang_CPlusPlus: lstr = "lang_CPlusPlus";
         break;
      case lang_GnuCPlusPlus: lstr = "lang_GnuCPlusPlus";
         break;
      case lang_Fortran: lstr = "lang_Fortran";
         break;
      case lang_Fortran_with_pretty_debug: lstr = "lang_Fortran_sun";
         break;
      case lang_CMFortran: lstr = "lang_CMFortran";
         break;
      default: lstr = "lang_Undefined";
         break;
    };

  getSDXML().xml_value(lstr, tag);
}

void SymtabTranslatorXML::symtab_end(Symtab &, const char *)
{
  getSDXML().end_element();
}
