.. _`sec:DynCommon.h`:

DynCommon.h
###########

Shortcuts for type casting

.. code:: cpp

  #define DYN_CAST(type, obj)  boost::dynamic_pointer_cast<type>(obj)

  #define SCAST_MO(o) static_cast<mapped_object*>(o)
  #define SCAST_EI(e) static_cast<edge_instance*>(e)
  #define SCAST_BI(b) static_cast<block_instance*>(b)
  #define SCAST_PB(b) static_cast<parse_block*>(b)
  #define SCAST_PF(f) static_cast<parse_func*>(f)
  #define SCAST_FI(f) static_cast<func_instance*>(f)
