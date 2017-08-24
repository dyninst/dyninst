\appendix

\section{Building SymtabAPI}

This appendix describes how to build SymtabAPI from source code, which can be
downloaded from http://www.paradyn.org or http://www.dyninst.org.  

\subsection{Building on Unix}

Building SymtabAPI on UNIX platforms is a four step process that involves:
unpacking the SymtabAPI source, installing any SymtabAPI dependencies,
configuring paths in make.config.local, and running the build.  

SymtabAPI's source code is packaged in a tar.gz format. If your SymtabAPI
source tarball is called \texttt{symtab\_src\_1.0.tar.gz}, then you could extract
it with the command \texttt{gunzip symtab\_src\_1.0.tar.gz; tar -xvf
symtab\_src\_1.0.tar}. This will create two directories: core and scripts.  

SymtabAPI has several dependencies, depending on what platform you are using,
which must be installed before SymtabAPI can be built. Note that for most of
these packages Symtab needs to be able to access the package's include files,
which means that development versions are required. If a version number is
listed for a packaged, then there are known bugs that may affect Symtab with
earlier versions of the package. 

\begin{center}
    \begin{tabular}{ll}
Linux/x86       & libdwarf-200120327 \\
                & libelf\\
Linux/x86-64    & libdwarf-200120327 \\
                & libelf \\
Windows/x86     & <none> \\
\end{tabular}
\end{center}

At the time of this writing the Linux packages could be found at:
\begin{itemize}
    \item libdwarf - http://reality.sgiweb.org/davea/dwarf.html
    \item libelf - http://www.mr511.de/software/english.html
\end{itemize}

Once the dependencies for SymtabAPI have been installed, SymtabAPI must be
configured to know where to find these packages. This is done through
SymtabAPI's \texttt{core/make.config.local} file. This file must be written in
GNU Makefile syntax and must specify directories for each dependency.
Specifically, LIBDWARFDIR, LIBELFDIR and LIBXML2DIR variables must be set.
LIBDWARFDIR should be set to the absolute path of libdwarf library where
\texttt{dwarf.h} and \texttt{libdw.h} files reside. LIBELFDIR should be set
to the absolute path where \texttt{libelf.a} and \texttt{libelf.so} files are
located. Finally, LIBXML2DIR to the absolute path where libxml2 is located.

The next thing is to set DYNINST\_ROOT, PLATFORM, and LD\_LIBRARY\_PATH
environment variables. DYNINST\_ROOT should be set to the path of the directory
that contains core and scripts subdirectories.

PLATFORM should be set to one of the following values depending upon what operating system you are running on:
\begin{description}
    \item i386-unknown-linux2.4Linux 2.4/2.6 on an Intel x86 processor
    \item x86\_64-unknown-linux2.4Linux 2.4/2.6 on an AMD-64 processor
\end{description}

LD\_LIBRARY\_PATH variable should be set in a way that it includes libdwarf home
directory/lib and \${DYNINST\_ROOT}/\${PLATFORM}/lib directories.

Once \texttt{make.config.local} is set you are ready to build SymtabAPI. Change
to the core directory and execute the command make SymtabAPI. This will build
the SymtabAPI library. Successfully built binaries will be stored in a directory
named after your platform at the same level as the core directory. 

\subsection{Building on Windows}

SymtabAPI for Windows is built with Microsoft Visual Studio 2003 project and
solution files.   Building SymtabAPI for Windows is similar to UNIX in that it
is a four step process: unpack the SymtabAPI source code, install SymtabAPI's
package dependencies, configure Visual Studio to use the dependencies, and run
the build system. 

SymtabAPI's source code is distributed as part of a tar.gz package. Most popular
unzipping programs are capable of handling this format. Extracting the Symtab
tarball results in two directories: core and scripts. 

Symtab for Windows depends on Microsoft's Debugging Tools for Windows, which
could be found at http://www.microsoft.com/whdc/devtools/debugging/default.mspx
at the time of this writing. Download these tools and install them at an
appropriate location. Make sure to do a custom install and install the SDK,
which is not always installed by default. For the rest of this section, we will
assume that the Debugging Tools are installed at \texttt{C:\textbackslash
    Program Files\textbackslash Debugging
Tools for Windows}. If this is not the case, then adjust the following
instruction appropriately.

Once the Debugging Tools are installed, Visual Studio must be configured to use
them. We need to add the Debugging Tools include and library directories to
Visual Studios search paths. In Visual Studio 2003 select \texttt{Options...} from the
tools menu. Next select Projects and VC++ Directories from the pane on the left.
You should see a list of directories that are sorted into categories such as
`Executable files', `Include files', etc. The current category can
be changed with a drop down box in the upper right hand side of the Dialog. 

First, change to the `Library files' category, and add an entry that
points to \\
\texttt{C:\textbackslash Program Files\textbackslash Debugging Tools
for Windows\textbackslash sdk\textbackslash lib\textbackslash i386}. Make sure
that this entry is above Visual Studio's default search paths.

Next, Change to the `Include files' category and make a new entry in the
list that points to \texttt{C:\textbackslash Program Files\textbackslash Debugging Tools for
Windows\textbackslash sdk\textbackslash inc}. Also
make sure that this entry is above Visual Studio's default search paths. Some
users have had a problem where Visual Studio cannot find the \texttt{cvconst.h} file. You
may need to add the directory containing this file to the include search path.
We have seen it installed at \texttt{\$(VCInstallDir)\textbackslash
    ..\textbackslash Visual Studio SDKs\textbackslash DIA
SDK\textbackslash include}, although you may need to search for it. You also need to add the
libxml2 include path depending on the where the libxml2 is installed on the
system.  

Once you have installed and configured the Debugging Tools for Windows
you are ready to build Symtab. First, you need to create the directories where
Dyninst will install its completed build. From the core directory you need to
create the directories \texttt{..\textbackslash i386-unknown-nt4.0\textbackslash bin} and
\texttt{..\textbackslash i386-unknown-nt4.0\textbackslash lib}.
Next open the solution file core/SymtabAPI.sln with Visual Studio.   You can
then build SymtabAPI by select `Build Solution' from the build menu. This
will build the SymtabAPI library.

