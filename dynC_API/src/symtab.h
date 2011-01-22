//symtab.h

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "BPatch_snippet.h"
#include "BPatch_type.h"
#include "BPatch_point.h"

#include <map>
#include <utility>

typedef enum symbolKind {ST_userVar, ST_internalVar, ST_snippet, ST_boolean, ST_mutateeVar, ST_function, ST_cFunction};

enum Scope {LOCAL_, GLOBAL_MUTATEE_, GLOBAL_MUTATOR_};

enum varType {VT_string, VT_int, VT_long, VT_double, VT_bool};

enum insertActions {get_local_var};

class ST_Entry{

  public:
   ST_Entry(){};
   ~ST_Entry(){};
//variable constructors
   ST_Entry (const symbolKind kind_, BPatch_variableExpr *varExpr_, const BPatch_type *type_){   
      ST_Entry();
      kind = kind_;
      type = type_;
      varExpr = varExpr_;
   };

   ST_Entry (const symbolKind kind_, BPatch_variableExpr *varExpr_, const BPatch_type *type_, char *value_) {ST_Entry(kind_, varExpr_, type_); vtype = VT_string; varValue.sval = strdup(value_);}
   ST_Entry (const symbolKind kind_, BPatch_variableExpr *varExpr_, const BPatch_type *type_, int value_) {ST_Entry(kind_, varExpr_, type_); vtype = VT_int; varValue.ival = value_;};
   ST_Entry (const symbolKind kind_, BPatch_variableExpr *varExpr_, const BPatch_type *type_, long value_) {ST_Entry(kind_, varExpr_, type_); vtype = VT_long; varValue.lval = value_;};
   ST_Entry (const symbolKind kind_, BPatch_variableExpr *varExpr_, const BPatch_type *type_, double value_) {ST_Entry(kind_, varExpr_, type_); vtype = VT_double; varValue.dval = value_;};
   ST_Entry (const symbolKind kind_, BPatch_variableExpr *varExpr_, const BPatch_type *type_, bool value_) {ST_Entry(kind_, varExpr_, type_); vtype = VT_bool; varValue.bval = value_;};

//BPatch_function constructior

   ST_Entry (const symbolKind kind_, BPatch_function * function_) {ST_Entry(); kind = kind_; function = function_;};

//BPatch_snippet constructor
   ST_Entry (const symbolKind kind_, BPatch_snippet * snippet_) {ST_Entry(); kind = kind_; snippet = snippet_;};

//accessor methods

   symbolKind getKind() { return kind; };
   void setKind(symbolKind kind_) { kind = kind_; };
   
   const BPatch_type * getType() { return type; };
   void setType(BPatch_type * type_) { type = type_; };

   int getTimesCalled() { return timesCalled; };
   void setTimesCalled(int timesCalled_) { timesCalled = timesCalled_ ;};
  
   int getTimesModified() { return timesModified; };
   void setTimesModified(int timesModified_) { timesCalled = timesModified_ ;};
   
   bool getHasAbout() { return hasAbout; };
   void setHasAbout(bool hasAbout_) { hasAbout = hasAbout_; };
   
   bool getIsStatic() { return isStatic; };
   void setIsStatic(bool isStatic_) { isStatic = isStatic_; };
   
   bool getIsConst() { return isConst; };
   void setIsConst(bool isConst_) { isConst = isConst_; };
   
   Scope getScope() { return scope; };
   void setScope(Scope scope_) { scope = scope_; };
   
   char * getDeclaredInFile(char * buf, int len) { strncpy(declaredInFile, buf, len); return buf; };
   void setDeclaredInFile(char *declaredInFile_) { declaredInFile = strdup(declaredInFile_); };
   
   int getDeclaredOnLine() { return declaredOnLine; };
   void setDeclaredOnLine(int declaredOnLine_) { declaredOnLine = declaredOnLine_ ;};

   const std::vector<BPatch_point *> * getLocations() { return locations; };
   void setLocations(std::vector<BPatch_point *> * locations_) { locations = locations_; };
   void addLocation(BPatch_point * pt) { locations->push_back(pt); }
   void removeLocation(BPatch_point * pt) 
   { std::vector< BPatch_point * >::iterator it;
     it = std::find(locations->begin(), locations->end(), pt);
     locations->erase(it); }

   varType getVariabletype() { return vtype; };
   void setVariableType(varType vtype_) { vtype = vtype_; };

//user must free the returned string
   char * getSValue() { return strdup(varValue.sval); };
   void setValue(char *value_) { varValue.sval = strdup(value_); };
   
   int getIValue() { return varValue.ival; };
   void setValue(int value_) { varValue.ival = value_; };
   
   long getLValue() { return varValue.lval; };
   void setValue(long value_) { varValue.lval = value_; };
   
   double getDValue() { return varValue.dval; };
   void setValue(double value_) { varValue.dval = value_; };
   
   bool getBValue() { return varValue.bval; };
   void setValue(bool value_) { varValue.bval = value_; };

   

   BPatch_function * getFunction() { return function; };
   void setFunction(BPatch_function * function_) { function = function_; };

   BPatch_variableExpr * getVarExpr() { return varExpr; };
   void setVarExpr(BPatch_variableExpr * varExpr_) { varExpr = varExpr_; };

   BPatch_snippet * getSnippet(){ return snippet; };
   void setSnippet(BPatch_snippet * snippet_) { snippet = snippet_; };

   std::map<BPatch_snippet *, std::pair<insertActions, char *> > *getActionsOnInsert(){return actionsOnInsert;};
   void addActionOnInsert(BPatch_snippet * snippet, insertActions action, char *info){(*actionsOnInsert)[snippet] = std::make_pair(action, strdup(info));};


  private:

   //all   
   symbolKind kind;
   const BPatch_type *type;
   int timesCalled;
   int timesModified;
   bool hasAbout; //true if an about has been placed on the symbol
   bool isStatic;
   bool isConst;
   Scope scope;
   //_USERVAR, _SNIPPET, _BOOLEAN, _MUTATEEVAR, _CFUNCTION
   char *declaredInFile; //file where declared
   int declaredOnLine; //this is possible for mutattee vars (see symtabAPI pg 54)
   
   std::vector<BPatch_point *> *locations;
   std::map<BPatch_snippet *, std::pair<insertActions, char *> > *actionsOnInsert;
   varType vtype;     

   //_USERVAR, _MUTATEEVAR
   union varVal{
      char *sval;
      int ival;
      long lval;
      double dval;
      bool bval;
   } varValue;
   
   BPatch_function *function;
   BPatch_variableExpr *varExpr;
   BPatch_snippet *snippet;
};



class SymbolTable{

  public:

   bool put(char *name, ST_Entry entry) { 
      if (SymbolTable::contains(name)){
         return false;
      }
      table[name] = entry;
      return true;
   };

   ST_Entry * get(char *name){
      if (!SymbolTable::contains(name)){
         return NULL;
      }
      return & table[name];
   };

   bool contains(char *name) { return table.find(name) != table.end(); };
   
   bool empty(){ return table.empty(); };

   void clear(){ table.clear(); };
  private:
   std::map<char *, ST_Entry> table;

};


#endif
