.. _`sec:dyninstAPI_RT.h`:

dyninstAPI_RT.h
###############

Standard instrumentation functions that are provided by the run-time instrumentation layer.


.. code:: c

  /*
   * Define the size of the per process data area.
   *
   *  This should be a power of two to reduce paging and caching shifts.
   *  Note that larger sizes may result in requiring longjumps within
   *  mini-trampolines to reach within this area.
   */

  #if !defined(target_smallmem)
  # define SYN_INST_BUF_SIZE (1024*1024*4)
  #else
  # define SYN_INST_BUF_SIZE (1024*1024*1)
  #endif

  #define DYNINST_BREAKPOINT_SIGNUM (SIGRTMIN+4)

  #if !defined(DYNINST_SINGLETHREADED)
  # define DYNINST_SINGLETHREADED -128
  #endif
  #define DYNINST_TRACEPIPE_ERRVAL -1
  #define DYNINST_PRINTF_ERRVAL -2
  #define DYNINST_NOT_IN_HASHTABLE ((unsigned)-1)
  #define TRAP_HEADER_SIG 0x759191D6
  #define DT_DYNINST 0x6D191957
  #define RTprintf  if (DYNINSTdebugPrintRT) printf
  #define TARGET_CACHE_WIDTH 128
  #define TARGET_CACHE_WAYS 2
  #define THREAD_AWAITING_DELETION -2
  #define ERROR_STRING_LENGTH 256
  #define MAX_MEMORY_MAPPER_ELEMENTS 1024

.. c:type::unsigned char RT_Boolean 

  If we must make up a boolean type, we should make it unique.

.. c:var:: static const RT_Boolean RT_TRUE=1
.. c:var:: static const RT_Boolean RT_FALSE=0

.. c:var:: extern char gLoadLibraryErrorString[]
.. c:var:: extern void *gBRKptr

.. c:struct:: DYNINST_bootstrapStruct

  .. c:member:: int event

  Takes on one of these values:

    |  0 --> nothing
    |  1 --> end of DYNINSTinit (normal)
    |  2 --> end of DYNINSTinit (forked process)
    |  3 --> start of DYNINSTexec (before exec)

  .. c:member:: int pid
  .. c:member:: int ppid

    parent of forked process

.. c:enum:: DYNINST_synch_event_t

  .. c:enumerator:: DSE_undefined
  .. c:enumerator:: DSE_forkEntry
  .. c:enumerator:: DSE_forkExit
  .. c:enumerator:: DSE_execEntry
  .. c:enumerator:: DSE_execExit
  .. c:enumerator:: DSE_exitEntry
  .. c:enumerator:: DSE_loadLibrary
  .. c:enumerator:: DSE_lwpExit
  .. c:enumerator:: DSE_snippetBreakpoint
  .. c:enumerator:: DSE_stopThread
  .. c:enumerator:: DSE_userMessage
  .. c:enumerator:: DSE_dynFuncCall

.. c:var:: extern int DYNINSTdebugPrintRT

  control run-time lib debug/trace prints

.. c:enum:: rtBPatch_asyncEventType

  .. c:enumerator:: rtBPatch_nullEvent
  .. c:enumerator:: rtBPatch_newConnectionEvent
  .. c:enumerator:: rtBPatch_internalShutDownEvent
  .. c:enumerator:: rtBPatch_threadCreateEvent
  .. c:enumerator:: rtBPatch_threadDestroyEvent
  .. c:enumerator:: rtBPatch_dynamicCallEvent
  .. c:enumerator:: rtBPatch_userEvent

.. c:function:: const char *asyncEventType2str(rtBPatch_asyncEventType)

.. c:struct:: rtBPatch_asyncEventRecord

  .. c:member:: unsigned int pid
  .. c:member:: rtBPatch_asyncEventType type
  .. c:member:: unsigned int event_fd
  .. c:member:: unsigned int size


.. c:struct:: BPatch_dynamicCallRecord

  .. c:member:: void *call_site_addr
  .. c:member:: void *call_target


.. c:struct:: BPatch_newThreadEventRecord

  .. c:member:: int ppid

    Parent process's pid

  .. c:member:: dyntid_t tid

    Thread library ID for thread

  .. c:member:: int lwp

    OS id for thread

  .. c:member:: int index

    The dyninst index for this thread

  .. c:member:: void *stack_addr

    The top of this thread's stack

  .. c:member:: void *start_pc

    The pc of this threads initial function

......

.. rubric::
  Only defined for x86_64. Cannot use MUTATEE_32 here b/c libdyninstAPI.so compiles this.

.. c:struct:: BPatch_dynamicCallRecord32

  These are the 32 bit structures for use with 32 bit mutatees on AMD64

  .. c:member:: unsigned int call_site_addr
  .. c:member:: unsigned int call_target


.. c:struct:: BPatch_newThreadEventRecord32

  .. c:member:: int ppid

    Parent process's pid

  .. c:member:: unsigned int tid

    Thread library ID for thread

  .. c:member:: int lwp

    OS id for thread

  .. c:member:: int index

    The dyninst index for this thread

  .. c:member:: unsigned int stack_addr

    The top of this thread's stack

  .. c:member:: unsigned int start_pc

    The pc of this threads initial function

......

. c:struct:: BPatch_deleteThreadEventRecord

  .. c:member:: int index

    Index of the dead thread

.. c:var:: extern int DYNINST_break_point_event

.. c:struct:: trapMapping_t

  .. c:member:: void *source
  .. c:member:: void *target

.. c:struct:: trap_mapping_header

  .. c:member:: uint32_t signature
  .. c:member:: uint32_t num_entries
  .. c:member:: int32_t pos
  .. c:member:: uint32_t padding
  .. c:member:: uint64_t low_entry
  .. c:member:: uint64_t high_entry
  .. c:member:: trapMapping_t traps[]

    Don't change this to a pointer, despite any compiler warnings

.. c:struct:: MemoryMapperCopyElement

  .. c:member:: long start
  .. c:member:: long size


.. c:struct:: MemoryMapperElement

  .. c:member:: unsigned long lo
  .. c:member:: unsigned long hi
  .. c:member:: long shift
  .. c:member:: MemoryMapperCopyElement *copyList


.. c:struct:: MemoryMapper

  .. c:member:: int guard1
  .. c:member:: int guard2
  .. c:member:: int size
  .. c:member:: int padding
  .. c:member:: MemoryMapperElement elements[MAX_MEMORY_MAPPER_ELEMENTS]

......

.. rubric::
  32/64 bit versions for the mutator



.. c:struct:: MemoryMapperElement32

  .. c:member:: uint32_t lo
  .. c:member:: uint32_t hi
  .. c:member:: uint32_t shift
  .. c:member:: void *copyList


.. c:struct:: MemoryMapperElement64

  .. c:member:: uint64_t lo
  .. c:member:: uint64_t hi
  .. c:member:: uint64_t shift
  .. c:member:: void *copyList


.. c:struct:: MemoryMapper32

  .. c:member:: int guard1
  .. c:member:: int guard2
  .. c:member:: int size
  .. c:member:: int padding
  .. c:member:: MemoryMapperElement32 elements[MAX_MEMORY_MAPPER_ELEMENTS]


.. c:struct:: MemoryMapper64

  .. c:member:: int guard1
  .. c:member:: int guard2
  .. c:member:: int size
  .. c:member:: int padding
  .. c:member:: MemoryMapperElement64 elements[MAX_MEMORY_MAPPER_ELEMENTS]

.. c:var:: extern struct MemoryMapper RTmemoryMapper

