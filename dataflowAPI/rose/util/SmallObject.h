// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_SmallObject_H
#define Sawyer_SmallObject_H

#include <stddef.h>
#include "PoolAllocator.h"
#include "Sawyer.h"

namespace Sawyer {

/** Small object support.
 *
 *  Small objects that inherit from this class will use a pool allocator instead of the global allocator. */
class SAWYER_EXPORT SmallObject {
#include "WarningsOff.h"
    static SynchronizedPoolAllocator allocator_;
#include "WarningsRestore.h"
public:
    /** Return the pool allocator for this class. */
    static SynchronizedPoolAllocator& poolAllocator() { return allocator_; }

    static void *operator new(size_t size) { return allocator_.allocate(size); }
    static void operator delete(void *ptr, size_t size) { allocator_.deallocate(ptr, size); }
};

} // namespace
#endif
