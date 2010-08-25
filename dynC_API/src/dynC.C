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

   BPatch_snippet * createSnippet(char *s, BPatch_point *point, char *name){
      if(debug) printf("Hello World! from createSnippet\n");
      set_lex_input(strdup(s));
      if(debug) printf("Set lex input to:\n%s", s);
      if(strlen(name) == 0){
         char autoName[32];
         sprintf(autoName, "%s%d", "Snippet_", snippetCount);
         name = autoName;
      }
      snippetCount++;
      dynCSnippetName = name;
      snippetPoint = point;
      snippetGen = new SnippetGenerator(point, name);
      if(dynCparse() == 0){
         if(debug){
            dyn_debug_ast = 1;
            parse_result->ast_wrapper->debugPrint();
            dyn_debug_ast = 0;
         }
         return parse_result;
      }
      return NULL; //error
   }

   BPatch_snippet * createSnippet(std::string str, BPatch_point *point, char *name){
      return createSnippet((char *)strdup(str.c_str()), point, name);
   }

   BPatch_snippet * createSnippet(FILE *f, BPatch_point *point, char *name){
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
      return createSnippet(fileString.c_str(), point, name);
   }

   BPatch_snippet * createSnippet(std::ifstream *is, BPatch_point *point, char *name){
      std::string fileString;
      std::string line;
      if(!is->is_open()){
         fprintf(stderr, "Error: Unable to open file\n");
         return NULL;
      }
      while(!is->eof())
      {
         std::getline(*is, line);
         fileString += line + "\n";
      }
      return createSnippet(fileString.c_str(), point, name);
   }
   
   std::string mangle(const char *varName, const char *snippetName, const char *typeName, const bool isGlobal){
      std::stringstream namestr;
      namestr << varNameBase << varName << "_" << snippetName;
      namestr << "_" << typeName;
      namestr << "_" << (isGlobal ? "global" : "local");
      std::string retName = namestr.str();
      return retName;
   }

   std::string getMangledStub(const char *varName){
      std::stringstream namestr;
      namestr << varNameBase << varName << "_";
      std::string retName = namestr.str();
      return retName;
   }   
}
