# Microsoft Developer Studio Project File - Name="xplat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=xplat - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xplat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xplat.mak" CFG="xplat - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xplat - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "xplat - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xplat - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\mrnet\h" /I "..\..\mrnet\xplat\include" /I "..\..\mrnet" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x0500 /FD /TP /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"libxplat.lib"

!ELSEIF  "$(CFG)" == "xplat - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\mrnet\h" /I "..\..\mrnet\xplat\include" /I "..\..\mrnet" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x0500 /FD /GZ /TP /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"libxplat.lib"

!ENDIF 

# Begin Target

# Name "xplat - Win32 Release"
# Name "xplat - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\Error-win.C"
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\Monitor-win.C"
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\Mutex-win.C"
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\NCIO-win.C"
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\NetUtils-win.C"
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\src\NetUtils.C
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\Once-win.C"
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\PathUtils-win.C"
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\Process-win.C"
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\src\Process.C
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\SharedObject-win.C"
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\SocketUtils-win.C"
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\Thread-win.C"
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\TLSKey-win.C"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\Error.h
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\Monitor-win.h"
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\Monitor.h
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\Mutex-win.h"
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\NCIO.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\NetUtils.h
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\Once-win.h"
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\Once.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\PathUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\Process.h
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\SharedObject-win.h"
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\SharedObject.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\Thread.h
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\src\TLSKey-win.h"
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\TLSKey.h
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\Tokenizer.h
# End Source File
# Begin Source File

SOURCE="..\..\mrnet\xplat\include\xplat\Types-win.h"
# End Source File
# Begin Source File

SOURCE=..\..\mrnet\xplat\include\xplat\Types.h
# End Source File
# End Group
# End Target
# End Project
