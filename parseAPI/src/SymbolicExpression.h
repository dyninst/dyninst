#ifndef SYMBOLIC_EXPRESSION_H
#define SYMBOLIC_EXPRESSION_H

#include "DynAST.h"
#include "Absloc.h"
#include "CodeSource.h"
#include <stddef.h>
#include <stdint.h>
#include <utility>
#include <map>
using Dyninst::AST;
using namespace Dyninst;

class SymbolicExpression {

    dyn_hash_map<Assignment::Ptr, AST::Ptr, Assignment::AssignmentPtrHasher> expandCache;

public:

    AST::Ptr SimplifyRoot(AST::Ptr ast, Address addr, bool keepMultiOne = false);
    AST::Ptr SimplifyAnAST(AST::Ptr ast, Address addr, bool keepMultiOne = false);
    static AST::Ptr SubstituteAnAST(AST::Ptr ast, const std::map<AST::Ptr, AST::Ptr>& aliasMap);
    static AST::Ptr DeepCopyAnAST(AST::Ptr ast);
    static bool ContainAnAST(AST::Ptr root, AST::Ptr check);
    bool ReadMemory(Address addr, uint64_t &val, int size);
    ParseAPI::CodeSource* cs; 
    ParseAPI::CodeRegion* cr;
    std::pair<AST::Ptr, bool> ExpandAssignment(Assignment::Ptr, bool keepMultiOne = false);

    static Address PCValue(Address cur, size_t insnSize, Architecture a);


};

#endif
