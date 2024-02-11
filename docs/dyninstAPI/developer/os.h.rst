.. _`sec:os.h`:

os.h
####

This should enforce the abstract OS operations

.. cpp:class:: OS

  .. cpp:function:: static void osTraceMe(void)
  .. cpp:function:: static void osDisconnect(void)
  .. cpp:function:: static bool osKill(int)
  .. cpp:function:: static void make_tempfile(char *)
  .. cpp:function:: static bool execute_file(char *)
  .. cpp:function:: static void unlink(char *)
  .. cpp:function:: static bool executableExists(const std::string &file)
  .. cpp:function:: static void get_sigaction_names(std::vector<std::string> &names)


Temporary prototype for a remote debugging BPatch interface.

.. cpp:namespace:: dev

.. cpp:function:: bool OS_isConnected(void)
.. cpp:function:: bool OS_connect(BPatch_remoteHost &remote)
.. cpp:function:: bool OS_getPidList(BPatch_remoteHost &remote, BPatch_Vector<unsigned int> &tlist)
.. cpp:function:: bool OS_getPidInfo(BPatch_remoteHost &remote, unsigned int pid, std::string &pidStr)
.. cpp:function:: bool OS_disconnect(BPatch_remoteHost &remote)
