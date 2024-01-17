.. _`sec:emitWin.h`:

emitWin.h
#########

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: emitWin

  .. cpp:function:: emitWin(PCHAR baseaddress, Object* o_nt, void (*)(const char *) = log_msg)
  .. cpp:function:: bool driver(Symtab *obj, std::string fName)

  .. cpp:member:: private const static unsigned int SizeOfSecHeader = 40

      size of section header entry is 40 bytes

  .. cpp:member:: private PCHAR base_addr

      the base address of the mapped image file

  .. cpp:member:: private Offset bit_addr

      the offset of bound import table

  .. cpp:member:: private unsigned int bit_size

      the size of bound import table

  .. cpp:member:: private Object* obj_nt

  .. cpp:member:: private bool isMoveAhead

      Whether or not we need to move things ahead to Dos Stub Area

  .. cpp:function:: private Offset PEAlign(Offset dwAddr,Offset dwAlign)
  .. cpp:function:: private unsigned int NumOfTotalAllowedSec()
  .. cpp:function:: private unsigned int NumOfAllowedSecInSectionTable()
  .. cpp:function:: private unsigned int NumOfAllowedSecInDosHeader()
  .. cpp:function:: private PIMAGE_SECTION_HEADER CreateSecHeader(unsigned int size,PIMAGE_SECTION_HEADER preSecHdr)
  .. cpp:function:: private bool AlignSection(PIMAGE_SECTION_HEADER p)
  .. cpp:function:: private bool writeImpTable(Symtab*)

  .. cpp:type:: void (*err_func_)(const char*)
  .. cpp:function:: private void log_winerror(void (*err_func)(const char *), const char* msg)
