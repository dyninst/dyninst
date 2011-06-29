#include "ParseCallbacks.h"
#include "PatchObject.h"
#include "PatchCFG.h"

using namespace Dyninst;
using namespace PatchAPI;

void PatchParseCallback::split_block_cb(ParseAPI::Block *first, ParseAPI::Block *second) {
   PatchBlock *p1 = _obj->getBlock(first, false);
   if (!p1) return;

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

void PatchParseCallback::add_block_cb(ParseAPI::Function *func, ParseAPI::Block *block) {
   PatchBlock *pb = _obj->getBlock(block, false);
   if (!pb) return;

   PatchFunction *pf = _obj->getFunc(func, false);
   if (!pf) return;

   pf->addBlock(pb);
}


