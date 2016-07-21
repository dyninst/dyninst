#ifndef ROSE_FormatRestorer_H
#define ROSE_FormatRestorer_H

#include <iostream>

/** Restores output stream flags to original values.
 *
 *  To make temporary changes to stream flags within a function, first create one of these FormatRestorer objects in the
 *  same scope where the temporary changes are being made.  When this object goes out of scope the format flags will be
 *  restored to their original values.
 *
 *  Example
 *
 *  This code does not work because it assumes that the format was decimal to start with:
 * @code
 *  void foo(std::ostream &o) {
 *      o <<std::hex <<bar() <<std::dec;
 *  }
 * @endcode
 *
 *  This code isn't much better, because it doesn't restore the original format if bar() throws an exception:
 * @code
 *  void foo(std::ostream &o) {
 *      std::ios_base::fmtflags old_flags = o.flags();
 *      o <<std::hex <<bar();
 *      o.flags(old_flags);
 *  }
 * @endcode
 *
 *  This code is overly verbose, and gets worse as you add other return points and exception handling.
 * @code
 *  void foo(std::ostream &o) {
 *      std::ios_base::fmtflags old_flags = o.flags();
 *      try {
 *          o <<std::hex <<bar();
 *          o.flags(old_flags);
 *      } catch(...) {
 *          o.flags(old_flags);
 *          throw();
 *      }
 *  }
 * @endcode
 *
 *  Using the FormatRestorer is cleaner:
 * @code
 *  void foo(std::ostream &o) {
 *      FormatRestorer restore(o);
 *      o <<std::hex <<bar() <<std::dec <<baz();
 *  }
 * @endcode
 */
class FormatRestorer {
protected:
    std::ostream &stream;
    std::ios_base::fmtflags fmt;

public:
    /** Constructor saves output stream flags. */
    FormatRestorer(std::ostream &o): stream(o) {
        save(o);
    }

    /** Destructor restores output stream flags. */
    ~FormatRestorer() {
        restore();
    }

    /** Save current output stream flags. */
    void save(std::ostream &o) {
        fmt = o.flags();
    }

    /** Restore saved flags. Flags can be restored as many times as desired. */
    void restore() {
        stream.flags(fmt);
    }
};
    
#endif
