#if !defined(POOL_ALLOCATORS_H)
#define POOL_ALLOCATORS_H


#include <boost/pool/pool_alloc.hpp>
#include <set>
#include <vector>

template <typename T>
struct unlocked_fast_alloc
{
  typedef boost::fast_pool_allocator<T, boost::default_user_allocator_new_delete, boost::details::pool::null_mutex> type;
};

template <typename T>
struct unlocked_pool_alloc
{
  typedef boost::pool_allocator<T, boost::default_user_allocator_new_delete, boost::details::pool::null_mutex> type;
};

template <typename T>
struct pooled_set
{
  typedef std::set<T, typename std::less<T>, typename unlocked_fast_alloc<T>::type > type;
};


template <typename T>
struct pooled_vector
{
  typedef std::vector<T, typename unlocked_pool_alloc<T>::type > type;
};




#endif //!defined(POOL_ALLOCATORS_H)
