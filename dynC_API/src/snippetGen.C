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
#include "snippetGen.h"  
#include "dyntypes.h"

extern "C" {
   void getErrorBase(char *errbase, int length); 
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
      lastError << "Unable to find type: " << type;
      lastErrorInfo.type = SG_LookUpFailure;
      lastErrorInfo.fatal = true;
      return NULL;
   }
//   varExpr = addSpace->malloc(*bptype);
   varExpr = addSpace->createVariable(std::string(name), (Dyninst::Address)NULL, bptype);

   if(varExpr == NULL){
      lastError << "FIXME: varExpr is null!";
      lastErrorInfo.type = SG_InternalError;
      lastErrorInfo.fatal = true;
      return NULL;
   }
   
   if(!(varExpr->writeValue(initialValue))){
      //unable to initialize... what to do?
      lastError << "Internal: Variable initialization failed";
      lastErrorInfo.type = SG_InternalError;
      lastErrorInfo.fatal = false;
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
           lastErrorInfo.type = SG_LookUpFailure;
           lastErrorInfo.fatal = true;
           return NULL;
        }
        
        BPatch_type *array = BPatch::bpatch->createArray(arrayTypeName.str().c_str(), type, 0, size - 1);

        if(array == NULL){
           lastError << "Failed to create array";
           lastErrorInfo.type = SG_InternalError;
           lastErrorInfo.fatal = true;
           return NULL;
        }

        varExpr = addSpace->malloc(*array);
        varExpr = addSpace->createVariable(std::string(name), (Dyninst::Address)varExpr->getBaseAddr(), array);
        
        if(varExpr == NULL){
           lastError << "FIXME: varExpr is null!";
           lastErrorInfo.type = SG_InternalError;
           lastErrorInfo.fatal = true;
           return NULL;
        }

        return varExpr;

}

BPatch_snippet *SnippetGenerator::findRegister(const char *name){
   lastError.str() = "";

   if(!addSpace->getRegisters(registers)){
      lastError << "Could not retrive registers. Register access may not be available on this platform.";
      return NULL;
   }
   
   for(unsigned int i = 0; i < registers.size(); i++){
      BPatch_register r = registers[i];
      if(r.name() == name){
         return new BPatch_registerExpr(r);
      }
   }
   
   lastError << "Register " << name << " not found";
   return NULL;
}


BPatch_snippet *SnippetGenerator::findAppVariable(const char * name, bool global, bool local){
   lastError.str() = "";
   if(global && local){
      lastError << "Cannot specify a variable as both global and local.";
      lastErrorInfo.type = SG_SyntaxError;
      lastErrorInfo.fatal = true;
      return NULL;
   }
   BPatch_variableExpr *varExpr = NULL;
   if(point != NULL){
      varExpr = image->findVariable(*point, name, false);;
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
      lastErrorInfo.type = SG_LookUpFailure;
      lastErrorInfo.fatal = true;
      return NULL;
   }
   BPatch_variableExpr *varExprGbl = image->findVariable(name);    
   if(!global && point != NULL){
      if(local && NULL != varExprGbl && varExpr == varExprGbl){
         lastError << "Can't find local variable \"" << name << "\". There is a global variable of the same name.";
         lastErrorInfo.type = SG_LookUpFailure;
         lastErrorInfo.fatal = true;
         return NULL;
      }
      if(NULL == varExprGbl || (local && NULL != varExprGbl && varExprGbl != varExpr)){ // if var is local
         if (point->getPointType() == BPatch_entry || point->getPointType() == BPatch_exit){
            lastError << "Cannot access local variables at entry or exit points.";
            lastErrorInfo.type = SG_ScopeViolation;
            lastErrorInfo.fatal = true;
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
      lastErrorInfo.type = SG_ScopeViolation;
      lastErrorInfo.fatal = true;
      return NULL;
   }
   if(point->getPointType() != BPatch_entry){
      lastError << "Parameters only valid at entry points.";
      lastErrorInfo.type = SG_ScopeViolation;
      lastErrorInfo.fatal = true;
      return NULL;
   }
   char fnName[512] = "";   
   std::vector<BPatch_localVar *> *locals = point->getFunction()->getParams();
   if(locals->size() == 0){
      lastError << "No parameters for " << point->getFunction()->getTypedName(fnName, 512) << "";
      lastErrorInfo.type = SG_LookUpFailure;
      lastErrorInfo.fatal = false; //should this be false?
      return NULL;
   }
         
   for(unsigned int i = 0; i < locals->size(); ++i){
      if (strcmp(((*locals)[i])->getName(), name) == 0){
         return new BPatch_paramExpr(i);
      }
   }

   lastError << "Parameter \'" << name << "\' not found for " << point->getFunction()->getTypedName(fnName, 512) << "" ;
   lastErrorInfo.type = SG_LookUpFailure;
   lastErrorInfo.fatal = false; //should this be false?
   return NULL;
}
BPatch_snippet *SnippetGenerator::findParameter(int index){
   lastError.str() = "";       
   if(point == NULL){
      lastError << "Cannot access local varaibles without point information.";
      lastErrorInfo.type = SG_ScopeViolation;
      lastErrorInfo.fatal = true;
      return NULL;
   }
   if(point->getPointType() != BPatch_entry){
      lastError << "Parameters only valid at entry points.";
      lastErrorInfo.type = SG_ScopeViolation;
      lastErrorInfo.fatal = true;
      return NULL;
   }

   char fnName[512] = "";
   std::vector<BPatch_localVar *> *locals = point->getFunction()->getParams();
   if(locals->size() == 0){
      
      lastError << "No parameters for " << point->getFunction()->getTypedName(fnName, 512) << "";
      lastErrorInfo.type = SG_LookUpFailure;
      lastErrorInfo.fatal = false; //should this be false?
      return NULL;
   }
         
   if(locals->size() <= (unsigned int)index || index < 0){
      lastError << "Parameter index out of range:" << index << "for " << point->getFunction()->getTypedName(fnName, 512) << "";
      lastErrorInfo.type = SG_LookUpFailure;
      lastErrorInfo.fatal = false; //should this be false?
      return NULL;
   }   
   return new BPatch_paramExpr(index);
}
BPatch_snippet *SnippetGenerator::findInstVariable(const char *mangledStub, const char * name){
   lastError.str() = "";
   std::vector<BPatch_variableExpr *> *vars = image->getGlobalVariables();
   if(vars->size() == 0){
      lastError << "No global variables!";
      lastErrorInfo.type = SG_InternalError ; lastErrorInfo.fatal = true;
      return NULL;
   }
   //  BPatch_variableExpr *varExpr = NULL;
   BPatch_variableExpr *varExprGbl = NULL;
   for(unsigned int i = 0; i < vars->size(); ++i){
      const char *substr = strstr((*vars)[i]->getName(), mangledStub);
      if(substr != NULL){
         return (*vars)[i];            
      }
   }
   if(varExprGbl != NULL){
      return varExprGbl;
   }
   lastError << "Unable to find InstVar \"" << name << "\"";
   lastErrorInfo.type = SG_LookUpFailure; 
   lastErrorInfo.fatal = true; //should this be false?
   return NULL;
}
BPatch_snippet *SnippetGenerator::generateArrayRef(BPatch_snippet *base, BPatch_snippet *index){
   lastError.str() = "";
   if(base->getType() != NULL && base->getType()->getDataClass() != BPatch_dataArray){
      lastError << "Base of array reference is not an array";
      lastErrorInfo.type = SG_TypeError; 
      lastErrorInfo.fatal = true;
      return NULL;
   }
   if(index->getType() != NULL && index->getType()->getDataClass() != BPatch_scalar){
      lastError << "Array index is not a scalar";
      lastErrorInfo.type = SG_TypeError; 
      lastErrorInfo.fatal = true;
      return NULL;
   }

   BPatch_arithExpr *ref = new BPatch_arithExpr(BPatch_ref, *base, *index);
   if(ref == NULL){
      lastError << "Array reference cannot be generated - received null pointer";
      lastErrorInfo.type = SG_InternalError;
      lastErrorInfo.fatal = true;
      return NULL;
   }
   return ref;
}

BPatch_function *SnippetGenerator::findFunction(const char * name, std::vector<BPatch_snippet *> params){
   lastError.str() = "";
   
   std::vector<BPatch_function *> functions;
   
   if(NULL == image->findFunction(name, functions, false)){
      lastError << "Couldn't find function \'" << name << "\'";
      lastErrorInfo.type = SG_LookUpFailure; 
      lastErrorInfo.fatal = true;
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

   char errorHeader[256];
   getErrorBase(errorHeader, 256);
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
      lastErrorInfo.type = SG_LookUpFailure ; lastErrorInfo.fatal = true;
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
      lastErrorInfo.type = SG_ScopeViolation ;
      lastErrorInfo.fatal = true;
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
            lastErrorInfo.type = SG_InternalError;
            lastErrorInfo.fatal = true;
            return NULL;
        }      
        return new BPatch_tidExpr(image->getProcess());
      default:
         lastError << "Internal: Unrecognized SGContext!";
         lastErrorInfo.type = SG_InternalError;
         lastErrorInfo.fatal = true;
         return NULL;
   }
}

