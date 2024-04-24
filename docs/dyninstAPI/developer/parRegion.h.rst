.. _`sec:parRegion.h`:

parRegion.h
###########

.. cpp:class:: image_parRegion : public codeRange

  .. cpp:function:: image_parRegion(parse_func *imageFunc)
  .. cpp:function:: image_parRegion(Dyninst::Address firstOffset, parse_func *imageFunc)
  .. cpp:function:: Dyninst::Address firstInsnOffset() const
  .. cpp:function:: void setLastInsn(Dyninst::Address last)
  .. cpp:function:: Dyninst::Address lastInsnOffset() const
  .. cpp:function:: Dyninst::Address getSize() const
  .. cpp:function:: Dyninst::Address get_address() const
  .. cpp:function:: unsigned int get_size() const
  .. cpp:function:: parRegType getRegionType()
  .. cpp:function:: void setRegionType(parRegType rt)
  .. cpp:function:: const parse_func *getAssociatedFunc() const
  .. cpp:function:: void setParentFunc(parse_func *parentFunc)
  .. cpp:function:: parse_func *getParentFunc()
  .. cpp:function:: void setClause(const char *key, int value)
  .. cpp:function:: int getClause(const char *key)
  .. cpp:function:: void setClauseLoc(const char *key, Dyninst::Address value)
  .. cpp:function:: Dyninst::Address getClauseLoc(const char *key)
  .. cpp:function:: void printDetails()
  .. cpp:function:: void decodeClauses(int bitmap)
  .. cpp:member:: private parse_func *regionIf_
  .. cpp:member:: private parse_func *parentIf_
  .. cpp:member:: private Dyninst::Address firstInsnOffset_
  .. cpp:member:: private Dyninst::Address lastInsnOffset_
  .. cpp:member:: private parRegType regionType
  .. cpp:member:: private std::map<const char *, int, ltstr> clauses
  .. cpp:member:: private std::map<const char *, Dyninst::Address, ltstr> clause_locations

.. cpp:class:: int_parRegion

  .. cpp:function:: int_parRegion(image_parRegion *ip, Dyninst::Address baseAddr, func_instance *)
  .. cpp:function:: ~int_parRegion()
  .. cpp:function:: Dyninst::Address firstInsnAddr()
  .. cpp:function:: Dyninst::Address endAddr()
  .. cpp:function:: const image_parRegion *imagePar() const
  .. cpp:function:: void printDetails()
  .. cpp:function:: const func_instance *intFunc()
  .. cpp:function:: int getClause(const char *key)
  .. cpp:function:: Dyninst::Address getClauseLoc(const char *key)
  .. cpp:function:: int replaceOMPParameter(const char *key, int value)
  .. cpp:member:: Dyninst::Address addr_

    Absolute address of start of region

  .. cpp:member:: Dyninst::Address endAddr_

    Address of end of region

  .. cpp:member:: func_instance *intFunc_
  .. cpp:member:: image_parRegion *ip_

.. cpp:struct:: ltstr

  .. cpp:function:: bool operator()(const char *s1, const char *s2) const

