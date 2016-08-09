// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_Buffer_H
#define Sawyer_Buffer_H

#include "Sawyer.h"
#include "SharedPointer.h"
#include <string>

namespace Sawyer {
namespace Container {

/** Base class for all buffers.
 *
 *  A buffer stores a sequence of elements somewhat like a vector, but imparts a read/write paradigm for accessing those
 *  elements. */
template<class A, class T>
class Buffer: public SharedObject {
    std::string name_;
    bool copyOnWrite_;
protected:
    Buffer(): name_(generateSequentialName()), copyOnWrite_(false) {}
public:
    /** Reference counting smart pointer.
     *
     *  @sa smart_pointers */
    typedef SharedPointer<Buffer> Ptr;

    /** Key type for addressing data.
     *
     *  The key type should be an unsigned integral type and is the type used to index the data. */
    typedef A Address;

    /** Type of values stored in the buffer. */
    typedef T Value;

public:
    virtual ~Buffer() {}

    /** Create a new copy of buffer data.
     *
     *  Returns a new buffer containing the same data as the old buffer.  Some buffer types cannot make an exact copy, in which
     *  case they should return an AllocatingBuffer that holds a snapshot of the source buffer's data as it existed at the time
     *  of this operation. */
    virtual Ptr copy() const = 0;

    /** Distance to end of buffer.
     *
     *  The distance in units of the Value type from the specified address (inclusive) to the last element of the buffer
     *  (inclusive).  If the address is beyond the end of the buffer then a distance of zero is returned rather than a negative
     *  distance. Note that the return value will overflow to zero if the buffer spans the entire address space. */
    virtual Address available(Address address) const = 0;
    
    /** Size of buffer.
     *
     *  Returns the number of @ref Value elements stored in the buffer. In other words, this is the first address beyond the
     *  end of the buffer.  Note that the return value may overflow to zero if the buffer spans the entire address space. */
    virtual Address size() const { return available(Address(0)); }

    /** Change the size of the buffer.
     *
     *  Truncates the buffer to a smaller size, or extends the buffer as necessary to make its size @p n values. */
    virtual void resize(Address n) = 0;

    /** Synchronize buffer with persistent storage.
     *
     *  If the buffer is backed by a file of some sort, then synchronize the buffer contents with the file by writing to the
     *  file any buffer values that have changed. */
    virtual void sync() {}

    /** Property: Name.
     *
     *  An arbitrary string stored as part of the buffer, usually used for debugging.
     *
     *  @{ */
    const std::string& name() const { return name_; }
    void name(const std::string &s) { name_ = s; }
    /** @} */

    /** Reads data from a buffer.
     *
     *  Reads up to @p n values from this buffer beginning at the specified address and copies them to the caller-supplied
     *  argument. Returns the number of values actually copied, which may be smaller than the number requested. The output
     *  buffer is not zero-padded for short reads. Also note that the return value may overflow to zero if the entire address
     *  space is read.
     *
     *  As a special case, if @p buf is a null pointer, then no data is copied and the return value indicates the number of
     *  values that would have been copied had @p buf been non-null. */
    virtual Address read(Value *buf, Address address, Address n) const = 0;

    /** Writes data to a buffer.
     *
     *  Writes up to @p n values from @p buf into this buffer starting at the specified address.  Returns the number of values
     *  actually written, which might be smaller than @p n. The return value will be less than @p n if an error occurs, but
     *  note that the return value may overflow to zero if the entire address space is written. */
    virtual Address write(const Value *buf, Address address, Address n) = 0;

    /** Data for the buffer.
     *
     *  Returns a pointer for the buffer data.  Those subclasses that don't support this method will return the null pointer. */
    virtual const Value* data() const = 0;

    /** Property: Copy on write.
     *
     *  This is a bit stored in the buffer and is used by higher layers to implement copy-on-write.  The buffer itself does
     *  nothing with this bit.
     *
     * @{ */
    bool copyOnWrite() const { return copyOnWrite_; }
    void copyOnWrite(bool b) { copyOnWrite_ = b; }
    /** @} */
};

} // namespace
} // namespace

#endif
