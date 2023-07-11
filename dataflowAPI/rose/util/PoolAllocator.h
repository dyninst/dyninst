// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_PoolAllocator_H
#define Sawyer_PoolAllocator_H

#include <assert.h>
#include <ostream>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <utility>
#include <boost/version.hpp>
#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>
#include <boost/cstdint.hpp>
#include <list>
#include "Assert.h"
#include "Interval.h"
#include "IntervalMap.h"
#include "Sawyer.h"
#include "Synchronization.h"
#include <vector>

namespace Sawyer {

/** Small object allocation from memory pools.
 *
 *  This class manages allocation and deallocation of small objects from pools. This allocator has pools available for a
 *  variety of small object sizes, or falls back to the global <code>new</code> and <code>delete</code> operators for larger
 *  objects.  Each pool contains zero or more large chunks of contiguous memory from which storage for the small objects are
 *  obtained. The following template parameters control the number and sizes of pools:
 *
 *  @li @p smallestCell is the size in bytes of the smallest cells.  An "cell" is a small unit of storage and may be larger
 *      than what is requested when allocating memory for an object.  The value must be at least as large as a pointer.
 *      Memory requests that are smaller than a cell lead to internal fragmentation.
 *  @li @p sizeDelta is the difference in size in bytes between the cells of two neighboring pools.  The value must be
 *      positive. When looking for a pool that will satisfy an allocation request, the allocator chooses the pool having the
 *      smallest cells that are at least as large as the request.
 *  @li @p nPools is the total number of pools, each having a different size of cells. If pools are numbered zero through
 *      @em n then the size of cells is \f$\|cell_i\| = \|cell_0\| + i \Delta\f$
 *  @li @p chunkSize is the size of each chunk in bytes.  Chunks are the unit of allocation requested from the runtime. All
 *      chunks in the allocator are the same size regardless of the cell size, and this may lead to external
 *      fragmentation&mdash;extra space at the end of a chunk that is not large enough for a complete cell.  The chunk size
 *      should be large in relation to the largest cell size (that's the whole point of pool allocation).
 *  @li @p Sync is the syncrhonization mechanism and can be either @ref Sawyer::SingleThreadedTag or @ref
 *      Sawyer::MultiThreadedTag. A single-threaded pool requires synchronization in the caller in order to prevent concurrent
 *      calls to the allocator, but a multi-threaded pool performs the synchronization internally. Single-threaded pools are
 *      somewhat faster.
 *
 *  The @ref SynchronizedPoolAllocator and @ref UnsynchronizedPoolAllocator typedefs provide reasonable template arguments.
 *
 *  When a pool allocator is copied, only its settings are copied, not the pools.  Since containers typically copy their
 *  constructor-provided allocators, each container will have its own pools even if one provides the same pool to all the
 *  constructors.  See @ref ProxyAllocator for a way to avoid this, and to allow different containers to share the same
 *  allocator.
 *
 *  Deleting a pool allocator deletes all its pools, which deletes all the chunks, which deallocates memory that might be in
 *  use by objects allocated from this allocator.  In other words, don't destroy the allocator unless you're willing that the
 *  memory for any objects in use will suddenly be freed without even calling the destructors for those objects. */
template<size_t smallestCell, size_t sizeDelta, size_t nPools, size_t chunkSize, typename Sync>
class PoolAllocatorBase {
public:
    enum { SMALLEST_CELL = smallestCell };
    enum { SIZE_DELTA = sizeDelta };
    enum { N_POOLS = nPools };
    enum { CHUNK_SIZE = chunkSize };
    enum { N_FREE_LISTS = 32 };                          // number of free lists per pool

private:

    // Singly-linked list of cells (units of object backing store) that are not being used by the caller.
    struct FreeCell { FreeCell *next; };

    typedef Sawyer::Container::Interval<boost::uint64_t> ChunkAddressInterval;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Basic unit of allocation.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
    class Chunk {
        unsigned char data_[chunkSize];
    public:
        BOOST_STATIC_ASSERT(chunkSize >= sizeof(FreeCell));

        FreeCell* fill(size_t cellSize) {               // create a free list for this chunk
            ASSERT_require(cellSize >= sizeof(FreeCell));
            ASSERT_require(cellSize <= chunkSize);
            FreeCell *retval = NULL;
            size_t n = chunkSize / cellSize;
            for (size_t i=n; i>0; --i) {                // free list in address order is easier to debug at no extra expense
                FreeCell *cell = reinterpret_cast<FreeCell*>(data_+(i-1)*cellSize);
                cell->next = retval;
                retval = cell;
            }
            return retval;
        }

        ChunkAddressInterval extent() const {
            return ChunkAddressInterval::hull(reinterpret_cast<boost::uint64_t>(data_),
                                              reinterpret_cast<boost::uint64_t>(data_+chunkSize-1));
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Interesting info about a chunk
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
    struct ChunkInfo {
        const Chunk *chunk;
        size_t nUsed;
        ChunkInfo(): chunk(NULL), nUsed(0) {}
        ChunkInfo(const Chunk *chunk_, size_t nUsed_): chunk(chunk_), nUsed(nUsed_) {}
        bool operator==(const ChunkInfo &other) const {
            return chunk==other.chunk && nUsed==other.nUsed;
        }
    };

    typedef Sawyer::Container::IntervalMap<ChunkAddressInterval, ChunkInfo> ChunkInfoMap;

    class Pool;

    // Aquire all locks for a pool.
    class LockEverything {
        SAWYER_THREAD_TRAITS::Mutex *freeListMutexes_, &chunkMutex_;
        size_t nLocked_;
    public:
        LockEverything(SAWYER_THREAD_TRAITS::Mutex *freeListMutexes, SAWYER_THREAD_TRAITS::Mutex &chunkMutex)
            : freeListMutexes_(freeListMutexes), chunkMutex_(chunkMutex), nLocked_(0) {
            while (nLocked_ < N_FREE_LISTS) {
                freeListMutexes_[nLocked_].lock();
                ++nLocked_;
            }
            chunkMutex_.lock();
        }

        ~LockEverything() {
            while (nLocked_ > 0) {
                freeListMutexes_[nLocked_-1].unlock();
                --nLocked_;
            }
            chunkMutex_.unlock();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Pool of single-sized cells; collection of chunks
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class Pool {
        size_t cellSize_;                               // only modified immediately after construction

        // Multiple free-lists for parallelism reduces the contention on the pool. The aquire and release methods select a
        // free-list uniformly at random in order to keep the sizes of the free-lists relatively equal. There is no requirement
        // that an object allocated from one free-list be released back to the same free-list. Each free-list has its own
        // mutex. When locking multiple free-lists, the locks should be aquired in order of their indexes.
        SAWYER_THREAD_TRAITS::Mutex freeListMutexes_[N_FREE_LISTS];
        FreeCell *freeLists_[N_FREE_LISTS];

        // The chunk-list stores the memory allocated for objects.  The chunk-list is protected by a mutex. When locking
        // free-list(s) and the chunk-list, the free-list locks should be aquired first.
        mutable SAWYER_THREAD_TRAITS::Mutex chunkMutex_;
        std::list<Chunk*> chunks_;

    private:
        Pool(const Pool&);                              // nonsense

    public:
        Pool(): cellSize_(0), freeLists_{} {}

        void init(size_t cellSize) {
            assert(cellSize_ == 0);
            assert(cellSize > 0);
            cellSize_ = cellSize;
	    for (int i = 0; i < N_FREE_LISTS; ++i) {
	        freeLists_[i] = NULL;
	    }
        }
        
    public:
        ~Pool() {
            for (typename std::list<Chunk*>::iterator ci=chunks_.begin(); ci!=chunks_.end(); ++ci)
                delete *ci;
        }

DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_UNUSED_VARIABLE

        bool isEmpty() const {
            SAWYER_THREAD_TRAITS::LockGuard lock(chunkMutex_);
            return chunks_.empty();
        }

        // Obtains the cell at the front of the free list, allocating more space if necessary.
        void* aquire() {                                // hot
            const size_t freeListIdx = fastRandomIndex(N_FREE_LISTS);
            SAWYER_THREAD_TRAITS::LockGuard lock(freeListMutexes_[freeListIdx]);
            if (!freeLists_[freeListIdx]) {
                Chunk *chunk = new Chunk;
                freeLists_[freeListIdx] = chunk->fill(cellSize_);
                SAWYER_THREAD_TRAITS::LockGuard chunkLock(chunkMutex_);
                chunks_.push_back(chunk);
            }
            ASSERT_not_null(freeLists_[freeListIdx]);
            FreeCell *cell = freeLists_[freeListIdx];
            freeLists_[freeListIdx] = freeLists_[freeListIdx]->next;
            cell->next = NULL;                          // optional
            return cell;
        }

        // Returns an cell to the front of the free list.
        void release(void *cell) {                      // hot
            const size_t freeListIdx = fastRandomIndex(N_FREE_LISTS);
            SAWYER_THREAD_TRAITS::LockGuard lock(freeListMutexes_[freeListIdx]);
            ASSERT_not_null(cell);
            FreeCell *freedCell = reinterpret_cast<FreeCell*>(cell);
            freedCell->next = freeLists_[freeListIdx];
            freeLists_[freeListIdx] = freedCell;
        }

DYNINST_DIAGNOSTIC_END_SUPPRESS_UNUSED_VARIABLE

        // Information about each chunk.
        ChunkInfoMap chunkInfoNS() const {
            ChunkInfoMap map;
            BOOST_FOREACH (const Chunk* chunk, chunks_)
                map.insert(chunk->extent(), ChunkInfo(chunk, chunkSize / cellSize_));
            for (size_t freeListIdx = 0; freeListIdx < N_FREE_LISTS; ++freeListIdx) {
                for (FreeCell *cell=freeLists_[freeListIdx]; cell!=NULL; cell=cell->next) {
                    typename ChunkInfoMap::ValueIterator found = map.find(reinterpret_cast<boost::uint64_t>(cell));
                    ASSERT_require2(found!=map.values().end(), "each freelist item must be some chunk cell");
                    ASSERT_require2(found->nUsed > 0, "freelist must be consistent with chunk capacities");
                    --found->nUsed;
                }
            }
            return map;
        }

        // Reserve objects to satisfy future allocation requests.
        void reserve(size_t nObjects) {
            LockEverything guard(freeListMutexes_, chunkMutex_);
            size_t nFree = 0;
            for (size_t freeListIdx = 0; freeListIdx < N_FREE_LISTS; ++freeListIdx) {
                for (FreeCell *cell = freeLists_[freeListIdx]; cell != NULL; cell = cell->next) {
                    ++nFree;
                    if (nFree >= nObjects)
                        return;
                }
            }

            size_t freeListIdx = fastRandomIndex(N_FREE_LISTS);
            size_t nNeeded = nObjects - nFree;
            const size_t cellsPerChunk = chunkSize / cellSize_;
            while (1) {
                // Allocate a new chunk of object cells
                Chunk *chunk = new Chunk;
                FreeCell *newCells = chunk->fill(cellSize_);
                chunks_.push_back(chunk);

                // Insert the new object cells into the free lists in round-robin order
                while (newCells) {
                    FreeCell *cell = newCells;
                    newCells = cell->next;
                    cell->next = freeLists_[freeListIdx];
                    freeLists_[freeListIdx] = cell;
                    if (++freeListIdx >= N_FREE_LISTS)
                        freeListIdx = 0;
                }

                if (nNeeded < cellsPerChunk)
                    return;
            }
        }
        
        // Free unused chunks
        void vacuum() {
            // We must aquire all the free list-locks plus the chunks-lock before we call chunkInfoNS. Free-list locks must be
            // aquired before the chunk-list lock.
            LockEverything guard(freeListMutexes_, chunkMutex_);
            ChunkInfoMap map = chunkInfoNS();

            // Scan the free lists, creating new free lists in the process.  For any cell on an old free list, if the cell
            // belongs to a chunk that we're keeping, then copy the cell to a new free list.  The cells are copied round-robin
            // to the new free lists so that the lists stay balanced.
            FreeCell *newFreeLists[N_FREE_LISTS];
            memset(newFreeLists, 0, sizeof newFreeLists);
            size_t newFreeListIdx = 0;
            for (size_t oldFreeListIdx=0; oldFreeListIdx<N_FREE_LISTS; ++oldFreeListIdx) {
                FreeCell *next = NULL;
                for (FreeCell *cell = freeLists_[oldFreeListIdx]; cell != NULL; cell = next) {
                    next = cell->next;
                    boost::uint64_t cellAddr = reinterpret_cast<boost::uint64_t>(cell);
                    if (map[cellAddr].nUsed != 0) {
                        // Keep this cell by round-robin inserting it into a new free list.
                        cell->next = newFreeLists[newFreeListIdx];
                        newFreeLists[newFreeListIdx] = cell;
                        if (++newFreeListIdx >= N_FREE_LISTS)
                            newFreeListIdx = 0;
                    }
                }
            }
            memcpy(freeLists_, newFreeLists, sizeof newFreeLists);

            // Delete chunks that have no used cells.
            typename std::list<Chunk*>::iterator iter = chunks_.begin();
            while (iter!=chunks_.end()) {
                Chunk *chunk = *iter;
                boost::uint64_t cellAddr = chunk->extent().least(); // any cell will do
                if (map[cellAddr].nUsed == 0) {
                    delete chunk;
                    iter = chunks_.erase(iter);
                } else {
                    ++iter;
                }
            }
        }

        size_t showInfo(std::ostream &out) const {
            ChunkInfoMap cim;
            {
                LockEverything guard(const_cast<SAWYER_THREAD_TRAITS::Mutex*>(freeListMutexes_), chunkMutex_);
                cim = chunkInfoNS();
            }

            const size_t nCells = chunkSize / cellSize_;
            size_t totalUsed=0;
            BOOST_FOREACH (const ChunkInfo &info, cim.values()) {
                out <<"  chunk " <<info.chunk <<"\t" <<info.nUsed <<"/" <<nCells <<"\t= " <<100.0*info.nUsed/nCells <<"%\n";
                totalUsed += info.nUsed;
            }
            return totalUsed;
        }

        std::pair<size_t, size_t> nAllocated() const {
            ChunkInfoMap cim;
            {
                LockEverything guard(const_cast<SAWYER_THREAD_TRAITS::Mutex*>(freeListMutexes_), chunkMutex_);
                cim = chunkInfoNS();
            }

            const size_t nCells = chunkSize / cellSize_;
            size_t nReserved = nCells * cim.nIntervals();
            size_t nAllocated = 0;
            BOOST_FOREACH (const ChunkInfo &info, cim.values())
                nAllocated += info.nUsed;
            return std::make_pair(nAllocated, nReserved);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Private data members and methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
    Pool *pools_;                                       // modified only in constructors and destructor

    // Called only by constructors
    void init() {
        pools_ = new Pool[nPools];
        for (size_t i=0; i<nPools; ++i)
            pools_[i].init(cellSize(i));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Construction
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Default constructor. */
    PoolAllocatorBase() {
        init();
    }

    /** Copy constructor.
     *
     *  Copying an allocator does not copy its pools, but rather creates a new allocator that is empty but has the same
     *  settings as the source allocator. */
    PoolAllocatorBase(const PoolAllocatorBase&) {
        init();
    }

private:
    // Assignment is nonsensical
    PoolAllocatorBase& operator=(const PoolAllocatorBase&);

public:
    /** Destructor.
     *
     *  Destroying a pool allocator destroys all its pools, which means that any objects that use storage managed by this pool
     *  will have their storage deleted. */
    virtual ~PoolAllocatorBase() {
        delete[] pools_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Public methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Pool number for a request size.
     *
     *  The return value is a pool number assuming that an infinite number of pools is available.  In practice, if the return
     *  value is greater than or equal to the @p nPools template argument then allocation is handled by the global "new"
     *  operator rather than this allocator. */
    static size_t poolNumber(size_t size) {
        return size <= smallestCell ? 0 : (size - smallestCell + sizeDelta - 1) / sizeDelta;
    }

    /** Size of each cell for a given pool.
     *
     *  Returns the number of bytes per cell for the given pool. */
    static size_t cellSize(size_t poolNumber) {
        return smallestCell + poolNumber * sizeDelta;
    }

    /** Number of cells per chunk.
     *
     *  Returns the number of cells contained in each chunk of the specified pool. */
    static size_t nCells(size_t poolNumber) {
        return chunkSize / cellSize(poolNumber);
    }

    /** Allocate one object of specified size.
     *
     *  Allocates one cell from an allocation pool, using the pool with the smallest-sized cells that are large enough to
     *  satisfy the request.  If the request is larger than that available from any pool then the global "new" operator is
     *  used.
     *
     *  The requested size must be positive.
     *
     *  Thread safety: This method is thread safe.
     *
     *  @sa DefaultAllocator::allocate */
    void *allocate(size_t size) {                       // hot
        ASSERT_require(size>0);
        size_t pn = poolNumber(size);
        return pn < nPools ? pools_[pn].aquire() : ::operator new(size);
    }

    /** Reserve a certain number of objects in the pool.
     *
     *  The pool for the specified object size has its storage increased if necessary so that it is prepared to allocate the
     *  indicated additional number of objects (beyond the number of objects already allocated).  I.e., upon return from this
     *  call, the free lists will contain in total, at least the specified number of objects. The reserved objects are divided
     *  equally between all free lists, and since allocations randomly select free lists from which to satisfy requests, one
     *  should reserve slightly more than what will be needed. Reserving storage is entirely optional. */
    void reserve(size_t objectSize, size_t nObjects) {
        ASSERT_require(objectSize > 0);
        size_t pn = poolNumber(nObjects);
        if (pn >= nPools)
            return;
        pools_[pn].reserve(nObjects);
    }
    
    /** Number of objects allocated and reserved.
     *
     *  Returns a pair containing the number of objects currently allocated in the pool, and the number of objects that the
     *  pool can hold (including those that are allocated) before the pool must request more memory from the system.
     *
     *  Thread safety: This method is thread-safe. Of course, for a heavily contested pool the results are probably outdated by
     *  time they're returned to the caller */
    std::pair<size_t, size_t> nAllocated() const {
        size_t nAllocated = 0, nReserved = 0;
        for (size_t pn=0; pn<nPools; ++pn) {
            std::pair<size_t, size_t> pp = pools_[pn].nAllocated();
            nAllocated += pp.first;
            nReserved += pp.second;
        }
        return std::make_pair(nAllocated, nReserved);
    }
    
    /** Deallocate an object of specified size.
     *
     *  The @p addr must be an object address that was previously returned by the @ref allocate method and which hasn't been
     *  deallocated in the interim.  The @p size must be the same as the argument passed to the @ref allocate call that
     *  returned this address.
     *
     *  Thread safety: This method is thread-safe.
     *
     *  @sa DefaultAllocator::deallocate */
    void deallocate(void *addr, size_t size) {          // hot
        if (addr) {
            ASSERT_require(size>0);
            size_t pn = poolNumber(size);
            if (pn < nPools) {
                pools_[pn].release(addr);
            } else {
                ::operator delete(addr);
            }
        }
    }

    /** Delete unused chunks.
     *
     *  A pool allocator is optimized for the utmost performance when allocating and deallocating small objects, and therefore
     *  does minimal bookkeeping and does not free chunks.  This method traverses the free lists to discover which chunks have
     *  no cells in use, removes those cells from the free list, and frees the chunk.
     *
     *  Thread safety: This method is thread-safe. */
    void vacuum() {
        for (size_t pn=0; pn<nPools; ++pn)
            pools_[pn].vacuum();
    }

    /** Print pool allocation information.
     *
     *  Prints some interesting information about each chunk of each pool. The output will be multiple lines.
     *
     *  Thread safety: This method is thread-safe. */
    void showInfo(std::ostream &out) const {
        for (size_t pn=0; pn<nPools; ++pn) {
            if (!pools_[pn].isEmpty()) {
                out <<"  pool #" <<pn <<"; cellSize = " <<cellSize(pn) <<" bytes:\n";
                size_t nUsed = pools_[pn].showInfo(out);
                out <<"    total objects in use: " <<nUsed <<"\n";
            }
        }
    }
};

/** Small object allocation from memory pools.
 *
 *  Thread safety:  This allocator is not thread safe; the caller must synchronize to prevent concurrent calls.
 *
 *  See @ref PoolAllocatorBase for details. */
typedef PoolAllocatorBase<sizeof(void*), 4, 32, 40960, SingleThreadedTag> UnsynchronizedPoolAllocator;

/** Small object allocation from memory pools.
 *
 *  Thread safety: This allocator is thread safe.
 *
 *  See @ref PoolAllocatorBase for details. */
typedef PoolAllocatorBase<sizeof(void*), 4, 32, 40960, MultiThreadedTag> SynchronizedPoolAllocator;

} // namespace
#endif
