/* PatchAPI has a CFG that it uses to specify instrumentation
   and modification. Users may extend this CFG by deriving
   their own classes, or annotate it with their own data. To
   inform users when the CFG changes we provide a callback
   interface. */

#if !defined(_PATCH_CALLBACK_H_)
#define _PATCH_CALLBACK_H_

#include <vector>


namespace Dyninst {
namespace PatchAPI {

class PatchObject;
class PatchFunction;
class PatchBlock;
class PatchEdge;
class Point;

class PatchCallback {

  public:
  PatchCallback() : batching_(false) {};
   virtual ~PatchCallback() {};

   typedef enum {
      source,
      target } edge_type_t;

   // Users override these to provide their callbacks
  protected:
   virtual void destroy_cb(PatchBlock *) {};
   virtual void destroy_cb(PatchEdge *) {};
   virtual void destroy_cb(PatchFunction *) {};
   virtual void destroy_cb(PatchObject *) {};


   virtual void create_cb(PatchBlock *) {};
   virtual void create_cb(PatchEdge *) {};
   virtual void create_cb(PatchFunction *) {};
   virtual void create_cb(PatchObject *) {};

   // Some more abstract ones
   virtual void split_block_cb(PatchBlock *, PatchBlock *) {};

   virtual void remove_edge_cb(PatchBlock *, PatchEdge *, edge_type_t) {};
   virtual void add_edge_cb(PatchBlock *, PatchEdge *, edge_type_t) {};

   virtual void remove_block_cb(PatchFunction *, PatchBlock *) {};
   virtual void add_block_cb(PatchFunction *, PatchBlock *) {};

   // Points
   virtual void destroy_cb(Point *) {};
   virtual void create_cb(Point *) {};
   // If we split a block, we may change the block a Point belongs to. 
   virtual void change_cb(Point *, PatchBlock *, PatchBlock *) {};

  public:
   // And these methods are used by PatchAPI and should not be overridden.
   void batch_begin();
   void batch_end();

   void destroy(PatchBlock *);
   void destroy(PatchEdge *);
   void destroy(PatchFunction *);
   void destroy(PatchObject *);

   void create(PatchBlock *); 
   void create(PatchEdge *); 
   void create(PatchFunction *); 
   void create(PatchObject *); 

   void split_block(PatchBlock *, PatchBlock *);
   void remove_edge(PatchBlock *, PatchEdge *, edge_type_t);
   void add_edge(PatchBlock *, PatchEdge *, edge_type_t);
   
   void remove_block(PatchFunction *, PatchBlock *);
   void add_block(PatchFunction *, PatchBlock *);

   void destroy(Point *);
   void create(Point *);
   void change(Point *, PatchBlock *, PatchBlock *);


  private:

   bool batching_;

   typedef enum { removed, added } mod_t;
   struct BlockMod {
      PatchBlock *block;
      PatchEdge *edge;
      edge_type_t type;
      mod_t mod;
   BlockMod(PatchBlock *b, PatchEdge *e, edge_type_t t, mod_t m) : block(b), edge(e), type(t), mod(m) {};
   };

   struct FuncMod {
      PatchFunction *func;
      PatchBlock *block;
      mod_t mod;
   FuncMod(PatchFunction *f, PatchBlock *b, mod_t m) : func(f), block(b), mod(m) {};
   };

   struct PointMod {
      Point *point;
      PatchBlock *old_block;
      PatchBlock *new_block;
   PointMod(Point *p, PatchBlock *ob, PatchBlock *nb) : point(p), old_block(ob), new_block(nb) {};
   };

   typedef std::pair<PatchBlock *, PatchBlock *> BlockSplit;
   std::vector<PatchEdge *> destroyedEdges_;
   std::vector<PatchBlock *> destroyedBlocks_;
   std::vector<PatchFunction *> destroyedFuncs_;
   std::vector<PatchObject *> destroyedObjects_;

   std::vector<PatchEdge *> createdEdges_;
   std::vector<PatchBlock *> createdBlocks_;
   std::vector<PatchFunction *> createdFuncs_;
   std::vector<PatchObject *> createdObjects_;

   std::vector<BlockMod> blockMods_;
   std::vector<FuncMod> funcMods_;
   std::vector<BlockSplit> blockSplits_;

   std::vector<Point *> destroyedPoints_;
   std::vector<Point *> createdPoints_;
   std::vector<PointMod> pointMods_;
};

};
};

#endif
