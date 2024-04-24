.. _`sec:pathName.h`:

pathName.h
##########

.. deprecated:: 12.0
   Use `Boost.Filesystem <https://www.boost.org/doc/libs/1_84_0/libs/filesystem/doc/index.htm>`_.

.. cpp:function:: std::string expand_tilde_pathname(const std::string &dir)

  Convert "~tamches/hello" to "/u/t/a/tamches/hello", or convert "~/hello" to same.

.. cpp:function:: std::string concat_pathname_components(const std::string &part1, const std::string &part2)

  Concatenate path1 and part2, adding a "/" between them if neither part1 ends in a "/" or part2 begins in one.

.. cpp:function:: bool extractNextPathElem(const char * &ptr, std::string &result)

  Assumes that ``ptr`` points to the value of the ``PATH`` environment
  variable.  Extracts the next element (writing to result, updating
  ptr, returning true) if available else returns ``false``.

.. cpp:function:: bool exists_executable(const std::string &fullpathname)
.. cpp:function:: bool executableFromArgv0AndPathAndCwd(std::string &result, const std::string &i_argv0, const std::string &path, const std::string &cwd)
.. cpp:function:: std::string extract_pathname_tail(const std::string &path)
.. cpp:function:: std::string resolve_file_path(char const* path)
.. cpp:function:: std::string resolve_file_path(std::string)
