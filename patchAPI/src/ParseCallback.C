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
#include "ParseCallback.h"
#include "PatchCallback.h"
#include "PatchObject.h"
#include "PatchCFG.h"
#include "AddrSpace.h"

using namespace Dyninst;
using namespace PatchAPI;

void PatchParseCallback::split_block_cb(ParseAPI::Block *first, ParseAPI::Block *second) {

   // 1) we create blocks lazily, so do nothing if the first block doesn't exist yet
   // 2) have the object split the block, it will create the second block and add it
   // 3) tell the first block to split off the second block and all points inside of it
   // 4) have the function split the block and re-calculate its entry,exit,etc blocks
   // 5) trigger higher-level callbacks

   // 1)
   PatchBlock *p1 = _obj->getBlock(first, false);
   if (!p1) return;

   // 2)
   _obj->splitBlock(p1, second);

   // 3)
   PatchBlock *p2 = _obj->getBlock(second,false);
   assert(p2);
   p1->splitBlock(p2);

   // 4)
   std::vector<PatchFunction *> funcs;
   p1->getFuncs(std::back_inserter(funcs));
   for (unsigned i = 0; i < funcs.size(); ++i) {
      funcs[i]->splitBlock(p1, p2);
   }

   // 5)
   _obj->cb()->split_block(p1,p2);
}

void PatchParseCallback::destroy_cb(ParseAPI::Block *block) {
   PatchBlock *pb = _obj->getBlock(block,false);
   if (pb) {
       pb->destroyPoints();
       _obj->removeBlock(block);
       _obj->cb()->destroy(pb);
   }
}

void PatchParseCallback::destroy_cb(ParseAPI::Edge *edge) {
   bool inSrc = true;
   PatchObject *srcObj = _obj->addrSpace()->findObject(edge->src()->obj());
   PatchObject *dstObj = _obj->addrSpace()->findObject(edge->trg()->obj());
   PatchEdge *pe = srcObj->getEdge(edge,NULL,NULL,false);
   if (!pe) {
      pe = dstObj->getEdge(edge,NULL,NULL,false);
      inSrc = false;
   }
   // remove edge from both source and target objects
   if (inSrc) {
      srcObj->removeEdge(edge);
   }
   if (srcObj != dstObj) {
      dstObj->removeEdge(edge);
   }
   // destroy edge, can only do this once
   if (pe) {
      if (inSrc) srcObj->cb()->destroy(pe, srcObj);
      else       dstObj->cb()->destroy(pe, dstObj);
   }
}

void PatchParseCallback::destroy_cb(ParseAPI::Function *func) {
   PatchFunction *pf = _obj->getFunc(func,false);
   _obj->removeFunc(func);
   if (pf) {
       pf->destroyPoints();
       _obj->cb()->destroy(pf);
   }
}

void PatchParseCallback::remove_edge_cb(ParseAPI::Block *block, ParseAPI::Edge *edge, edge_type_t type) {
   // We get this callback before the edge is deleted; we need to tell the block that we 
   // nuked an edge.
   PatchObject *pbObj = _obj->addrSpace()->findObject(block->obj());
   PatchEdge *pe = pbObj->getEdge(edge, NULL, NULL, false);
   if (!pe) return;

   PatchBlock *pb = _obj->addrSpace()->findObject(block->obj())->getBlock(block, false);
   vector<PatchFunction*> funcs;
   assert(pb); // If we have an edge we better DAMN well have the block
   
   if (type == source) pb->removeSourceEdge(pe);
   else { 
      pb->removeTargetEdge(pe);
      // remove block from callBlocks, if this is the block's one only call edge
      int numCalls = pb->numCallEdges();
      if (numCalls == 1) {
         pb->getFuncs(std::back_inserter(funcs));
         for (vector<PatchFunction*>::iterator fit = funcs.begin(); fit != funcs.end(); fit++) {
            (*fit)->call_blocks_.erase(pb);
         }
      }
   }

   if (pe->points_.during) {
       if (funcs.empty()) {
          pb->getFuncs(std::back_inserter(funcs));
       }
       for (vector<PatchFunction*>::iterator fit = funcs.begin(); fit != funcs.end(); fit++) {
           (*fit)->remove(pe->points_.during);
       }
       pb->obj()->cb()->destroy(pe->points_.during);
       pe->points_.during = NULL;
   }
}

void PatchParseCallback::add_edge_cb(ParseAPI::Block *block, ParseAPI::Edge *edge, edge_type_t type) {
   PatchObject *pbObj = _obj->addrSpace()->findObject(block->obj());
    if(!pbObj) return; // e.g. sink block
   PatchBlock *pb = pbObj->getBlock(block, false);
   if (!pb) return; // We haven't created the block, so we'll get around to the edge later.
   
   // If we haven't requested edges then ignore and we'll get it later.
   if (type == source) {
       if (pb->srclist_.empty())
           return;
   }
   else 
       if (pb->trglist_.empty()) 
           return;

   ParseAPI::Block *block2 = (type == source) ? edge->src() : edge->trg();
    if(!block2->obj()) return;
   PatchObject *pb2Obj = _obj->addrSpace()->findObject(block2->obj());
   PatchBlock *pb2 = pb2Obj->getBlock(block2);
   assert(pb2);
   PatchEdge *pe = (type == source) ? pb2Obj->getEdge(edge, pb2, pb) : pbObj->getEdge(edge,pb,pb2);
   if (type == source) pb->addSourceEdge(pe, false);
   else pb->addTargetEdge(pe, false);
}

void PatchParseCallback::modify_edge_cb(ParseAPI::Edge *edge,
                                        ParseAPI::Block *block, edge_type_t type) {
   PatchEdge *pe = _obj->getEdge(edge, NULL, NULL, false);
   if (!pe) return; // No edge, so we'll get to it later. 

   PatchBlock *pb = _obj->getBlock(block, true);
   assert(pb); // We need the target. 

   if (type == source) {
      pe->src_ = pb;
   }
   else {
      pe->trg_ = pb;
   }
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
   _obj->cb()->add_block(pf,pb);
}

// returns the load address of the code object containing an absolute address
bool PatchParseCallback::absAddr(Address absolute, 
                                 Address & loadAddr, 
                                 ParseAPI::CodeObject *& codeObj)
{
    AddrSpace::ObjMap objs = _obj->addrSpace()->objMap();
    AddrSpace::ObjMap::iterator oit = objs.begin();
    for (; oit != objs.end(); oit++) {
        if (absolute > oit->second->codeBase()) {
            ParseAPI::CodeSource *cs = oit->first->cs();
            set<ParseAPI::CodeRegion*> regs;
            cs->findRegions(absolute - oit->second->codeBase(), regs);
            if (1 == regs.size()) {
                loadAddr = oit->second->codeBase();
                codeObj = const_cast<ParseAPI::CodeObject*>(oit->first);
                return true;
            }
        }
    }
    return false;
}

