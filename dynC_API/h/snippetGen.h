#ifndef SNIPPET_GEN_H
#define SNIPPET_GEN_H

#include "BPatch_snippet.h"
#include "BPatch_addressSpace.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
#include "BPatch_module.h"
#include "BPatch.h"
#include <sstream>
#include <string>



class SnippetGenerator{
  public:
   enum SGErrorType{
      SG_LookUpFailure,
      SG_TypeError
   };
   
   enum SGContext{
      SG_FunctionName,
      SG_ModuleName,
      SG_TID
   };
  private:
   std::stringstream lastError;
   SGErrorType lastErrorType;   
   BPatch_point *point;
   BPatch_addressSpace *addSpace;
   BPatch_image *image;
   
   char *snippetName;
  
  public:
   std::string getError() {return lastError.str();};
   SGErrorType getErrorType() {return lastErrorType;};
  
  public:

  SnippetGenerator() :  point(NULL), addSpace(NULL), image(NULL) {};
  SnippetGenerator(BPatch_point *pt, char *snName) : point(pt), snippetName(snName) 
   { 
      assert(point != NULL); 
      addSpace = pt->getAddressSpace();
      image = addSpace->getImage();
   };

   ~SnippetGenerator(){};

   BPatch_snippet *findOrCreateVariable(const char * name, const char * type, const void * initialValue = NULL);
   BPatch_snippet *findOrCreateArray(const char * name, const char * elementType, long size);
   BPatch_snippet *findOrCreateStruct();
   BPatch_snippet *findOrCreateUnion();
   BPatch_snippet *findAppVariable(const char * name, bool global = false, bool local = false);
   BPatch_snippet *findParameter(const char * name);
   BPatch_snippet *findParameter(int index);
   BPatch_snippet *findInstVariable(const char *mangledStub, const char * name);
   BPatch_snippet *generateArrayRef(BPatch_snippet *base, BPatch_snippet *index);
   BPatch_function *findFunction(const char * name, std::vector<BPatch_snippet *> params);
   BPatch_snippet *getContextInfo(SGContext context);

   BPatch_point *getPoint() {return point;};
};

#endif
