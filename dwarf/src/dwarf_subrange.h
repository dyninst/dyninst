#include "util.h"
#include <boost/optional.hpp>
#include <dwarf.h>
#include <elfutils/libdw.h>

namespace Dyninst {
namespace DwarfDyninst {

struct dwarf_error {};

template <typename T> struct dwarf_result {
  boost::optional<T> value;
  bool error = false;
  dwarf_result() = default;
  dwarf_result(T &&t) : value{std::move(t)} {}
  dwarf_result(dwarf_error) : error{true} {}
  explicit operator bool() const { return error; }
};

DYNDWARF_EXPORT dwarf_result<long> dwarf_subrange_upper_bound(Dwarf_Die *die);
DYNDWARF_EXPORT dwarf_result<long> dwarf_subrange_lower_bound(Dwarf_Die *die);
DYNDWARF_EXPORT dwarf_result<long>
dwarf_subrange_length_from_enum(Dwarf_Die *die);

} // namespace DwarfDyninst
} // namespace Dyninst
