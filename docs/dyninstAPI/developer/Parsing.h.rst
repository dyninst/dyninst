.. _`sec:Parsing.h`:

Parsing.h
#########


.. cpp:class:: DynCFGFactory : public Dyninst::ParseAPI::CFGFactory

  .. cpp:function:: DynCFGFactory(image *im)
  .. cpp:function:: ~DynCFGFactory()
  .. cpp:function:: Dyninst::ParseAPI::Function *mkfunc(Dyninst::Address addr, FuncSource src, std::string name, Dyninst::ParseAPI::CodeObject *obj, Dyninst::ParseAPI::CodeRegion *reg, Dyninst::InstructionSource *isrc)
  .. cpp:function:: Dyninst::ParseAPI::Block *mkblock(Dyninst::ParseAPI::Function *f, Dyninst::ParseAPI::CodeRegion *r, Dyninst::Address addr)
  .. cpp:function:: Dyninst::ParseAPI::Edge *mkedge(Dyninst::ParseAPI::Block *src, Dyninst::ParseAPI::Block *trg, EdgeTypeEnum type)
  .. cpp:function:: Dyninst::ParseAPI::Block *mksink(Dyninst::ParseAPI::CodeObject *obj, Dyninst::ParseAPI::CodeRegion *r)
  .. cpp:function:: void dump_stats()
  .. cpp:member:: private boost::mutex _mtx
  .. cpp:member:: private image *_img
  .. cpp:member:: private std::vector<int> _func_allocs
  .. cpp:member:: private std::vector<int> _edge_allocs
  .. cpp:member:: private int _block_allocs
  .. cpp:member:: private int _sink_block_allocs
  .. cpp:function:: private void _record_func_alloc(Dyninst::ParseAPI::FuncSource fs)
  .. cpp:function:: private void _record_edge_alloc(Dyninst::ParseAPI::EdgeTypeEnum et, bool sink)
  .. cpp:function:: private void _record_block_alloc(bool sink)


.. cpp:class:: DynParseCallback : public Dyninst::ParseAPI::ParseCallback

  .. cpp:function:: DynParseCallback(image *img)
  .. cpp:function:: ~DynParseCallback()

  ......

  .. rubric::
    Defensive and exploratory mode callbacks

  .. cpp:function:: protected virtual void abruptEnd_cf(Dyninst::Address, Dyninst::ParseAPI::Block *, default_details *)
  .. cpp:function:: protected virtual void newfunction_retstatus(Dyninst::ParseAPI::Function *)
  .. cpp:function:: protected virtual void patch_nop_jump(Dyninst::Address)
  .. cpp:function:: protected virtual bool hasWeirdInsns(const Dyninst::ParseAPI::Function *) const
  .. cpp:function:: protected virtual void foundWeirdInsns(Dyninst::ParseAPI::Function *)

  ......

  .. rubric::
    Other callbacks

  .. cpp:function:: protected virtual void interproc_cf(Dyninst::ParseAPI::Function *, Dyninst::ParseAPI::Block *, Dyninst::Address, interproc_details *)
  .. cpp:function:: protected virtual void overlapping_blocks(Dyninst::ParseAPI::Block *, Dyninst::ParseAPI::Block *)
  .. cpp:function:: protected virtual bool updateCodeBytes(Dyninst::Address target)

    updates if needed

  .. cpp:function:: protected virtual void split_block_cb(Dyninst::ParseAPI::Block *, Dyninst::ParseAPI::Block *)

    needed for defensive mode

  .. cpp:function:: protected virtual void destroy_cb(Dyninst::ParseAPI::Block *)
  .. cpp:function:: protected virtual void destroy_cb(Dyninst::ParseAPI::Edge *)
  .. cpp:function:: protected virtual void destroy_cb(Dyninst::ParseAPI::Function *)
  .. cpp:function:: protected virtual void remove_edge_cb(Dyninst::ParseAPI::Block *, Dyninst::ParseAPI::Edge *, edge_type_t)
  .. cpp:function:: protected virtual void remove_block_cb(Dyninst::ParseAPI::Function *, Dyninst::ParseAPI::Block *)
  .. cpp:function:: protected void instruction_cb(Dyninst::ParseAPI::Function *, Dyninst::ParseAPI::Block *, Dyninst::Address, insn_details *)
  .. cpp:member:: private image *_img
