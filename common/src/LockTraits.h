//
// Created by bill on 5/1/17.
//

#ifndef DYNINST_LOCKTRAITS_H
#define DYNINST_LOCKTRAITS_H

#include "dthread.h"


template <typename Mutex = boost::null_mutex,
        typename ReadLock = boost::interprocess::scoped_lock<Mutex> >,
    typename WriteLock = ReadLock>
class LockTraits {
    typedef Mutex mutex_t;
    typedef WriteLock lock_t;
    typedef ReadLock readlock_t;
};


#endif //DYNINST_LOCKTRAITS_H
