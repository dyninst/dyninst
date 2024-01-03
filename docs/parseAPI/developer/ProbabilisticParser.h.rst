.. _`sec:ProbabilisticParser.h`:

ProbabilisticParser.h
#####################

.. cpp:namespace:: hd

.. code:: c

  #define WILDCARD_ENTRY_ID   0xaaaa
  #define NOARG               0xffff
  #define IMMARG              (NOARG-1)
  #define MEMARG              (NOARG-2)
  #define MULTIREG            (NOARG-3)
  #define CALLEESAVEREG       (NOARG-4)
  #define ARGUREG             (NOARG-5)
  #define OTHERREG            (NOARG-6)
  #define JUNK_OPCODE         0xffff
  #define ENTRY_SHIFT         32ULL
  #define ARG1_SHIFT          16ULL
  #define ARG2_SHIFT          0ULL
  #define ENTRY_SIZE          16ULL
  #define ARG_SIZE            16ULL
  #define ENTRY_MASK          (((uint64_t)(1<<ENTRY_SIZE)-1) << ENTRY_SHIFT)
  #define ARG1_MASK           (((uint64_t)(1<<ARG_SIZE)-1) << ARG1_SHIFT)
  #define ARG2_MASK           (((uint64_t)(1<<ARG_SIZE)-1) << ARG2_SHIFT)

.. cpp:class:: ProbabilityCalculator

  **Probabilistic gap parsing**

  .. cpp:member:: static clock_t totalClocks

  .. cpp:function:: ProbabilityCalculator(Dyninst::ParseAPI::CodeRegion *reg, Dyninst::ParseAPI::CodeSource *source, Dyninst::ParseAPI::Parser *parser, std::string model_spec)
  .. cpp:function:: double calcProbByMatchingIdioms(Dyninst::Address addr)
  .. cpp:function:: void calcProbByEnforcingConstraints()
  .. cpp:function:: double getFEPProb(Dyninst::Address addr)
  .. cpp:function:: bool isFEP(Dyninst::Address addr)
  .. cpp:function:: void prioritizedGapParsing()

.. cpp:struct:: IdiomTerm

  .. cpp:member:: unsigned short entry_id
  .. cpp:member:: unsigned short arg1
  .. cpp:member:: unsigned short arg2

  .. cpp:function:: IdiomTerm()
  .. cpp:function:: IdiomTerm(unsigned short a, unsigned short b, unsigned short c)
  .. cpp:function:: bool matchOpcode(unsigned short entry_id) const
  .. cpp:function:: bool match(const IdiomTerm &it) const
  .. cpp:function:: std::string human_format() const

.. cpp:function:: static IdiomTerm WILDCARD_TERM(WILDCARD_ENTRY_ID, NOARG, NOARG)

.. cpp:struct:: Idiom

  .. cpp:member:: std::vector<IdiomTerm> terms
  .. cpp:member:: double w
  .. cpp:member:: bool prefix

  .. cpp:function:: Idiom()
  .. cpp:function:: Idiom(std::string f, double weight, bool pre)
  .. cpp:function:: std::string human_format() const

.. cpp:class:: IdiomPrefixTree

  .. cpp:type:: std::vector<std::pair<IdiomTerm, IdiomPrefixTree*>> ChildrenType
  .. cpp:type:: dyn_hash_map<unsigned short, ChildrenType> ChildrenByEntryID

  .. cpp:function:: IdiomPrefixTree()
  .. cpp:function:: void addIdiom(const Idiom& idiom)
  .. cpp:function:: bool findChildrenWithOpcode(unsigned short entry_id, ChildrenType &ret)
  .. cpp:function:: bool findChildrenWithArgs(unsigned short arg1, unsigned short arg2, const ChildrenType &candidate, ChildrenType &ret)
  .. cpp:function:: bool isFeature()
  .. cpp:function:: bool isLeafNode()
  .. cpp:function:: double getWeight()
  .. cpp:function:: const ChildrenType* getChildrenByEntryID(unsigned short entry_id)
  .. cpp:function:: const ChildrenType* getWildCardChildren()

.. cpp:class:: IdiomModel

  .. cpp:function:: IdiomModel(std::string model_spec)
  .. cpp:function:: double getBias()
  .. cpp:function:: double getProbThreshold()
  .. cpp:function:: IdiomPrefixTree* getNormalIdiomTreeRoot()
  .. cpp:function:: IdiomPrefixTree* getPrefixIdiomTreeRoot()
