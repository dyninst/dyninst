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
# ADD CPP /nologo /W3 /GX /O2 /I ".." /I "..\.." /I "..\..\mrnet\h" /I "..\..\mrnet\xplat\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /D _WIN32_WINNT=0x0500 /D "os_windows" /FD /TP /c
# SUBTRACT CPP /YX
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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".." /I "..\.." /I "..\..\mrnet\h" /I "..\..\mrnet\xplat\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /D _WIN32_WINNT=0x0500 /D "os_windows" /FD /GZ /TP /c
# SUBTRACT CPP /YX
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

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\mrnet\src\BackEndNode.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\byte_order.c
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\ChildNode.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\CommunicationNode.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\CommunicatorImpl.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\DataElement.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\EndPointImpl.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\Errors.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\EventImpl.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\Filter.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\FilterDefinitions.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\FrontEndNode.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\InternalNode.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\Message.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\Network.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\NetworkGraph.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\NetworkImpl.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\Packet.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\ParentNode.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\pdr.c
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\pdr_mem.c
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\pdr_sizeof.c
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\RemoteNode.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\StreamImpl.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\StreamManager.C
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\utils.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\mrnet\src\BackEndNode.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\byte_order.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\ChildNode.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\CommunicationNode.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\CommunicatorImpl.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\DataElement.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\EndPointImpl.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\Errors.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\EventImpl.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\Filter.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\FilterDefinitions.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\FrontEndNode.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\InternalNode.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\Message.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\NetworkGraph.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\NetworkImpl.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\Packet.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\ParentNode.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\pdr.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\pdr_mem.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\refCounter.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\RemoteNode.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\StreamImpl.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\StreamManager.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\Types.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\src\utils.h
# End Source File
# End Group
# End Target
# End Project
