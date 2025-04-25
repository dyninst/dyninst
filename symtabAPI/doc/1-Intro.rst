\section{Introduction}
\label{sec:intro}

SymtabAPI is a multi-platform library for parsing symbol tables,
object file headers and debug information. SymtabAPI currently
supports the ELF (IA-32, AMD-64, ARMv8-64, and POWER) and PE
(Windows) object file formats. In addition, it also supports the DWARF
debugging format.

The main goal of this API is to provide an abstract view of binaries and
libraries across multiple platforms. An abstract interface provides two
benefits: it simplifies the development of a tool since the complexity of a
particular file format is hidden, and it allows tools to be easily ported
between platforms. Each binary object file is represented in a canonical
platform independent manner by the API. The canonical format consists of four
components: a header block that contains general information about the object
(e.g., its name and location), a set of symbol lists that index symbols within
the object for fast lookup, debug information (type, line number and local
variable information) present in the object file and a set of additional data
that represents information that may be present in the object (e.g., relocation
or exception information). Adding a new format requires no changes to the
interface and hence will not affect any of the tools that use the SymtabAPI. 

Our other design goal with SymtabAPI is to allow users and tool developers to
easily extend or add symbol or debug information to the library through a
platform-independent interface. Often times it is impossible to satify all the
requirements of a tool that uses SymtabAPI, as those requirements can vary from
tool to tool. So by providing extensible structures, SymtabAPI allows tools to
modify any structure to fit their own requirements. Also, tools frequently use
more sophisticated analyses to augment the information available from the binary
directly; it should be possible to make this extra information available to the
SymtabAPI library. An example of this is a tool operating on a stripped binary.
Although the symbols for the majority of functions in the binary may be missing,
many can be determined via more sophisticated analysis. In our model, the tool
would then inform the SymtabAPI library of the presence of these functions; this
information would be incorporated and available for subsequent analysis. Other
examples of such extensions might involve creating and adding new types or
adding new local variables to certain functions.
