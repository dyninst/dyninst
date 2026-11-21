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
// This class tracks the expanded assignments,
// and also provides several helper functions for manipulating ASTs
class SymbolicExpression {

    dyn_hash_map<Dyninst::Assignment::Ptr, AST::Ptr, Dyninst::Assignment::AssignmentPtrHasher> expandCache;

public:

    AST::Ptr SimplifyRoot(AST::Ptr ast, Dyninst::Address addr, bool keepMultiOne = false);
    AST::Ptr SimplifyAnAST(AST::Ptr ast, Dyninst::Address addr, bool keepMultiOne = false);
    static AST::Ptr SubstituteAnAST(AST::Ptr ast, const std::map<AST::Ptr, AST::Ptr>& aliasMap);
    static AST::Ptr DeepCopyAnAST(AST::Ptr ast);
    static bool ContainAnAST(AST::Ptr root, AST::Ptr check);
    bool ReadMemory(Dyninst::Address addr, uint64_t &val, int size);
    Dyninst::ParseAPI::CodeSource* cs; 
    // For archive, there are overlapping regions.
    // Need to use the right region.
    Dyninst::ParseAPI::CodeRegion* cr;
    std::pair<AST::Ptr, bool> ExpandAssignment(Dyninst::Assignment::Ptr, bool keepMultiOne = false);

    //On x86 and x86-64, the value of PC is post-instruction, 
    // which is the current address plus the length of the instruction.
    // On ARMv8, the value of PC is pre-instruction,
    // which is the current address
    static Dyninst::Address PCValue(Dyninst::Address cur, size_t insnSize, Dyninst::Architecture a);


};

#endif
