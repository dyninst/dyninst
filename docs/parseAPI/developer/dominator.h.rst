.. _`sec:dominator.h`:

dominator.h
###########

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:class:: dominatorBB

  .. cpp:member:: protected int dfs_no
  .. cpp:member:: protected int size
  .. cpp:member:: protected dominatorBB *semiDom
  .. cpp:member:: protected dominatorBB *immDom
  .. cpp:member:: protected dominatorBB *label
  .. cpp:member:: protected dominatorBB *ancestor
  .. cpp:member:: protected dominatorBB *parent
  .. cpp:member:: protected dominatorBB *child
  .. cpp:member:: protected Block *parseBlock
  .. cpp:member:: protected dominatorCFG *dom_cfg
  .. cpp:member:: protected std::set<dominatorBB *> bucket
  .. cpp:member:: protected vector<dominatorBB *> pred
  .. cpp:member:: protected vector<dominatorBB *> succ

  .. cpp:function:: dominatorBB(Block *bb, dominatorCFG *dc)
  .. cpp:function:: dominatorBB *eval()
  .. cpp:function:: void compress()
  .. cpp:function:: int sdno()

.. cpp:class:: dominatorCFG

  .. cpp:member:: protected std::unordered_map<Address, dominatorBB *> map_
  .. cpp:member:: protected const Function *func
  .. cpp:member:: protected vector<dominatorBB *> all_blocks
  .. cpp:member:: protected vector<dominatorBB *> sorted_blocks
  .. cpp:member:: protected int currentDepthNo
  .. cpp:member:: protected dominatorBB *entryBlock
  .. cpp:member:: protected dominatorBB *nullNode

  .. cpp:function:: protected void performComputation()
  .. cpp:function:: protected void depthFirstSearch(dominatorBB *v)
  .. cpp:function:: protected void eval(dominatorBB *v)
  .. cpp:function:: protected void link(dominatorBB *v, dominatorBB *w)
  .. cpp:function:: protected dominatorBB *parseToDomBB(Block *bb)

  .. cpp:function:: dominatorCFG(const Function *f)
  .. cpp:function:: void calcDominators()
  .. cpp:function:: void calcPostDominators()
