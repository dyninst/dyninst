Archive.h
=========

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: Archive

This is used only on ELF platforms. This class represents an archive.
This class has information of all the members in the archives.

.. code-block:: cpp

    static bool openArchive(Archive *&img, string name)

This factory method creates a new ``Archive`` object for an archive file
on disk. This object serves as a handle to the parsed archive file.
``name`` represents the name of the archive to be parsed. The
``Archive`` object is returned in ``img`` if the parsing succeeds. This
method returns ``false`` if the given file is not an archive. The error
is set to ``Not_An_Archive``. This returns ``true`` if the archive is
parsed without an error. ``printSymtabError()`` should be called to get
more error details.

.. code-block:: cpp

    static bool openArchive(Archive *&img, char *mem_image, size_t size)

This factory method creates a new ``Archive`` object for an archive file
in memory. This object serves as a handle to the parsed archive file.
``mem_image`` represents the pointer to the archive to be parsed.
``size`` represents the size of the memory image. The ``Archive`` object
is returned in ``img`` if the parsing succeeds. This method returns
``false`` if the given file is not an archive. The error is set to
``Not_An_Archive``. This returns ``true`` if the archive is parsed
without an error. ``printSymtabError()`` should be called to get more
error details. This method is not supported currently on all ELF
platforms.

.. code-block:: cpp

    bool getMember(Symtab *&img, string member_name)

This method returns the member object handle if the member exists in the
archive. ``img`` corresponds to the object handle for the member. This
method returns ``false`` if the member with name ``member_name`` does
not exist else returns ``true``.

.. code-block:: cpp

    bool getMemberByOffset(Symtab *&img, Offset memberOffset)

This method returns the member object handle if the member exists at the
start offset ``memberOffset`` in the archive. ``img`` corresponds to the
object handle for the member. This method returns ``false`` if the
member with name ``member_name`` does not exist else returns ``true``.

.. code-block:: cpp

    bool getAllMembers(vector <Symtab *> &members)

This method returns all the member object handles in the archive.
Returns ``true`` on success with ``members`` containing the ``Symtab``
Objects for all the members in the archive.

.. code-block:: cpp

    bool isMemberInArchive(string member_name)

This method returns ``true`` if the member with name ``member_name``
exists in the archive or else returns ``false``.

.. code-block:: cpp

    bool findMemberWithDefinition(Symtab *&obj, string name)

This method retrieves the member in an archive which contains the
definition to a symbol with mangled name ``name``. Returns ``true`` with
``obj`` containing the ``Symtab`` handle to that member or else returns
``false``.

.. code-block:: cpp

    static SymtabError getLastError()

This method returns an error value for the previously performed
operation that resulted in a failure. SymtabAPI sets a global error
value in case of error during any operation. This call returns the last
error that occurred while performing any operation.

.. code-block:: cpp

    static string printError(SymtabError serr)

This method returns a detailed description of the enum value ``serr`` in
human readable format.