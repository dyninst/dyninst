//command.h
#ifndef DYN_C_H
#define DYN_C_H

#include "BPatch.h"
#include "BPatch_snippet.h"
#include <fstream>
#include "stdio.h"
#include "snippetGen.h"
#include <sstream>
#include <string>

namespace dynC_API{
   BPatch_snippet *createSnippet(char *s, BPatch_point *point, char *name = "");
   BPatch_snippet *createSnippet(std::string str, BPatch_point *point, char *name = "");
   BPatch_snippet *createSnippet(FILE *f, BPatch_point *point,char *name = "");
   BPatch_snippet *createSnippet(std::ifstream *is, BPatch_point *point, char *name = "");
   
   std::string mangle(const char *varName, const char *snippetName, const char *typeName, const bool isGlobal = false/*, const bool isStatic = false*/);
   std::string getMangledStub(const char *varName);
   std::string getSnippetName(const char *mangled);
//   BPatch_variableExpr demangle(const char *mangledName);
}

#endif
