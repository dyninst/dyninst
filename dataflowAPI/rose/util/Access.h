// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_Access_H
#define Sawyer_Access_H

namespace Sawyer {
namespace Access {

/** Bits to define accessibility.
 *
 *  These constants are used throughout the AddressMap framework. */
enum Access {
    EXECUTABLE    = 0x00000001,                         /**< Execute accessibility bit. */
    WRITABLE      = 0x00000002,                         /**< Write accessibility bit. */
    READABLE      = 0x00000004,                         /**< Read accessibility bit. */
    IMMUTABLE     = 0x00000008,                         /**< Underlying buffer is immutable. E.g., mmap'd read-only. */
    RESERVED_MASK = 0x000000ff,                         /**< Accessibility bits reserved for use by the library. */
    USERDEF_MASK  = 0xffffff00                          /**< Accessibility bits available to users. */
};

} // namespace
} // namespace
#endif
