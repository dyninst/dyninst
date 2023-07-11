// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_SharedPtr_H
#define Sawyer_SharedPtr_H

#include <assert.h>
#include <stddef.h>
#include <cstddef>
#include <ostream>
#include "Assert.h"
#include "Optional.h"                            // FIXME[Robb Matzke 2014-08-22]: only needed for Sawyer::Nothing
#include "Sawyer.h"
#include "Synchronization.h"

namespace Sawyer {

/** Reference-counting smart pointer.
 *
 *  This class is a reference-counting pointer to an object that inherits from @ref SharedObject. Usage is similar to
 *  <code>boost::shared_ptr</code>.
 *  
 *  @sa SharedObject, @ref SharedFromThis
 *
 *  @todo Write documentation for SharedPointer. */
template<class T>
class SharedPointer {
public:
    typedef T Pointee;
private:
    Pointee *pointee_;

    static void acquireOwnership(Pointee *rawPtr);

    // Returns number of owners remaining
    static size_t releaseOwnership(Pointee *rawPtr);

public:
    /** Constructs an empty shared pointer. */
    SharedPointer(): pointee_(NULL) {}

    /** Constructs a new pointer that shares ownership of the pointed-to object with the @p other pointer. The pointed-to
     *  object will only be deleted after both pointers are deleted.
     * @{ */
    SharedPointer(const SharedPointer &other): pointee_(other.pointee_) {
        acquireOwnership(pointee_);
    }
    template<class Y>
    SharedPointer(const SharedPointer<Y> &other): pointee_(getRawPointer(other)) {
        acquireOwnership(pointee_);
    }
    /** @} */
    
    /** Constructs a shared pointer for an object.
     *
     *  If @p obj is non-null then its reference count is incremented. It is possible to create any number of shared pointers
     *  to the same object using this constructor. The expression "delete obj" must be well formed and must not invoke
     *  undefined behavior. */
    template<class Y>
    explicit SharedPointer(Y *rawPtr): pointee_(rawPtr) {
        if (pointee_!=NULL)
            acquireOwnership(pointee_);
    }

    /** Conditionally deletes the pointed-to object.  The object is deleted when its reference count reaches zero. */
    ~SharedPointer() {
        if (0==releaseOwnership(pointee_))
            delete pointee_;
    }

    /** Assignment. This pointer is caused to point to the same object as @p other, decrementing the reference count for the
     * object originally pointed to by this pointer and incrementing the reference count for the object pointed by @p other.
     * @{ */
    SharedPointer& operator=(const SharedPointer &other) {
        return operator=<T>(other);
    }
    template<class Y>
    SharedPointer& operator=(const SharedPointer<Y> &other) {
        if (pointee_!=getRawPointer(other)) {
            if (pointee_!=NULL && 0==releaseOwnership(pointee_))
                delete pointee_;
            pointee_ = getRawPointer(other);
            acquireOwnership(pointee_);
        }
        return *this;
    }
    /** @} */

    /** Assignment.  This pointer is caused to point to nothing. */
    SharedPointer& operator=(const Sawyer::Nothing&) {
        if (pointee_!=NULL && 0==releaseOwnership(pointee_))
            delete pointee_;
        pointee_ = NULL;
        return *this;
    }

    /** Reference to the pointed-to object.  An assertion will fail if assertions are enabled and this method is invoked on an
     *  empty pointer. */
    T& operator*() const {
        ASSERT_not_null2(pointee_, "shared pointer points to no object");
        ASSERT_require(ownershipCount(pointee_)>0);
        return *pointee_;
    }

    /** Dereference pointed-to object. The pointed-to object is returned. Returns null for empty pointers. */
    T* operator->() const {
        ASSERT_not_null2(pointee_, "shared pointer points to no object");
        ASSERT_require(ownershipCount(pointee_)>0);
        return pointee_;
    }
    /** @} */

    /** Dynamic cast.
     *
     *  Casts the specified pointer to a new pointer type using dynamic_cast. */
    template<class U>
    SharedPointer<U> dynamicCast() const {
        return SharedPointer<U>(dynamic_cast<U*>(pointee_));
    }
    
    /** Comparison of two pointers.
     *
     *  Comparisons operators compare the underlying pointers to objects.
     *
     *  @{ */
    template<class U>
    bool operator==(const SharedPointer<U> &other) const {
        return pointee_ == getRawPointer(other);
    }
    template<class U>
    bool operator!=(const SharedPointer<U> &other) const {
        return pointee_ != getRawPointer(other);
    }
    template<class U>
    bool operator<(const SharedPointer<U> &other) const {
        return pointee_ < getRawPointer(other);
    }
    template<class U>
    bool operator<=(const SharedPointer<U> &other) const {
        return pointee_ <= getRawPointer(other);
    }
    template<class U>
    bool operator>(const SharedPointer<U> &other) const {
        return pointee_ > getRawPointer(other);
    }
    template<class U>
    bool operator>=(const SharedPointer<U> &other) const {
        return pointee_ >= getRawPointer(other);
    }

    /** Comparison of two pointers.
     *
     *  Compares the underlying pointer with the specified pointer.
     *
     *  @{ */
    template<class U>
    bool operator==(const U *ptr) const {
        return pointee_ == ptr;
    }
    template<class U>
    bool operator!=(const U *ptr) const {
        return pointee_ != ptr;
    }
    template<class U>
    bool operator<(const U *ptr) const {
        return pointee_ < ptr;
    }
    template<class U>
    bool operator<=(const U *ptr) const {
        return pointee_ <= ptr;
    }
    template<class U>
    bool operator>(const U *ptr) const {
        return pointee_ > ptr;
    }
    template<class U>
    bool operator>=(const U *ptr) const {
        return pointee_ >= ptr;
    }
    /** @} */

    /** Boolean complement.
     *
     *  This operator allows shared pointers to be used in situations like this:
     *
     * @code
     *  SharedPointer<Object> obj = ...;
     *  if (!obj) ...
     * @endcode */
    bool operator!() const { return pointee_==NULL; }

    /** Print a shared pointer.
     *
     *  Printing a shared pointer is the same as printing the pointee's address. */
    friend std::ostream& operator<<(std::ostream &out, const SharedPointer &ptr) {
        out <<getRawPointer(ptr);
        return out;
    }
    
    // The following trickery is to allow things like "if (x)" to work but without having an implicit
    // conversion to bool which would cause no end of other problems.
private:
    typedef void(SharedPointer::*unspecified_bool)() const;
    void this_type_does_not_support_comparisons() const {}
public:
    /** Type for Boolean context.
     *
     *  Implicit conversion to a type that can be used in a boolean context such as an <code>if</code> or <code>while</code>
     *  statement.  For instance:
     *
     *  @code
     *   SharedPointer<Object> x = ...;
     *   if (x) {
     *      //this is reached
     *   }
     *  @endcode */
    operator unspecified_bool() const {
        return pointee_ ? &SharedPointer::this_type_does_not_support_comparisons : 0;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Functions
    // These are functions because we don't want to add any pointer methods that could be confused with methods on the
    // pointee. For instance, if the pointee has an "ownershipCount" method then errors could be easily be introduced. For
    // instance, these two lines both compile but do entirely different things:
    //     object->ownershipCount()
    //     object.ownershipCount();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    /** Obtain the pointed-to object.
     *
     *  The pointed-to object is returned. Returns null for empty pointers. An idiom for getting a raw pointer for a shared
     *  pointer that's known to be non-null is to dereference the shared pointer and then take the address:
     *
     *  @code
     *   SharedPointer<MyType> ptr = ...;
     *   MyType *obj = &*ptr;
     *  @endcode */
    friend Pointee* getRawPointer(const SharedPointer &ptr) {
        return ptr.pointee_;
    }

    /** Returns the pointed-to object's reference count. Returns zero for empty pointers. */
    friend size_t ownershipCount(const SharedPointer &ptr) {
        return ptr.ownershipCount(ptr.pointee_);
    }
private:
    static size_t ownershipCount(Pointee *rawPtr);
};

/** Make pointer point to nothing.
 *
 *  Clears the pointer so it points to no object.  A cleared pointer is like a default-constructed pointer. */
template<class T>
void clear(SharedPointer<T> &ptr) {
    ptr = SharedPointer<T>();
}
    
/** Base class for reference counted objects.
 *
 *  Any reference counted object should inherit from this class, which provides a default constructor, virtual destructor, and
 *  a private reference count data member.
 *
 *  @sa SharedPointer, @ref SharedFromThis */
class SAWYER_EXPORT SharedObject {
    template<class U> friend class SharedPointer;
    mutable SAWYER_THREAD_TRAITS::Mutex mutex_;
    mutable size_t nrefs_;
public:
    /** Default constructor.  Initializes the reference count to zero. */
    SharedObject(): nrefs_(0) {}

    /** Copy constructor.
     *
     *  The reference count of the new object is set to zero since no pointers can exist yet. */
    SharedObject(const SharedObject &/*other*/): nrefs_(0) {}

    /** Assignment.
     *
     *  Assigning one object to another doesn't change the reference count or mutex of either object. */
    SharedObject& operator=(const SharedObject &/*other*/) {
        return *this;
    }

    /** Virtual destructor. Verifies that the reference count is zero. */
    virtual ~SharedObject() {
        ASSERT_require(nrefs_==0);
    }

};

/** Creates SharedPointer from this.
 *
 *  This class provides a @ref sharedFromThis method that returns a @ref SharedPointer pointing to an object of type @c T.
 *  The template parameter @c T is usually the name of the class derived from SharedFromThis. For instance, the following
 *  example declares @c MyBaseClass to be a shared object (able to be pointed to by a SharedPointer), and declares that it
 *  is possible to create a SharedPointer from a raw object pointer.
 *
 * @code
 *  class MyBaseClass: public SharedObject, public SharedFromThis<MyBaseClass> { ... };
 * @endcode
 *
 *  Some method in @c MyBaseClass might want to return such a pointer:
 *
 * @code
 *  SharedPointer<MyBaseClass> update() const {
 *      ...
 *      return sharedFromThis();
 *  }
 * @endcode
 *
 *  Subclasses in a class hierarchy need not all have the same version of @c sharedFromThis.  If the user's base class is
 *  the only one to directly derive from SharedFromThis, then any subclass calling @c sharedFromThis will return a pointer to
 *  the user's base class.  On the other hand, the various subclasses can directly inherit from SharedFromThis also in order to
 *  return pointers to objects of their type:
 *
 * @code
 *  class MyDerivedA: public MyBaseClass {
 *      // sharedFromThis() returns pointers to MyBaseClass
 *  };
 *
 *  class MyDerivedB: public MyBaseClass, public SharedFromThis<MyDerivedB> {
 *      // sharedFromThis() is overridden to return pointers to MyDerivedB
 *  }
 * @endcode */
template<class T>
class SharedFromThis {
public:
    virtual ~SharedFromThis() {}

    /** Create a shared pointer from <code>this</code>.
     *
     *  Returns a shared pointer that points to this object.  The type @c T must be derived from SharedObject.
     *
     *  @{ */
    SharedPointer<T> sharedFromThis() {
        T *derived = dynamic_cast<T*>(this);
        ASSERT_not_null(derived);
        return SharedPointer<T>(derived);
    }
    SharedPointer<const T> sharedFromThis() const {
        const T *derived = dynamic_cast<const T*>(this);
        ASSERT_not_null(derived);
        return SharedPointer<const T>(derived);
    }
    /** @} */
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Implementations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_UNUSED_VARIABLE

template<class T>
inline size_t SharedPointer<T>::ownershipCount(T *rawPtr) {
    if (rawPtr) {
        SAWYER_THREAD_TRAITS::LockGuard lock(rawPtr->SharedObject::mutex_);
        return rawPtr->SharedObject::nrefs_;
    }
    return 0;
}

template<class T>
inline void SharedPointer<T>::acquireOwnership(Pointee *rawPtr) {
    if (rawPtr!=NULL) {
        SAWYER_THREAD_TRAITS::LockGuard lock(rawPtr->SharedObject::mutex_);
        ++rawPtr->SharedObject::nrefs_;
    }
}

template<class T>
inline size_t SharedPointer<T>::releaseOwnership(Pointee *rawPtr) {
    if (rawPtr!=NULL) {
        SAWYER_THREAD_TRAITS::LockGuard lock(rawPtr->SharedObject::mutex_);
        assert(rawPtr->SharedObject::nrefs_ > 0);
        return --rawPtr->SharedObject::nrefs_;
    } else {
        return 0;
    }
}

DYNINST_DIAGNOSTIC_END_SUPPRESS_UNUSED_VARIABLE

} // namespace
#endif
