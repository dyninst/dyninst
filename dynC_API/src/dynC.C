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

#include "dynC.h"
#include "ast.h"
#include "snippetGen.h"

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
//extern int dynCdebug;
namespace dynC_API{

   const std::string varNameBase = "dynC_mangled_";
   static int snippetCount = 0;

  std::map<BPatch_point *, BPatch_snippet *> *createSnippet(const char *s, std::vector<BPatch_point *> points){
//     dynCdebug = 1;
      char *mutS = strdup(s);
      printf("Set lex input to:\n");
      printf("---->%s<-", s);
      printf("---\n");
      set_lex_input(mutS);

      std::map<BPatch_point *, BPatch_snippet *> *ret_map = new std::map<BPatch_point *, BPatch_snippet *>();
      
      if(ret_map == NULL){
         fprintf(stderr, "DYNC_INTERNAL Error [%s:%d]: 'new' operation failed!\n", __FILE__, __LINE__);
         return NULL;
      }

      //keep track of number of snippets for internal variable names
      snippetCount++;

      std::vector<BPatch_point *>::iterator it;
      std::stringstream name;
      name.str() = "";
      name << "Snippet_" << snippetCount;
      char *mutName = strdup(name.str().c_str());      
      dynCSnippetName = mutName;

      for(it = points.begin(); it != points.end(); ++it){
         printf("%s", s);
         set_lex_input(mutS);
         snippetPoint = (*it);
         snippetGen = new SnippetGenerator(**it);

         if(dynCparse() == 0){
            printf("parse_result is %s\n", (parse_result == NULL ? "null" : "not null"));
            if(parse_result != NULL){
	      std::cerr << parse_result->ast_wrapper->format("") << std::endl;
	      
            }
            ret_map->insert(std::pair<BPatch_point *, BPatch_snippet *>((*it), parse_result));
         }else{
            free(mutName);
            free(mutS);
            delete ret_map;
            delete snippetGen;
            return NULL; //error
         }
         delete snippetGen;
         
      }
      free(mutName);

      return ret_map;
   }

   BPatch_snippet * createSnippet(const char *s, BPatch_point &point){
      std::vector<BPatch_point*> points;
      points.push_back(&point);
      std::map<BPatch_point *, BPatch_snippet *> *retMap = createSnippet(s, points);
      if (retMap->empty()){
         return NULL;
      }
      return (*retMap->begin()).second;
   }

   BPatch_snippet * createSnippet(const char *s, BPatch_addressSpace &addSpace){

      char *mutS = strdup(s);
      set_lex_input(mutS);

      snippetCount++;
      std::string name = "Snippet_";
      name += snippetCount;
      char *mutName = strdup(name.c_str());      
      dynCSnippetName = mutName;
      snippetPoint = NULL;
      snippetGen = new SnippetGenerator(addSpace);
      if(dynCparse() == 0){
         free(mutS);
         free(mutName);
         return parse_result;
      }
      free(mutS);
      free(mutName);
      return NULL; //error
   }

   BPatch_snippet * createSnippet(FILE *f, BPatch_point &point){
      std::string fileString;
      if(f == NULL){
         fprintf(stderr, "Error: Unable to open file\n");
         return NULL;
      }
      int c;
      while((c = fgetc(f)) != EOF)
      {
         fileString += (unsigned char)c;
      }
      fileString += '\0';
      rewind(f);

      char *cstr = strdup(fileString.c_str());
      BPatch_snippet *retSn = createSnippet(cstr, point);
      free(cstr);
      return retSn;
   }

   std::map<BPatch_point *, BPatch_snippet *> *createSnippet(FILE *f, std::vector<BPatch_point *> points){
      std::string fileString;
      if(f == NULL){
         fprintf(stderr, "Error: Unable to open file\n");
         return NULL;
      }
      int c;
      while((c = fgetc(f)) != EOF)
      {
         fileString += (unsigned char)c;
      }
      fileString += '\0';
      rewind(f);

      char *cstr = strdup(fileString.c_str());
      std::map<BPatch_point *, BPatch_snippet *> *retMap = createSnippet(cstr, points);
      free(cstr);
      return retMap;
   }


   BPatch_snippet * createSnippet(FILE *f, BPatch_addressSpace &addSpace){
      std::string fileString;
      if(f == NULL){
         fprintf(stderr, "Error: Unable to open file\n");
         return NULL;
      }
      int c;
      while((c = fgetc(f)) != EOF)
      {
         fileString += (unsigned char)c;
      }
      fileString += '\0';
      rewind(f);
      char *cstr = strdup(fileString.c_str());
      BPatch_snippet *retSn = createSnippet(cstr, addSpace);
      free(cstr);
      return retSn;
   }

   BPatch_snippet * createSnippet(std::string str, BPatch_point &point){
      char *cstr = strdup(str.c_str());
      BPatch_snippet *retSn = createSnippet(cstr, point);
      free(cstr);
      return retSn;
   }

   std::map<BPatch_point *, BPatch_snippet *> *createSnippet(std::string str, std::vector<BPatch_point *> points){
      char *cstr = strdup(str.c_str());
      std::map<BPatch_point *, BPatch_snippet *> *retMap = createSnippet(cstr, points);
      free(cstr);
      return retMap;
   }


   BPatch_snippet * createSnippet(std::string str, BPatch_addressSpace &addSpace){
      char *cstr = strdup(str.c_str());
      BPatch_snippet *retSn = createSnippet(cstr, addSpace);
      free(cstr);
      return retSn;      
   }


   std::string mangle(const char *varName, const char *snippetName, const char *typeName){
      std::stringstream namestr;
      namestr << varNameBase << varName << "_" << snippetName;
      namestr << "_" << typeName;
      std::string retName = namestr.str();
      return retName;
   }

   std::string getMangledStub(const char *varName, const char *snippetName){
      std::stringstream namestr;
      namestr << varNameBase << varName << "_" << snippetName;
      std::string retName = namestr.str();
      return retName;
   }

}
