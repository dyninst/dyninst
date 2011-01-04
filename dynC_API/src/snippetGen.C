#include "snippetGen.h"  

extern "C" {
   std::string getErrorBase();
}

BPatch_snippet *SnippetGenerator::findOrCreateVariable(const char * name, const char * type, const void * initialValue){
   lastError.str() = "";
   int zero = 0;
   if(initialValue == NULL){
           //expand?
      if (strcmp(type, "int") == 0)
         initialValue = &zero; 
      if (strcmp(type, "char *") == 0)
         initialValue = "";
   }
   
   BPatch_variableExpr *varExpr = image->findVariable(name, false);
   if(varExpr != NULL){
      return varExpr;
   }
   BPatch_type *bptype = image->findType(type);
   
   if(bptype == NULL){
      lastError << "Unable to find type:" << type;
      return NULL;
   }
//   varExpr = addSpace->malloc(*bptype);
   varExpr = addSpace->createVariable(std::string(name), (Dyninst::Address)NULL, bptype);

   if(varExpr == NULL){
      lastError << "FIXME: varExpr is null!";
      return NULL;
   }
   
   if(!(varExpr->writeValue(initialValue))){
      //unable to initialize... what to do?
   }
   
   return varExpr;

}
BPatch_snippet *SnippetGenerator::findOrCreateArray(const char * name, const char * elementType, long size){
   lastError.str() = "";        
        BPatch_variableExpr *varExpr = image->findVariable(name, false);
        if(varExpr != NULL){
           return varExpr;
        }

        std::stringstream arrayTypeName;
        arrayTypeName << elementType << "[" << size << "]";
        BPatch_type *type = image->findType(elementType);
       
        if(type == NULL){
           lastError << "Unable to find type:" << elementType;
           return NULL;
        }
        
        BPatch_type *array = BPatch::bpatch->createArray(arrayTypeName.str().c_str(), type, 0, size - 1);

        if(array == NULL){
           lastError << "Failed to create array";
           return NULL;
        }

        varExpr = addSpace->malloc(*array);
        varExpr = addSpace->createVariable(std::string(name), (Dyninst::Address)varExpr->getBaseAddr(), array);
        
        if(varExpr == NULL){
           lastError << "FIXME: varExpr is null!";
           return NULL;
        }

        return varExpr;

}
BPatch_snippet *SnippetGenerator::findOrCreateStruct(){
   return NULL;
}
BPatch_snippet *SnippetGenerator::findOrCreateUnion(){
   return NULL;
}

BPatch_snippet *SnippetGenerator::findAppVariable(const char * name, bool global, bool local){
   lastError.str() = "";
   if(global && local){
      lastError << "Cannot specify a variable as both global and local.";
      return NULL;
   }
   BPatch_variableExpr *varExpr = NULL;
   if(point != NULL){
      varExpr  = image->findVariable(*point, name, false);;
   }
   if(global || point == NULL){
      varExpr = image->findVariable(name, false);
   }else if(local){
      varExpr = image->findVariable(*point, name, false);
   }
   if(varExpr == NULL){ // If var not found
      lastError << "Can't find ";
      lastError << (local ? "local " : (global ? "global " : ""));
//      printf("%s\n", name);
      lastError << "variable \"" << name << "\"";
      return NULL;
   }
   BPatch_variableExpr *varExprGbl = image->findVariable(name);    
   if(!global && point != NULL){
      if(local && NULL != varExprGbl && varExpr == varExprGbl){
         lastError << "Can't find local variable \"" << name << "\". There is a global variable of the same name.";
         return NULL;
      }
      if(NULL == varExprGbl || (local && NULL != varExprGbl && varExprGbl != varExpr)){ // if var is local
         if (point->getPointType() == BPatch_entry || point->getPointType() == BPatch_exit){
            lastError << "Cannot access local variables at entry or exit points.";
            return NULL;
         }
         return varExpr; //found a local variable
      }
   }
   if(point != NULL && !local && !global && (NULL != varExprGbl && varExprGbl != varExpr)){// if BPatch_AppVar && both local and global
      if (point->getPointType() != BPatch_entry && point->getPointType() != BPatch_exit){
         return varExpr; //found a local variable`
      }
   }
   
   return varExprGbl; //found a global variable
   
}
BPatch_snippet *SnippetGenerator::findParameter(const char * name){
   lastError.str() = "";
   if(point == NULL){
      lastError << "Cannot access local varaibles without point information.";
      return NULL;
   }
   if(point->getPointType() != BPatch_entry){
      lastError << "Parameters only valid at entry points.";
      return NULL;
   }
   char fnName[512] = "";   
   std::vector<BPatch_localVar *> *locals = point->getFunction()->getParams();
   if(locals->size() == 0){
      lastError << "No parameters for " << point->getFunction()->getTypedName(fnName, 512) << "";
      return NULL;
   }
         
   for(unsigned int i = 0; i < locals->size(); ++i){
      if (strcmp(((*locals)[i])->getName(), name) == 0){
         return new BPatch_paramExpr(i);
      }
   }

   lastError << "Parameter \'" << name << "\' not found for " << point->getFunction()->getTypedName(fnName, 512) << "" ;
   return NULL;
}
BPatch_snippet *SnippetGenerator::findParameter(int index){
   lastError.str() = "";       
   if(point == NULL){
      lastError << "Cannot access local varaibles without point information.";
      return NULL;
   }
   if(point->getPointType() != BPatch_entry){
      lastError << "Parameters only valid at entry points.";
      return NULL;
   }

   char fnName[512] = "";
   std::vector<BPatch_localVar *> *locals = point->getFunction()->getParams();
   if(locals->size() == 0){
      
      lastError << "No parameters for " << point->getFunction()->getTypedName(fnName, 512) << "";
      return NULL;
   }
         
   if(locals->size() <= (const unsigned int)index || index < 0){
      lastError << "Parameter index out of range:" << index << "for " << point->getFunction()->getTypedName(fnName, 512) << "";
      return NULL;
   }   
   return new BPatch_paramExpr(index);
}
BPatch_snippet *SnippetGenerator::findInstVariable(const char *mangledStub, const char * name){
   lastError.str() = "";
   std::vector<BPatch_variableExpr *> *vars = image->getGlobalVariables();
   if(vars->size() == 0){
      lastError << "No global variables!";
      return NULL;
   }
   //  BPatch_variableExpr *varExpr = NULL;
   BPatch_variableExpr *varExprGbl = NULL;
   for(unsigned int i = 0; i < vars->size(); ++i){
      char *substr = strstr((*vars)[i]->getName(), mangledStub);
      if(substr != NULL){
         // if(strstr(substr, snippetName) == substr){
            //found snippet local var
            return (*vars)[i];            
//         }
/*
         if(strncmp(substr, "_", 1)){
            //found snippet global var
            if(varExprGbl != NULL){
               lastError << "DYNC INTERNAL ERROR: Multiple global variables fitting mangled \"" << mangledStub << "\"";
               return NULL;
            }
            varExprGbl = (*vars)[i];

         }
*/
      }
   }
   if(varExprGbl != NULL){
      return varExprGbl;
   }
   lastError << "Unable to find InstVar \"" << name << "\"";
   return NULL;
}
BPatch_snippet *SnippetGenerator::generateArrayRef(BPatch_snippet *base, BPatch_snippet *index){
   lastError.str() = "";
   if(base->getType() != NULL && base->getType()->getDataClass() != BPatch_dataArray){
      lastError << "Base of array reference is not an array";
      return NULL;
   }
   if(index->getType() != NULL && index->getType()->getDataClass() != BPatch_scalar){
      lastError << "Array index is not a scalar";
      return NULL;
   }

   return new BPatch_arithExpr(BPatch_ref, *base, *index);
}

BPatch_function *SnippetGenerator::findFunction(const char * name, std::vector<BPatch_snippet *> params){
   lastError.str() = "";
   
   std::vector<BPatch_function *> functions;
   
   if(NULL == image->findFunction(name, functions, false)){
      lastError << "Couldn't find function \'" << name << "\'";
      return NULL;
   }
   char funcName[512];
   bool foundDebugInfo, foundMatch, setFunc;
   BPatch_function *noParamFunc = NULL; //for storing potentially variadic functions
   foundDebugInfo = false;
   foundMatch = false;
   setFunc = false;
   std::vector<BPatch_localVar *> *funcParams;
   BPatch_function *func = functions[0];
   std::vector<Dyninst::SymtabAPI::localVar *>SymTabParams;

   for(unsigned int n = 0; n < functions.size(); ++n){
      //if has debug info for params
      if(functions[n]->hasParamDebugInfo()){
         foundDebugInfo = true;
         funcParams = functions[n]->getParams();
         if(funcParams->size() == 0){
            noParamFunc = functions[n]; //store a potentially variadic function
         }
         foundMatch = true;
         if(funcParams->size() == params.size()){
            for(unsigned int i = 0; i < funcParams->size(); ++i){
               if(!(*funcParams)[i]->getType()->isCompatible(params[i]->getType())){
                  foundMatch = false;
                  break;
               }
            }
            if(foundMatch){
               func = functions[n];
               setFunc = true;
               break;
            }
         }                           
      }
   }

   const char *errorHeader = getErrorBase().c_str();
   if(noParamFunc != NULL && !setFunc){
         func = noParamFunc;
   }else if(!setFunc && foundDebugInfo){
      lastError << "No matching function for call to '" << name << "(";
      if(params.size() > 0){
         lastError << params[0]->getType()->getName();
      }
      for(unsigned int m = 1; m < params.size(); ++m){
         lastError << ", ";
         if(params[m]->getType() == NULL){
            lastError << "...";
         }else{
            lastError << params[m]->getType()->getName();
         }
      }
      lastError << ")'.\n";
      for(unsigned int i = 0; i < strlen(errorHeader); ++i){
            lastError << ' ';
      }
      lastError << "  note: candidates are: ";
      lastError << functions[0]->getTypedName(funcName, 512);
      for(unsigned int n = 1; n < functions.size(); ++n){
         lastError << "\n";
         for(unsigned int i = 0; i < strlen(errorHeader); ++i){
            lastError << ' ';
         }
         lastError << "  note:                ";
         lastError << functions[n]->getTypedName(funcName, 512);
      }
         lastError << "\n";
      return NULL;
   }
   

   if(!foundDebugInfo){
      func = functions[0]; //<- Pick the first one if there is no debug info
   }
   return func;
}
BPatch_snippet *SnippetGenerator::getContextInfo(SGContext context){
   char name[512];
   lastError.str() = "";
   if(point == NULL){
      lastError << "Cannot provide context information without a point";
      return NULL;
   }
   switch(context){
      case SG_FunctionName:
          point->getFunction()->getName(name, 512);
          return new BPatch_constExpr(const_cast<char *>(name));
          break;
      case SG_ModuleName:
         point->getFunction()->getModuleName(name, 512);
         return new BPatch_constExpr(const_cast<char *>(name));
         break;
      case SG_TID:
         if(image->getProcess() == NULL){
            lastError << "Process is null!"; // doesn't do anything
            return NULL;
        }      
        return new BPatch_tidExpr(image->getProcess());
      default:
         lastError << "Internal: Unrecognized SGContext!";
         return NULL;
   }
}

