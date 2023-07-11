// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_MappedBuffer_H
#define Sawyer_MappedBuffer_H

#include "AllocatingBuffer.h"
#include "Buffer.h"
#include "Sawyer.h"

#include <string.h>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

namespace Sawyer {
namespace Container {

/** Memory mapped file.
 *
 *  This buffer points to a file that is mapped into memory by the operating system.  The API supports a common divisor for
 *  POSIX and Microsoft Windows and is therefore not all that powerful, but it does allow simple maps to be created that have a
 *  file as backing store.  See http://www.boost.org/doc/libs for more information.
 *
 *  Access modes are the following enumerated constants:
 *
 *  @li <code>boost::iostreams::mapped_file::readonly</code>: shared read-only access
 *  @li <code>boost::iostreams::mapped_file::readwrite</code>: shared read/write access
 *  @li <code>boost::iostreams::mapped_file::priv</code>: private read/write access
 *  
 *  When a file is mapped with private access changes written to the buffer are not reflected in the underlying file. */
template<class A, class T>
class MappedBuffer: public Buffer<A, T> {
    boost::iostreams::mapped_file_params params_;
    boost::iostreams::mapped_file device_;
public:
    typedef A Address;
    typedef T Value;
protected:
    MappedBuffer(const boost::iostreams::mapped_file_params &params): params_(params), device_(params) {}

public:
    /** Map a file according to boost parameters.
     *
     *  The parameters describe which file (by name) and part thereof should be mapped into memory. */
    static typename Buffer<A, T>::Ptr instance(const boost::iostreams::mapped_file_params &params) {
        return typename Buffer<A, T>::Ptr(new MappedBuffer(params));
    }

    /** Map a file by name.
     *
     *  The specified file, which must already exist, is mapped into memory and pointed to by this new buffer. */
    static typename Buffer<A, T>::Ptr
    instance(const std::string &path,
             boost::iostreams::mapped_file::mapmode mode=boost::iostreams::mapped_file::readonly,
             boost::intmax_t offset=0,
             boost::iostreams::mapped_file::size_type length=boost::iostreams::mapped_file::max_length) {
        boost::iostreams::mapped_file_params params(path);
        params.flags = mode;
        params.length = length;
        params.offset = offset;
        return typename Buffer<A, T>::Ptr(new MappedBuffer(params));
    }

    // It doesn't make sense to copy a memory-mapped buffer since the point of copying is to result in two independent buffers
    // pointing to non-shared data. If a shared, writable, memory-mapped buffer is backed by a file and we make a new copy also
    // backed by the file, then changing one buffer would change the other.  Therefore, we allocate new memory that will hold a
    // snapshot of the source buffer.
    typename Buffer<A, T>::Ptr copy() const /*override*/ {
        typename Buffer<A, T>::Ptr newBuffer = AllocatingBuffer<A, T>::instance(this->size());
        Address nWritten = newBuffer->write((const Value*)device_.data(), 0, this->size());
        if (nWritten != this->size()) {
            throw std::runtime_error("MappedBuffer::copy() failed after copying " +
                                     boost::lexical_cast<std::string>(nWritten) + " of " +
                                     boost::lexical_cast<std::string>(this->size()) +
                                     (1==this->size()?" value":" values"));
        }
        return newBuffer;
    }
    
    Address available(Address address) const /*override*/ {
        return address >= device_.size() ? Address(0) : (Address(device_.size()) - address) / sizeof(Value);
    }

    void resize(Address n) /*override*/ {
        if (n != device_.size())
            throw std::runtime_error("resizing not allowed for MappedBuffer");
    }

    Address read(Value *buf, Address address, Address n) const /*override*/ {
        Address nread = std::min(n, available(address));
        memcpy(buf, device_.const_data() + address, nread * sizeof(Value));
        return nread;
    }

    Address write(const Value *buf, Address address, Address n) /*override*/ {
        Address nwritten = std::min(n, available(address));
        memcpy(device_.data() + address, buf, nwritten * sizeof(Value));
        return nwritten;
    }

    const Value* data() const /*override*/ {
        return (Value*)device_.const_data();
    }
};

} // namespace
} // namespace
#endif
