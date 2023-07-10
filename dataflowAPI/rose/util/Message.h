// See also rose::Diagnostics in $ROSE/src/roseSupport/Diagnostics.h
// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_Message_H
#define Sawyer_Message_H

#include "Map.h"
#include "Optional.h"
#include "Sawyer.h"
#include "SharedPointer.h"
#include "Synchronization.h"

#include <stddef.h>
#include <utility>
#include <boost/config.hpp>
#include <boost/logic/tribool.hpp>
#include <cassert>
#include <cstring>
#include <list>
#include <ostream>
#include <set>
#include <streambuf>
#include <string>
#include <vector>

namespace Sawyer {

/** Formatted diagnostic messages emitted to various backends.
 *
 *  This namespace provides functionality to conditionally emit diagnostic messages based on software component and message
 *  importance.  Although the library has extensive capabilities for controlling the message format and where to send the
 *  message, most uses of this namespace are to emit a message, so we'll describe that by way of introduction...
 *
 * @section emitting Emitting a message
 *
 * @code
 *  using namespace Sawyer::Message;
 *  mlog[INFO] <<"about to process \"" <<filename <<"\"\n"
 * @endcode
 *
 *  The @c mlog is a message Facility that holds a number of streams, each of which can be enabled or disabled. Inserting to a
 *  disabled message stream causes the message to be thrown away.  The @c INFO is a message Importance, of which the library
 *  defines eight levels ranging from debug through fatal.  The expression <code>mlog[INFO]</code> selects one of the message
 *  Stream objects from the facility. The linefeed marks the end of a message; messages are "partial" until the linefeed is
 *  inserted, but depending on the sink (final destination) even partial messages might be shown. Since a message stream is a
 *  subclass of std::ostream, all the usual output insertion operators just work.
 *
 *  One of the benefits of using this namespace is that the output will have a consistent look and feel. It can even be
 *  colorized based on message importance (at least for ANSI terminal output). The output will look something like this:
 *
 * @code
 *  identityTranslator[31044] 0.00128 disassembler[INFO]: about to process "grep"
 * @endcode
 *
 *  The "identityTranslator" is the name of the executable, the 31044 is the process ID, "0.00128" is the time in seconds since
 *  the program started, "disassembler" is the software component that is reporting this messsage (i.e., the messsage
 *  facility), and "INFO" is the importance level.
 *
 * @section facility Defining facilities
 *
 *  Before one can start emitting messages, he needs to define a message Facility (@c mlog in the first example).  The
 *  library defines one facility, Sawyer::Message::mlog, which can be used, but it's better to define a message facility for
 *  each software component because then messages can be controlled per software component, and the component will be clearly
 *  indicated in the output ("disassembler" in the previous example). A software component could be a function, class,
 *  namespace, entire library, or entire program--whatever granularity one wants.
 *
 *  Here's an example that creates a facility at class level:
 *
 * @code
 *  class Disassembler {
 *      static Sawyer::Message::Facility mlog;
 *      ...
 *  };
 *
 *  Sawyer::Message::Facility Disassembler::mlog("disassembler");
 * @endcode
 *
 *  The @c mlog is constructed in a useable state with all streams enabled and output going to standard error via file
 *  descriptor 2. It is given a string name, "disassembler", that will show up in the output.  The class may want to make two
 *  adjustments at run-time:
 *
 * @code
 *  void Disassembler::initClass() {
 *      mlog.initStreams(destination);
 *      mfacilities.insert(mlog);
 *  }
 * @endcode
 *
 *  The @c initStreams call reinitializes the message streams so they use your library's message plumbing, @c destination, which
 *  is a Sawyer::Message::DestinationPtr (types whose names end with "Ptr" point to reference counted objects). The streams had
 *  previously been using Sawyer::Message::merr, but pointing them all to your destination means that the destination
 *  for all messages can be changed in one place: if you want messages to go to syslogd, you change how @c destination is
 *  created. As we'll see below, the destination also controls things like colored output.
 *
 *  The <code>mfacilities.insert</code> call registers the new @c mlog with a global Facilities object, which we describe
 *  next...
 *
 * @section facilities Collections of facility objects
 *
 *  Any combination of Facility objects can be grouped together into one or more Facilities (note plural) objects so they can
 *  be enabled and disabled collectively. A Facilities object, also referred to as a facility group, is able to parse a
 *  simple language provided as a string, which is often provided from the command-line parser.
 *
 *  This example creates three facilities and groups them together:
 *
 * @code
 *  Facility mlog1("disassembler", destination);
 *  Facility mlog2("c++ parser", destination)
 *  Facility mlog3("StringUtility::split", destination);
 *
 *  Facilities facilities;
 *  facilities.insert(mlog1);
 *  facilities.insert(mlog2, "cpp_parser");
 *  facilities.insert(mlog3);
 * @endcode
 *
 *  The three @c insert calls incorporate the three facilities into the group by reference and give them names in the
 *  configuration language.  The names in the config language must look like C++ names (including <code>::</code> and
 *  <code>.</code> operators), so the second @c insert needs to supply a valid name. The other two use the facility's name,
 *  which is also the string that is printed when messages are output. A unique, non-empty name must be supplied for each
 *  facility in a group, although the names for the facilities themselves need not be unique or even non-empty. The library
 *  provides Sawyer::Message::mfacilities and users are encouraged to use it as their primary facility group.
 *
 *  The facilities in a group can be controlled (enabled or disabled) with the Facilities::control method, which takes a string
 *  in the control language (see method for details).  For example, to enable output for all levels of importance except
 *  debugging, across all registered facilities, but only warning and higher messages for the disassembler's facility:
 *
 * @code
 *  facilities.control("all, !debug, disassembler(>=warn)")
 * @endcode
 *
 *  The terms are processed from left to right, but only if the entire string is valid. If the string is invalid then an
 *  exception is thrown that includes an error message and the exact position in the string where the error occurred.
 *
 * @section plumbing Plumbing lattice
 *
 *  Inserting a message to a stream with <code><<</code> operators is only half the story--the plumbing lattice deals with how
 *  a message is conveyed to the final destination(s) and what transformations happen along the way. The plumbing is a run-time
 *  constructed lattice containing internal nodes and leaf nodes. Each leaf node is a final destination, or sink, for a message
 *  and can be a C++ <code>std::ostream</code>, a C @c FILE pointer, a Unix file descriptor, the syslog daemon, or a
 *  user-defined destination.  The internal nodes serve as multiplexers, filters, rate limiters, etc.
 *
 *  All plumbing lattice nodes are dynamically allocated and reference counted. Instead of using the normal C++ constructors,
 *  plumbing objects are created with static @c instance methods that perform the allocation and return a smart pointer. The
 *  type names for smart pointers are the class names with a "Ptr" suffix, and @ref Sawyer::SharedPointer serves as the
 *  implementation.
 *
 *  For instance, a lattice that accepts any kind of message, limits the output to at most one message per second,
 *  and sends the messages to standard error and a file can be constructed like this:
 *
 * @code
 *  DestinationPtr destination = TimeFilter::instance(1.0)
 *                                 ->to(FdSink::instance(2),     //standard error on Unix
 *                                      FileSink::instance(f));  // f is some FILE* we opened already
 * @endcode
 *
 *  The @c destination can then be used to construct streams and facilities. The Destination class is the base class for all
 *  plumbing lattice nodes, thus DestinationPtr is the base class for all pointers to nodes.  Of course, similar redirection
 *  can be done outside the program using standard Unix tools and file redirection, except that performing redirection via
 *  a plumbing lattice has a few advantages:
 *
 * @li the program itself can control the redirection
 * @li different paths through the lattice can result in different message formats
 * @li final destinations can be programmatic, such as writing to syslogd or a database
 * @li the library might do a better presentation when two streams go to the same destination
 *
 * @section tips Tips and tricks
 *
 * @subsection name Naming tips
 *
 *  Avoid using the name "log" since it may conflict with <code>\::log</code> from math.h.
 *
 *  If facilities are consistently named (e.g., always "mlog") then code that emits messages can always use that name and will
 *  automatically get the most narrowly scoped facility that's visible according to C++ visibility rules, which is probably the
 *  facility that is most applicable.
 *
 *  A <code>using namespace Sawyer::Message</code> will prevent you from having to qualify the message importance symbols, but
 *  if you name your facilities "mlog" they may be ambiguous with Sawyer::Message::mlog.  Sawyer also provides the namespace
 *  Sawyer::Message::Common, which defines only the most commonly used types (the importance levels, Stream, Facility, and
 *  Facilities).
 *
 * @subsection perf Performance tips
 *
 *  Although every attempt was made to keep the library efficient, when features and performance conflict, features win.  If
 *  messages insert operators <code><<</code> are expensive then it is best to avoid calling them when the stream to which
 *  they're inserted is disabled--they'd just be thrown away anyway.  Programs that already emit messages to streams probably
 *  use @c if statements or conditional compilation already, and those constructs can continue to be used.  In fact, when a
 *  stream is evaluated in a Boolean context it returns true when enabled and false when disabled:
 *
 * @code
 *  if (mlog[DEBUG])
 *      mlog[DEBUG] <<"the result is " <<something_expensive() <<"\n";
 * @endcode
 *
 *  When @c if statements are used extensively for this purpose in a project whose style guide mandates that the body must be
 *  on the following line, the logging code occupies twice as much vertical space as necessary (or three or four times, with
 *  curly braces) and can become quite obtrusive. Here are two other ways to get the same effect:
 *
 * @code
 *  mlog[DEBUG] and mlog[DEBUG] <<"the result is " <<something_expensive() <<"\n";
 *  SAWYER_MESG(mlog[DEBUG]) <<"the result is " <<something_expensive() <<"\n";
 * @endcode
 *
 * @subsection temp Temporary streams
 *
 *  Although <code>std::ostream</code> objects are not copyable, and not movable before c++11, Sawyer's message streams
 *  implement a form of copying/moving. Namely, a Stream copy constructor can be used to create a new message stream identical
 *  to the source message stream, and any partial message in the source stream is moved to the destination stream.  This
 *  enables a way to create a temporary stream for use inside a function:
 *
 * @code
 * void longFunctionWithLotsOfDebugging() {
 *     Stream debug(mlog[DEBUG]);
 *     ...
 *     debug <<"got here\n";
 * }
 * @endcode
 *
 * @subsection partial Partial messages
 *
 *  One problem that authors often encounter when emitting diagnostics is that they want to print an incomplete line, then
 *  perform some work, then complete the line.  But if the "some work" also prints diagnostics then the output will be all
 *  messed up.  This library attempts to fix that, at least when the software uses this library for all its diagnostic
 *  output. Here's the most basic form:
 *
 * @code
 *  mlog[DEBUG] <<"first part";          // partial since no line-feed
 *  mlog[INFO] <<"some other message\n"; // unrelated message
 *  mlog[DEBUG] <<"; second part\n";     // complete the partial message
 * @endcode
 *
 *  The output to sinks like standard error that support partial messages will be, sans prefixes:
 *
 * @code
 *  first part...
 *  some other message
 *  first part; second part
 * @endcode
 *
 *  But there's a slight problem with that code: if the second line had been a message written to <code>mlog[DEBUG]</code>
 *  instead of <code>mlog[INFO]</code> then the library would not have been able to tell that "some other message" is unrelated
 *  to "first part" and would have emitted something that's not any better than using plain old <code>std::cerr</code>:
 *
 * @code
 *  first partsome other message
 *  ; second part
 * @endcode
 *
 *  This most often occurs when "some other message" is emitted from some other function, so its not immediately evident that
 *  something is inserting a message in the midst of our partial message. A better way is to create a temporary message stream
 *  and use that stream to complete the partial message.  Since the copy constructor also moves any partial message from the
 *  source stream to the new stream, we can even emit the partial message on the same line as the declaration.  Here's a silly
 *  example of the process:
 *
 * @code
 *  unsigned multiply(unsigned a, unsigned b) {
 *      Stream mesg(mlog[DEBUG] <<"multiply(" <<a <<", " <<b <<")");
 *      unsigned result = 0;
 *      for (unsigned i=0; i<b; ++i)
 *          result = add(result, b); //might also write to mlog[DEBUG]
 *      mesg <<" = " <<result <<"\n"; // finish the message
 *      return result;
 *  }
 * @endcode
 *
 *  The @c mesg stream need not be used only for completing the one message. In fact, one of the benefits of the @ref temp hint
 *  is that partial messages emitted using the @c debug stream cannot be interferred with from called functions that might use
 *  <code>mlog[DEBUG]</code>.
 */
namespace Message {

/** Explicitly initialize the library.
 *
 *  This initializes any global objects provided by the library to users.  This happens automatically for many API calls, but
 *  sometimes needs to be called explicitly. Calling this after the library has already been initialized does nothing. The
 *  function always returns true.
 *
 *  Thread safety: This function is thread-safe. */
SAWYER_EXPORT bool initializeLibrary();

// Any header that #defines words that are this common is just plain stupid!
#if defined(DEBUG) || defined(TRACE) || defined(WHERE) || defined(MARCH) || \
    defined(INFO) || defined(WARN) || defined(ERROR) || defined(FATAL)
# ifdef _MSC_VER
#  pragma message("Undefining common words from the global namespace: DEBUG, TRACE, WHERE, MARCH, INFO, WARN, ERROR, FATAL")
# else
#  warning "Undefining common words from the global namespace: DEBUG, TRACE, WHERE, MARCH, INFO, WARN, ERROR, FATAL"
# endif
# undef DEBUG
# undef TRACE
# undef WHERE
# undef MARCH
# undef INFO
# undef WARN
# undef ERROR
# undef FATAL
#endif

/** Level of importance for a message.
 *
 *  The library defines only these importance levels and does not provide a mechanism by which additional levels can be
 *  added. The reasoning is that multiple message facilities should be used to subdivide universal stream of messages into
 *  smaller categories, and that message sinks should be able to expect a well defined set of importance levels.
 *
 *  The message importance symbols are sorted numerically so that more critical levels have a higher numeric value than less
 *  critical levels. */
enum Importance {
    DEBUG,              /**< Messages intended to be useful primarily to the author of the code. This level of importance is
                         *   not often present in publically-released source code and serves mostly as an alternative for
                         *   authors that like to debug-by-print. */
    TRACE,             /**< Detailed tracing information useful to end-users that are trying to understand program
                         *   internals. These messages can also be thought of as debug messages that are useful to end
                         *   users. Tracing occurs in two levels, where @c TRACE is the low-level tracing that includes all
                         *   traceable messages. It can be assumed that if @c TRACE messages are enabled then @c WHERE messages
                         *   are also enabled to provide the broader context. */
    WHERE,             /**< Granular tracing information useful to end-users that are trying to understand program internals.
                         *   These can also be thought of as debug messages that are useful to end users.  Tracing occurs in
                         *   two levels, where @c WHERE provides a more granular overview of the trace. */
    MARCH,              /**< Progress reports and other similar rapidly updating partial messages. */
    INFO,               /**< Informative messages. These messages confer information that might be important but do not
                         *   indicate situations that are abnormal. */
    WARN,               /**< Warning messages that indicate an unusual situation from which the program was able to fully
                         *   recover. */
    ERROR,              /**< Error messages that indicate an abnormal situation from which the program was able to at least
                         *   partially recover. */
    FATAL,              /**< Messages that indicate an abnormal situation from which the program was unable to
                         *   recover. Producing a message of this type does not in itself terminate the program--it merely
                         *   indicates that the program is about to terminate. Since one software component's fatal error might
                         *   be a calling components's recoverable error, exceptions should generally be thrown.  About the
                         *   only time @c FATAL is used is when a logic assertion is about to fail, and even then it's usually
                         *   called from inside assert-like functions (see Sawyer::Assert for examples). */
    N_IMPORTANCE        /**< Number of distinct importance levels. */
};

/** Colors used by sinks that write to terminals. Note that most modern terminal emulators allow the user to specify the actual
 *  colors that are displayed for each of these, so the display color might not match the color name. */
enum AnsiColor {    // the values are important: they are the ANSI foreground and background color offsets
    COLOR_BLACK         = 0,                            /**< ANSI "black" color. */
    COLOR_RED           = 1,                            /**< ANSI "red" color. */
    COLOR_GREEN         = 2,                            /**< ANSI "green" color. */
    COLOR_YELLOW        = 3,                            /**< ANSI "yellow" color. */
    COLOR_BLUE          = 4,                            /**< ANSI "blue" color. */
    COLOR_MAGENTA       = 5,                            /**< ANSI "magenta" color. */
    COLOR_CYAN          = 6,                            /**< ANSI "cyan" color. */
    COLOR_WHITE         = 7,                            /**< ANSI "white" color. */
    COLOR_DEFAULT       = 8                             /**< ANSI default color. */
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SAWYER_EXPORT std::string stringifyImportance(Importance); /**< Convert an @ref Importance enum to a string. */
SAWYER_EXPORT std::string stringifyColor(AnsiColor);       /**< Convert an @ref AnsiColor enum to a string. */
SAWYER_EXPORT double now();                                /**< Current system time in seconds. */
SAWYER_EXPORT std::string escape(const std::string&);      /**< Convert a string to its C representation. */



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Colors
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** ANSI Color specification for text written to a terminal.
 *
 *  Thread safety: This object uses no global state, but is otherwise not thread-safe except where noted. */
struct SAWYER_EXPORT ColorSpec {
    AnsiColor foreground;                               /**< Foreground color, or @ref COLOR_DEFAULT. */
    AnsiColor background;                               /**< Background color, or @ref COLOR_DEFAULT. */
#include "WarningsOff.h"
    boost::tribool bold;                                /**< Use ANSI "bold" attribute? */
#include "WarningsRestore.h"

    /** Constructs an object with default foreground and background colors. */
    ColorSpec(): foreground(COLOR_DEFAULT), background(COLOR_DEFAULT), bold(false) {}

    /** Constructs an object that specifies only a foreground color. */
    explicit ColorSpec(AnsiColor fg): foreground(fg), background(COLOR_DEFAULT), bold(false) {}

    /** Constructs an object with fully-specified colors. */
    ColorSpec(AnsiColor fg, AnsiColor bg, bool bold): foreground(fg), background(bg), bold(bold) {}

    /** Returns true if this object is in its default-constructed state. */
    bool isDefault() const { return COLOR_DEFAULT==foreground && COLOR_DEFAULT==background
                                    && static_cast<bool>(!bold); }
};

/** Colors to use for each message importance.
 *
 *  Thread safety: This object uses no global state but is otherwise not thread safe. */
class SAWYER_EXPORT ColorSet {
    ColorSpec spec_[N_IMPORTANCE];
public:
    /** Returns a color set that uses only default colors.  Warning, error, and fatal messages will use the bold attribute, but
     *  no colors are employed. */
    static ColorSet blackAndWhite();

    /** Returns a color set that uses various foreground colors for the different message importance levels. */
    static ColorSet fullColor();

    /** Colors for a message. Returns a reference to the color specification for a particular message importance.
     * @{ */
    const ColorSpec& operator[](Importance imp) const { return spec_[imp]; }
    ColorSpec& operator[](Importance imp) { return spec_[imp]; }
    /** @} */
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Message Properties
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Properties for messages.
 *
 *  Each message property is optional.  When a message is sent through the plumbing, each node of the plumbing lattice may
 *  provide default values for properties that are not set, or may override properties that are set.
 *
 *  Thread safety:  This object uses no global state, but is otherwise not thread-safe except where noted. */
struct SAWYER_EXPORT MesgProps {
#include "WarningsOff.h"
    Optional<std::string> facilityName;                 /**< The name of the logging facility that produced this message. */
    Optional<Importance> importance;                    /**< The message importance level. */
    boost::tribool isBuffered;                          /**< Whether the output buffered and emitted on a per-message basis. */
    Optional<std::string> completionStr;                /**< String to append to the end of each complete message. */
    Optional<std::string> interruptionStr;              /**< String to append when a partial message is interrupted. */
    Optional<std::string> cancelationStr;               /**< String to append to a partial message when it is destroyed. */
    Optional<std::string> lineTermination;              /**< Line termination for completion, interruption, and cancelation. */
    boost::tribool useColor;                            /**< Whether to use ANSI escape sequences to colorize output. */
#include "WarningsRestore.h"

    MesgProps(): isBuffered(boost::indeterminate), useColor(boost::indeterminate) {}

    /** Merge the specified properties into this object and return new properties.  Each property of the return value will be
     *  the value from this object, except when this object's property is missing, in which case the property value from the
     *  argument is used (which may also be missing). */
    MesgProps merge(const MesgProps&) const;

    /** Print the values for all properties.  This is used mainly for debugging. */
    void print(std::ostream&) const;
};

/** Print the values for all message properties. @sa MesgProps::print(). */
SAWYER_EXPORT std::ostream& operator<<(std::ostream &o, const MesgProps &props);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Type defintions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Smart pointer.  Sawyer uses reference-counting smart pointers for most objects in order to alleviate the user from
 *  having to keep track of which objects own which other objects and when it is safe to delete an object.  This is especially
 *  convenient for the plumbing part of Sawyer where objects are connected to each other in a lattice and may have multiple
 *  parents.
 *
 *  Sawyer uses the following conventions for objects that use smart pointers:
 *
 *  <ol>
 *    <li>The name of the smart pointer type is the name of the base class followed by "Ptr".  For instance, a smart
 *        pointer for the FileSink class is named FileSinkPtr.</li>
 *    <li>C++ constructors for such objects are protected to that users don't inadvertently create such an object on
 *        the stack and then try to use its address in a smart pointer situation.</li>
 *    <li>Users should use the @c instance class methods instead of constructors to instantiate an instance of such a class.
 *        These methods allocate a new object and return a smart pointer to that object.</li>
 *    <li>Sawyer uses @ref Sawyer::SharedPointer "SharedPointer" for its smart pointer implementation.</li>
 * </ol>
 * @{ */
typedef SharedPointer<class Destination> DestinationPtr;
typedef SharedPointer<class Multiplexer> MultiplexerPtr;
typedef SharedPointer<class Filter> FilterPtr;
typedef SharedPointer<class SequenceFilter> SequenceFilterPtr;
typedef SharedPointer<class TimeFilter> TimeFilterPtr;
typedef SharedPointer<class ImportanceFilter> ImportanceFilterPtr;
typedef SharedPointer<class Gang> GangPtr;
typedef SharedPointer<class Prefix> PrefixPtr;
typedef SharedPointer<class UnformattedSink> UnformattedSinkPtr;
typedef SharedPointer<class FdSink> FdSinkPtr;
typedef SharedPointer<class FileSink> FileSinkPtr;
typedef SharedPointer<class StreamSink> StreamSinkPtr;
typedef SharedPointer<class SyslogSink> SyslogSinkPtr;
/** @} */

/** Baked properties for a destination.  Rather than recompute properties every time characters of a message are inserted into
 *  a stream, the library computes the properties just once when the first character of a message is inserted. This process is
 *  called "baking the properties" and the result is an <code>std::pair</code> that contains a destination and its baked
 *  properties. */
typedef std::pair<DestinationPtr, MesgProps> BakedDestination;

/** Baked properties for multiple destinations. */
typedef std::vector<BakedDestination> BakedDestinations;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Messages
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** A single message.
 *
 *  A message consists of a string and can be in a partial, completed, or canceled state.  More text can be added to a message
 *  while it is in the partial state, but once the message is marked as completed or canceled the text becomes immutable.
 *  Messages also have properties, which are fed into the plumbing lattice and adjusted as they traverse to the final
 *  destinations during a process called "baking".
 *
 *  Thread safety: This object uses global state, but is otherwise not thread-safe except where noted. */
class SAWYER_EXPORT Mesg {
    static unsigned nextId_;                            // class-wide unique ID numbers
    unsigned id_;                                       // unique message ID
#include "WarningsOff.h"
    std::string text_;                                  // text of the message
#include "WarningsRestore.h"
    bool isComplete_;                                   // true when the message is complete
    bool isCanceled_;                                   // true when message is being canceled without completing
    MesgProps props_;                                   // message properties
public:

    /** Creates a new, partial message that is empty. */
    Mesg()
        : id_(nextId_++), isComplete_(false), isCanceled_(false) {}

    /** Creates a new, partial message that is empty.  Sets the facility name and importance properties as specified. */
    Mesg(const std::string &facilityName, Importance imp)
        : id_(nextId_++), isComplete_(false), isCanceled_(false) {
        props_.facilityName = facilityName;
        props_.importance = imp;
    }

    /** Creates a new, partial message that is empty. Initializes message properties as specified. */
    explicit Mesg(const MesgProps &props)
        : id_(nextId_++), isComplete_(false), isCanceled_(false), props_(props) {}

    /** Creates a new, completed message with the specified text. */
    explicit Mesg(const std::string &mesg)
        : id_(nextId_++), text_(mesg), isComplete_(true), isCanceled_(false) {}

    /** Creates a new, completed message with the specified text. Also sets the facility name and importance properties
     *  as specified. */
    Mesg(const std::string &facilityName, Importance imp, const std::string &mesg)
        : id_(nextId_++), text_(mesg), isComplete_(true), isCanceled_(false) {
        props_.facilityName = facilityName;
        props_.importance = imp;
    }

    /** Creates a new, completed message with the specified text.  Also sets the message properties as specified. */
    Mesg(const MesgProps &props, const std::string &mesg)
        : id_(nextId_++), text_(mesg), isComplete_(true), isCanceled_(false), props_(props) {}

    /** Return unique message ID.  Each message has an identification number that is unique within a process. */
    unsigned id() const { return id_; }

    /** Return the message text. */
    const std::string& text() const { return text_; }

    /** Returns true if the message has entered the completed state. Once a message is completed its text cannot be changed. */
    bool isComplete() const { return isComplete_; }

    /** Returns true if the message has entered the canceled state.  Messages typically only enter this state when they
     *  are being destroyed. */
    bool isCanceled() const { return isCanceled_; }

    /** Returns true if the message has no text. This is similar, but not identical to the the inverse of the @ref
     *  hasText method. */
    bool isEmpty() const { return text_.empty(); }

    /** Returns true if the message has text other than white space. This is similar, but not identical to the inverse
     *  of the @ref isEmpty method. */
    bool hasText() const;

    /** Returns a reference to message properties. Once a message's properties are "baked" by sending them through the
     *  plumbing lattice, changing the message properties has no effect on the text that is emitted by the message
     *  sinks. A stream typically bakes the message when the first text is inserted.
     *  @{ */
    const MesgProps& properties() const { return props_; }
    MesgProps& properties() { return props_; }
    /** @} */

    /** Cause the message to enter the completed state. Once a message enters this state its text cannot be modified. */
    void complete() { isComplete_ = true; }

    /** Cause the message to enter the canceled state.  This typically happens automatically when a message is being
     *  destroyed. Once a message enters this state its text cannot be modified. */
    void cancel() { isCanceled_ = true; }

    /** Append additional text to the end of the message.  If the message is in the completed or canceled state then
     *  an std::runtime_error is thrown.
     *  @{ */
    void insert(const std::string&);
    void insert(char);
    /** @} */

    /** Send a message to its final destinations. */
    void post(const BakedDestinations&) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Plumbing lattice nodes
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Base class for all types of message destinations.
 *
 *  This is the base class for all nodes in the plumbing lattice. */
class SAWYER_EXPORT Destination: public SharedObject, public SharedFromThis<Destination> {
protected:
    mutable SAWYER_THREAD_TRAITS::RecursiveMutex mutex_; /**< Mutex protecting data members here and in subclasses. */
    MesgProps dflts_;                                   /**< Default properties merged into each incoming message. */
    MesgProps overrides_;                               /**< Override properties applied to incoming message. */
protected:
    Destination() {}
public:
    virtual ~Destination() {}

    /** Default values for message properties.
     *
     *  Any message property that has no value coming from a higher layer in the plumbing lattice will be set to the default
     *  value if one is defined.
     *
     *  Thread safety: Not thread safe.
     *  
     *  @{ */
    const MesgProps& defaultPropertiesNS() const { return dflts_; }
    MesgProps& defaultPropertiesNS() { return dflts_; }
    /** @} */

    /** Overrides message properties.
     *
     *  Any overriding property value that is defined will override the value coming from a higher layer in the plumbing
     *  lattice.
     *
     *  Thread safety: Not thread safe.
     *  
     *  @{ */
    const MesgProps& overridePropertiesNS() const { return overrides_; }
    MesgProps& overridePropertiesNS() { return overrides_; }
    /** @} */

    /** Bakes message properties according to the plumbing lattice.
     *
     *  The given message properties are applied to the plumbing lattice rooted at this Destination, adjusted according to the
     *  default and override properties at this node of the lattice and all lower nodes.  The property values at the bottom
     *  nodes of the lattice are appended to the @p baked argument.
     *
     *  Thread safety: All implementations must be thread-safe. */
    virtual void bakeDestinations(const MesgProps&, BakedDestinations &baked);

    /** Causes a message to be emitted. The @p bakedProperties argument is one of the values returned by the @ref
     *  bakeDestinations method. */
    virtual void post(const Mesg&, const MesgProps &bakedProperties) = 0;

    /** Merge properties of this lattice node into the specified properties.
     *
     *  Any property in @p props that is not set will be set to this node's default value for the property (if any).  Any
     *  override property defined for this node will override the value (or lack of value) in @p props.  The merged result is
     *  returned.
     *
     *  Thread safety: Not thread safe. */
    MesgProps mergePropertiesNS(const MesgProps &props);
};

/** Sends incoming messages to multiple destinations.
 *
 *  This is the base class for all internal nodes of the plumbing lattice. */
class SAWYER_EXPORT Multiplexer: public Destination {
    typedef std::list<DestinationPtr> Destinations;
#include "WarningsOff.h"
    Destinations destinations_;
#include "WarningsRestore.h"
protected:
    /** Constructor for derived classes. Non-subclass users should use @ref instance instead. */
    Multiplexer() {}
public:
    /** Allocating constructor. */
    static MultiplexerPtr instance() { return MultiplexerPtr(new Multiplexer); }

    virtual void bakeDestinations(const MesgProps&, BakedDestinations&) /*override*/;
    virtual void post(const Mesg&, const MesgProps&) /*override*/;

    /** Adds a child node to this node of the lattice.
     *
     *  An <code>std::runtime_error</code> is thrown if the addition of this child would cause the lattice to become malformed
     *  by having a cycle.
     *
     *  Thread safety: This method is not thread-safe.
     *
     *  @sa to */
    MultiplexerPtr addDestination(const DestinationPtr&);

    /** Removes the specified child from this node of the lattice.
     *
     *  Thread safety: This method is thread-safe.
     *
     *  @sa to */
    MultiplexerPtr removeDestination(const DestinationPtr&);

    /** Add a child nodes to this node of the lattice and returns this node.
     *
     *  It is often more convenient to call this method instead of @ref addDestination.
     *
     *  Thread safety: This method is not thread-safe.
     *
     *  @{ */
    MultiplexerPtr to(const DestinationPtr&);           // more convenient form of addDestination()
    MultiplexerPtr to(const DestinationPtr&, const DestinationPtr&);
    MultiplexerPtr to(const DestinationPtr&, const DestinationPtr&, const DestinationPtr&);
    MultiplexerPtr to(const DestinationPtr&, const DestinationPtr&, const DestinationPtr&, const DestinationPtr&);
    /** @} */
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Filters
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Base class for internal nodes that filter messages.
 *
 *  Filtering is applied when properties are baked, which should happen exactly once for each message, usually just before the
 *  message is posted for the first time. */
class SAWYER_EXPORT Filter: public Multiplexer {
protected:
    /** Constructor for derived classes. Non-subclass users should use @ref instance instead. */
    Filter() {}
public:
    virtual void bakeDestinations(const MesgProps&, BakedDestinations&) /*override*/;

    /** Predicate determines when a message should be forwarded onward.
     *
     *  This method is called once from bakeDestinations(), and if it returns true then the baking is forwarded on to the child
     *  nodes in the plumbing lattice. If this method returns false then none of the descendents will see the properties
     *  (unless they can be reached by some other path), and the unreached leaf nodes will not be returned by
     *  bakeDestinations().
     *
     *  Thread safety: all implementations must be thread-safe. */
    virtual bool shouldForward(const MesgProps&) = 0;   // return true if message should be forwarded to children when baking

    /** Called once by bakeDestinations if shouldForward() returned true.
     *
     *  This method is called after the properties have been forwarded to the child nodes. */
    virtual void forwarded(const MesgProps&) = 0;       // message was forwarded to children
};

/** Filters messages based on how many messages have been seen.
 *
 *  The first @e n messages are skipped (not forwarded to children), the first one of every @e m messages thereafter is
 *  forwarded, for a total of @e t messages forwarded. */
class SAWYER_EXPORT SequenceFilter: public Filter {
    size_t nSkip_;                                      // skip initial messages posted to this sequencer
    size_t rate_;                                       // emit only 1/Nth of the messages (0 and 1 both mean every message)
    size_t limit_;                                      // emit at most this many messages (0 means infinite)
    size_t nPosted_;                                    // number of messages posted (including those suppressed)
protected:
    /** Constructor for derived classes. Non-subclass users should use @ref instance instead. */
    SequenceFilter(size_t nskip, size_t rate, size_t limit)
        : nSkip_(nskip), rate_(rate), limit_(limit), nPosted_(0) {}
public:
    /** Construct an instance.
     *
     *  The node will skip the first @p nskip messages, then forward the first of every @p rate following messages, for a grand
     *  total of @p limit messages forwarded.  If @p rate is zero or one then every message after the first @p nskip is
     *  forwarded (up to the @p limit).  If @p limit is zero then there is no limit. */
    static SequenceFilterPtr instance(size_t nskip, size_t rate, size_t limit) {
        return SequenceFilterPtr(new SequenceFilter(nskip, rate, limit));
    }

    /** Property: number of initial messages to skip.
     *
     *  The first @p n messages sent through this filter are discarded.
     *
     *  Thread safety: This method is thread-safe.
     *
     *  @{ */
    size_t nSkip() const;
    SequenceFilterPtr nSkip(size_t n);
    /** @} */

    /** Property: rate of messages to emit after initial messages are skipped.
     *
     *  A rate of @e n means the first message of every group of @e n messages is forwarded to children nodes in the plumbing
     *  lattice.  A rate of zero means the same thing as a rate of one--every message is forwarded.
     *
     *  Thread safety: This method is thread-safe.
     *
     * @{ */
    size_t rate() const;
    SequenceFilterPtr rate(size_t n);
    /** @} */

    /** Property: total number of messages forwarded.
     *
     *  At most @e n messages are forwarded to children in the lattice, after which messages are discarded. A value of zero
     *  means no limit is in effect.
     *
     *  Thread safety: This method is thread-safe.
     *
     *  @{ */
    size_t limit() const;
    SequenceFilterPtr limit(size_t n);
    /** @} */

    /** Number of messages processed.
     *
     *  This includes messages forwarded and messages not forwarded.
     *
     *  Thread safety: This method is thread-safe. */
    size_t nPosted() const;

    virtual bool shouldForward(const MesgProps&) /*override*/;
    virtual void forwarded(const MesgProps&) /*override*/ {}
};

/** Filters messages based on time.
 *
 *  Any message posted within some specified time of a previously forwarded message will not be forwarded to children nodes in
 *  the plumbing lattice. */
class SAWYER_EXPORT TimeFilter: public Filter {
    double initialDelay_;                               // amount to delay before emitting the first message
    double minInterval_;                                // minimum time between messages
    double prevMessageTime_;                            // time previous message was emitted
    double lastBakeTime_;                               // time cached by shouldForward, used by forwarded
    size_t nPosted_;                                    // number of messages posted (including those suppressed)
protected:
    /** Constructor for derived classes.
     *
     *  Non-subclass users should use @ref instance instead. */
    explicit TimeFilter(double minInterval)
        : initialDelay_(0.0), minInterval_(minInterval), prevMessageTime_(0.0), lastBakeTime_(0.0) {}
public:
    /** Allocating constructor.
     *
     *  Creates an instance that limits forwarding of messages to at most one message every @p minInterval seconds. */
    static TimeFilterPtr instance(double minInterval) {
        return TimeFilterPtr(new TimeFilter(minInterval));
    }

    /** Property: minimum time between messages.
     *
     *  Any message arriving within the specified interval from a previous message that was forwarded to children in the
     *  lattice will be discarded.
     *
     *  Thread safety: This method is thread-safe.
     *
     *  @{ */
    double minInterval() const;
    TimeFilterPtr minInterval(double d);
    /** @} */

    /** Property: delay before the next message.
     *
     *  Any message arriving within the specified number of seconds from this call will be discarded.
     *
     *  Thread safety: This method is thread-safe.
     *
     *  @{ */
    double initialDelay() const;
    TimeFilterPtr initialDelay(double d);
    /** @} */

    /** Number of messages processed.
     *
     *  This includes messages forwarded and messages not forwarded.
     *
     *  Thread safety: This method is thread-safe. */
    size_t nPosted() const;

    virtual bool shouldForward(const MesgProps&) /*override*/;
    virtual void forwarded(const MesgProps&) /*override*/;
};

/** Filters messages based on importance level.
 *
 *  For instance, to disable all messages except fatal and error messages:
 *
 * @code
 *  DestinationPtr d = ImportanceFilter::instance(false)->enable(FATAL)->enable(ERROR);
 * @endcode */
class SAWYER_EXPORT ImportanceFilter: public Filter {
    bool enabled_[N_IMPORTANCE];
protected:
    /** Constructor for derived classes.
     *
     *  Non-subclass users should use @ref instance instead. */
    explicit ImportanceFilter(bool dflt) {
        memset(enabled_, dflt?0xff:0, sizeof enabled_);
    }
public:
    /** Allocating constructor.
     *
     *  Creates an instance that either allows or disallows all importance levels depending on whether @p dflt is true or
     *  false, respectively. */
    static ImportanceFilterPtr instance(bool dflt) {
        return ImportanceFilterPtr(new ImportanceFilter(dflt));
    }

    /** Property: the enabled/disabled state for a message importance level.
     *
     *  Importance levels that are enabled are forwarded to children in the lattice, and those that are disable are discarded.
     *
     *  Thread safety: This method is thread-safe.
     *
     *  @{ */
    bool enabled(Importance imp) const;
    ImportanceFilterPtr enabled(Importance imp, bool b);
    /** @} */

    /** Enable an importance level.
     *
     *  Returns this node so that the method can be chained. */
    ImportanceFilterPtr enable(Importance);

    /** Disable an importance level.
     *
     *  Returns this node so that the method can be chained. */
    ImportanceFilterPtr disable(Importance);

    virtual bool shouldForward(const MesgProps&) /*override*/;
    virtual void forwarded(const MesgProps&) /*override*/ {}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Support for final destinations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** @internal
 *  Keeps track of how much of a partial message was already emitted.
 *
 *  Thread safety: All methods in this class are thread safe. */
class HighWater {
    Optional<unsigned> id_;                             /**< ID number of last message to be emitted, if any. */
    MesgProps props_;                                   /**< Properties used for the last emission. */
    size_t ntext_;                                      /**< Number of characters of the message we've seen already. */
protected:
    mutable SAWYER_THREAD_TRAITS::RecursiveMutex mutex_;
    HighWater(const HighWater&) { abort(); }          // not copyable
    HighWater& operator=(const HighWater&) { abort(); } // not copyable
public:
    HighWater(): ntext_(0) {}
    explicit HighWater(const Mesg &m, const MesgProps &p) { emitted(m, p); }
    SAWYER_THREAD_TRAITS::RecursiveMutex& mutex() const { return mutex_; }
    void emitted(const Mesg&, const MesgProps&);        /**< Make specified message the high water mark. */
    void clear();                                       /**< Reset to initial state. */
    bool isValid() const;                               /**< Returns true if high water is defined. */
    unsigned id() const;                                /**< Exception unless <code>isValid()</code>. */
    MesgProps properties() const;
    size_t ntext() const;                               /**< Zero if <code>!isValid()</code>. */
};

/** @internal
 *  Coordination between two or more sinks.
 *
 *  A Gang is used to coordinate output from two or more sinks.  This is normally used by sinks writing the tty, such as a sink
 *  writing to stdout and another writing to stderr--they need to coordinate with each other if they're both going to the
 *  terminal.  A gang just keeps track of what message was most recently emitted.
 *
 *  Thread safety: All methods in this class are thread safe. */
class Gang: public HighWater, public SharedObject {
    typedef Sawyer::Container::Map<int, GangPtr> GangMap;
    static GangMap *gangs_;                             /**< Gangs indexed by file descriptor or other ID. */
    static const int TTY_GANG = -1;                     /**< The ID for streams that are emitting to a terminal device. */
    static SAWYER_THREAD_TRAITS::Mutex classMutex_;     /**< Mutex for class data. */
protected:
    Gang() {}
public:
    static GangPtr instance() { return GangPtr(new Gang); }
    static GangPtr instanceForId(int id);               /**< The gang for the specified ID, creating a new one if necessary. */
    static GangPtr instanceForTty();                    /**< Returns the gang for streams that are emitting to a tty. */
    static void removeInstance(int id);                 /**< Remove specified gang from global list. */
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Messages prefixes
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Information printed at the beginning of each free-format message. */
class SAWYER_EXPORT Prefix: public SharedObject, public SharedFromThis<Prefix> {
#include "WarningsOff.h"
public:
    /** When to show something. */
    enum When {
        NEVER=0,                                        /**< Never show this item. */
        SOMETIMES=1,                                    /**< Show this item only sometimes (depends on the item). */
        ALWAYS=2                                        /**< Always show this item. */
    };
private:
    mutable SAWYER_THREAD_TRAITS::RecursiveMutex mutex_; /**< Mutex for data members here and in subclasses. */
    ColorSet colorSet_;                                 /**< Colors to use if <code>props.useColor</code> is true. */
    Optional<std::string> programName_;                 /**< Name of program as it will be displayed (e.g., "a.out[12345]"). */
    bool showProgramName_;
    bool showThreadId_;
    Optional<double> startTime_;                        /**< Time at which program started. */
    bool showElapsedTime_;
    When showFacilityName_;                             /**< Whether the facility name should be displayed. */
    bool showImportance_;                               /**< Whether the message importance should be displayed. */
    void initFromSystem();                              /**< Initialize data from the operating system. */
#include "WarningsRestore.h"
protected:
    /** Constructor for derived classes.
     *
     *  Non-subclass users should use @ref instance instead. */
    Prefix()
        : showProgramName_(true), showThreadId_(true), showElapsedTime_(true), showFacilityName_(SOMETIMES),
          showImportance_(true) {
        initFromSystem();
        colorSet_ = ColorSet::fullColor();
    }
public:
    virtual ~Prefix() {}

    /** Allocating constructor.
     *
     *  Creates a new prefix instance and returns a pointer to it. The new object is initialized with information about the
     *  thread that created it, such as program name, thread ID, time of creation, etc. */
    static PrefixPtr instance() { return PrefixPtr(new Prefix); }

    /** Construct a prefix that is silent.
     *
     *  Constructs a prefix whose output properties are all false, thus producing no output. */
    static PrefixPtr silentInstance();

    /** Property: colors to use for the prefix if coloring is enabled.
     *
     *  Colors can be specified even for sinks that don't support them--they just won't be used.
     *
     *  Thread safety: Not thread-safe.
     *
     * @{ */
    const ColorSet& colorSet() const { return colorSet_; }
    ColorSet& colorSet() { return colorSet_; }
    /** @} */

    /** Property: program name.
     *
     *  The default is the base name of the file that was executed.  An "lt-" prefix is removed from the base name since this
     *  is typically added by libtool and doesn't correspond to the name that the user executed.
     *
     *  Thread safety: Not thread safe.
     *
     *  @sa setProgramName @ref showProgramName
     *
     * @{ */
    const Optional<std::string>& programName() const { return programName_; }
    PrefixPtr programName(const std::string &s) { programName_ = s; return sharedFromThis(); }
    /** @} */

    /** Reset the program name from operating system information.
     *
     *  Thread safety: This method is thread-safe.
     *
     *  @sa programName showProgramName */
    void setProgramName();

    /** Property: whether to show the program name in the message prefix area.
     *
     *  The default is true.
     *
     *  Thread safety: This method is not thread-safe.
     *
     *  @sa programName
     * @{ */
    bool showProgramName() const { return showProgramName_; }
    PrefixPtr showProgramName(bool b) { showProgramName_ = b; return sharedFromThis(); }
    /** @} */

    /** Property: whether to show the thread ID in the message prefix area.
     *
     *  The default is true.
     *
     *  Thread safety: This method is not thread-safe.
     *
     * @{ */
    bool showThreadId() const { return showThreadId_; }
    PrefixPtr showThreadId(bool b) { showThreadId_ = b; return sharedFromThis(); }
    /** @} */

    /** Property: start time when emitting time deltas.
     *
     *  On some systems the start time will be the time at which this object was created.
     *
     *  Thread safety: This method is not thread-safe.
     *
     *  @sa setStartTime @ref showElapsedTime
     *
     * @{ */
    const Optional<double> startTime() const;
    PrefixPtr startTime(double t);
    /** @} */

    /** Reset the start time from operating system information.
     *
     *  On some systems this will be the time at which the first prefix object was created rather than the time the operating
     *  system created the main process.
     *
     *  Thread safety: This method is not thread-safe.
     *
     *  @sa startTime showElapsedTime */
    void setStartTime();

    /** Property: whether to show time deltas.
     *
     *  Time deltas are displayed as fractional seconds since some starting time.
     *
     *  Thread safety: This method is not thread-safe.
     *
     *  @sa startTime setStartTime
     * @{ */
    bool showElapsedTime() const { return showElapsedTime_; }
    PrefixPtr showElapsedTime(bool b) { showElapsedTime_ = b; return sharedFromThis(); }
    /** @} */

    /** Property: whether to show the facilityName property.
     *
     *  When set to SOMETIMES, the facility name is shown when it differs from the program name and the program name has been
     *  shown. In any case, the facility name is not shown if it has no value.
     *
     *  Thread safety: This method is not thread-safe.
     *
     * @{ */
    When showFacilityName() const { return showFacilityName_; }
    PrefixPtr showFacilityName(When w) { showFacilityName_ = w; return sharedFromThis(); }
    /** @} */

    /** Property: whether to show the importance property.
     *
     *  In any case, the importance level is not shown if it has no value.
     *
     *  Thread safety: This method is not thread-safe.
     *
     * @{ */
    bool showImportance() const { return showImportance_; }
    PrefixPtr showImportance(bool b) { showImportance_ = b; return sharedFromThis(); }
    /** @} */

    /** Return a prefix string.
     *
     *  Generates a string from this prefix object.
     *
     *  Thread safety: This method is not thread-safe. */
    virtual std::string toString(const Mesg&, const MesgProps&) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Final destinations (sinks)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Base class for final destinations that are free-format.
 *
 *  These are destinations that emit messages as strings. */
class SAWYER_EXPORT UnformattedSink: public Destination {
#include "WarningsOff.h"
    GangPtr gang_;
    PrefixPtr prefix_;
#include "WarningsRestore.h"

protected:
    bool partialMessagesAllowed_;

protected:
    /** Constructor for derived classes. Non-subclass users should use <code>instance</code> factory methods instead. */
    explicit UnformattedSink(const PrefixPtr &prefix): partialMessagesAllowed_(true) {
        gang_ = Gang::instance();
        prefix_ = prefix ? prefix : Prefix::instance();
        init();
    }
public:
    /** Property: sink gang.
     *
     *  Message sinks use a gang to coordinate their output. For instance, two sinks that both write to the same file should
     *  share the same gang.  A shared gang can also be used for sinks that write to two different locations that both happen
     *  to be visible collectively, such as when writing to standard error and standard output when both are writing to the
     *  terminal or when both have been redirected to the same file.  Not all sinks support automatically choosing the correct
     *  gang (e.g., C++ std::ofstream doesn't have the necessary capabilities), so it must be chosen manually sometimes for the
     *  output to be coordinated.  See Gang::instanceForId() and Gang::instanceForTty().
     *
     *  Thread safety: This method is thread-safe.
     *
     * @{ */
    GangPtr gang() const;
    UnformattedSinkPtr gang(const GangPtr &g);
    /** @} */

    /** Property: allow partial message output.
     *
     *  If partial messages are not allowed, then a message is emitted only when its terminating linefeed is encountered.
     *  The default for unformatted sinks is to allow partial message output. Disabling partial message output is sometimes
     *  useful in multi-threaded applications to prevent two threads from competing with each other when they both try to
     *  output a complete message at the same time: one thread might interrupt the other midway through the other's message, in
     *  which case the other thread would have emitted a partial message.  I.e., disabling partial messages works like this:
     *
     * @code
     *  concurrently {
     *     mlog0[INFO] <<"message from first thread\n";
     *     mlog1[INFO] <<"message from second thread\n";
     *  }
     * @endcode
     *
     *  If @c mlog0 and @c mlog1 are both emitting to standard error then the output might look like this (sans prefixes):
     *
     * @code
     *  mlog0[INFO]: message from fir...
     *  mlog1[INFO]: message from second thread
     *  mlog0[INFO]: message from first thread
     * @endcode
     *
     *  Disabling partial message output would have prevented the first partial message.
     *
     * @{ */
    bool partialMessagesAllowed() const;
    UnformattedSinkPtr partialMessagesAllowed(bool);
    /** @} */

    /** Property: how to generate message prefixes.
     *
     *  This is a pointer to the object that is responsible for generating the string that appears at the beginning of each
     *  line and usually contains such things as the program and importance level.
     *
     *  Thread safety: This method is thread-safe.
     *
     * @{ */
    PrefixPtr prefix() const;
    UnformattedSinkPtr prefix(const PrefixPtr &p);
    /** @} */

    /** Support function for emitting a message.
     *
     *  This string terminates the previous partial message from the same gang. This method must be called exactly once during
     *  @ref render for its possible side effects, although the return value doesn't have to be used.
     *
     *  Thread safety: not thread-safe. However, it is usually only called from inside @ref render after locks for this object
     *  and the gang have been obtained. */
    virtual std::string maybeTerminatePrior(const Mesg&, const MesgProps&);

    /** Support function for emitting a message.
     *
     *  This string is the prefix generated by calling Prefix::toString if a prefix is necessary.  This method must be called
     *  exactly once during @ref render for its possible side effects, although the return value doesn't have to be used.
     *
     *  Thread safety: not thread-safe. However, it is usually only called from inside @ref render after locks for this object
     *  and the gang have been obtained. */
    virtual std::string maybePrefix(const Mesg&, const MesgProps&);

    /** Support function for emitting a message.
     *
     *  This string is the message body, or at least the part of the body that hasn't been emitted yet.  This method must be
     *  called exactly once during @ref render for its possible side effects, although the return value doesn't have to be
     *  used.
     *
     *  Thread safety: not thread-safe. However, it is usually only called from inside @ref render after locks for this object
     *  and the gang have been obtained. */
    virtual std::string maybeBody(const Mesg&, const MesgProps&);

    /** Support function for emitting a message.
     *
     *  This string is the message termination, interruption, or cancelation and the following line termination as
     *  necessary. This method must be called exactly once during @ref render for its possible side effects, although the
     *  return value doesn't have to be used.
     *
     *  Thread safety: not thread-safe. However, it is usually only called from inside @ref render after locks for this object
     *  and the gang have been obtained. */
    virtual std::string maybeFinal(const Mesg&, const MesgProps&);

    /** Support function for emitting a message.
     *
     *  The return string is constructed by calling @ref maybeTerminatePrior, @ref maybePrefix, @ref maybeBody, and @ref
     *  maybeFinal one time each (because some of them have side effects).
     *
     *  Thread safety: This method is thread safe. It obtains and holds locks for this object and the gang across all the calls
     *  to @ref maybeTerminatePrior, @ref maybePrefix, @ref maybeBody, and @ref maybeFinal. */
    virtual std::string render(const Mesg&, const MesgProps&);
protected:
    /** Cause this sink to be coordinated with others.
     *
     *  Thread safety: This method is thread-safe. */
    void gangInternal(const GangPtr &g);
private:
    void init();
};

/** Send free-format messages to a Unix file descriptor. */
class SAWYER_EXPORT FdSink: public UnformattedSink {
    int fd_;                                            // file descriptor or -1
protected:
    /** Constructor for derived classes. Non-subclass users should use @ref instance instead. */
    FdSink(int fd, const PrefixPtr &prefix): UnformattedSink(prefix), fd_(fd) { init(); }
public:
    /** Allocating constructor.  Constructs a new message sink that sends messages to the specified Unix file descriptor. */
    static FdSinkPtr instance(int fd, const PrefixPtr &prefix=PrefixPtr()) {
        return FdSinkPtr(new FdSink(fd, prefix));
    }

    virtual void post(const Mesg&, const MesgProps&) /*override*/;
private:
    void init();
};

/** Send free-format messages to a C @c FILE pointer. */
class SAWYER_EXPORT FileSink: public UnformattedSink {
    FILE *file_;
protected:
    /** Constructor for derived classes. Non-subclass users should use @ref instance instead. */
    FileSink(FILE *f, const PrefixPtr &prefix): UnformattedSink(prefix), file_(f) { init(); }
public:
    /** Allocating constructor.  Constructs a new message sink that sends messages to the specified C @c FILE pointer. */
    static FileSinkPtr instance(FILE *f, const PrefixPtr &prefix=PrefixPtr()) {
        return FileSinkPtr(new FileSink(f, prefix));
    }

    virtual void post(const Mesg&, const MesgProps&) /*override*/;
private:
    void init();
};

/** Send free-format messages to a C++ I/O stream. */
class SAWYER_EXPORT StreamSink: public UnformattedSink {
    std::ostream &stream_;
protected:
    /** Constructor for derived classes. Non-subclass users should use @ref instance instead. */
    StreamSink(std::ostream &stream, const PrefixPtr &prefix): UnformattedSink(prefix), stream_(stream) {}
public:
    /** Allocating constructor.  Constructs a new message sink that sends messages to the specified C++ output stream. */
    static StreamSinkPtr instance(std::ostream &stream, const PrefixPtr &prefix=PrefixPtr()) {
        return StreamSinkPtr(new StreamSink(stream, prefix));
    }

    virtual void post(const Mesg&, const MesgProps&) /*override*/;
};

#ifndef BOOST_WINDOWS
/** Sends messages to the syslog daemon.
 *
 *  Thread safety: Unknown (depends on whether syslog is thread-safe). */
class SAWYER_EXPORT SyslogSink: public Destination {
protected:
    /** Constructor for derived classes. Non-subclass users should use @ref instance instead. */
    SyslogSink(const char *ident, int option, int facility);
public:
    /** Allocating constructor.  Constructs a new message sink that sends messages to the syslog daemon.  The syslog API
     *  doesn't use a handle to refer to the syslog, nor does it specify what happens when @c openlog is called more than once,
     *  or whether the @p ident string is copied.  Best practice is to use only constant strings as the @c ident argument. */
    static SyslogSinkPtr instance(const char *ident, int option, int facility) {
        return SyslogSinkPtr(new SyslogSink(ident, option, facility));
    }
    virtual void post(const Mesg&, const MesgProps&) /*override*/;
private:
    void init();
};
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Message streams
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Stream;
class StreamBuf;

/** @internal
 *  Adjusts reference counts for a stream and deletes the stream when the reference count hits zero.
 *
 *  This object acts like a POD pointer type except it adjusts the reference count for the stream to which it
 *  points in a manner similar to SharedPointer.  The reason we don't use SharedPointer is that SProxy needs special
 *  constructors to handle std::ostream operands with runtime type of Stream.
 *
 *  Thread safety: Adjustments to reference counts are synchronized. */
class SAWYER_EXPORT SProxy {
    Stream *stream_;
public:
    SProxy(): stream_(NULL) {}
    SProxy(std::ostream*); /*implicit*/
    SProxy(std::ostream&); /*implicit*/
    SProxy(const SProxy&);
    SProxy& operator=(const SProxy&);
    ~SProxy() { reset(); }
    Stream& operator*() { return *stream_; }
    Stream* operator->() { return stream_; }
    Stream* get() const { return stream_; }
    operator bool() const;
    void reset();
};

/** Converts text to messages.
 *
 *  A message stream is a subclass of <code>std::ostream</code> and therefore allows all the usual stream insertion operators
 *  (<code><<</code>).  A stream converts each line of output text to a single message, creating the message with properties
 *  defined for the stream and sending the results to a specified destination. Streams typically impart a facility name and
 *  importance level to each message via the stream's properties. */
class SAWYER_EXPORT Stream: public std::ostream {
    friend class StreamBuf;
    mutable SAWYER_THREAD_TRAITS::Mutex mutex_;
    size_t nrefs_;                                      // used when we don't have std::move semantics
    StreamBuf *streambuf_;                              // each stream has its own, protected by our mutex
public:

    /** Construct a stream and initialize its name and importance properties. */
    Stream(const std::string facilityName, Importance imp, const DestinationPtr &destination);

    /** Construct a stream and initialize its properties as specified. */
    Stream(const MesgProps &props, const DestinationPtr &destination);

    /** Construct a new stream from an existing stream.
     *
     *  If @p other has a pending message then ownership of that message is moved to this new stream.
     *
     *  Thread safety: This method is thread-safe. */
    Stream(const Stream &other);

    /** Initialize this stream from another stream.
     *
     *  If @p other has a pending message then ownership of that message is moved to this stream.
     *
     *  Thread safety: This method is thread-safe. */
    Stream& operator=(const Stream &other);

    /** Copy constructor with dynamic cast.
     *  
     *  Same as <code>Stream(const Stream&)</code> but declared so we can do things like this:
     *
     * @code
     *  Stream m1(mlog[INFO] <<"first part of the message");
     *  m1 <<"; finalize message\n";
     * @endcode
     *
     *  Thread safety: This method is thread-safe. */
    Stream(const std::ostream &other_);

    /** Assignment with dynamic cast.
     *
     *  Same as <code>operator=(const Stream&)</code> but declared so we can do things like this:
     *
     *  @code
     *  Stream m1 = mlog[INFO] <<"first part of the message");
     *  m1 <<"; finalize message\n";
     *  @endcode
     *
     *  Thread safety:  This method is thread-safe. */
    Stream& operator=(const std::ostream &other_);
    
    ~Stream();

    // used internally by SProxy; thread-safe
    void incrementRefCount();
    size_t decrementRefCount();                         // returns new ref count

    /** Used for partial messages when std::move is missing.
     *
     *  Thread safety: This method is thread-safe. */
    SProxy dup() const;

protected:
    /** Initiaize this stream from @p other.
     *
     *  This stream will get its own StreamBuf (if it doesn't have one already), and any pending message from @p other will be
     *  moved (not copied) to this stream.
     *
     *  Thread safety: This method is not thread-safe--the caller is expected to obtain the necessary locks. */
    void initFromNS(const Stream &other);

public:
    /** Returns true if a stream is enabled.
     *
     *  Thread safety: This method is thread-safe. */
    bool enabled() const;

    // We'd like bool context to return a value that can't be used in arithmetic or comparison operators, but unfortunately
    // we need to also work with the super class (std::basic_ios) that has an implicit "void*" conversion which conflicts with
    // the way Sawyer normally handles this (see SharedPointer for an example).  We therefore override the super-class'
    // void* conversion and "!" operator instead.

    /** Returns true if this stream is enabled.
     *
     *  This implicit conversion to bool can be used to conveniently avoid expensive insertion operations when a stream is
     *  disabled.  For example, if printing <code>MemoryMap</code> is an expensive operation then the logging can be written
     *  any of these ways to avoid formatting memorymap when logging is disabled:
     *
     * @code
     *  if (mlog[DEBUG])
     *      mlog[DEBUG] <<"the memory map is: " <<memoryMap <<"\n";
     *
     *  mlog[DEBUG] && mlog[DEBUG] <<"the memory map is: " <<memoryMap <<"\n";
     *  
     *  SAWYER_MESG(mlog[DEBUG]) <<"the memory map is: " <<memoryMap <<"\n";
     * @endcode
     *
     * Thread safety: This method is thread-safe.
     *
     * @{ */
    operator void*() const {
        return enabled() ? const_cast<Stream*>(this) : NULL;
    }
#if __cplusplus >= 201103L
    explicit operator bool() const {
        return enabled();
    }
#endif
    /** @} */

    /** Returns false if this stream is enabled. */
    bool operator!() const { return !enabled(); }
        
    // See Stream::bool()
    #define SAWYER_MESG(message_stream) message_stream && message_stream

    /** Enable or disable a stream.
     *
     *  A disabled stream buffers the latest partial message and enabling the stream will cause the entire accumulated message
     *  to be emitted--whether the partial message immediately appears on the output is up to the message sinks.
     *
     *  Thread safety: This method is thread-safe.
     * 
     * @{ */
    void enable(bool b=true);
    void disable() { enable(false); }
    /** @} */

    /** Set message or stream property.
     *
     *  If @p asDefault is false then the property is set only in the current message and the message must be empty (e.g., just
     *  after the previous message was completed).  When @p asDefault is true the new value becomes the default for all future
     *  messages. The default also affects the current message if the current message is empty.
     *
     *  Thread safety: This method is thread-safe.
     *
     *  @{ */
    void completionString(const std::string &s, bool asDefault=true);
    void interruptionString(const std::string &s, bool asDefault=true);
    void cancelationString(const std::string &s, bool asDefault=true);
    void facilityName(const std::string &s, bool asDefault=true);
     /** @} */

    /** Property: message destination.
     *
     *  This is the pointer to the top of the plumbing lattice.
     *
     *  Thread safety: This method is thread-safe.
     *
     *  @{ */
    DestinationPtr destination() const;
    Stream& destination(const DestinationPtr &d);
    /** @} */

    /** Return the default properties for this stream.
     *
     *  These are the properties of the stream itself before they are adjusted by the destination lattice. The default
     *  properties are used each time the stream creates a message.
     *
     *  Thread safety: This method is thread-safe. */
    MesgProps properties() const;
};

/** Collection of streams.
 *
 *  This forms a collection of message streams for a software component and contains one stream per message importance level.
 *  A facility is intended to be used by a software component at whatever granularity is desired by the author (program, name
 *  space, class, method, etc.) and is usually given a string name that is related to the software component which it serves.
 *  The string name becomes part of messages and is also the default name used by Facilities::control.  All message streams
 *  created for the facility are given the same name, message prefix generator, and message sink, but they can be adjusted
 *  later on a per-stream basis.
 *
 *  The C++ name for the facility is often just "mlog" or "logger" (appropriately scoped) so that code to emit messages is self
 *  documenting. The name "log" is sometimes used, but can be ambiguous with the <code>\::log</code> function in math.h.
 *
 * @code
 *  mlog[ERROR] <<"I got an error\n";
 * @endcode
 *
 *  Thread safety: This object is thread-safe except where noted. */
class SAWYER_EXPORT Facility {
    static const unsigned CONSTRUCTED_MAGIC = 0x73617779;
    unsigned constructed_;
    mutable SAWYER_THREAD_TRAITS::Mutex mutex_;
#include "WarningsOff.h"
    std::string name_;
    std::vector<SProxy> streams_;
#include "WarningsRestore.h"

public:
    /** Construct an empty facility.
     *
     *  The facility will have no name and all streams will be uninitialized.  Any attempt to emit anything to a facility in
     *  the default state will cause an <code>std::runtime_error</code> to be thrown with a message similar to "stream INFO is
     *  not initialized yet".  This facility can be initialized by assigning it a value from another initialized facility. */
    Facility(): constructed_(CONSTRUCTED_MAGIC) {}

    /** Construct a new facility from an existing facility.
     *
     *  The new facility will point to the same streams as the existing facility. */
    Facility(const Facility &other);

    /** Assignment operator.
     *
     *  The destination facility will point to the same streams as the source facility. */
    Facility& operator=(const Facility &src);

    /** Create a named facility with default destinations.  All streams are enabled and all output goes to file descriptor
     *  2 (standard error) via unbuffered system calls.  Facilities initialized to this state can typically be used before the
     *  C++ runtime is fully initialized and before @ref Sawyer::initializeLibrary is called. */
    explicit Facility(const std::string &name): constructed_(CONSTRUCTED_MAGIC), name_(name) {
        //initializeLibrary() //delay until later
        initStreams(FdSink::instance(2));
    }

    /** Creates streams of all importance levels. */
    Facility(const std::string &name, const DestinationPtr &destination): constructed_(CONSTRUCTED_MAGIC), name_(name) {
        initStreams(destination);
    }

    ~Facility() {
        constructed_ = 0;
    }

    /** Returns true if called on an object that has been constructed.
     *
     *  Returns true if this is constructed, false if it's allocated but not constructed. For instance, this method may return
     *  false if the object is declared at namespace scope and this method is called before the C++ runtime has had a chance to
     *  initialize it.
     *
     *  Thread safety: This method is not thread-safe. */
    bool isConstructed() const { return constructed_ == CONSTRUCTED_MAGIC; }

    /** Returns a stream for the specified importance level.
     *
     *  It is permissible to do the following:
     *
     * @code
     *     Stream m1 = (mlog[INFO] <<"message 1 part 1");
     *     Stream m2 = (mlog[INFO] <<"message 2 part 1");
     *     m1 <<" part 2\n";
     *     m2 <<" part 2\n";
     * @endcode
     *
     *  Thread safety: This method is thread-safe.
     *
     * @{ */
    Stream& get(Importance imp);
    Stream& operator[](Importance imp) {
        return get(imp);
    }
    /** @} */

    /** Return the name of the facility.
     *
     *  This is a read-only field initialized at construction time.
     *
     *  Thread safety: This method is thread-safe. */
    std::string name() const;

    /** Renames all the facility streams.
     *
     *  Invokes Stream::facilityName for each stream. If a name is given then that name is used, otherwise this facility's name
     *  is used.
     *
     *  Thread safety: This method is thread-safe. */
    Facility& renameStreams(const std::string &name = "");

    /** Cause all streams to use the specified destination.
     *
     *  This can be called for facilities that already have streams and destinations, but it can also be called to initialize
     *  the streams for a default-constructed facility.
     *
     *  Thread safety: This method is thread-safe. */
    Facility& initStreams(const DestinationPtr&);
};

/** Collection of facilities.
 *
 *  A facility group is a collection of message facilities that allows its members to be configured collectively, such as from
 *  command-line parsing.  Each stream can be enabled or disabled individually, a member facility can be enabled or disabled as
 *  a whole, or specific importance levels can be enabled or disabled across the entire collection.
 *
 *  Whenever a member facility is enabled as a whole, the current set of enabled importance levels is used.  This set is
 *  maintained automatically: when the first facility is inserted, its enabled importance levels initialize the default set.
 *  Whenever a message importance level is enabled or diabled via the version of @ref enable or @ref disable that takes an
 *  importance argument, the set is adjusted.
 *
 *  Thread safety: This object is thread safe. */
class SAWYER_EXPORT Facilities {
public:
    typedef std::set<Importance> ImportanceSet;         /**< A set of importance levels. */
private:
    typedef Container::Map<std::string, Facility*> FacilityMap;
    mutable SAWYER_THREAD_TRAITS::Mutex mutex_;
#include "WarningsOff.h"
    FacilityMap facilities_;
    ImportanceSet impset_;
    bool impsetInitialized_;
#include "WarningsRestore.h"
public:

    /** Constructs an empty container of Facility objects. */
    Facilities() {
        //initializeLibrary(); -- delay until later for sake of static initialization of Message::mfacilities
        // Initialize impset_ for the sake of insertAndAdjust(), but do not consider it to be truly initialized until after
        // a MessageFacility is inserted.
        impset_.insert(WARN);
        impset_.insert(ERROR);
        impset_.insert(FATAL);
        impsetInitialized_ = false;
    }

    /** Copy constructor. */
    Facilities(const Facilities&);

    /** Assignment operator. */
    Facilities& operator=(const Facilities&);

    /** Return the set of default-enabled message importances.
     *
     *  See Facilities class documentation for details.
     *
     *  Thread safety: This method is thread-safe. */
    ImportanceSet impset() const;

    /** Add or remove a default importance level. The specified level is inserted or removed from the set of default enabled
     *  importance levels without affecting any member facility objects.  If this %Facilities doesn't have a default importance
     *  set then it is initialized to {WARN, ERROR, FATAL} prior to adding or removing the specified level. Calling this
     *  function also prevents the first @ref insert from initializing the set of default importance levels. See Facilities
     *  class documentation for details.
     *
     *  Thread safety: This method is thread-safe.
     *
     * @sa insert, @ref insertAndAdjust, @ref reenable. */
    Facilities& impset(Importance, bool enabled);

    /** Register a facility so it can be controlled as part of a collection of facilities.  The optional @p name is the @e
     *  FACILITY_NAME string of the control language for the inserted @p facility, and defaults to the facility's name (see
     *  @ref control).  The name consists of one or more symbols separated by "::", or "." and often corresponds to the source
     *  code component that it serves.  Names are case-sensitive in the control language.  No two facilities in the same
     *  facility group may have the same name, but a single facility may appear multiple times in the group with different
     *  names.
     *
     *  This method comes in two flavors: @ref insert, and @ref insertAndAdjust.  The latter immediately enables and disables
     *  the facility's streams according to the current default importance levels of this facility group.  If @p facility is
     *  the first facility to be inserted, and it is inserted by @ref insert rather than @ref insertAndAdjust, then the
     *  facility's currently enabled streams are used to initialize the set of default enabled importance levels.
     *
     *  The @p facility is incorporated by reference and should not be destroyed until after this facility group is
     *  destroyed.
     *
     *  Thread safety: This method is thread-safe.
     *
     * @{ */
    Facilities& insert(Facility &facility, const std::string &name="");
    Facilities& insertAndAdjust(Facility &facility, std::string name="");
    /** @} */

    /** Remove a facility by name.
     *
     *  Thread safety: This method is thread-safe. */
    Facilities& erase(const std::string &name);

    /** Remove all occurrences of the facility.
     *
     *  Thread safety: This method is thread-safe. */
    Facilities& erase(Facility &facility);

    /** Return an existing facility by name.
     *
     *  Returns a reference to the specified facility. Throws an <code>std::domain_error</code> if no facility exists with the
     *  specified name.
     *
     *  Thread safety: This method is thread safe, but it returns a reference to a facility that could be deleted before it can
     *  be used.  The caller must ensure that no other thread is invoking the @ref erase method concurrently on this same
     *  Facilities object. */
    Facility& facility(const std::string &name) const;

    /** Return names for all known facilities.
     *
     *  Thread safety: This method is thread-safe. */
    std::vector<std::string> facilityNames() const;
    
    /** Parse a single command-line switch and enable/disable the indicated streams.  Returns an empty string on success, or an
     *  error message on failure.  No configuration changes are made if a failure occurs.
     *
     *  The control string, @p s, is a sentence in the following language:
     *
     * @code
     *  Sentence             := (FacilitiesControl ( ',' FacilitiesControl )* )?
     *
     *  FacilitiesControl    := AllFacilitiesControl | NamedFacilityControl
     *  AllFacilitiesControl := StreamControlList
     *  NamedFacilityControl := FACILITY_NAME '(' StreamControlList ')'
     *
     *  StreamControlList    := StreamControl ( ',' StreamControl )*
     *  StreamControl        := StreamState? Relation? STREAM_NAME | 'all' | 'none'
     *  StreamState          := '+' | '!'
     *  Relation             := '>=' | '>' | '<=' | '<'
     * @endcode
     *
     *  The language is processed left-to-right and stream states are affected immediately.  This allows, for instance, turning
     *  off all streams and then selectively enabling streams.
     *
     *  Some examples:
     * @code
     *  all                             // enable all event messages
     *  none,debug                      // turn on only debugging messages
     *  all,!debug                      // turn on all except debugging
     *  fatal                           // make sure fatal errors are enabled (without changing others)
     *  none,<=trace                    // disable all except those less than or equal to 'trace'
     *
     *  frontend(debug)                 // enable debugging streams in the 'frontend' facility
     *  warn,frontend(!warn)            // turn on warning messages everywhere except the frontend
     * @endcode
     *
     *  The global list, @e AllFacilitiesControl, also affects the list of default-enabled importance levels.
     *
     *  Thread safety: This method is thread-safe.
     */
    std::string control(const std::string &s);

    /** Returns a configuration string.
     *
     *  Returns a string suitable for passing to @ref control. */
    std::string configuration() const;

    /** Readjust all member facilities.
     *
     *  All members are readjusted to enable only those importance levels that are part of this facility group's default
     *  importance levels.  It is as if we called @ref disable then @ref enable for each message facility by name. See
     *  Facilities class documentation for details.
     *
     *  Thread safety: This method is thread-safe.
     *
     *  @sa impset
     *
     * @{ */
    Facilities& reenable();
    Facilities& reenableFrom(const Facilities &other);
    /** @} */

    /** Enable/disable specific importance level across all facilities.
     *
     *  This method also affects the set of "current importance levels" used when enabling an entire facility.
     *
     *  Thread safety: This method is thread-safe.
     *
     * @sa impset
     *  
     * @{ */
    Facilities& enable(Importance, bool b=true);
    Facilities& disable(Importance imp) { return enable(imp, false); }
    /** @} */

    /** Enable/disable a facility by name.
     *
     *  When disabling, all importance levels of the specified facility are disabled. When enabling, only the current
     *  importance levels are enabled for the facility.  If the facility is not found then nothing happens.
     *
     *  Thread safety: This method is thread-safe.
     *
     * @{ */
    Facilities& disable(const std::string &switch_name) { return enable(switch_name, false); }
    Facilities& enable(const std::string &switch_name, bool b=true);
    /** @} */

    /** Enable/disable all facilities.
     *
     *  When disabling, all importance levels of all facilities are disabled.  When enabling, only the current importance
     *  levels are enabled for each facility.
     *
     *  Thread safety: This method is thread-safe.
     *
     * @{ */
    Facilities& enable(bool b=true);
    Facilities& disable() { return enable(false); }
    /** @} */

    /** Print the list of facilities and their states.
     *
     *  This is mostly for debugging purposes. The output may have internal line feeds and will end with a line feed.
     *
     *  Thread safety: This method is thread-safe. */
    void print(std::ostream&) const;

private:
    /** @internal Private info used by control() to indicate what should be adjusted. */
    struct ControlTerm {
        ControlTerm(const std::string &facilityName, bool enable)
            : facilityName(facilityName), lo(DEBUG), hi(DEBUG), enable(enable) {}
        std::string toString() const;                   /**< String representation of this struct for debugging. */
        std::string facilityName;                       /**< %Optional facility name. Empty implies all facilities. */
        Importance lo, hi;                              /**< Inclusive range of importances. */
        bool enable;                                    /**< New state. */
    };

    Facilities& insertNS(Facility&, std::string);       // non-synchronized version
    Facilities& enableNS(Importance, bool);             // non-synchronized version

    // Error information thrown internally.
    struct ControlError {
        ControlError(const std::string &mesg, const char *position): mesg(mesg), inputPosition(position) {}
        std::string mesg;
        const char *inputPosition;
    };

    // Functions used by the control() method
    static std::string parseFacilityName(const char* &input);
    static std::string parseEnablement(const char* &input);
    static std::string parseRelation(const char* &input);
    static std::string parseImportanceName(const char* &input);
    static Importance importanceFromString(const std::string&);
    static std::list<ControlTerm> parseImportanceList(const std::string &facilityName, const char* &input, bool isGlobal);

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Library-provided message destination.  This is the top of a lattice that sends all messages to file descriptor 2, which is
 *  usually standard error. */
SAWYER_EXPORT extern DestinationPtr merr;

/** Facility used by Sawyer components. This facility is added to the @ref mfacilities group with the name "sawyer". */
SAWYER_EXPORT extern Facility mlog;

/** Library-provided facility group. When the library is initialized, this group contains only @ref mlog with the name
 *  "sawyer". Users are free to manipulate this facility however they choose, and if everyone uses this group then all the
 *  facilities across all users can be controlled from this one place. */
SAWYER_EXPORT extern Facilities mfacilities;

/** The stream to be used for assertions. The default is to use <code>Sawyer::Message::mlog[FATAL]</code>. This variable is
 *  initialized at the first call to @ref Assert::fail if it is a null pointer. Users can assign a different stream to it any
 *  time before then:
 *
 * @code
 * int main(int argc, char *argv[]) {
 *     Sawyer::Message::assertionStream = Sawer::Message::mlog[FATAL];
 * @endcode */
SAWYER_EXPORT extern SProxy assertionStream;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Facilities guard
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Saves and restores facilities.
 *
 *  The constructor saves the facilities settings (which streams are enabled and disabled) and the destructor restores
 *  them. This does not save the actual list of streams, so if streams are erased/inserted they will not be restored. */
class SAWYER_EXPORT FacilitiesGuard {
    Facilities &facilities_;
    typedef Sawyer::Container::Map<std::string, std::vector<bool> > State;
    State state_;
public:
    /** Saves and restores the global message facilities.
     *
     *  Saves and restores which streams are enabled in @ref mfacilities. */
    FacilitiesGuard()
        : facilities_(mfacilities) {
        save();
    }

    /** Saves and restores specified message facilities. */
    explicit FacilitiesGuard(Facilities &facilities)
        : facilities_(facilities) {
        save();
    }

    /** Restores previously saved facility settings. */
    ~FacilitiesGuard() {
        restore();
    }

private:
    void save();
    void restore();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      The most commonly used stuff
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Commonly used message types.
 *
 *  This namespace exists so that users can say <code>using namespace Sawyer::Message::Common</code> to be able to use the most
 *  important message types without name qualification and without also bringing in all the things that are less frequently
 *  used.  In particular, this does not include Sawyer::Message::mlog since users often name their own facilities "mlog". */
namespace Common {

using Message::DEBUG;
using Message::TRACE;
using Message::WHERE;
using Message::MARCH;
using Message::INFO;
using Message::WARN;
using Message::ERROR;
using Message::FATAL;

using Message::Stream;
using Message::Facility;
using Message::Facilities;

} // namespace


} // namespace
} // namespace

#endif
