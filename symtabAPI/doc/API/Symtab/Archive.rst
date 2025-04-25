\subsection{Class Archive}\label{Archive}
This is used only on ELF platforms. This class represents an archive. This class has information of all the members in the archives.

\begin{apient}
static bool openArchive(Archive *&img,
                        string name)
\end{apient}
\apidesc{
This factory method creates a new \code{Archive} object for an archive file on disk. This object serves as a handle to the parsed archive file. \code{name} represents the name of the archive to be parsed. The \code{Archive} object is returned in \code{img} if the parsing succeeds.
This method returns \code{false} if the given file is not an archive. The error is set
to \code{Not\_An\_Archive}. This returns \code{true} if the archive is parsed without an error. \code{printSymtabError()} should be called to get more error details.
}

\begin{apient}
static bool openArchive(Archive *&img,
                        char *mem_image,
                        size_t size)
\end{apient}
\apidesc{
This factory method creates a new \code{Archive} object for an archive file in memory.
This object serves as a handle to the parsed archive file. \code{mem\_image} represents the pointer to the archive to be parsed. \code{size} represents the size of the memory image. The \code{Archive} object is returned in \code{img} if the parsing succeeds.
This method returns \code{false} if the given file is not an archive. The error is set
to \code{Not\_An\_Archive}. This returns \code{true} if the archive is parsed without an error. \code{printSymtabError()} should be called to get more error details. This method is not supported currently on all ELF platforms.
}

\begin{apient}
bool getMember(Symtab *&img,
               string member_name)
\end{apient}
\apidesc{
This method returns the member object handle if the member exists in the archive. \code{img} corresponds to the object handle for the member.
This method returns \code{false} if the member with name \code{member\_name} does not exist else returns \code{true}.
}

\begin{apient}
bool getMemberByOffset(Symtab *&img,
                       Offset memberOffset)
\end{apient}
\apidesc{
This method returns the member object handle if the member exists at the start offset \code{memberOffset} in the archive. \code{img} corresponds to the object handle for the member.
This method returns \code{false} if the member with name \code{member\_name} does not exist else returns \code{true}.
}

\begin{apient}
bool getAllMembers(vector <Symtab *> &members)
\end{apient}
\apidesc{
This method returns all the member object handles in the archive. Returns \code{true} on success with \code{members} containing the \code{Symtab} Objects for all the members in the archive.
}

\begin{apient}
bool isMemberInArchive(string member_name)
\end{apient}
\apidesc{
This method returns \code{true} if the member with name \code{member\_name} exists in the archive or else returns \code{false}.
}

\begin{apient}
bool findMemberWithDefinition(Symtab *&obj,
                              string name)
\end{apient}
\apidesc{
This method retrieves the member in an archive which contains the definition to a symbol with mangled name \code{name}.
Returns \code{true} with \code{obj} containing the \code{Symtab} handle to that member or else returns \code{false}.
}

\begin{apient}
static SymtabError getLastError()
\end{apient}
\apidesc{
This method returns an error value for the previously performed operation that resulted in a failure. 
SymtabAPI sets a global error value in case of error during any operation. This call returns the last error that occurred while performing any operation.
}

\begin{apient}
static string printError(SymtabError serr)
\end{apient}
\apidesc{
This method returns a detailed description of the enum value \code{serr} in human
readable format.
}
