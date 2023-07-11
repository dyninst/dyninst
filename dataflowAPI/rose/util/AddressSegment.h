// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_AddressSegment_H
#define Sawyer_AddressSegment_H

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <boost/cstdint.hpp>
#include "Access.h"
#include "AllocatingBuffer.h"
#include "MappedBuffer.h"
#include "NullBuffer.h"
#include "StaticBuffer.h"
#include "Sawyer.h"

namespace Sawyer {
namespace Container {

/** A homogeneous interval of an address space.
 *
 *  A segment imparts access permissions and other address-map related characteristics to a buffer. The segment represents one
 *  contiguous, homogeneous interval of an address space and thus points to one buffer that implements the value-storage for
 *  that part of the address space.
 *
 *  A segment doesn't store information about where it's mapped in an address space since that's the responsibility of the map
 *  containing the segment.  A segment points to a buffer which might not be the same size as what is entered in the map;
 *  buffers that are longer than the mapped interval of address space have data which is not accessible (at least not through
 *  that segment), and buffers that are shorter may return short data from read and write operations. */
template<class A = size_t, class T = boost::uint8_t>
class AddressSegment {
    typename Buffer<A, T>::Ptr buffer_;                 // reference counted buffer
    A offset_;                                          // initial offset into buffer
    unsigned accessibility_;                            // uninterpreted bit mask
    std::string name_;                                  // for debugging

public:
    typedef A Address;                                  /**< Address types expected to be used by the underlying buffer. */
    typedef T Value;                                    /**< Type of values stored by the underlying buffer. */

    //-------------------------------------------------------------------------------------------------------------------
    // Constructors
    //-------------------------------------------------------------------------------------------------------------------
public:

    /** Default constructor.
     *
     *  Constructs a segment that does not point to any buffer.  This is mainly to fulfill the requirement that values in an
     *  IntervalMap are default constructable. */
    AddressSegment(): offset_(0), accessibility_(0) {}

    /** Copy constructor.
     *
     *  Creates a new segment that's an exact copy of @p other.  Both segments will point to the same underlying buffer, which
     *  is reference counted. */
    AddressSegment(const AddressSegment &other)
        : buffer_(other.buffer_), offset_(other.offset_), accessibility_(other.accessibility_), name_(other.name_) {}

    /** Construct a segment with buffer.
     *
     *  This is the usual way that segments are created: by specifying a buffer and some access permissions. */
    explicit AddressSegment(const typename Buffer<Address, Value>::Ptr &buffer, Address offset=0, unsigned accessBits=0,
                            const std::string &name="")
        : buffer_(buffer), offset_(offset), accessibility_(accessBits), name_(name) {}

    //-------------------------------------------------------------------------------------------------------------------
    // The following static methods are convenience wrappers around various buffer types.
    //-------------------------------------------------------------------------------------------------------------------
public:

    /** Create a segment that points to no data.
     *
     *  Creates a segment of the specified size that points to a NullBuffer. This creates a segment which returns default
     *  constructed values when read, which fails when written.  Such a segment is appropriate and efficient for mapping very
     *  large areas of an address space. */
    static AddressSegment nullInstance(Address size, unsigned accessBits=0, const std::string &name="") {
        return AddressSegment(NullBuffer<A, T>::instance(size), 0, accessBits, name);
    }

    /** Create a segment with no backing store.
     *
     *  Creates a segment by allocating default-constructed values. Writes to this segment will update the underlying buffer,
     *  but the buffer is only stored in memory and not attached to any type of file or permanent storage. */
    static AddressSegment anonymousInstance(Address size, unsigned accessBits=0, const std::string &name="") {
        return AddressSegment(AllocatingBuffer<A, T>::instance(size), 0, accessBits, name);
    }

    /** Create a segment that points to a static buffer.
     *
     *  Ownership of the buffer is not transferred to the segment.
     *
     * @{ */
    static AddressSegment staticInstance(Value *buffer, Address size, unsigned accessBits=0, const std::string &name="") {
        return AddressSegment(StaticBuffer<A, T>::instance(buffer, size), 0, accessBits, name);
    }
    static AddressSegment staticInstance(const Value *buffer, Address size, unsigned accessBits=0, const std::string &name="") {
        return AddressSegment(StaticBuffer<A, T>::instance(buffer, size), 0, accessBits|Access::IMMUTABLE, name);
    }
    /** @} */

    /** Map a file into an address space. */
    static AddressSegment fileInstance(const std::string &fileName, unsigned accessBits=Access::READABLE,
                                       const std::string &name="") {
        boost::iostreams::mapped_file::mapmode mode = (accessBits & Access::WRITABLE)!=0 ?
                                                      boost::iostreams::mapped_file::readwrite :
                                                      boost::iostreams::mapped_file::readonly;
        return AddressSegment(MappedBuffer<A, T>::instance(fileName, mode), 0, accessBits, name);
    }

    //-------------------------------------------------------------------------------------------------------------------
    // Properties
    //-------------------------------------------------------------------------------------------------------------------
public:
    /** Property: buffer.
     *
     *  This is the @ref Buffer object that represents the values stored in this interval of the address space.  The first
     *  value in this segment, no matter where or if it's mapped in an address-mapping container, corresponds to the value
     *  stored at address zero of the buffer.
     *
     *  @{ */
    typename Buffer<A, T>::Ptr buffer() const { return buffer_; }
    AddressSegment& buffer(const typename Buffer<A, T>::Ptr &b) { buffer_ = b; return *this; }
    /** @} */

    /** Property: buffer offset.
     *
     *  The offset into the buffer corresponding to the first value of this segment.
     *
     *  @{ */
    A offset() const { return offset_; }
    AddressSegment& offset(A n) { offset_ = n; return *this; }
    /** @} */

    /** Property: access rights.
     *
     *  This property stores access rights as an unsigned integer.  It makes no interpretation of what the various bits mean
     *  since the meaning is imparted by the mapping container.
     *
     *  @{ */
    unsigned accessibility() const { return accessibility_; }
    AddressSegment& accessibility(unsigned bits) { accessibility_ = bits; return *this; }
    /** @} */

    /** Property: name.
     *
     *  Each segment may be given a name which can be used for debugging.
     *
     * @{ */
    const std::string &name() const { return name_; }
    AddressSegment& name(const std::string &s) { name_ = s; return *this; }
    /** @} */
    
    //-------------------------------------------------------------------------------------------------------------------
    // Other methods
    //-------------------------------------------------------------------------------------------------------------------
public:
    /** Determines whether this segment is accessible.
     *
     *  Returns true if all bits that are set in @p requiredAccess are also set in this segment, and none of the bits set in @p
     *  prohibitedAccess are set in this segment. */
    bool isAccessible(unsigned requiredAccess, unsigned prohibitedAccess) const {
        return requiredAccess==(accessibility_ & requiredAccess) && 0==(accessibility_ & prohibitedAccess);
    }
    
};

} // namespace
} // namespace

#endif
