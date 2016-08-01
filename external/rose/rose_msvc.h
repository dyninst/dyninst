// This is support for Microsoft Visual Studio C++ use with ROSE
// (it is initial work with no clear idea if MSVC will be supported in the future).



// DQ (3/22/2009): Added support for detection of Microsoft specific usage.
// Determine if this is a WIN32 (e.g., Windows NT or Windows 95) system.
#ifndef ROSE_WIN32
  #if defined(__WATCOMC__) && defined(__NT__)
 // Some versions of the Watcom compiler fail to set _WIN32.  Set ROSE_WIN32 when running the Watcom compiler on NT.
    #define ROSE_WIN32 1
  #endif
#endif

#ifndef ROSE_WIN32
  #ifdef _WIN32
    #define ROSE_WIN32 1
  #else
    #define ROSE_WIN32 0
  #endif
#endif


#ifndef ROSE_MSDOS
  #if defined(MSDOS) || defined(__MSDOS__)
// Turbo-C defines __MSDOS__ and Microsoft C defines MSDOS, so this is MS-DOS.
    #define ROSE_MSDOS 1
  #else
    #define ROSE_MSDOS 0
  #endif
#endif

// Set a flag that indicates that some Microsoft operating system is being used.
#ifndef ROSE_MICROSOFT_OS
  #if ROSE_WIN32 || ROSE_MSDOS
    #define ROSE_MICROSOFT_OS 1
  #else
    #define ROSE_MICROSOFT_OS 0
  #endif
#endif

// If this is a Microsoft operating system, as indicated by the macro
// "ROSE_MICROSOFT_OS", determine which compiler it is.  Borland, Zortech,
// and Microsoft are supported.
#if ROSE_MICROSOFT_OS
  #ifdef __TURBOC__
// Borland's (Turbo-C or C++) library is ANSI compatible.
    #define __ANSIC__ 1
  #else /* __TURBOC__ */
    #ifdef __ZTC__
   // Zortech's library is ANSI compatible.
      #define __ANSIC__ 1
    #else /* __ZTC__ */
   // Then it must be MSC.
      #define __MSC__ 1
   // MSC's library is ANSI compatible.
      #define __ANSIC__ 1
    #endif
  #endif
#endif

// If this has not be set yet, then set it explicitly
#ifndef __MSC__
  #define __MSC__ 0
#endif



#if 0
// These options are specific to the platforms and the Microsoft Visual C++ compilers supported by the MKS Toolkit.

// _MSC_VER : defines the compiler version for the versions supported by the current version of MKS Toolkit. Possible values include:
//         Microsoft Visual C++ 7.1     _MSC_VER = 1310
//         Microsoft Visual C++ 7.0     _MSC_VER = 1300
//         Microsoft Visual C++ 6.0     _MSC_VER = 1200
//         Microsoft Visual C++ 5.0     _MSC_VER = 1100
// 
// _WIN32 : is defined for Win32 applications and is always defined as 1.
// 
// _M_IX86 : defines the processor. Possible values include:
//         Blend                _M_IX86 = 500 
//         Pentium              _M_IX86 = 500
//         Pentium Pro  _M_IX86 = 600
//         80386                _M_IX86 = 300
//         80486                _M_IX86 = 400
#endif


// DQ (3/22/2009): MS does not define some of the standard Linux types
#if ROSE_MICROSOFT_OS
// Using boost/cstdint.hpp instead.
// typedef uint64_t unsigned long long;

// DQ (3/22/2009): This is defined in <linux/limits.h>, for MS we pick a value consistatn with MKS.
// Expanded API PATH_MAX. The size of PATH_MAX used by the MKS Toolkit UNIX APIs has been increased 
// to 4096 bytes on Windows NT/2000/XP/2003 systems.
#define PATH_MAX 4096

// This is defined in <sys/param.h> but MSVS does not support that header file.
#define MAXPATHLEN PATH_MAX

#if 0 // def _MSC_VER
// DQ (11/27/2009): Needed to cope with bug in MS library: it fails to define min/max 
template <class T>
inline T max(const T& a, const T& b) 
   {
     return (a > b) ? a : b;
   }

template <class T>
inline T min(const T& a, const T& b) 
   { 
     return (a < b) ? a : b;
   }
#endif 

// DQ (11/27/2009): Needed to cope with bug in MS library: it fails to define min/max 
// This solution is from the web at: http://www.codeproject.com/Messages/3178857/Overcoming-problem-with-std-min-std-max-sharpdefin.aspx
// And then I simplified it to just undefine the min and max macros (which seems to be all that is required.
// #define NOMINMAX
// #ifndef max
// #define max(a,b)            (((a) > (b)) ? (a) : (b))
// #endif
// #ifndef min
// #define min(a,b)            (((a) < (b)) ? (a) : (b))
// #endif
// #include <afxcontrolbars.h>
#define NOMINMAX
#undef max
#undef min
// ...contine to use std::mix, std::max from here on

#ifdef _MSC_VER
// DQ (11/27/2009): "__func__" is C99, but MSVC uses "__FUNCTION__" instead.
#define __func__ __FUNCTION__
#endif

// DQ (12/28/2009): Moved this from where is was placed by Thomas in the ROSETTA grenerated code.
// This simplifies the generated code to support splitting large generated files into smaller files.
// tps (11/25/2009) : Added ssize_t for Windows
// tps (01/04/2010) LONG_PTR is defined in windows.h
#ifdef _MSC_VER
#include <windows.h>
typedef LONG_PTR ssize_t;
#endif

#endif
