#include "ParseCallback.h"
#include "PatchObject.h"
#include "PatchCFG.h"
#include "AddrSpace.h"

using namespace Dyninst;
using namespace PatchAPI;

void PatchParseCallback::split_block_cb(ParseAPI::Block *first, ParseAPI::Block *second) {
   PatchBlock *p1 = _obj->getBlock(first, false);
   if (!p1) {
       assert(0);//KEVINTEST: isn't this a bad case? 
       return;
   }

   _obj->splitBlock(p1, second);
}

void PatchParseCallback::destroy_cb(ParseAPI::Block *block) {
   _obj->removeBlock(block);
}

void PatchParseCallback::destroy_cb(ParseAPI::Edge *edge) {
   _obj->removeEdge(edge);
}

void PatchParseCallback::destroy_cb(ParseAPI::Function *func) {
   _obj->removeFunc(func);
}

void PatchParseCallback::remove_edge_cb(ParseAPI::Block *block, ParseAPI::Edge *edge, edge_type_t type) {
   // We get this callback before the edge is deleted; we need to tell the block that we 
   // nuked an edge. 
   PatchEdge *pe = _obj->getEdge(edge, NULL, NULL, false);
   if (!pe) return;

   PatchBlock *pb = _obj->getBlock(block, false);
   assert(pb); // If we have an edge we better DAMN well have the block
   
   if (type == source) pb->removeSourceEdge(pe);
   else pb->removeTargetEdge(pe);

}

void PatchParseCallback::add_edge_cb(ParseAPI::Block *block, ParseAPI::Edge *edge, edge_type_t type) {
   PatchBlock *pb = _obj->getBlock(block, false);
   if (!pb) return; // We haven't created the block, so we'll get around to the edge later.
   
   // If we haven't requested edges then ignore and we'll get it later.
   if ((type == source) && (pb->srclist_.empty())) return;
   else if (pb->trglist_.empty()) return;

   PatchBlock *pb2 = (type == source) ? _obj->getBlock(edge->src()) : _obj->getBlock(edge->trg());
   assert(pb2);
   PatchEdge *pe = _obj->getEdge(edge, pb, pb2);
   if (type == source) pb->addSourceEdge(pe, false);
   else pb->addTargetEdge(pe, false);
}

void PatchParseCallback::remove_block_cb(ParseAPI::Function *func, ParseAPI::Block *block) {
   PatchBlock *pb = _obj->getBlock(block, false);
   if (!pb) return;

   PatchFunction *pf = _obj->getFunc(func, false);
   if (!pf) return;

   pf->removeBlock(pb);
}

/* Adds blocks lazily, basically does nothing unless block and function have already
   been created, in which case it adds the block to the function */
void PatchParseCallback::add_block_cb(ParseAPI::Function *func, ParseAPI::Block *block) {
   PatchBlock *pb = _obj->getBlock(block, false);
   if (!pb) return; 

   PatchFunction *pf = _obj->getFunc(func, false);
   if (!pf) return; 

   pf->addBlock(pb);
}

// returns the load address of the code object containing an absolute address
bool PatchParseCallback::absAddr(Address absolute, 
                                 Address & loadAddr, 
                                 ParseAPI::CodeObject *& codeObj)
{
    AddrSpace::ObjSet objs = _obj->addrSpace()->objSet();
    AddrSpace::ObjSet::iterator oit = objs.begin();
    for (; oit != objs.end(); oit++) {
        if (absolute > (*oit)->codeBase()) {
            ParseAPI::CodeSource *cs = (*oit)->co()->cs();
            set<ParseAPI::CodeRegion*> regs;
            cs->findRegions(absolute - (*oit)->codeBase(), regs);
            if (1 == regs.size()) {
                loadAddr = (*oit)->codeBase();
                codeObj = (*oit)->co();
                return true;
            }
        }
    }
    return false;
}

