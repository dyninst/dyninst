.. _`sec:mapped_module.h`:

mapped_module.h
###############

pdmodule equivalent The internals tend to use images, while the
BPatch layer uses modules. On the other hand, "module" means
"compilation unit for the a.out, or the entire image for a
library". At some point this will need to be fixed, which will be a
major pain.

.. cpp:class:: mapped_module

  .. cpp:function:: static mapped_module *createMappedModule(mapped_object *obj, pdmodule *pdmod)
  .. cpp:function:: mapped_object *obj() const
  .. cpp:function:: pdmodule *pmod() const
  .. cpp:function:: const string &fileName() const
  .. cpp:function:: AddressSpace *proc() const
  .. cpp:function:: SymtabAPI::supportedLanguages language() const

    A lot of stuff shared with the internal module

  .. cpp:function:: const std::vector<func_instance *> &getAllFunctions()
  .. cpp:function:: const std::vector<int_variable *> &getAllVariables()
  .. cpp:function:: bool findFuncVectorByPretty(const std::string &funcname, std::vector<func_instance *> &funcs)
  .. cpp:function:: bool findFuncVectorByMangled(const std::string &funcname, std::vector<func_instance *> &funcs)

    Yeah, we can have multiple mangled matches -- for libraries there is a single module. Even if we went multiple,
    we might not have module information, and so we can get collisions.

  .. cpp:function:: bool findFuncsByAddr(const Dyninst::Address addr, std::set<func_instance *> &funcs)
  .. cpp:function:: bool findBlocksByAddr(const Dyninst::Address addr, std::set<block_instance *> &blocks)
  .. cpp:function:: void getAnalyzedCodePages(std::set<Dyninst::Address> &pages)
  .. cpp:function:: void dumpMangled(std::string prefix) const
  .. cpp:function:: std::string processDirectories(const std::string &fn) const
  .. cpp:function:: void addFunction(func_instance *func)
  .. cpp:function:: void addVariable(int_variable *var)
  .. cpp:function:: int_variable *createVariable(std::string name, Dyninst::Address offset, int size)
  .. cpp:function:: void remove(func_instance *func)
  .. cpp:function:: unsigned int getFuncVectorSize()
  .. cpp:member:: private pdmodule *internal_mod_
  .. cpp:member:: private mapped_object *obj_
  .. cpp:function:: private mapped_module()
  .. cpp:function:: private mapped_module(mapped_object *obj, pdmodule *pdmod)
  .. cpp:member:: private std::vector<func_instance *> everyUniqueFunction
  .. cpp:member:: private std::vector<int_variable *> everyUniqueVariable


.. code:: cpp

  paradyn might need it
  #define CHECK_ALL_CALL_POINTS

