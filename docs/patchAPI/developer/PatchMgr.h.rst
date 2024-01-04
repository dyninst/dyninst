.. _`sec-dev:PatchMgr.h.rst`:

.. cpp:namespace:: Dyninst::PatchAPI::dev

PatchMgr.h
##########

Notes
=====

The basic logic of finding a point is outlined in the pseudocode below.

.. code-block:: cpp
    
  if (point is in the buffer) {
   return point;
  }

  if(!create) return NULL;

  create point

  if (point creation fails) return NULL;

  put the point in the buffer
