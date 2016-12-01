#include "rangemap.h"
#include <cmath>

#ifdef _MSC_VER
    #include <float.h>
    #define isnan(x) _isnan(x)
    #define INFINITY (DBL_MAX+DBL_MAX)
    #define NAN (INFINITY-INFINITY)
#else
    #define isnan(x) std::isnan(x)
#endif

/******************************************************************************************************************************
 *                                      Specializations for Range<double>
 ******************************************************************************************************************************/

template<>
Range<double>::Range(): r_first(0), r_last(NAN) {}

template<>
bool
Range<double>::empty() const {
    return isnan(r_last);
}

template<>
void
Range<double>::clear() {
    r_last = NAN;
}

template<>
// DQ (9/3/2015): Intel v14 compiler warns that use of "const" is meaningless.
// I think this is correct since this is being returned by value.
// const double
double
Range<double>::relaxed_first() const {
    return r_first;
}

template<>
double
Range<double>::size() const {
    return empty() ? 0 : r_last-r_first;
}

template<>
void
Range<double>::resize(const double &new_size) {
    assert(!empty());
    if (new_size<0) {
        clear();
    } else {
        r_last = r_first + new_size;
    }
}

template<>
void
Range<double>::relaxed_resize(const double &new_size) {
    if (new_size<0) {
        clear();
    } else {
        r_last = r_first + new_size;
    }
}

template<>
Range<double>::Pair
Range<double>::split_range_at(const double &at) const {
    assert(!empty());
    assert(at>=first() && at<=last());
    Range<double> left = Range<double>::inin(first(), at);
    Range<double> right = Range<double>::inin(at, last());
    return std::make_pair(left, right);
}

template<>
double
Range<double>::minimum() {
    return -INFINITY;
}

template<>
double
Range<double>::maximum() {
    return INFINITY;
}


/******************************************************************************************************************************
 *                                      Specializations for Range<float>
 ******************************************************************************************************************************/

template<>
Range<float>::Range(): r_first(0), r_last(NAN) {}

template<>
bool
Range<float>::empty() const {
    return isnan(r_last);
}

template<>
void
Range<float>::clear() {
    r_last = NAN;
}

template<>
// DQ (9/3/2015): Intel v14 compiler warns that use of "const" is meaningless.
// I think this is correct since this is being returned by value.
// const float
float
Range<float>::relaxed_first() const {
    return r_first;
}

template<>
float
Range<float>::size() const {
    return empty() ? 0 : r_last-r_first;
}

template<>
void
Range<float>::resize(const float &new_size) {
    assert(!empty());
    if (new_size<0) {
        clear();
    } else {
        r_last = r_first + new_size;
    }
}

template<>
void
Range<float>::relaxed_resize(const float &new_size) {
    if (new_size<0) {
        clear();
    } else {
        r_last = r_first + new_size;
    }
}

template<>
Range<float>::Pair
Range<float>::split_range_at(const float &at) const {
    assert(!empty());
    assert(at>=first() && at<=last());
    Range<float> left = Range<float>::inin(first(), at);
    Range<float> right = Range<float>::inin(at, last());
    return std::make_pair(left, right);
}

template<>
float
Range<float>::minimum() {
    return -INFINITY;
}

template<>
float
Range<float>::maximum() {
    return INFINITY;
}
