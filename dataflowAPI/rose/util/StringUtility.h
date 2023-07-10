#ifndef ROSE_StringUtility_H
#define ROSE_StringUtility_H

//#include "commandline_processing.h"
#include <ostream>
#include <stddef.h>
#include <vector>
#include <map>
#include <list>
#include <string>
#include <sstream>
#include <stdint.h>
#include <limits.h>

#include "../rose.h"


/** Functions for operating on strings.
 *
 *  This name space provides functions for operating on strings.
 *
 *  It also provides some unrelated functions: functions for operating on file names (see also @ref rose::FileSystem and
 *  boost::filesystem for better implementations), and functions for performing file I/O. These unrelated functions should
 *  eventually move to other name spaces. */
namespace StringUtility {

/** String with source location information.
 *
 *  The location information is a file name string (empty means generated code) and a line number. Line numbers are one-origin;
 *  the first line of a file is numbered 1, not 0.
 *
 *  @todo What does it mean to have a non-positive line number? What line number should be used for generated code when the @p
 *  filename member is empty? */
    struct StringWithLineNumber {
        std::string str;                                    // DQ (1/23/2010): this name is difficult to trace within the code.
        std::string filename;                               // Empty string means generated code
        unsigned int line;

        StringWithLineNumber(const std::string &str, const std::string &filename, unsigned int line)
                : str(str), filename(filename), line(line) { }

        std::string toString() const;
    };

#ifndef USE_ROSE
    typedef std::vector <StringWithLineNumber> FileWithLineNumbers;
#else
    // workaround of bug 315, separating definitions for a namespace [Liao, 2/16/2009]
    }
    namespace StringUtility {
    typedef std::vector<StringUtility::StringWithLineNumber> FileWithLineNumbers;
#endif

/** Prints a StringWithLineNumber. */
    inline std::ostream &operator<<(std::ostream &os, const StringWithLineNumber &s) {
        os << s.toString();
        return os;
    }

/** Generate C preprocessor #line directives.
 *
 *  Given a vector of strings with source location information, print those strings with intervening C preprocessor #line
 *  directives. For strings that have no source location, use the specified @p filename argument as the name of the file. The
 *  @p physicalLine is the line number to assume for the first line of output (the return value). The #line directives are
 *  not emitted into the return value when they're not needed (e.g., two @p strings are at successive line numbers of the same
 *  source file) or if ROSE has been configured to not emit #line directivies (i.e., the @c
 *  SKIP_HASH_LINE_NUMBER_DECLARATIONS_IN_GENERATED_FILES preprocessor symbol is defined). The @p strings should not have
 *  trailing linefeeds or else the output will contain extra blank lines. */
    std::string toString(const FileWithLineNumbers &strings, const std::string &filename = "<unknown>", int line = 1);

/** Append strings with source location information to vector of such.
 *
 *  Modifies @p a by appending those strings from @p b. */
    inline FileWithLineNumbers &operator+=(FileWithLineNumbers &a, const FileWithLineNumbers &b) {
        a.insert(a.end(), b.begin(), b.end());
        return a;
    }

/** Concatenate vectors of strings with source location.
 *
 *  Returns a new vector of strings with location information by concatenating vector @p a with @p b. */
    inline FileWithLineNumbers operator+(const FileWithLineNumbers &a, const FileWithLineNumbers &b) {
        FileWithLineNumbers f = a;
        f += b;
        return f;
    }

/** Append string to vector of strings with location information.
 *
 *  Appends @p str to the last string in the vector of strings with location information. If the vector is empty then a new
 *  element is created.
 *
 *  The new code is marked as either generated (empty file name) or not generated (non-empty name) based on whether the vector
 *  was initially empty or whether the final element of the vector was generated.  Adding a string to an empty vector makes it
 *  always be a generated string; adding it to a non-empty vector makes it generated or not generated depending on whether the
 *  last element of the vector is generated or not generated.
 *
 *  The string @p str should not include line termination. (see @ref toString).
 *
 *  @{ */
    inline FileWithLineNumbers &operator<<(FileWithLineNumbers &f, const std::string &str) {
        if (!f.empty() && f.back().filename == "") {
            f.back().str += str;
        } else {
            f.push_back(StringWithLineNumber(str, "", 1));
        }
        return f;
    }

    inline FileWithLineNumbers &operator<<(FileWithLineNumbers &f, const char *str) {
        f << std::string(str);
        return f;
    }
/** @} */

/** Replace all occurrences of a string with another string.
 *
 *  Finds all occurrences of the @p oldToken string in @p inputString and replaces them each with @p newToken, returning the
 *  result.
 *
 * @{ */
    std::string copyEdit(const std::string &inputString, const std::string &oldToken, const std::string &newToken);

    FileWithLineNumbers copyEdit(const FileWithLineNumbers &inputString, const std::string &oldToken,
                                 const std::string &newToken);

    FileWithLineNumbers copyEdit(const FileWithLineNumbers &inputString, const std::string &oldToken,
                                 const FileWithLineNumbers &newToken);
/** @} */

/** Convert an integer to a string.
 *
 *  These functions are wrappers around <code>boost::lexical_cast<std::string></code>.
 *
 *  @{ */
    std::string numberToString(long long);

    std::string numberToString(unsigned long long);

    std::string numberToString(long);

    std::string numberToString(unsigned long);

    std::string numberToString(int);

    std::string numberToString(unsigned int);
/** @} */

/** Convert an integer to a hexadecimal string. */
    std::string intToHex(uint64_t);

#ifndef _MSC_VER
// #if !defined(__STRICT_ANSI__) && defined(_GLIBCXX_USE_INT128)
// #if ((BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER == 4) && (BACKEND_CXX_COMPILER_MINOR_VERSION_NUMBER > 6))
#if (defined(BACKEND_CXX_IS_GNU_COMPILER) && (((BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER == 4) && (BACKEND_CXX_COMPILER_MINOR_VERSION_NUMBER > 6)) || (BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER > 4)))
    // DQ (2/22/2014): Required code for GNU versions greater than 4.6.
// #if (UINTMAX_MAX == ULONG_MAX)
#if (__WORDSIZE == 64)

    // PHLin (10/12/2015): check if 64-bit support is enabled
        std::string numberToString ( __int128 x );
        std::string numberToString ( unsigned __int128 x );
#endif
#endif
#endif

/** Convert a pointer to a string. */
    std::string numberToString(const void *);

/** Convert a floating-point number to a string.
 *
 *  The returned string uses @c printf with "%2.2f" format. */
    std::string numberToString(double);

/** Convert a virtual address to a string.
 *
 *  Converts a virtual address to a hexadecimal string with a leading "0x". The string is zero-padded so that it explicitly
 *  represents at least @p nbits bits (four bits per hexadecimal digits). If @p nbits is zero then the function uses 32 bits
 *  for values that fit in 32 bits, otherwise 64 bits. */
    std::string addrToString(uint64_t value, size_t nbits = 0);

/** Formatting support for generated code strings. */
    std::string indentMultilineString(const std::string &inputString, int statementColumnNumber);

/** Generate a string from a list of integers.
 *
 *  The return value is the concatenation of substrings. Each substring is formed by converting the corresponding integer from
 *  the list into a string via @ref numberToString and then adding a single space character and an optional line feed. The line
 *  feeds are added only if @p separateStrings is true. */
    std::string listToString(const std::list<int> &, bool separateStrings = false);

/** Generate a string from a container of strings.
 *
 *  The return value is the concatenation of substrings. Each substring is formed by adding a single space to the corresponding
 *  list element and an optional line feed. The line feeds are added only if @p separateStrings is true.
 *
 *  @{ */
    std::string listToString(const std::list <std::string> &, bool separateStrings = false);

    std::string listToString(const std::vector <std::string> &, bool separateStrings = false);
/** @} */

/** Split a string into substrings at line feeds.
 *
 *  Splits the input string into a list of substrings at the line-feed characters (the line feeds are at the end of the
 *  substrings). Substrings that are empty or consist of a single line-feed character are removed from the return value,
 *  although substrings that contain other sequences of only white space are not removed.
 *
 *  Warning: This function is slow when invoked on a very large input string because it makes excessive copies of the input. */
    std::list <std::string> stringToList(const std::string &);

/** Split a string into a list based on a separator character.
 *
 *  Scans the input string for delimiter characters and splits the input into substrings at each delimiter positions. The
 *  delimiter is not included in the substring. Consecutive delimiter characters will result in an empty substring. */
    std::list <std::string> tokenize(std::string X, char delim);

/** Splits string into substring based on a separator character.
 *
 *  Empty strings are removed from the result, which is returned in the @p stringList argument. The return argument is cleared
 *  before the splitting begins. */
    void splitStringIntoStrings(const std::string &inputString, char separator, std::vector <std::string> &stringList);

/** Remove redundant and blank lines.
 *
 *  Splits the input string into substrings according to @ref listToString, sorts the substrings and removes duplicates and
 *  lines that are empty (a line of only horizontal white space is not considered to be empty), then concatenates the
 *  substrings in their sorted order into the return value using @ref listToString, inserting extra white space at the
 *  beginning of all but the first line. */
    std::string removeRedundantSubstrings(const std::string &);

// [Robb Matzke 2016-01-06]: deprecated due to being misspelled
    std::string removeRedundentSubstrings(std::string);

/** Remove redundant lines containing special substrings of form string#. */
    std::string removePseudoRedundantSubstrings(const std::string &);

// [Robb Matzke 2016-01-06]: deprecated due to being misspelled
    std::string removePseudoRedundentSubstrings(std::string);

/** Append an abbreviation or full name to a string. */
    void add_to_reason_string(std::string &result, bool isset, bool do_pad,
                              const std::string &abbr, const std::string &full);

/** Determines whether one string contains another.
 *
 *  Returns true if @p longString contains @p shortString as a subsequence. */
    inline bool isContainedIn(const std::string &longString, const std::string &shortString) {
        return longString.find(shortString) != std::string::npos;
    }

/** Compute a checkshum.
 *
 *  This function returns a unique checksum from the mangled name used it provides a simple means to obtain a unique value for
 *  any C++  declaration.  At a later date was should use the MD5 Checksum  implementation (but we can do that later).
 *
 *  The declaration is the same under One-time Definition Rule (ODR) if and only if the checksum values for each declaration
 *  are the same. */
    unsigned long generate_checksum(std::string s);

/** Convert to lower case.
 *
 *  Returns a new string by converting each of the input characters to lower case with @c tolower. */
    std::string convertToLowerCase(const std::string &inputString);

/** Escapes line feeds and double quotes.
 *
 *  Scans the input string character by character and replaces line-feed characters with a backslash followed by the letter "l"
 *  and replaces double quotes by a backslash followed by a double qoute. */
    std::string escapeNewLineCharaters(const std::string &);

/** Escapes HTML special characters.
 *
 *  Replaces "<", ">", and "&" with HTML character names and returns the result. */
    std::string htmlEscape(const std::string &);

/** Escapes characters that are special to C/C++.
 *
 *  Replaces special characters in the input so that it is suitable for the contents of a C string literal. That is, things
 *  like double quotes, line-feeds, tabs, non-printables, etc. are replace by their C backslash escaped versions. Returns the
 *  resulting string. */
    std::string cEscape(const std::string &);

/** Convert an ASCII hexadecimal character to an integer.
 *
 *  Converts the characters 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e, f, A, B, C, D, E, and F into their hexadecimal integer
 *  equivalents. Returns zero if the input character is not in this set. */
    unsigned hexadecimalToInt(char);

/** Convert a number to a hexadecimal and decimal string.
 *
 *  The returned string starts with the hexadecimal representation of the number and an optional decimal representation
 *  in angle brackets.  The decimal portion will contain a signed and/or unsigned value depending on whether the value
 *  is interpretted as signed and whether the sign bit is set.  The signedToHex versions print the decimal value for only
 *  the signed interpretation; the unsignedToHex versions print only the decimal unsigned interpretation, and the toHex
 *  versions print both (but not redunantly).
 *
 *  @{ */
    std::string toHex2(uint64_t value, size_t nbits,
                       bool show_unsigned_decimal = true, bool show_signed_decimal = true,
                       uint64_t decimal_threshold = 256);

    std::string signedToHex2(uint64_t value, size_t nbits);

    std::string unsignedToHex2(uint64_t value, size_t nbits);

    template<typename T>
    std::string toHex(T value) { return toHex2((uint64_t) value, 8 * sizeof(T)); }

    template<typename T>
    std::string signedToHex(T value) { return signedToHex2((uint64_t) value, 8 * sizeof(T)); }

    template<typename T>
    std::string unsignedToHex(T value) { return unsignedToHex2((uint64_t) value, 8 * sizeof(T)); }
/** @} */

/** Append an assembly comment to a string.  Assembly comments are surrounded by "<" and ">" characters.  If the string
 *  already ends with an assembly comment, then the specified comment is inserted before the final ">" and separated from
 *  the previous comment with a comma.  Assembly comments are usually used for things like printing a decimal representation
 *  of a hexadecimal value, etc.
 *
 *  Example: after executing these statements:
 *
 *  @code
 *   std::string s = "0xff";
 *   s = appendAsmComment(s, "255");
 *   s = appendAsmComment(s, "-1");
 *  @endcode
 *
 *  The variable "s" will contain "0xff<255,-1>" */
    std::string appendAsmComment(const std::string &s, const std::string &comment);

/** Insert a prefix string before every line.
 *
 *  This function breaks the @p lines string into individual lines, inserts the @p prefix string at the beginning of each line,
 *  then concatenates the lines together into a return value.  If @p prefixAtFront is true (the default) then the prefix is
 *  added to the first line of @p lines, otherwise the first line is unchanged.  An empty @p lines string is considered to be a
 *  single line.  If @p prefixAtBack is false (the default) then the prefix is not appended to the @p lines string if @p lines
 *  ends with a linefeed. */
    std::string prefixLines(const std::string &lines, const std::string &prefix,
                            bool prefixAtFront = true, bool prefixAtBack = false);

/** Returns true if the string ends with line termination.
 *
 *  Only common ASCII-based line terminations are recognized: CR+LF, LF+CR, CR (only), or LF (only). */
    bool isLineTerminated(const std::string &s);

/** Normalizes line termination.
 *
 *  Changes ASCII-based line termination conventions used by various operating systems into the LF (line-feed) termination used
 *  by Multics, Unix and Unix-like systems (GNU/Linux, Mac OS X, FreeBSD, AIX, Xenix, etc.), BeOS, Amiga, RISC OS and others.
 *  Any occurrance of CR+LF, LF+CR, or CR by itself (in that order of left-to-right matching) is replaced by a single LF
 *  character. */
    std::string fixLineTermination(const std::string &input);

/** Converts a multi-line string to a single line.
 *
 *  This function converts a multi-line string to a single line by replacing line-feeds and carriage-returns (and their
 *  surrounding white space) with a user-supplied replacement string (that defaults to a single space). Line termination (and
 *  it's surrounding white space) that appears at the front or back of the input string is removed without replacing it.
 *
 *  See roseTests/utilTests/stringTests.C for lots of examples.
 *
 *  A new string is returned. */
    std::string makeOneLine(const std::string &s, std::string replacement = " ");

/** Convert binary data to base-64.
 *
 *  The base64 number system uses the characters A-Z, a-z, 0-9, +, and / (in that order). The returned string does not include
 *  linefeeds.  If @p do_pad is true then '=' characters may appear at the end to make the total length a multiple of four.
 *
 * @{ */
    std::string encode_base64(const std::vector <uint8_t> &data, bool do_pad = true);

    std::string encode_base64(const uint8_t *data, size_t nbytes, bool do_padd = true);
/** @} */

/** Convert base-64 to binary. */
    std::vector <uint8_t> decode_base64(const std::string &encoded);

/** Join individual strings to form a single string.
 *
 *  Unlike @ref listToString, this function allows the caller to indicate how the strings should be separated from one another:
 *  the @p separator (default SPC) is inserted between each pair of strings, but not at the beginning or end, even if strings
 *  are empty.
 *
 * @{ */
    template<class Container>
    std::string join(const std::string &separator, const Container &strings) {
        std::string retval;
        for (typename Container::const_iterator i = strings.begin(); i != strings.end(); ++i)
            retval += (i == strings.begin() ? std::string() : separator) + *i;
        return retval;
    }

    template<class Iterator>
    std::string join_range(const std::string &separator, Iterator begin, Iterator end) {
        std::string retval;
        for (Iterator i = begin; i != end; ++i)
            retval += (i == begin ? std::string() : separator) + *i;
        return retval;
    }

    std::string join(const std::string &separator, char *strings[], size_t nstrings);

    std::string join(const std::string &separator, const char *strings[], size_t nstrings);
/** @} */

/** Splits strings into parts.
 *
 *  Unlink @ref stringToList, this function allows the caller to indicate where to split the string.  The parts are the
 *  portions of the string on either side of the separator: if the separator appears at the beginning of the string, then the
 *  first part is empty; likewise if the separator appears at the end of the string then the last part is empty. At most @p
 *  maxparts are returned, the last of which may contain occurrences of the separator.  If @p trim_white_space is true then
 *  white space is removed from the beginning and end of each part. Empty parts are never removed from the returned vector
 *  since the C++ library already has functions for that. The first few arguments are in the same order as for Perl's "split"
 *  operator.
 *
 * @{ */
    std::vector <std::string> split(const std::string &separator, const std::string &str,
                                    size_t maxparts = (size_t)(-1),
                                    bool trim_white_space = false);

    std::vector <std::string> split(char separator, const std::string &str, size_t maxparts = (size_t)(-1),
                                    bool trim_white_space = false);
/** @} */

/** Trims white space from the beginning and end of a string.
 *
 *  Caller may specify the characters to strip and whether the stripping occurs at the begining, the end, or both. */
    std::string trim(const std::string &str, const std::string &strip = " \t\r\n",
                     bool at_beginning = true, bool at_end = true);

/** Expand horizontal tab characters. */
    std::string untab(const std::string &str, size_t tabstops = 8, size_t firstcol = 0);

/** Converts a bunch of numbers to strings.
 *
 *  This is convenient when one has a container of numbers and wants to call @ref join to turn it into a single string.  For
 *  instance, here's how to convert a set of integers to a comma-separated list:
 *
 * @code
 *  using namespace StringUtility;
 *  std::set<int> numbers = ...;
 *  std::string s = join(", ", toStrings(numbers));
 * @endcode
 *
 *  Here's how to convert a vector of addresses to space-separated hexadecimal values:
 * @code
 *  using namespace StringUtility;
 *  std::vector<rose_addr_t> addresses = ...;
 *  std::string s = join(" ", toStrings(addresses, addrToString));
 * @endcode
 *
 *  Here's how one could surround each address with angle brackets:
 * @code
 *  using namespace StringUtility;
 *  struct AngleSurround {
 *      std::string operator()(rose_addr_t addr) {
 *         return "<" + addrToString(addr) + ">";
 *      }
 *  };
 *  std::string s = join(" ", toStrings(addresses, AngleSurround()));
 * @endcode
 * @{ */
    template<class Container, class Stringifier>
    std::vector <std::string> toStrings(const Container &numbers, const Stringifier &stringifier = numberToString) {
        return toStrings_range(numbers.begin(), numbers.end(), stringifier);
    }

    template<class Iterator, class Stringifier>
    std::vector <std::string> toStrings_range(Iterator begin, Iterator end,
                                              const Stringifier &stringifier = numberToString) {
        std::vector <std::string> retval;
        for (/*void*/; begin != end; ++begin)
            retval.push_back(stringifier(*begin));
        return retval;
    }
/** @} */

/** Helpful way to print singular or plural words.
 *
 * @code
 *  size_t n = ...;
 *  std::cout <<"received " <<plural(n, "values") <<"\n";
 * @encode
 *
 *  Output for various values of <em>n</em> will be:
 *
 * @code
 *  received 0 values
 *  received 1 value
 *  received 2 values
 * @endcode
 *
 * This function uses a handful of grade-school rules for converting the supplied plural word to a singular word when
 * necessary.  If these are not enough, then the singular form can be supplied as the third argument.
 *
 * @code
 *  std::cout <<"graph contains " <<plural(nverts, "vertices", "vertex") <<"\n";
 * @endcode
 */
    template<typename T>
    std::string plural(T n, const std::string &plural_word, const std::string &singular_word = "") {
        //assert(!plural_word.empty());
        std::string retval = numberToString(n) + " ";
        if (1 == n) {
            if (!singular_word.empty()) {
                retval += singular_word;
            } else if (plural_word.size() > 3 && 0 == plural_word.substr(plural_word.size() - 3).compare("ies")) {
                // string ends with "ies", as in "parties", so emit "party" instead
                retval += plural_word.substr(0, plural_word.size() - 3) + "y";
            } else if (plural_word.size() > 1 && plural_word[plural_word.size() - 1] == 's') {
                // just drop the final 's'
                retval += plural_word.substr(0, plural_word.size() - 1);
            } else {
                // I give up.  Use the plural and risk being grammatically incorrect.
                retval += plural_word;
            }
        } else {
            retval += plural_word;
        }
        return retval;
    }

// demangledName is defined in rose_support.cpp

/** Compute demangled version of mangled name.
 *
 *  Runs the c++filt command on the input string and returns the result.  If c++filt cannot be run then it prints an error to
 *  standard error and another to standard output. The pipes opened to communicate with the c++filt subcommand might not be
 *  closed if there's an error. */
    std::string demangledName(std::string);








////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This part of the StringUtility API deals with operating system information. It has nothing to do with string utilities and
// should be moved elsewhere.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    enum OSType {
        OS_TYPE_UNKNOWN,
        OS_TYPE_LINUX,
        OS_TYPE_OSX,
        OS_TYPE_WINDOWS,
        OS_TPYE_WINDOWSXP
    };

/** Obtain operating system type information. */
    OSType getOSType();










////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This part of the StringUtility API deals with file names and should be moved to some other name space. In particular, it
// provides no definitions for "path", "filename", "extension", etc. and many of these functions won't work properly on a
// non-POSIX system. Therefore, consider using rose::FileSystem, which is mostly a thin wrapper around boost::filesystem. The
// boost::filesystem documentation has good definitions for what the various terms should mean and works on non-POSIX file
// systems.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Create a file.
 *
 *  Creates a new file, truncating any existing file with the same name, and writes the string @p outputString into the
 *  file. The name of the file is constructed by concatenating @p directoryName and @p fileNameString without any intervening
 *  component separator (e.g., no "/").
 *
 *  If the file cannot be created then this function silently fails (or aborts if ROSE is compiled in debug mode). */
    void writeFile(const std::string &outputString, const std::string &fileNameString,
                   const std::string &directoryName);

/** Reads entire text file.
 *
 *  Opens the specified file, reads its contents into a string, closes the file, and returns that string.
 *
 *  If the file cannot be opened then an std::string error message is thrown. The message reads "File not found" regardless of
 *  the actual error condition. */
    std::string readFile(const std::string &fileName);

/** Reads an entire text file.
 *
 *  Opens the specified file, reads its contents line by line into a vector of strings with location information, closes the
 *  file, and returns the vector.  The file names in the returned value are absolute names. The strings in the return value
 *  have their final line-feeds removed.
 *
 *  If the file cannot be opened then an std::string error message is thrown. The message reads "File not found" regardless of
 *  the actual error condition. */
    FileWithLineNumbers readFileWithPos(const std::string &fileName);

/** Name of the home directory.
 *
 *  Returns the value of the "HOME" environment variable by copying it into the @p dir argument. Will segfault if this
 *  environment variable is not set. */
    void homeDir(std::string &dir);

/** Returns the last component of a path in a filesystem.
 *
 *  Removes the "path" part of a "filename" (if there is one) and returns just the file name.
 *
 *  Terms are loosely defined and not likely to work for non-POSIX systems; consider using boost::filesystem instead. */
    std::string stripPathFromFileName(const std::string &fileNameWithPath);

/** Returns all but the last component of a path in a filesystem.
 *
 *  This function removes the filename from the combined path and filename if it includes a path and returns only the path.
 *  Make it safe to input a filename without a path name (return the filename).
 *
 *  Terms are loosely defined and this function possibly doesn't work for non-POSIX file systems; consider using
 *  boost::filesystem instead. */
    std::string getPathFromFileName(const std::string &fileNameWithPath);

/** Get the file name without the ".suffix".
 *
 *  Terms are loosely defined and it's not clear what happens for inputs like ".", ".foo", "..", ".foo.bar", "/.",
 *  etc. Consider using boost::filesystem instead. */
    std::string stripFileSuffixFromFileName(const std::string &fileNameWithSuffix);

/** Get the absolute path from the relative path.
 *
 *  Terms are loosely defined and this function is not likely to work on non-POSIX systems. Consider using boost::filesystem
 *  instead. */
    std::string getAbsolutePathFromRelativePath(const std::string &relativePath, bool printErrorIfAny = false);

/** Get the file name suffix (extension) without the leading dot.
 *
 *  Filename could be either base name or name with full path. If no dot is found in the input fileName, the function just
 *  returns the original fileName.
 *
 *  Terms are loosely defined and this function is not likely to work correctly in some situations, such as when the "." is not
 *  in the last component of the file name.  Consider using boost::filesystem instead. */
    std::string fileNameSuffix(const std::string &fileName);

/** Find file names non-recursively.
 *
 *  Scans the directory named @p pathString and returns a list of files in that directory which have @p patternString as a
 *  substring of their name. Note that @p patternString is not a glob or regular expression.  The return value strings are
 *  formed by concatenating the @p pathString and the file name with an intervening slash.
 *
 *  This function does not work for non-POSIX systems. Consider using boost::filesystem instead, which has a directory iterator
 *  that works for non-POSIX systems also. */
    std::list <std::string> findfile(std::string patternString, std::string pathString);

/* File name location.
 *
 * Files can be classified as being in one of three locations: We don't know if it's user or system It is a user (application)
 * file It is a system library This file does not exist */
    enum FileNameLocation {
        FILENAME_LOCATION_UNKNOWN,
        FILENAME_LOCATION_USER,
        FILENAME_LOCATION_LIBRARY,
        FILENAME_LOCATION_NOT_EXIST
    };

    static const std::string FILENAME_LIBRARY_UNKNOWN = "Unknown";
    static const std::string FILENAME_LIBRARY_USER = "User";
    static const std::string FILENAME_LIBRARY_C = "C";
    static const std::string FILENAME_LIBRARY_STDCXX = "C++";
    static const std::string FILENAME_LIBRARY_STL = "STL";
    static const std::string FILENAME_LIBRARY_LINUX = "Linux";
    static const std::string FILENAME_LIBRARY_GCC = "GCC";
    static const std::string FILENAME_LIBRARY_BOOST = "Boost";
    static const std::string FILENAME_LIBRARY_ROSE = "Rose";

// CH (2/16/2010): Use this typedef to avoid following changes
    typedef std::string FileNameLibrary;

/* This is the return type of classifyFileName, which provides all the details it infers */
    class FileNameClassification {
    private:
        FileNameLocation location;

        // CH (2/12/2010): Change 'library' type from enum to string to let user set it
        FileNameLibrary library;
        int distance;

    public:
        FileNameClassification(FileNameLocation loc, const FileNameLibrary &lib, int dist)
                : location(loc), library(lib), distance(dist) { }

        FileNameClassification()
                : location(FILENAME_LOCATION_UNKNOWN), library("Unknown"), distance(0) { }

        /* Return the FileNameLocation which is described above with the definition of the enum */
        FileNameLocation getLocation() const {
            return location;
        }

        /* Return the FileNameLibrary which is described above with the definition of the enum */
        FileNameLibrary getLibrary() const {
            return library;
        }

        /* Return the "distance" of the filename from the appPath that was supplied during the call.  The distance is defined as
         * the number of cd's that only move up or down one directory that it would take to move from the directory of the filename
         * to the directory that was given by appPath.  This is intended as a heuristic to gage whether or not one believes that
         * the filename is related to the source (appPath) directory.  Examples:
         *
         * Between /a/b/c/file.h and /a/b/d/e/ the distance is 3 because one must cd ..; cd d; cd e; to get to appPath
         *
         * *EXCEPTION*: if the appPath is an ancestor of filename then the distance will be 0.  The idea being that this filename
         * is "in" the appPath somewhere and thus part of the application. */
        int getDistanceFromSourceDirectory() const {
            return distance;
        }

        bool isUserCode() const {
            return location == FILENAME_LOCATION_USER;
        }

        bool isLibraryCode() const {
            return location == FILENAME_LOCATION_LIBRARY;
        }

        /* Return a string name for the library indicated by getLibrary() */
        std::string getLibraryName() const {
            return library;
        }
    };

/** Determine whether a file is source code or system library.
 *
 *  Given a fileName and an appPath that is a path to some application's source code directory, return a FileNameClassification
 *  indicating whether the fileName is part of the source code or some system library and automatically determine the operating
 *  system from the host uname */
    FileNameClassification classifyFileName(const std::string &fileName, const std::string &appPath);

/** Determine whether a file is source code or system library.
 *
 *  Given a fileName and an appPath that is a path to some application's source code directory, return a FileNameClassification
 *  indicating whether the fileName is part of the source code or some system library */
    FileNameClassification classifyFileName(const std::string &fileName, const std::string &appPath, OSType os);

/** Determine whether a file is source code or system library.
 *
 *  Given a fileName and an appPath that is a path to some application's source code directory, and a collection of library
 *  paths, return a FileNameClassification indicating whether the fileName is part of the source code or some system library
 *  and automatically determine the operating system from the host uname */
    FileNameClassification classifyFileName(const std::string &fileName, const std::string &appPath,
                                            const std::map <std::string, std::string> &libPathCollection);

/** Determine whether a file is source code or system library.
 *
 *  Given a fileName and an appPath that is a path to some application's source code directory, and a collection of library
 *  paths, return a FileNameClassification indicating whether the fileName is part of the source code or some system library */
    FileNameClassification classifyFileName(const std::string &fileName, const std::string &appPath,
                                            const std::map <std::string, std::string> &libPathCollection,
                                            OSType os);

/** Remove leading dots.
 *
 *  Removes leading dots plus a space from a header file name that is given in the format that g++ -H returns */
    const std::string stripDotsFromHeaderFileName(const std::string &name);

/** Edit distance between two directory names.
 *
 *  Essentially the edit distance without substituion in directory name tokens between two directories. Returns the "distance"
 *  between left and right. The distance is defined as the number of cd's that only move up or down one directory that it would
 *  take to move from the directory of the filename to the directory that was given by appPath.  This is intended as a
 *  heuristic to gage whether or not one believes that the left is related to the right directory.  Examples:
 *
 *  Between /a/b/c/file.h and /a/b/d/e/ the distance is 3 because one must cd ..; cd d; cd e */
    int directoryDistance(const std::string &left, const std::string &right);

/** Reads words from a file.
 *
 *  Opens the specified file for reading and reads all words from the file using <code>std::istream</code>
 *  <code>operator>></code> into <code>std::string</code>.  If the file cannot be opened then an error message is printed to
 *  standard output (not error) and the program exits with status 1. */
    std::vector <std::string> readWordsInFile(std::string filename);

// popen_wrapper is defined in sage_support.cpp

/** Simple wrapper for Unix popen command.
 *
 *  If there is a failure (cannot create pipes, command not found, command terminated abnormally, input buffer too small, etc),
 *  then an error is printed to standard error (with or without line termination) and false is returned.  When an error occurs
 *  the pipes that were opened to communicate with the subcommand might not be closed. */
    bool popen_wrapper(const std::string &command, std::vector <std::string> &result);

} // namespace

#endif
