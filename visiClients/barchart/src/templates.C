// templates.C

#include <X11/Xlib.h> // XColor
#include <array2d.h>

template class dynamic1dArray<XColor *>;
template class dynamic1dArray<double>;
template class dynamic1dArray<int>;
template class dynamic1dArray<bool>;

template class dynamic2dArray<double>;
template class dynamic2dArray<int>;
template class dynamic2dArray<bool>;
