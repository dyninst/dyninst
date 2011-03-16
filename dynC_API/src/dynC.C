/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
//command.C
#include "dynC.h"
#include "ast.h"

#define BASE_TEN 10
extern BPatch_snippet *parse_result;

extern "C" {
   void set_lex_input(char *s);
   int dynCparse();

}
extern char *dynCSnippetName;
extern int dyn_debug_ast;
extern SnippetGenerator *snippetGen;
extern BPatch_point *snippetPoint;

namespace dynC_API{

   const std::string varNameBase = "dynC_mangled_";
   const bool debug = false;
   static int snippetCount;

   BPatch_snippet * createSnippet(const char *s, BPatch_point &point, const char *name){
      if(debug) printf("Hello World! from createSnippet\n");
      if(debug) printf("Set lex input to:\n%s\n", s);
      char *mutS = strdup(s);
      set_lex_input(mutS);
      if(strlen(name) == 0){
         char *autoName = new char[32];
         sprintf(autoName, "%s%d", "Snippet_", snippetCount);
         name = autoName;
      }
      snippetCount++;
      char *mutName = strdup(name);      
      dynCSnippetName = mutName;
      snippetPoint = &point;
      snippetGen = new SnippetGenerator(point, mutName);
      if(dynCparse() == 0){
         if(debug){
            dyn_debug_ast = 1;
            parse_result->ast_wrapper->debugPrint();
            dyn_debug_ast = 0;
         }
         delete mutName;
         delete mutS;
         return parse_result;
      }
      delete mutName;
      delete mutS;
      return NULL; //error
   }

   BPatch_snippet * createSnippet(const char *s, BPatch_addressSpace &addSpace, const char *name){
      if(debug) printf("Hello World! from createSnippet\n");
      char *mutS = strdup(s);
      set_lex_input(mutS);
      if(debug) printf("Set lex input to:\n%s", s);
      if(strlen(name) == 0){
         char *autoName = new char[32];
         sprintf(autoName, "%s%d", "Snippet_", snippetCount);
         name = autoName;
      }
      snippetCount++;
      char *mutName = strdup(name);
      dynCSnippetName = mutName;
      snippetPoint = NULL;
      snippetGen = new SnippetGenerator(addSpace, mutName);
      if(dynCparse() == 0){
         if(debug){
            dyn_debug_ast = 1;
            parse_result->ast_wrapper->debugPrint();
            dyn_debug_ast = 0;
         }
         delete mutS;
         delete mutName;
         return parse_result;
      }
      delete mutS;
      delete mutName;
      return NULL; //error
   }

   BPatch_snippet * createSnippet(FILE *f, BPatch_point &point, const char *name){
      std::string fileString;
      char line[128];
      if(f == NULL){
         fprintf(stderr, "Error: Unable to open file\n");
         return NULL;
      }
      while(fgets(line, 128, f) != NULL)
      {
         fileString += line;
      }
      return createSnippet(strdup(fileString.c_str()), point, name);
   }


   BPatch_snippet * createSnippet(FILE *f, BPatch_addressSpace &addSpace, const char *name){
      std::string fileString;
      char line[128];
      if(f == NULL){
         fprintf(stderr, "Error: Unable to open file\n");
         return NULL;
      }
      while(fgets(line, 128, f) != NULL)
      {
         fileString += line;
      }
      return createSnippet(strdup(fileString.c_str()), addSpace, name);
   }

   std::string mangle(const char *varName, BPatch_point *point, const char *typeName){
      std::stringstream namestr;
      namestr << varNameBase << varName << "_0x" << (point == NULL ? 0 : point->getAddress());
      namestr << "_" << typeName;
      std::string retName = namestr.str();
      return retName;
   }

/* Old version
   std::string mangle(const char *varName, const char *snippetName, const char *typeName, const bool isGlobal){
      std::stringstream namestr;
      namestr << varNameBase << varName << "_" << point;
      namestr << "_" << typeName;
      namestr << "_" << (isGlobal ? "global" : "local");
      std::string retName = namestr.str();
      return retName;
   }
*/
   std::string getMangledStub(const char *varName, BPatch_point *point){
      std::stringstream namestr;
//      namestr << varNameBase << varName << "_";
      namestr << varNameBase << varName << "_0x" << (point == NULL ? 0 : point->getAddress());
      std::string retName = namestr.str();
      return retName;
   }   
}
