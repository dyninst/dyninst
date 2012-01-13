/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#include "CFGModifier.h"
#include "CFG.h"
#include "InstructionSource.h"
#include "CodeObject.h"
#include "ParseCallback.h"
#include "ParseData.h"
#include "Parser.h"

using namespace Dyninst;
using namespace ParseAPI;


/* If target is NULL, user is requesting a redirect to the sink block
 * (create edge only if source block doesn't have a sink edge of the 
 * same type already) */
bool CFGModifier::redirect(Edge *edge, Block *target) {
   // What happens if we try a redirect to the sink? I'm
   // thinking fail, as it's a virtual block.
   bool linkToSink = false;
   if (!edge) return false;
   if (!target) {
      target = edge->src()->obj()->parser->_sink;
      linkToSink = true;
   }
   if (edge->trg() == target) return true;

   // Have to stay within the same CodeObject.
   // TODO: Kevin claims we don't. 
   if (edge->trg()->obj() != target->obj()) return false;

   std::vector<Function *> modifiedFuncs;
   if (!edge->interproc()) {
      edge->src()->getFuncs(modifiedFuncs);
   }

   // if the source block has a sink edge of the same type, remove this edge
   bool hasSink = false;
   if (linkToSink) {
      const Block::edgelist & trgs = edge->src()->targets();
      for (Block::edgelist::iterator titer = trgs.begin(); titer != trgs.end(); titer++) {
         if ((*titer)->sinkEdge() && (*titer)->type() == edge->type()) {
            hasSink = true;
            break;
         }
      }
   }

   if (hasSink) { // sink & src are in same object, so remove edge from src->obj()
      Block *src = edge->src();
      CodeObject *obj = src->obj();
      obj->_pcb->removeEdge(src, edge, ParseCallback::target);
      obj->_pcb->removeEdge(edge->trg(), edge, ParseCallback::source); 
      edge->uninstall();
      Edge::destroy(edge,obj);
   }
   else {
      // Okay, that's done. Let's rock.
      // Pull edge from the old target's source list;
      // Add it to the new target's source list;
      // Queue up callbacks.

      if (!edge->sinkEdge()) {
         Block *oldTarget = edge->trg();
         oldTarget->obj()->_pcb->removeEdge(oldTarget, edge, ParseCallback::source);
         oldTarget->removeSource(edge);
         if (linkToSink) {
            edge->_type._sink = 1;
         }
      }
      else {
         // No longer a sink edge!
         edge->_type._sink = 0;
      }

      edge->_target = target;
      target->addSource(edge);
      target->obj()->_pcb->addEdge(target, edge, ParseCallback::source);
      edge->src()->obj()->_pcb->modifyEdge(edge, target, ParseCallback::target);
   }


   // If this is an intraprocedural edge it may have changed function boundaries, so we 
   // mark any function that contains the source block as out of date. 
   for (unsigned i = 0; i < modifiedFuncs.size(); ++i) {
      modifiedFuncs[i]->invalidateCache();
   }
   return true;
}

Block *CFGModifier::split(Block *b, Address a, bool trust, Address newlast) {
   if (!trust) {
      // Need to check whether this is a valid instruction offset
      // in the block. 
      assert(0 && "Implement me!");
      return NULL;
   }
   if (!b) return NULL;
   if (a < b->start()) return NULL;
   if (a > b->end()) return NULL;

   // This function is substantially similar to Parser.C's split_block;
   // however, there are minor differences and the more important point
   // that I don't want to mess with it. 
   
   // 1) Remove the old block from any lookup data structures
   // 2) Create a new "target" block, move all targets to the new
   //    block, and connect the two with a fallthrough edge 
   // 3) Add both blocks to the lookup data structures
   // 4) Add the new block to all functions that contain the original.

   std::vector<Function *> funcs;
   b->getFuncs(funcs);

   // 1)
   region_data *rd = b->obj()->parser->_parse_data->findRegion(b->region());
   assert(rd);
   rd->blocksByRange.remove(b);

   // 2a)
   Block *ret = b->obj()->_fact->_mkblock(funcs[0], b->region(), a);
   ret->_end = b->_end;
   ret->_lastInsn = b->_lastInsn;
   ret->_parsed = true;
   
   b->_end = a;
   b->_lastInsn = newlast;


   // 2b)
   for (vector<Edge *>::iterator iter = b->_targets.begin(); 
        iter != b->_targets.end(); ++iter) {
      b->obj()->_pcb->removeEdge(b, *iter, ParseCallback::target);
      (*iter)->_source = ret;
      ret->_targets.push_back(*iter);
      b->obj()->_pcb->addEdge(ret, *iter, ParseCallback::target);
   }
   b->_targets.clear();

   // 2c)
   Edge *ft = b->obj()->_fact->_mkedge(b, ret, FALLTHROUGH);
   ft->_type._sink = false;
   b->_targets.push_back(ft);
   b->obj()->_pcb->addEdge(b, ft, ParseCallback::target);
   ret->_sources.push_back(ft);
   b->obj()->_pcb->addEdge(ret, ft, ParseCallback::source);

   // 3)
   rd->blocksByRange.insert(b);
   rd->blocksByRange.insert(ret);

   // 4)
   for (std::vector<Function *>::iterator iter = funcs.begin();
        iter != funcs.end(); ++iter) {
      (*iter)->_cache_valid = false;
      (*iter)->finalize();
      b->obj()->_pcb->addBlock(*iter, ret);
   }

   b->obj()->_pcb->splitBlock(b, ret);

   return ret;
}

bool CFGModifier::remove(vector<Block*> &blks, bool force) { 

   malware_cerr << "removing "<< blks.size() << " ParseAPI blocks" << endl; 
   if (blks.empty()) return false;

   set<Function*> allFuncs;
   map<Block*,vector<Function*> > blocksToFuncs;
   for (vector<Block*>::iterator bit = blks.begin(); bit != blks.end(); bit++) {
      (*bit)->getFuncs(blocksToFuncs[*bit]);
   }

   // 1) Remove from containing functions
   // 2) Remove all source edges (and clear up src func's _call_edges)
   // 3) Remove all target edges. 
   // 4) Remove from Parser data structures;
   // 5) Destroy the block and related edges
   // 6) Finalize owner funcs so that they re-build their extents

   for (vector<Block*>::iterator bit = blks.begin(); bit != blks.end(); bit++) {
      Block *b = *bit;
      ParseCallbackManager *pcb = b->obj()->_pcb;
      vector<Edge*> deadEdges;

      // 1)
      for (std::vector<Function *>::iterator iter = blocksToFuncs[b].begin(); 
           iter != blocksToFuncs[b].end(); ++iter) 
      {
         Function *func = *iter;
         pcb->removeBlock(func, b);
         func->removeBlock(b);
         if (b->_func_cnt > 0) {// remove func if count is not 0, in which case 
                                // we haven't finalized the block
             b->removeFunc(func);
         }
         b->obj()->parser->_parse_data->remove_extents(func->_extents);
         func->_extents.clear();
         allFuncs.insert(func);
      }
      
      // 2)
      if (!b->_sources.empty()) {
         if (!force) return false;

         for (std::vector<Edge *>::iterator iter = b->_sources.begin();
              iter != b->_sources.end(); ++iter) 
         {
            Edge *edge = *iter;
            deadEdges.push_back(edge);
            // callbacks
            pcb->removeEdge(b, edge, ParseCallback::source);
            edge->src()->obj()->_pcb->removeEdge(edge->src(), edge, ParseCallback::target);
            // clear up _call_edges vector in the caller function         
            if (edge->type() == CALL) {
               std::vector<Function *> funcs;
               edge->src()->getFuncs(funcs);
               for (unsigned k = 0; k < funcs.size(); ++k) {
                   funcs[k]->_call_edges.erase(edge);
               }
            }
            // remove edge from source block
            edge->src()->removeTarget(edge);
         }
      }

      // 3)
      for (std::vector<Edge *>::iterator iter = b->_targets.begin();
           iter != b->_targets.end(); ++iter) 
      {
         Edge *edge = *iter;
         deadEdges.push_back(edge);
         pcb->removeEdge(b, edge, ParseCallback::target);
         ParseCallbackManager *pcbTrg = edge->trg()->obj()->_pcb;
         if (!edge->sinkEdge()) {
            pcbTrg->removeEdge(edge->trg(), edge, ParseCallback::source);
         } else {
            // we don't actually wire up edges to the sink block in the PatchAPI, 
            // but I'm not sure why not, so I'll keep this here in case that
            // changes in the future
            pcbTrg->removeEdge(edge->trg(), edge, ParseCallback::source);
         }
         edge->trg()->removeSource(edge); // even sink edge has source list
      }

      // 4)
      region_data *rd = b->obj()->parser->_parse_data->findRegion(b->region());
      assert(rd);
      rd->blocksByRange.remove(b);
      rd->blocksByAddr.erase(b->start());

      // 5)
      CFGFactory *fact = b->obj()->fact();
      for (vector<Edge*>::iterator eit = deadEdges.begin(); eit != deadEdges.end(); eit++) {
          pcb->destroy(*eit, fact);
      }
      pcb->destroy(b, fact);
   }

   // 6)
   for (std::set<Function *>::iterator iter = allFuncs.begin(); 
        iter != allFuncs.end(); ++iter) 
   {
      Function *func = *iter;
      func->_cache_valid = false;
      func->finalize();
   }

   return true;
}

bool CFGModifier::remove(Function *f) {
   // Remove this function and all of its blocks; 
   // actually refcount the blocks one lower. 
   vector<Block*> destroyBs;

   // remove blocks from func, store unshared block list for destruction
   vector<Block*> destBs;
   Function::blocklist blocks = f->blocks();
   for (Function::blocklist::iterator iter = blocks.begin(); 
        iter != blocks.end(); ++iter) 
   {
      (*iter)->removeFunc(f);
      if ((*iter)->_func_cnt == 0) {
          destBs.push_back(*iter);
      }
   }

   // destroy function
   f->obj()->destroy(f);

   // destroy function blocks
   CFGModifier::remove(destBs,true);

   return true;
}

InsertedRegion *CFGModifier::insert(CodeObject *obj, 
                         Address base, void *data, 
                         unsigned size) {
   cerr << "Inserting new code: " << hex << (unsigned) (*((unsigned *)data)) << dec << endl;

   // As per Nate's suggestion, we're going to add this data as a new
   // Region in the CodeObject. 
   Architecture arch = obj->cs()->getArch();

   InsertedRegion *newRegion = new InsertedRegion(base, data, size, arch);

   obj->cs()->addRegion(newRegion);
   obj->parse(newRegion, base, true);

   return newRegion;
}

InsertedRegion::InsertedRegion(Address b, void *d, unsigned s, Architecture arch) : 
   base_(b), buf_(NULL), size_(s), arch_(arch) {
   buf_ = malloc(s);
   assert(buf_);
   memcpy(buf_, d, s);
};

