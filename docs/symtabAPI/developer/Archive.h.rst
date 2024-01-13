.. _`sec:Archive.h`:

Archive.h
#########

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: ArchiveMember

  Helps facilitate lazy parsing and quick lookup once parsing is finished.

  .. cpp:function:: ArchiveMember()
  .. cpp:function:: ArchiveMember(const std::string name, const Offset offset, Symtab * img = NULL)
  .. cpp:function:: const std::string& getName()
  .. cpp:function:: Offset getOffset()
  .. cpp:function:: Symtab * getSymtab()
  .. cpp:function:: void setSymtab(Symtab *img)


.. cpp:class:: Archive
      
  This is used only on ELF platforms. This class represents an archive.
  This class has information of all the members in the archives.

  .. cpp:function:: static bool openArchive(Archive *&img, string name)

      Creates a handle in ``img`` to a parsed archive file with on-disk name ``name``.

      Returns ``false`` if the file is not an archive.

  .. cpp:function:: static bool openArchive(Archive *&img, char *mem_image, size_t size)

      Creates a handle in ``img`` to a parsed archive file with in-memory representation
      ``mem_image`` of ``size`` bytes.

      Returns ``false`` if the file is not an archive.

      .. warning:: This method is not supported currently on all ELF platforms.

  .. cpp:function:: static SymtabError getLastError()

      Returns an error value for the previously performed operation that resulted in a failure.

      .. note:: The error mechanism in Symtab is now well-maintained. Do not rely on it for debugging.

  .. cpp:function:: static std::string printError(SymtabError err)

      Returns a detailed description of the enum value ``serr`` in human-readable format.

      .. note:: The error mechanism in Symtab is now well-maintained. Do not rely on it for debugging.

  .. cpp:function:: bool getMember(Symtab *&img, string member_name)

      Returns in ``img`` the member with name ``member_name``.

      Returns ``false`` if the member does not exist.

  .. cpp:function:: bool getMemberByOffset(Symtab *&img, Offset memberOffset)

      Returns in ``img`` the member at offset ``memberOffset``.

      Returns ``false`` if the member does not exist.

  .. cpp:function:: bool getMemberByGlobalSymbol(Symtab *&img, std::string& symbol_name)

      Returns in ``img`` the member with global symbol name ``symbol_name``.

      Returns ``false`` on error or if ``symbol_name`` is not in this archive.

  .. cpp:function:: bool getAllMembers(vector<Symtab*>& members)

      Returns all members in the archive.

      Returns ``false`` on error.

  .. cpp:function:: bool isMemberInArchive(string member_name)

      Checks if ``member_name`` exists.

  .. cpp:function:: bool findMemberWithDefinition(Symtab *&obj, string name)

      Retrieves in ``obj`` the member containing the definition of a symbol with
      mangled name ``name``.

      Returns ``false`` on error or if ``name`` is not in this archive.

  .. cpp:function:: std::string name()

      Returns the on-disk filename of this archive, if it exists.

  .. cpp:function:: bool getMembersBySymbol(std::string name, std::vector<Symtab*> &matches)

      Returns in ``matches`` the members with symbol name ``name``.

      Returns ``false`` on error or if ``name`` is not in this archive.

  .. cpp:function:: private bool parseMember(Symtab *&img, ArchiveMember *member)

      This is architecture-specific.

  .. cpp:member:: private void *basePtr

      For ELF, the elf pointer for the archive.

  .. cpp:member:: private bool symbolTableParsed

      The symbol table is lazily parsed

  .. cpp:member:: private static std::vector<Archive*> allArchives

      Used to avoid duplicating an Archive that already exists.
