// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_StaticBuffer_H
#define Sawyer_StaticBuffer_H

#include <string>
#include <string.h>
#include "AllocatingBuffer.h"
#include "Assert.h"
#include "Buffer.h"
#include "Sawyer.h"

namespace Sawyer {
namespace Container {

/** Points to static data.
 *
 *  This buffer object points to storage which is not owned by this object and which is therefore not deleted when this object
 *  is deleted. */
template<class A, class T>
class StaticBuffer: public Buffer<A, T> {
public:
    typedef A Address;
    typedef T Value;

private:
    Value *values_;
    Address size_;
    bool rdonly_;

protected:
    StaticBuffer(Value *values, Address size): values_(values), size_(size), rdonly_(false) {
        ASSERT_require(size==0 || values!=NULL);
    }
    StaticBuffer(const Value *values, Address size): values_(const_cast<Value*>(values)), size_(size), rdonly_(true) {
        ASSERT_require(size==0 || values!=NULL);
    }

public:
    /** Construct from caller-supplied data.
     *
     *  The caller supplies a pointer to data and the size of that data.  The new buffer object does not take ownership of the
     *  data or copy it, thus the caller-supplied data must continue to exist for as long as this buffer exists.
     *
     * @{ */
    static typename Buffer<A, T>::Ptr instance(Value *values, Address size) {
        return typename Buffer<A, T>::Ptr(new StaticBuffer(values, size));
    }
    static typename Buffer<A, T>::Ptr instance(const Value *values, Address size) {
        return typename Buffer<A, T>::Ptr(new StaticBuffer(values, size));
    }
    /** @} */

    // It doesn't make sense to exactly copy a static buffer because the point is to create a new buffer that points to data
    // that is independent of the source buffer.  Therefore we create an allocating buffer instead.
    typename Buffer<A, T>::Ptr copy() const /*override*/ {
        typename Buffer<A, T>::Ptr newBuffer = AllocatingBuffer<A, T>::instance(size_);
        Address nWritten = newBuffer->write(values_, 0, size_);
        if (nWritten != size_) {
            throw std::runtime_error("StaticBuffer::copy() failed after copying " +
                                     boost::lexical_cast<std::string>(nWritten) + " of " +
                                     boost::lexical_cast<std::string>(size_) +
                                     (1==size_?" value":" values"));
        }
        return newBuffer;
    }
    
    Address available(Address start) const /*override*/ {
        return start < size_ ? size_-start : 0;
    }

    void resize(Address newSize) /*override*/ {
        if (newSize != size_)
            throw std::runtime_error("unable to resize StaticBuffer");
    }

    Address read(Value *buf, Address address, Address n) const /*override*/ {
        n = std::min(n, available(address));
        if (buf)
            memcpy(buf, values_+address, n*sizeof(values_[0]));
        return n;
    }

    Address write(const Value *buf, Address address, Address n) /*override*/ {
        if (rdonly_)
            return 0;
        n = std::min(n, available(address));
        if (buf)
            memcpy(values_+address, buf, n*sizeof(values_[0]));
        return n;
    }

    const Value* data() const /*override*/ {
        return values_;
    }
};

} // namespace
} // namespace

#endif
