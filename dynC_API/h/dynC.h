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
   BPatch_snippet *createSnippet(const char *s, BPatch_point &point, const char *name = "");
   BPatch_snippet *createSnippet(FILE *f, BPatch_point &point, const char *name = "");
   BPatch_snippet *createSnippet(const char *s, BPatch_addressSpace &addSpace, const char *name = "");
   BPatch_snippet *createSnippet(FILE *f, BPatch_addressSpace &addSpace, const char *name = "");
//   BPatch_snippet *createSnippet(std::string str, BPatch_point *point, char *name = "");
//   BPatch_snippet *createSnippet(std::ifstream *is, BPatch_point *point, char *name = "");
   
   std::string mangle(const char *varName, BPatch_point *point, const char *typeName);
   std::string getMangledStub(const char *varName, BPatch_point *point);
//   std::string getSnippetName(const char *mangled);
//   BPatch_variableExpr demangle(const char *mangledName);
}

#endif
