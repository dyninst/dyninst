# Microsoft Developer Studio Generated NMAKE File, Based on libmrnet.dsp
!IF "$(CFG)" == ""
CFG=libmrnet - Win32 Debug
!MESSAGE No configuration specified. Defaulting to libmrnet - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "libmrnet - Win32 Release" && "$(CFG)" != "libmrnet - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libmrnet.mak" CFG="libmrnet - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libmrnet - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libmrnet - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "libmrnet - Win32 Release"

OUTDIR=.\.
INTDIR=.
# Begin Custom Macros
OutDir=.\.
# End Custom Macros

ALL : ".\parser.tab.h" ".\parser.obj" ".\parser.C" ".\scanner.obj" ".\scanner.C" "$(OUTDIR)\libmrnet.lib"


CLEAN :
	-@erase "$(INTDIR)\BackEndNode.obj"
	-@erase "$(INTDIR)\byte_order.obj"
	-@erase "$(INTDIR)\ChildNode.obj"
	-@erase "$(INTDIR)\CommunicationNode.obj"
	-@erase "$(INTDIR)\CommunicationNodeMain.obj"
	-@erase "$(INTDIR)\CommunicatorImpl.obj"
	-@erase "$(INTDIR)\DataElement.obj"
	-@erase "$(INTDIR)\EndPointImpl.obj"
	-@erase "$(INTDIR)\Errors.obj"
	-@erase "$(INTDIR)\EventImpl.obj"
	-@erase "$(INTDIR)\Filter.obj"
	-@erase "$(INTDIR)\FilterDefinitions.obj"
	-@erase "$(INTDIR)\FrontEndNode.obj"
	-@erase "$(INTDIR)\InternalNode.obj"
	-@erase "$(INTDIR)\Message.obj"
	-@erase "$(INTDIR)\Network.obj"
	-@erase "$(INTDIR)\NetworkGraph.obj"
	-@erase "$(INTDIR)\NetworkImpl.obj"
	-@erase "$(INTDIR)\Packet.obj"
	-@erase "$(INTDIR)\ParentNode.obj"
	-@erase "$(INTDIR)\pdr.obj"
	-@erase "$(INTDIR)\pdr_mem.obj"
	-@erase "$(INTDIR)\pdr_sizeof.obj"
	-@erase "$(INTDIR)\RemoteNode.obj"
	-@erase "$(INTDIR)\StreamImpl.obj"
	-@erase "$(INTDIR)\StreamManager.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\libmrnet.lib"
	-@erase "parser.C"
	-@erase "parser.obj"
	-@erase "parser.tab.h"
	-@erase "scanner.C"
	-@erase "scanner.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "../../../mrnet" /I "../../../mrnet/h" /I "../.." /I "../../../" /I "../../../mrnet/xplat/include" /I "C:\Program Files\Microsoft SDK\Include" /I "C:\Program Files\Microsoft Visual Studio\VC98\Include" /D "NDEBUG" /D "os_windows" /D _WIN32_WINNT=0x0500 /D "WIN32" /D "_MBCS" /D "_LIB" /U "max" /U "min" /Fp"$(INTDIR)\libmrnet.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /TP /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libmrnet.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libmrnet.lib" 
LIB32_OBJS= \
	"$(INTDIR)\BackEndNode.obj" \
	"$(INTDIR)\byte_order.obj" \
	"$(INTDIR)\ChildNode.obj" \
	"$(INTDIR)\CommunicationNode.obj" \
	"$(INTDIR)\CommunicationNodeMain.obj" \
	"$(INTDIR)\CommunicatorImpl.obj" \
	"$(INTDIR)\DataElement.obj" \
	"$(INTDIR)\EndPointImpl.obj" \
	"$(INTDIR)\Errors.obj" \
	"$(INTDIR)\EventImpl.obj" \
	"$(INTDIR)\Filter.obj" \
	"$(INTDIR)\FilterDefinitions.obj" \
	"$(INTDIR)\FrontEndNode.obj" \
	"$(INTDIR)\InternalNode.obj" \
	"$(INTDIR)\Message.obj" \
	"$(INTDIR)\Network.obj" \
	"$(INTDIR)\NetworkGraph.obj" \
	"$(INTDIR)\NetworkImpl.obj" \
	"$(INTDIR)\Packet.obj" \
	"$(INTDIR)\ParentNode.obj" \
	"$(INTDIR)\pdr.obj" \
	"$(INTDIR)\pdr_mem.obj" \
	"$(INTDIR)\pdr_sizeof.obj" \
	"$(INTDIR)\RemoteNode.obj" \
	"$(INTDIR)\StreamImpl.obj" \
	"$(INTDIR)\StreamManager.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\parser.obj" \
	"$(INTDIR)\scanner.obj"

"$(OUTDIR)\libmrnet.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "libmrnet - Win32 Debug"

OUTDIR=.\.
INTDIR=.
# Begin Custom Macros
OutDir=.\.
# End Custom Macros

ALL : ".\parser.tab.h" ".\parser.obj" ".\parser.C" ".\scanner.obj" ".\scanner.C" "$(OUTDIR)\libmrnet.lib"


CLEAN :
	-@erase "$(INTDIR)\BackEndNode.obj"
	-@erase "$(INTDIR)\byte_order.obj"
	-@erase "$(INTDIR)\ChildNode.obj"
	-@erase "$(INTDIR)\CommunicationNode.obj"
	-@erase "$(INTDIR)\CommunicationNodeMain.obj"
	-@erase "$(INTDIR)\CommunicatorImpl.obj"
	-@erase "$(INTDIR)\DataElement.obj"
	-@erase "$(INTDIR)\EndPointImpl.obj"
	-@erase "$(INTDIR)\Errors.obj"
	-@erase "$(INTDIR)\EventImpl.obj"
	-@erase "$(INTDIR)\Filter.obj"
	-@erase "$(INTDIR)\FilterDefinitions.obj"
	-@erase "$(INTDIR)\FrontEndNode.obj"
	-@erase "$(INTDIR)\InternalNode.obj"
	-@erase "$(INTDIR)\Message.obj"
	-@erase "$(INTDIR)\Network.obj"
	-@erase "$(INTDIR)\NetworkGraph.obj"
	-@erase "$(INTDIR)\NetworkImpl.obj"
	-@erase "$(INTDIR)\Packet.obj"
	-@erase "$(INTDIR)\ParentNode.obj"
	-@erase "$(INTDIR)\pdr.obj"
	-@erase "$(INTDIR)\pdr_mem.obj"
	-@erase "$(INTDIR)\pdr_sizeof.obj"
	-@erase "$(INTDIR)\RemoteNode.obj"
	-@erase "$(INTDIR)\StreamImpl.obj"
	-@erase "$(INTDIR)\StreamManager.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\libmrnet.lib"
	-@erase "parser.C"
	-@erase "parser.obj"
	-@erase "parser.tab.h"
	-@erase "scanner.C"
	-@erase "scanner.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "../../../mrnet" /I "../../../mrnet/h" /I "../.." /I "../../../" /I "../../../mrnet/xplat/include" /I "C:\Program Files\Microsoft SDK\Include" /I "C:\Program Files\Microsoft Visual Studio\VC98\Include" /D "_DEBUG" /D "os_windows" /D _WIN32_WINNT=0x0500 /D "WIN32" /D "_MBCS" /D "_LIB" /U "max" /U "min" /Fp"$(INTDIR)\libmrnet.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /TP /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libmrnet.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libmrnet.lib" 
LIB32_OBJS= \
	"$(INTDIR)\BackEndNode.obj" \
	"$(INTDIR)\byte_order.obj" \
	"$(INTDIR)\ChildNode.obj" \
	"$(INTDIR)\CommunicationNode.obj" \
	"$(INTDIR)\CommunicationNodeMain.obj" \
	"$(INTDIR)\CommunicatorImpl.obj" \
	"$(INTDIR)\DataElement.obj" \
	"$(INTDIR)\EndPointImpl.obj" \
	"$(INTDIR)\Errors.obj" \
	"$(INTDIR)\EventImpl.obj" \
	"$(INTDIR)\Filter.obj" \
	"$(INTDIR)\FilterDefinitions.obj" \
	"$(INTDIR)\FrontEndNode.obj" \
	"$(INTDIR)\InternalNode.obj" \
	"$(INTDIR)\Message.obj" \
	"$(INTDIR)\Network.obj" \
	"$(INTDIR)\NetworkGraph.obj" \
	"$(INTDIR)\NetworkImpl.obj" \
	"$(INTDIR)\Packet.obj" \
	"$(INTDIR)\ParentNode.obj" \
	"$(INTDIR)\pdr.obj" \
	"$(INTDIR)\pdr_mem.obj" \
	"$(INTDIR)\pdr_sizeof.obj" \
	"$(INTDIR)\RemoteNode.obj" \
	"$(INTDIR)\StreamImpl.obj" \
	"$(INTDIR)\StreamManager.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\parser.obj" \
	"$(INTDIR)\scanner.obj"

"$(OUTDIR)\libmrnet.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "libmrnet - Win32 Release" || "$(CFG)" == "libmrnet - Win32 Debug"
SOURCE=..\..\..\mrnet\src\BackEndNode.C

"$(INTDIR)\BackEndNode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\byte_order.c

"$(INTDIR)\byte_order.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\ChildNode.C

"$(INTDIR)\ChildNode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\CommunicationNode.C

"$(INTDIR)\CommunicationNode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\CommunicationNodeMain.C

"$(INTDIR)\CommunicationNodeMain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\CommunicatorImpl.C

"$(INTDIR)\CommunicatorImpl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\DataElement.C

"$(INTDIR)\DataElement.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\EndPointImpl.C

"$(INTDIR)\EndPointImpl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\Errors.C

"$(INTDIR)\Errors.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\EventImpl.C

"$(INTDIR)\EventImpl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\Filter.C

"$(INTDIR)\Filter.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\FilterDefinitions.C

"$(INTDIR)\FilterDefinitions.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\FrontEndNode.C

"$(INTDIR)\FrontEndNode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\InternalNode.C

"$(INTDIR)\InternalNode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\Message.C

"$(INTDIR)\Message.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\Network.C

"$(INTDIR)\Network.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\NetworkGraph.C

"$(INTDIR)\NetworkGraph.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\NetworkImpl.C

"$(INTDIR)\NetworkImpl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\Packet.C

"$(INTDIR)\Packet.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\ParentNode.C

"$(INTDIR)\ParentNode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\parser.y

!IF  "$(CFG)" == "libmrnet - Win32 Release"

InputPath=..\..\..\mrnet\src\parser.y

"$(INTDIR)\parser.obj"	"$(INTDIR)\parser.tab.h"	"$(INTDIR)\parser.C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bison -p mrn -d -y $(InputPath) 
	perl ..\..\..\..\..\scripts\vcStripStd.pl < y.tab.c > parser.C 
	del y.tab.c 
	move y.tab.h parser.tab.h 
	cl /nologo /D "WIN32" /D "os_windows" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /D "_WIN32_WINNT=0x0500" /I "C:\Program Files\Microsoft SDK\Include" /I "C:\Program Files\Microsoft Visual Studio\VC98\Include" /I "..\..\.." /I "..\..\..\mrnet\h" /I "..\..\..\mrnet\xplat\include" /GX /TP /c parser.C
<< 
	

!ELSEIF  "$(CFG)" == "libmrnet - Win32 Debug"

InputPath=..\..\..\mrnet\src\parser.y

"$(INTDIR)\parser.obj"	"$(INTDIR)\parser.tab.h"	"$(INTDIR)\parser.C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bison -p mrn -d -y $(InputPath) 
	perl ..\..\..\..\..\scripts\vcStripStd.pl < y.tab.c > parser.C 
	del y.tab.c 
	move y.tab.h parser.tab.h 
	cl /nologo /D "WIN32" /D "os_windows" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /D "_WIN32_WINNT=0x0500" /I "C:\Program Files\Microsoft SDK\Include" /I "C:\Program Files\Microsoft Visual Studio\VC98\Include" /I "..\..\.." /I "..\..\..\mrnet\h" /I "..\..\..\mrnet\xplat\include" /GX /TP /c parser.C
<< 
	

!ENDIF 

SOURCE=..\..\..\mrnet\src\pdr.c

"$(INTDIR)\pdr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\pdr_mem.c

"$(INTDIR)\pdr_mem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\pdr_sizeof.c

"$(INTDIR)\pdr_sizeof.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\RemoteNode.C

"$(INTDIR)\RemoteNode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\scanner.l

!IF  "$(CFG)" == "libmrnet - Win32 Release"

InputPath=..\..\..\mrnet\src\scanner.l

"$(INTDIR)\scanner.obj"	"$(INTDIR)\scanner.C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	flex  -Pmrn ..\..\..\mrnet\src\scanner.l 
	perl ..\..\..\..\..\scripts\vcStripStd.pl < lex.mrn.c > scanner.C 
	del lex.mrn.c 
	cl /nologo /D "WIN32" /D "os_windows" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /D "_WIN32_WINNT=0x0500" /I "C:\Program Files\Microsoft SDK\Include" /I "C:\Program Files\Microsoft Visual Studio\VC98\Include" /I "..\..\.." /I "..\..\..\mrnet\h" /I "..\..\..\mrnet\xplat\include" /GX /TP /c scanner.C
<< 
	

!ELSEIF  "$(CFG)" == "libmrnet - Win32 Debug"

InputPath=..\..\..\mrnet\src\scanner.l

"$(INTDIR)\scanner.obj"	"$(INTDIR)\scanner.C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	flex  -Pmrn ..\..\..\mrnet\src\scanner.l 
	perl ..\..\..\..\..\scripts\vcStripStd.pl < lex.mrn.c > scanner.C 
	del lex.mrn.c 
	cl /nologo /D "WIN32" /D "os_windows" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /D "_WIN32_WINNT=0x0500" /I "C:\Program Files\Microsoft SDK\Include" /I "C:\Program Files\Microsoft Visual Studio\VC98\Include" /I "..\..\.." /I "..\..\..\mrnet\h" /I "..\..\..\mrnet\xplat\include" /GX /TP /c scanner.C
<< 
	

!ENDIF 

SOURCE=..\..\..\mrnet\src\StreamImpl.C

"$(INTDIR)\StreamImpl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\StreamManager.C

"$(INTDIR)\StreamManager.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\mrnet\src\utils.C

"$(INTDIR)\utils.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

