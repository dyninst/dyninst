// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_H
#define Sawyer_H

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <boost/cstdint.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <cstdio>
#include <string>

/** @defgroup sawyer Sawyer Support Library
 *
 *  %Sawyer is a library for library writers. It's used by software such as [ROSE](http://rosecompiler.org) and part of %Sawyer
 *  is distributed with ROSE. The complete, canonical documentation for %Sawyer is located
 *  [here](https://www.hoosierfocus.com/~matzke/sawyer). %Sawyer is a library that provides the following:
 *
 *  @li Conditionally enable streams for program diagnostics.  These are C++ <code>std::ostreams</code> organized by software
 *      component and message importance and which can be enabled/disabled with a simple language. A complete plumbing system
 *      similar to Unix file I/O redirection but more flexible can do things like multiplexing messages to multiple locations
 *      (e.g., stderr and syslogd), rate-limiting, colorizing on ANSI terminals, and so on. See Sawyer::Message for details.
 *
 *  @li Logic assertions in the same vein as <tt>\<cassert></tt> but using the diagnostic streams and multi-line output to make
 *      them more readable.  See Sawyer::Assert for details.
 *
 *  @li Progress bars for text-based output using the diagnostic streams so that progress bars interact sanely with other
 *      diagnostic output.  See Sawyer::ProgressBar for details.
 *
 *  @li Command-line parsing to convert program switches and their arguments into C++ objects with the goal of being able to
 *      support nearly any command-line style in use, including esoteric command switches from programs like tar and
 *      dd. Additionally, documentation can be provided in the C++ source code for each switch and Unix man pages can be
 *      produced. See Sawyer::CommandLine for details.
 *
 *  @li Container classes: @ref Sawyer::Container::Graph "Graph", storing vertex and edge connectivity information along with
 *      user-defined values attached to each vertex and edge, sequential ID numbers, and constant time complexity for most
 *      operations; @ref Sawyer::Container::IndexedList "IndexedList", a combination list and vector having constant time
 *      insertion and erasure and constant time lookup-by-ID; @ref Sawyer::Container::Interval "Interval" represents integral
 *      intervals including empty and whole intervals; @ref Sawyer::Container::IntervalSet "IntervalSet" and @ref
 *      Sawyer::Container::IntervalMap "IntervalMap" similar to STL's <code>std::set</code> and <code>std::map</code>
 *      containers but optimized for cases when very large numbers of keys are adjacent; @ref Sawyer::Container::Map "Map",
 *      similar to STL's <code>std::map</code> but with an API that's consistent with other containers in this library; @ref
 *      Sawyer::Container::BitVector "BitVector" bit vectors with operations defined across index intervals.  These and many
 *      others can be found in the Sawyer::Container namespace.
 *
 *   @li Multi-threading support: %Sawyer is designed to be used in multi-threaded programs. It also provides some capabilities
 *       not usually found in thread libraries, such as the ability to execute user-supplied functions concurrently subject to
 *       user-defined constraints on their execution order (see @ref Sawyer::ThreadWorkers).
 *
 *   @li Miscellaneous: @ref Sawyer::SynchronizedPoolAllocator "PoolAllocator" (synchronized and unsynchronized variants) to
 *       allocate memory from large pools rather than one object at a time; and @ref Sawyer::SmallObject "SmallObject", a base
 *       class for objects that are only a few bytes; @ref Sawyer::Stopwatch "Stopwatch" for high-resolution elapsed time; @ref
 *       Sawyer::Optional "Optional" for optional values.
 *
 *  Design goals for this library can be found in the [Design goals](group__design__goals.html) page.
 *
 *  Installation instructions can be found on the [Installation](group__installation.html) page.
 *
 *  Other things on the way but not yet ready:
 *
 *  @li A simple, extensible, terse markup language that lets users write documentation that can be turned into TROFF, HTML,
 *      Doxygen, PerlDoc, TeX, etc. The markup language supports calling of C++ functors to transform the text as it is
 *      processed.
 *
 *  @li C/C++ lexical analyzer
 *
 *  <b>The @ref Sawyer namespace is a good place to start for documentation.</b> */


/** @defgroup sawyer_installation Installation
 *  @ingroup sawyer
 *
 *  %Sawyer can be downloaded from <a href="https://github.com/matzke1/sawyer">GitHub</a>. For example:
 *
 * @code
 *  $ SAWYER_SRC=/directory/in/which/to/store/sources
 *  $ git clone https://github.com/matzke1/sawyer $SAWYER_SRC
 * @endcode
 *
 *  %Sawyer uses <a href="http://www.cmake.org/">cmake</a> as its configuration and build system and building is typically
 *  performed in a separate directory from the source. The author usually creates various "_build-whatever" directories at the
 *  top level of the source code directory, but the build directories can be located anywhere. CMake operates in two steps:
 *  first one configures the build environment using the "cmake" command, then the library is built and installed.  Here is a
 *  typical configuration step:
 *
 * @code
 *  $ SAWYER_BLD=/some/directory/in/which/to/build
 *  $ mkdir $SAWYER_BLD
 *  $ cd $SAWYER_BLD
 *  $ cmake $SAWYER_SRC -DBOOST_ROOT=/location/of/boost -DCMAKE_INSTALL_PREFIX:PATH=/place/to/install/sawyer
 * @endcode
 *
 *  The @c BOOST_ROOT should be the directory containing Boost's "include" and "lib" directories. It's necessary only when
 *  Boost isn't installed in a well-known location.  In addition to Boost header files, %Sawyer also requires these Boost
 *  libraries: iostreams, system, filesystem, regex, chrono, and thread. These libraries should have been compiled with the
 *  same C++ compiler and switches as %Sawyer.
 *
 *  The @c CMAKE_BUILD_TYPE can be specified to control whether a debug or release version of the library is created. Its value
 *  should be the word "Debug" or "Release". The default is "Release".  For instance, "-DCMAKE_BUILD_TYPE=Debug".
 *
 *  The @c CMAKE_INSTALL_PREFIX is the directory that will contain the "include" and "lib" subdirectories where %Sawyer is
 *  eventually installed. %Sawyer is also designed to be used directly from the build directory if desired.
 *
 *  CMake will configure %Sawyer to use the system's multi-threading support by default.  If you plan to use %Sawyer in only
 *  single-threaded programs you can avoid the thread dependencies by providing the "-DTHREAD_SAFE:BOOL=no" switch, in which
 *  case even those parts of the API that are documented as being thread-safe will probably not be safe. Within a program that
 *  uses %Sawyer, one can check whether multi-thread support was enabled by checking the @c SAWYER_MULTI_THREADED C
 *  preprocessor symbol; it will be defined as zero if multi-threading is disabled, and non-zero otherwise.  The @c
 *  SAWYER_THREAD_TRAITS will point to one of the @ref Sawyer::SynchronizationTraits specializations, and
 *  <code>SAWYER_THREAD_TRAITS::SUPPORTED</code> is true or false depending on whether multi-threading is supported.
 *
 *  The next step after configuration is to build the library and tests:
 *
 * @code
 *  $ cd $SAWYER_BLD
 *  $ make
 * @endcode
 *
 *  Parallel building is supported with "make -jN" where @e N is the maximum number of parallel compile commands to allow.  If
 *  you need to see the compilation commands that are executed, add "VERBOSE=1" to the "make" command.
 *
 *  Finally, the library can be installed and the build directory can be removed:
 *
 * @code
 *  $ cd $SAWYER_BLD
 *  $ make install
 * @endcode
 *
 *  The current build system makes no attempt to name libraries in ways that distinguish the Boost version, build type,
 *  thread-support, compiler version, etc.  We recommend that %Sawyer be used only in programs that are built with compatible
 *  configuration, and the CMAKE_INSTALL_PREFIX can be used to create a naming scheme to install multiple %Sawyer
 *  configurations on one machine.
 *
 *  @section mingw Cross compiling on Linux targeting Microsoft Windows
 *
 *  %Sawyer is not regularly tested on Microsoft platforms, but the MinGW compiler is supported.  On Debian this compiler can be
 *  installed with "sudo apt-get install mingw-w64".  One also needs cross-compiled Boost libraries, that can be created with
 *  these steps:
 *
 *  @li Download and untar the Boost source code (this example uses version 1.47).
 *  @li Run "boostrap.sh" in the Boost source directory
 *  @li Edit project-config.jam and replace the (commented out) "using gcc" line with "using gcc : mingw32 :
 *      i586-mingw32msvc-g++ ;". The space before the semi-colon is required.
 *  @li Create the installation directory. This example uses "$HOME/lib-mingw32/boost-1.47".
 *  @li Compile Boost with the following commands. The NO_ZLIB and NO_BZIP2 are for the iostreams package since we didn't
 *      cross compile zlib or bzip2 (%Sawyer doesn't use those features of iosreams).
 *
 * @code
 *  $ ./bjam --help
 *  $ ./bjam install toolset=gcc-mingw32 \
 *        --prefix=$HOME/lib-mingw32/boost-1.47 \
 *        --layout=versioned \
 *        --with-iostreams --with-system --with-filesystem --with-regex --with-chrono \
 *        -sNO_ZLIB=1 -sNO_BZIP2=1
 * @endcode
 *
 *  To compile %Sawyer use the same cmake command as for native compiling, but add
 *  "-DCMAKE_TOOLCHAIN_FILE=$SAWYER_SRC/Toolchain-cross-mingw32-linux.cmake". Also make sure the @c BOOST_ROOT parameter points
 *  to the cross-compiled version of Boost, which must be installed in a directory that's at or below one of the @c
 *  CMAKE_FIND_ROOT_PATH directories specified in the Toolchain-cross-mingw32-linux.cmake. Then run "make" and "make install"
 *  like normal.
 *
 *  @section msvc Native compiling with Microsoft Visual Studio
 *
 *  %Sawyer is seldom tested in this manner, but Microsoft Visual Studio versions 10 2010 and 12 2013 have been tested at
 *  time or another.  To install Boost, follow the instructions at the <a href="http://boost.org">Boost web site</a>. The
 *  instructions didn't work for my virtual machine with Windows 8.1 Enterprise and Visual Studio 12 2013, so I used
 *  precompiled Boost libraries I found at boost.org [Matzke].
 *
 *  Generate a Visual Studio project file with a command like this, and the usual CMake options described above.  IIRC, cmake
 *  expects Windows-style path names having backslashes instead of POSIX names.
 *
 * @code
 *  cmake -G "Visual Studio 12 2013" ...
 * @endcode
 *
 *  Then fire up the Visual Studio IDE, open the project file, right click on a "solution", and select "Build". */

/** @defgroup sawyer_design_goals Library design goals
 *  @ingroup sawyer
 *
 *  Goals that influence the design of this library.
 *
 *  @li The API should be well documented and tested.  Every public symbol is documented with doxygen and includes all
 *      pertinent information about what it does, how it relates to other parts of the library, restrictions and caveats, etc.
 *      This kind of information is not evident from the C++ interface itself and is often omitted in other libraries'
 *      documentation.
 *
 *  @li The library should be easy to use yet highly configurable. Common things should be simple and terse, but less common
 *      things should still be possible without significant work. Users should not have to worry about who owns what
 *      objects--the library uses reference counting pointers to control deallocation.  The library should use a consistent
 *      naming scheme. It should avoid extensive use of templates since they're a common cause of difficulty for beginners.
 *
 *  @li The library should be familiar to experienced C++ programmers. It should use the facilities of the C++ language, C++
 *      idioms, and an object oriented design.  The API should not look like a C library being used in a C++ program, and the
 *      library should not be concerned about being used in languages that are not object oriented.
 *
 *  @li The library should be safe to use.  Errors should be handled in clear, recoverable, and predictable ways. The library
 *      should make every attempt to avoid situations where the user can cause a non-recoverable fault due to misunderstanding
 *      the API.
 *
 *  @li Functionality is more important than efficiency. Every attempt is made to have an efficient implementation, but when
 *      there is a choice between functionality and efficiencey, functionality wins. */


/** @defgroup sawyer_smart_pointers Reference counting smart pointers
 *  @ingroup sawyer
 *
 *  Pointers that automatically delete the underlying object.
 *
 *  The library makes extensive use of referencing counting smart pointers.  It uses the following paradigm consistently for
 *  any class that intends to be reference counted.
 *
 *  @li The class shall mark all of its normal C++ constructors as having protected access.  This is to prevent users from
 *      allocating such objects statically or on the stack, yet allowing subclasses to use them.
 *
 *  @li For each class their shall be defined a "Ptr" type which is the smart pointer type for the class.  Class templates
 *      define this type as a public member; non-template classes may define it as a public member and/or in the same namespace
 *      as the class. In the latter case, the type name will be "Ptr" appended to the class name.
 *
 *  @li The class shall have a static <code>instance</code> method corresponding to each C++ constructor. Each such
 *      class method takes the same arguments as one of the constructors, allocates and constructs and object with
 *      <code>new</code>, and returns a <code>Ptr</code>. These methods are usually public.
 *
 *  @li The class shall have a public destructor only for the sake of the smart pointer.
 *
 *  @li If a class hierarchy needs virtual constructors they shall be named <code>create</code>.
 *
 *  @li If a class needs virtual copy constructors they shall be named <code>copy</code>.
 *
 *  @li The class shall have a factory function corresponding to each public <code>instance</code> method.
 *
 *  @li Factory functions shall have the same name as the class, but an initial lower-case letter.
 *
 *  A simple example:
 *
 * @code
 *  class SomeClass {
 *      int someData_;           // underscores are used for private data members
 *
 *  protected:
 *      SomeClass(): someData_(0) {}
 *
 *      explicit SomeClass(int n): someData_(n) {}
 *
 *  public:
 *      typedef Sawyer::SharedPointer<SomeClass> Ptr;
 *
 *      static Ptr instance() {
 *          return Ptr(new SomeClass);
 *      }
 *
 *      static Ptr instance(int n) {
 *          return Ptr(new SomeClass(n));
 *      }
 *  };
 *
 *  SomeClass::Ptr someClass() {
 *      return SomeClass::instance();
 *  }
 *
 *  SomeClass::Ptr someClass(int n) {
 *      return SomeClass::instance(n);
 *  }
 * @endcode */


/** @defgroup sawyer_class_properties Class properties
 *  @ingroup sawyer
 *
 *  Data members that that store a simple value.
 *
 *  Class data members that act like user-accessible properties are declared with private access. As with all private data
 *  members, they end with an underscore.  The class provides a pair of methods for accessing each property--one for reading the
 *  property and one for modifying the property. Some properties are read-only in which case only the writer is provided.
 *
 *  All writer properties return a reference to the object that is modified so that property settings can be chained.  If the
 *  class uses the reference-counting smart-pointer paradigm, then a pointer to the object is returned instead. (See @ref
 *  sawyer_smart_pointers).
 *
 * @code
 *  class SomeClass {
 *      int someProperty_;
 *  public:
 *      int someProperty() const {
 *          return someProperty_;
 *      }
 *      SomeClass& someProperty(int someProperty) {
 *          someProperty_ = someProperty;
 *          return *this;
 *      }
 *  };
 * @endcode */

/** @defgroup sawyer_examples Examples
 *  @ingroup sawyer */

// Version numbers (conditional compiliation is only so we can test version mismatch handling)
#ifndef SAWYER_VERSION_MAJOR
#define SAWYER_VERSION_MAJOR    0
#define SAWYER_VERSION_MINOR    1
#define SAWYER_VERSION_PATCH    0
#endif

// Macros for thread-safety portability. This allows Sawyer to be compiled with or without thread support and not have a huge
// proliferation of conditional compilation directives in the main body of source code.
#ifdef _REENTRANT
    #define SAWYER_MULTI_THREADED 1
    #define SAWYER_THREAD_TRAITS Sawyer::SynchronizationTraits<Sawyer::MultiThreadedTag>
#else
    #define SAWYER_MULTI_THREADED 0
    #define SAWYER_THREAD_TRAITS Sawyer::SynchronizationTraits<Sawyer::SingleThreadedTag>
#endif

#ifdef _REENTRANT
    #if __cplusplus >= 201103L
       #define SAWYER_THREAD_LOCAL thread_local
    #elif defined(_MSC_VER)
        // Visual C++, Intel (Windows), C++ Builder, Digital Mars C++
        #define SAWYER_THREAD_LOCAL __declspec(thread)
    #else
       // Solaris Studio, IBM XL, GNU, LLVM, Intel (linux)
       #define SAWYER_THREAD_LOCAL __thread
    #endif
#else
     #define SAWYER_THREAD_LOCAL /*void*/
#endif

#ifdef BOOST_WINDOWS
// FIXME[Robb Matzke 2014-06-18]: get rid of ROSE_UTIL_EXPORTS; cmake can only have one DEFINE_SYMBOL
#   if defined(SAWYER_DO_EXPORTS) || defined(ROSE_UTIL_EXPORTS) // defined in CMake when compiling libsawyer
#       define SAWYER_EXPORT __declspec(dllexport)
#       if _MSC_VER
#           define SAWYER_EXPORT_NORETURN __declspec(dllexport noreturn)
#       else
            // MinGW complains about __declspec(dllexport noreturn), so use only __declspec(dllexport) instead.
#           define SAWYER_EXPORT_NORETURN __declspec(dllexport)
#       endif
#   else
#       define SAWYER_EXPORT __declspec(dllimport)
#       define SAWYER_EXPORT_NORETURN __declspec(noreturn)
#   endif
#else
#   define SAWYER_EXPORT /*void*/
#   define SAWYER_EXPORT_NORETURN /*void*/
#endif

#define SAWYER_LINKAGE_INFO SAWYER_VERSION_MAJOR, SAWYER_VERSION_MINOR, SAWYER_VERSION_PATCH, SAWYER_MULTI_THREADED
#define SAWYER_CHECK_LINKAGE Sawyer::initializeLibrary(SAWYER_LINKAGE_INFO)
    

/** Name space for the entire library.  All %Sawyer functionality except for some C preprocessor macros exists inside this
 * namespace.  Most of the macros begin with the string "SAWYER_". */
namespace Sawyer {
    
/** Explicitly initialize the library.
 *
 *  This initializes any global objects provided by the library to users.  This happens automatically for many API calls, but
 *  sometimes needs to be called explicitly. It can be called as often as desired; each call checks caller-callee consistency
 *  (version number and configuration), but only the first call does any initialization. The function always returns true. */
SAWYER_EXPORT bool initializeLibrary(size_t vmajor=SAWYER_VERSION_MAJOR,
                                     size_t vminor=SAWYER_VERSION_MINOR,
                                     size_t vpatch=SAWYER_VERSION_PATCH,
                                     bool withThreads=SAWYER_MULTI_THREADED);

/** Portable replacement for ::strtoll
 *
 *  Microsoft doesn't define this function, so we define it in the Sawyer namespace. */
SAWYER_EXPORT boost::int64_t strtoll(const char*, char**, int);

/** Portable replacement for ::strtoull
 *
 *  Microsoft doesn't define this function, so we define it in the Sawyer namespace. */
SAWYER_EXPORT boost::uint64_t strtoull(const char*, char**, int);

/** Reads one line of input from a file.
 *
 *  Returns one line, including any line termination.  Returns an empty string at the end of the file. */
SAWYER_EXPORT std::string readOneLine(FILE*);

/** Semi-portable replacement for popen. */
SAWYER_EXPORT FILE *popen(const std::string&, const char *how);

/** Semi-portable replacement for pclose. */
SAWYER_EXPORT int pclose(FILE*);

/** Generate a sequential name.
 *
 *  A new string is generated each time this is called. */
SAWYER_EXPORT std::string generateSequentialName(size_t length=3);

} // namespace

// Define only when we have the Boost Chrono library, which was first available in boost-1.47.
//#define SAWYER_HAVE_BOOST_CHRONO


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Compiler portability issues
//
// The following macros are used to distinguish between different compilers:
//     _MSC_VER         Defined only when compiled by Microsoft's MVC C++ compiler. This macro is predefined by Microsoft's
//                      preprocessor.
//
// The following macros are used to distinguish between different target environments, regardless of what compiler is being
// used or the environment which is doing the compiling.  For instance, BOOST_WINDOWS will be defined when using the MinGW
// compiler on Linux to target a Windows environment.
//     BOOST_WINDOWS    The Windows API is present.  This is defined (or not) by including <boost/config.hpp>.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
//--------------------------
// Microsoft Windows
//--------------------------

# define SAWYER_ATTR_UNUSED /*void*/
# define SAWYER_ATTR_NORETURN /*void*/
# define SAWYER_PRETTY_FUNCTION __FUNCSIG__
# define SAWYER_MAY_ALIAS /*void*/
# define SAWYER_STATIC_INIT /*void*/
# define SAWYER_DEPRECATED(WHY) /*void*/

// Microsoft compiler doesn't support stack arrays whose size is not known at compile time.  We fudge by using an STL vector,
// which will be cleaned up propertly at end of scope or exceptions.
# define SAWYER_VARIABLE_LENGTH_ARRAY(TYPE, NAME, SIZE) \
    std::vector<TYPE> NAME##Vec_(SIZE);                 \
    TYPE *NAME = &(NAME##Vec_[0]);

#elif defined(__APPLE__) && defined(__MACH__)
//--------------------------
// Apple OSX, iOS, Darwin
//--------------------------

# define SAWYER_ATTR_UNUSED /*void*/
# define SAWYER_ATTR_NORETURN /*void*/
# define SAWYER_PRETTY_FUNCTION __PRETTY_FUNCTION__
# define SAWYER_MAY_ALIAS /*void*/
# define SAWYER_STATIC_INIT /*void*/
# define SAWYER_DEPRECATED(WHY) /*void*/

// Apple compilers doesn't support stack arrays whose size is not known at compile time.  We fudge by using an STL vector,
// which will be cleaned up propertly at end of scope or exceptions.
# define SAWYER_VARIABLE_LENGTH_ARRAY(TYPE, NAME, SIZE) \
    std::vector<TYPE> NAME##Vec_(SIZE);                 \
    TYPE *NAME = &(NAME##Vec_[0]);


#else
//--------------------------
// Other, GCC-based
//--------------------------

# define SAWYER_ATTR_UNUSED __attribute__((unused))
# define SAWYER_ATTR_NORETURN __attribute__((noreturn))
# define SAWYER_PRETTY_FUNCTION __PRETTY_FUNCTION__
# define SAWYER_MAY_ALIAS __attribute__((may_alias))
# define SAWYER_DEPRECATED(WHY) __attribute__((deprecated))

// Sawyer globals need to be initialized after the C++ standard runtime, but before other user-level stuff. The constant 101
// causes the initialization to happen as early as possible after the C++ runtime.
# define SAWYER_STATIC_INIT __attribute__((init_priority(101)))

#include "compiler_diagnostics.h"
# define SAWYER_VARIABLE_LENGTH_ARRAY(TYPE, NAME, SIZE) \
    DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA_ALL TYPE NAME[SIZE]; DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA_ALL

#endif

#define SAWYER_CONFIGURED /*void*/

#endif
