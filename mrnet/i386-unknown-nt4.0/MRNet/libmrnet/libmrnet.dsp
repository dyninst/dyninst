# Microsoft Developer Studio Project File - Name="libmrnet" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libmrnet - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libmrnet.mak".
!MESSAGE 
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

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libmrnet - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "."
# PROP Intermediate_Dir "."
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../../mrnet" /I "../../../mrnet/h" /I "../.." /I "../../../" /I "../../../mrnet/xplat/include" /I "C:\Program Files\Microsoft SDK\Include" /I "C:\Program Files\Microsoft Visual Studio\VC98\Include" /D "NDEBUG" /D "os_windows" /D _WIN32_WINNT=0x0500 /D "WIN32" /D "_MBCS" /D "_LIB" /U "max" /U "min" /YX /FD /TP /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libmrnet - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "."
# PROP Intermediate_Dir "."
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../../mrnet" /I "../../../mrnet/h" /I "../.." /I "../../../" /I "../../../mrnet/xplat/include" /I "C:\Program Files\Microsoft SDK\Include" /I "C:\Program Files\Microsoft Visual Studio\VC98\Include" /D "_DEBUG" /D "os_windows" /D _WIN32_WINNT=0x0500 /D "WIN32" /D "_MBCS" /D "_LIB" /U "max" /U "min" /YX /FD /GZ /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libmrnet - Win32 Release"
# Name "libmrnet - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat,l,y"
# Begin Source File

SOURCE=..\..\..\mrnet\src\BackEndNode.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\byte_order.c
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\ChildNode.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\CommunicationNode.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\CommunicationNodeMain.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\CommunicatorImpl.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\DataElement.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\EndPointImpl.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\Error.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\EventImpl.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\Filter.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\FilterDefinitions.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\FrontEndNode.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\InternalNode.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\Message.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\Network.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\NetworkGraph.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\NetworkImpl.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\Packet.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\ParentNode.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\pdr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\pdr_mem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\pdr_sizeof.c
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\RemoteNode.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\scanner.l

!IF  "$(CFG)" == "libmrnet - Win32 Release"

# Begin Custom Build
InputPath=..\..\..\mrnet\src\scanner.l

BuildCmds= \
	bison -p mrn -d -y ..\..\..\mrnet\src\parser.y \
	perl ..\..\..\..\..\scripts\vcStripStd.pl < y.tab.c > parser.C \
	del y.tab.c \
	move y.tab.h parser.tab.h \
	cl /nologo /D "WIN32" /D "os_windows" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /D "_WIN32_WINNT=0x0500" /I "C:\Program Files\Microsoft SDK\Include" /I "C:\Program Files\Microsoft Visual Studio\VC98\Include" /I "..\..\.." /I "..\..\..\mrnet\h" /I "..\..\..\mrnet\xplat\include" /GX /TP /c parser.C \
	flex  -Pmrn ..\..\..\mrnet\src\scanner.l \
	perl ..\..\..\..\..\scripts\vcStripStd.pl < lex.mrn.c > scanner.C \
	del lex.mrn.c \
	cl /nologo /D "WIN32" /D "os_windows" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /D "_WIN32_WINNT=0x0500" /I "C:\Program Files\Microsoft SDK\Include" /I "C:\Program Files\Microsoft Visual Studio\VC98\Include" /I "..\..\.." /I "..\..\..\mrnet\h" /I "..\..\..\mrnet\xplat\include" /GX /TP /c scanner.C \
	

"scanner.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"scanner.C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"parser.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"parser.tab.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"parser.C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "libmrnet - Win32 Debug"

# Begin Custom Build
InputPath=..\..\..\mrnet\src\scanner.l

BuildCmds= \
	bison -p mrn -d -y ..\..\..\mrnet\src\parser.y \
	perl ..\..\..\..\..\scripts\vcStripStd.pl < y.tab.c > parser.C \
	del y.tab.c \
	move y.tab.h parser.tab.h \
	cl /nologo /D "WIN32" /D "os_windows" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /D "_WIN32_WINNT=0x0500" /I "C:\Program Files\Microsoft SDK\Include" /I "C:\Program Files\Microsoft Visual Studio\VC98\Include" /I "..\..\.." /I "..\..\..\mrnet\h" /I "..\..\..\mrnet\xplat\include" /GX /TP /c parser.C \
	flex  -Pmrn ..\..\..\mrnet\src\scanner.l \
	perl ..\..\..\..\..\scripts\vcStripStd.pl < lex.mrn.c > scanner.C \
	del lex.mrn.c \
	cl /nologo /D "WIN32" /D "os_windows" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /D "_WIN32_WINNT=0x0500" /I "C:\Program Files\Microsoft SDK\Include" /I "C:\Program Files\Microsoft Visual Studio\VC98\Include" /I "..\..\.." /I "..\..\..\mrnet\h" /I "..\..\..\mrnet\xplat\include" /GX /TP /c scanner.C \
	

"scanner.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"scanner.C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"parser.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"parser.tab.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"parser.C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\StreamImpl.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\StreamManager.C
# End Source File
# Begin Source File

SOURCE=..\..\..\mrnet\src\utils.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
