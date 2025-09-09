\subsection{Class Region}

This class represents a contiguous range of code or data as encoded in the object file. For ELF, regions represent ELF sections. 

\begin{center}
\begin{tabular}{ll}
\toprule
perm\_t & Meaning \\
\midrule
RP\_R & Read-only data \\
RP\_RW & Read/write data \\
RP\_RX & Read-only code \\
RP\_RWX & Read/write code \\
\bottomrule
\end{tabular}
\end{center}

\begin{center}
\begin{tabular}{ll}
\toprule
RegionType & Meaning \\
\midrule
RT\_TEXT & Executable code \\
RT\_DATA & Read/write data \\
RT\_TEXTDATA & Mix of code and data \\
RT\_SYMTAB & Static symbol table \\
RT\_STRTAB & String table used by the symbol table \\
RT\_BSS & 0-initialized memory \\
RT\_SYMVERSIONS & Versioning information for symbols \\
RT\_SYMVERDEF & Versioning information for symbols \\
RT\_SYMVERNEEDED & Versioning information for symbols \\
RT\_REL & Relocation section \\
RT\_RELA & Relocation section \\
RT\_PLTREL & Relocation section for PLT (inter-library references) entries \\
RT\_PLTRELA & Relocation section for PLT (inter-library references) entries \\
RT\_DYNAMIC & Decription of library dependencies \\
RT\_HASH & Fast symbol lookup section \\
RT\_GNU\_HASH & GNU-specific fast symbol lookup section \\
RT\_OTHER & Miscellaneous information \\
\bottomrule
\end{tabular}
\end{center}

\begin{tabular}{p{1.25in}p{1in}p{3.25in}}
\toprule
Method name & Return type & Method description \\
\midrule
getRegionNumber & unsigned & Index of the region in the file, starting at 0. \\
getRegionName & std::string & Name of the region (e.g. .text, .data). \\
getPtrToRawData & void * & Read-only pointer to the region's raw data buffer. \\
getDiskOffset & Offset & Offset within the file where the region begins. \\
getDiskSize & unsigned long & Size of the region's data in the file. \\
getMemOffset & Offset & Location where the region will be loaded into memory, modified by the file's base load address. \\
getMemSize & unsigned long & Size of the region in memory, including zero padding. \\
isBSS & bool & Type query for uninitialized data regions (zero disk size, non-zero memory size). \\
isText & bool & Type query for executable code regions. \\
isData & bool & Type query for initialized data regions. \\
getRegionPermissions & perm\_t & Permissions for the region; perm\_ t is defined above. \\
getRegionType & RegionType & Type of the region as defined above. \\
isLoadable & bool & True if the region will be loaded into memory (e.g., code or data), false otherwise (e.g., debug information). \\
isDirty & bool & True if the region's raw data buffer has been modified by the user. \\
\bottomrule
\end{tabular}

\begin{apient}
static Region *createRegion(Offset diskOff,
                            perm_t perms,
                            RegionType regType,
                            unsigned long diskSize = 0,
                            Offset memOff = 0,
                            unsigned long memSize = 0,
                            std::string name = "",
                            char *rawDataPtr = NULL,
                            bool isLoadable = false,
                            bool isTLS = false,
                            unsigned long memAlign = sizeof(unsigned))
\end{apient}
\apidesc{
This factory method creates a new region with the provided arguments. The \code{memOff} and \code{memSize} parameters identify where the region should be loaded in memory (modified by the base address of the file); if \code{memSize} is larger than \code{diskSize} the remainder will be zero-padded (e.g., bss regions). 
}

\begin{apient}
bool isOffsetInRegion(const Offset &offset) const
\end{apient}
\apidesc{
Return \code{true} if the offset falls within the region data.
}

\begin{apient}
void setRegionNumber(unsigned index) const
\end{apient}
\apidesc{
Sets the region index; the value must not overlap with any other regions and is not checked. 
}

\begin{apient}
bool setPtrToRawData(void *newPtr,
                     unsigned long rawsize)
\end{apient}
\apidesc{
Set the raw data pointer of the region to \code{newPtr}. \code{rawsize} represents the size of the raw data buffer. 
Returns \code{true} if success or \code{false} when unable to set/change the raw data of the region. Implicitly changes the disk and memory sizes of the region.
}

\begin{apient}
bool setRegionPermissions(perm_t newPerms)
\end{apient}
\apidesc{
This sets the regions permissions to \code{newPerms}. Returns \code{true} on success.
}


\begin{apient}
bool setLoadable(bool isLoadable)
\end{apient}
\apidesc{
This method sets whether the region is loaded into memory at load time. Returns \code{true} on success.
}

\begin{apient}
bool addRelocationEntry(Offset relocationAddr,
                        Symbol *dynref,
                        unsigned long relType, 
                        Region::RegionType rtype = Region::RT_REL)
\end{apient}
\apidesc{
Creates and adds a relocation entry for this region. The symbol \code{dynref} represents the symbol used by he relocation, \code{relType} is the (platform-specific) relocation type, and \code{rtype} represents whether the relocation is REL or RELA (ELF-specific). 
}

\begin{apient}
vector<relocationEntry> &getRelocations()
\end{apient}
\apidesc{Get the vector of relocation entries that will modify this region. The vector should not be modified. }

\begin{apient}
bool addRelocationEntry(const relocationEntry& rel)
\end{apient}
\apidesc{Add the provided relocation entry to this region.}

\begin{apient}
bool patchData(Offset off,
               void *buf,
               unsigned size);
\end{apient}
\apidesc{
Patch the raw data for this region. \code{buf} represents the buffer to be patched at offset \code{off} and size \code{size}.
}

\subsubsection{REMOVED}

The following methods were removed since they were inconsistent and dangerous to use. 

\begin{apient}
Offset getRegionAddr() const
\end{apient}
\apidesc{
Please use \code{getDiskOffset} or \code{getMemOffset} instead, as appropriate. 
}

\begin{apient}
unsigned long getRegionSize() const
\end{apient}
\apidesc{
Please use \code{getDiskSize} or \code{getMemSize} instead, as appropriate. 
}

\subsection{Relocation Information}
This class represents object relocation information. 

\begin{apient}
Offset target_addr() const
\end{apient}
\apidesc{Specifies the offset that will be overwritten when relocations are processed.}

\begin{apient}
Offset rel_addr() const
\end{apient}
\apidesc{Specifies the offset of the relocation itself.}

\begin{apient}
Offset addend() const
\end{apient}
\apidesc{Specifies the value added to the relocation; whether this value is used or not is specific to the relocation type.}

\begin{apient}
const std::string name() const
\end{apient}
\apidesc{Specifies the user-readable name of the relocation.}


\begin{apient}
Symbol *getDynSym() const
\end{apient}
\apidesc{Specifies the symbol whose final address will be used in the relocation calculation. How this address is used is specific to the relocation type.}

\begin{apient}
unsigned long getRelType() const
\end{apient}
\apidesc{Specifies the platform-specific relocation type.}

