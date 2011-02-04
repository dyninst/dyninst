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
