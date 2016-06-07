// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_Assert_H
#define Sawyer_Assert_H

#include "Sawyer.h"
#include <string>

// If SAWYER_NDEBUG is defined then some of the macros defined in this header become no-ops.  For interoperability with the
// more standard NDEBUG symbol, we define SAWYER_NDEBUG if NDEBUG is defined.
#ifdef NDEBUG
#undef SAWYER_NDEBUG
#define SAWYER_NDEBUG
#endif

namespace Sawyer {                                      // documented elsewhere

/** Run-time logic assertions.
 *
 *  This library defines a collection of logic assertion macros (mostly) beginning with "ASSERT_". Many of the macros come in
 *  two flavors: with or without an <code>std::string</code> argument to describe what is being asserted.  Since there is not
 *  an official portable way for macros to have a variable number of arguments, macros that take a descriptive string have "2"
 *  appended to their names. Macros with "always" in their name are always enabled, while some of the others are only enabled
 *  when <code>NDEBUG</code> and <code>SAWYER_NDEBUG</code> are both undefined. Macros that are enabled evaluate their
 *  arguments one time each, and macros that are disabled do not evaluate their arguments at all.
 *
 *  The following macros are defined with "ASSERT_" prefix, optional "always", and optional trailing "2":
 *  
 *  @li <code>require(@e expr, [<em>note</em>])</code>: Asserts that <em>expr</em> evaluates to true, or fails
 *      with the message "assertion failed: required" with details about the failure.
 *  @li <code>forbid(<em>expr</em>, [<em>note</em>])</code>: Asserts that <em>expr</em> evaluates to false, or fails
 *      with the message "assertion failed: prohibitted" with details about the failure.
 *  @li <code>not_null(<em>expr</em>, [<em>note</em>])</code>: Asserts that <em>expr</em> evaluate to non-null, or
 *      fails with the message "null pointer" with details about the failure.
 *  @li <code>this()</code>: Asserts that the containing function is an object method invoked on a non-null object. If
 *      the function is not an object method then the compiler will emit an error. If the object is null at the time of
 *      invocation then the program fails with the message "'this' cannot be null in an object method".
 *  @li <code>not_reachable(<em>note</em>)</code>: Always fails with the message "reached impossible state". This macro
 *      is not disabled when SAWYER_NDEBUG is set.
 *  @li <code>not_implemented(<em>note</em>)</code>: Always fails with the message "not implemented yet". This macro
 *      is not disabled when SAWYER_NDEBUG is set.
 *
 *  Some examples:
 *
 *  @code
 *    // Require the condition to be true, or fail with this message.
 *    ASSERT_require2(sz1 > sz2, "vector one must have more elements than vector two");
 *
 *    // Forbid a vector from being empty.
 *    ASSERT_forbid2(users.empty(), "search must have yielded at least one user");
 *
 *    // Plain, old-fashioned assert.
 *    ASSERT_require(this!=NULL)
 *
 *    // A better way to write the same thing.
 *    ASSERT_not_null(this)
 *
 *    // An even better way to write the same thing.
 *    ASSERT_not_null2(this, "'this' cannot be null in an object method");
 *
 *    // The best way to write the same thing.
 *    ASSERT_this();
 *  @endcode
 *
 *  The following macros do not start with "ASSERT_" because they are widely recognized by IDEs. They are aso not disabled
 *  when <code>SAWYER_NDEBUG</code> is set.
 *
 *  <ul>
 *    <li><code>TODO(<em>note</em>)</code>: Always fails with the message "not implemented yet".</li>
 *    <li><code>FIXME(<em>note</em>)</code>: Always fails with the message "needs to be fixed".</li>
 *  </ul>
 *
 *  When using a <em>note</em> argument, the note should be written in the affirmative, i.e., a statement of what condition
 *  causes the assertion to pass.  It should describe what the macro is asserting, not what went wrong when the macro failed.
 *  For instance, if you're checking that two vectors are the same length, then the note should be something like "name and ID
 *  vectors must be the same size". Writing notes in the affirmative has two benefits: (1) the note documents the code, and (2)
 *  if the assertion fails the message makes sense to the user, namely "assertion failed: name and ID vectors must be the same
 *  size".
 *
 *  Failed assertions output to Sawyer::Message::assertionStream, which defaults to
 *  <code>Sawyer::Message::mlog[FATAL]</code>. This variable is initialized at the first call to @ref fail if it is a null
 *  pointer. Users can assign a different stream to it any time before then:
 *
 * @code
 *  int main(int argc, char *argv[]) {
 *      Sawyer::Message::assertionStream = Sawer::Message::mlog[FATAL];
 * @endcode */
namespace Assert {

/** Cause immediate failure.  This function is the low-level function called by most of the other Sawyer::Assert macros
 *  when an assertion fails. Calls to this function do not return. */
SAWYER_EXPORT_NORETURN void fail(const char *mesg, const char *expr, const std::string &note,
                                 const char *filename, unsigned linenum, const char *funcname) SAWYER_ATTR_NORETURN;

/** Type for user-defined assertion failure handler. */
typedef void (*AssertFailureHandler)(const char *mesg, const char *expr, const std::string &note,
                                     const char *filename, unsigned linenum, const char *funcname);

/** %Optional user callback to handle assertion failures.  If this variable has a non-null value, then that function is called
 *  after the failed assertion message is emitted. This allows the user to terminate the program some other way than the
 *  default @c abort call.  In any case, this function should not return; if it does, then @c abort is called. */
SAWYER_EXPORT extern AssertFailureHandler assertFailureHandler;

} // namespace
} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// These "always" macros are enabled regardless of whether SAWYER_NDEBUG is defined.  Don't use them for
// expensive assertions.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ASSERT_always_require(expr) ASSERT_always_require2(expr, "")
#define ASSERT_always_require2(expr, note)                                                                                     \
    ((expr) ?                                                                                                                  \
        static_cast<void>(0) :                                                                                                 \
        Sawyer::Assert::fail("assertion failed", "required: " #expr, (note),                                                   \
                             __FILE__, __LINE__, SAWYER_PRETTY_FUNCTION))

#define ASSERT_always_forbid(expr) ASSERT_always_forbid2(expr, "")
#define ASSERT_always_forbid2(expr, note)                                                                                      \
    (!(expr) ?                                                                                                                 \
        static_cast<void>(0) :                                                                                                 \
        Sawyer::Assert::fail("assertion failed",                                                                               \
                             "forbidden: " #expr, (note), __FILE__, __LINE__, SAWYER_PRETTY_FUNCTION))

#define ASSERT_always_not_null(expr) ASSERT_always_not_null2(expr, "")
#define ASSERT_always_not_null2(expr, note)                                                                                    \
    ((expr)!=NULL ?                                                                                                            \
        static_cast<void>(0) :                                                                                                 \
        Sawyer::Assert::fail("null pointer",                                                                                   \
                             #expr, (note), __FILE__, __LINE__, SAWYER_PRETTY_FUNCTION))

#define ASSERT_always_not_reachable(note)                                                                                      \
    Sawyer::Assert::fail("reached impossible state", NULL, (note),                                                             \
                         __FILE__, __LINE__, SAWYER_PRETTY_FUNCTION);

#define ASSERT_always_not_implemented(note)                                                                                    \
    Sawyer::Assert::fail("not implemented yet", NULL, (note),                                                                  \
                         __FILE__, __LINE__, SAWYER_PRETTY_FUNCTION)

#define ASSERT_always_this()                                                                                                   \
    (this ?                                                                                                                    \
        static_cast<void>(0) :                                                                                                 \
        Sawyer::Assert::fail("assertion failed",                                                                               \
                             "required: this!=NULL", "'this' cannot be null in an object method",                              \
                             __FILE__, __LINE__, SAWYER_PRETTY_FUNCTION))

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The non-"always" macros might change behavior based on whether SAWYER_NDEBUG is defined.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SAWYER_NDEBUG

#define ASSERT_require(expr)            /*void*/
#define ASSERT_require2(expr, note)     /*void*/
#define ASSERT_forbid(expr)             /*void*/
#define ASSERT_forbid2(expr, note)      /*void*/
#define ASSERT_not_null(expr)           /*void*/
#define ASSERT_not_null2(expr, note)    /*void*/
#define ASSERT_not_reachable(note)      ASSERT_always_not_reachable(note)
#define ASSERT_not_implemented(note)    ASSERT_always_not_implemented(note)
#define ASSERT_this()                   /*void*/

#else

#define ASSERT_require(expr)            ASSERT_always_require(expr)
#define ASSERT_require2(expr, note)     ASSERT_always_require2(expr, note)
#define ASSERT_forbid(expr)             ASSERT_always_forbid(expr)
#define ASSERT_forbid2(expr, note)      ASSERT_always_forbid2(expr, note)
#define ASSERT_not_null(expr)           ASSERT_always_not_null(expr)
#define ASSERT_not_null2(expr, note)    ASSERT_always_not_null2(expr, note)
#define ASSERT_not_reachable(note)      ASSERT_always_not_reachable(note)
#define ASSERT_not_implemented(note)    ASSERT_always_not_implemented(note)
#define ASSERT_this()                   ASSERT_always_this()

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Macros recognized by some IDEs
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TODO(note)                                                                                                             \
    Sawyer::Assert::fail("not implemented yet", NULL, (note),                                                                  \
                         __FILE__, __LINE__, SAWYER_PRETTY_FUNCTION)

#define FIXME(note)                                                                                                            \
    Sawyer::Assert::fail("needs to be fixed", NULL, (note),                                                                    \
                         __FILE__, __LINE__, SAWYER_PRETTY_FUNCTION)

#endif
