#ifndef DYNINST_DYNCOMPAT_ITERATOR_FUNCTION_OUTPUT_ITERATOR_HPP
#define DYNINST_DYNCOMPAT_ITERATOR_FUNCTION_OUTPUT_ITERATOR_HPP

#include <iterator>
#include <utility>

namespace dyncompat {

template <typename Function>
class function_output_iterator {
public:
  using iterator_category = std::output_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = void;
  using pointer = void;
  using reference = void;

  explicit function_output_iterator(Function function) : function_(std::move(function)) {}

  function_output_iterator& operator*() { return *this; }
  function_output_iterator& operator++() { return *this; }
  function_output_iterator operator++(int) { return *this; }

  template <typename T>
  function_output_iterator& operator=(T&& value) {
    function_(std::forward<T>(value));
    return *this;
  }

private:
  Function function_;
};

template <typename Function>
function_output_iterator<Function> make_function_output_iterator(Function function) {
  return function_output_iterator<Function>(std::move(function));
}

} // namespace dyncompat

#endif
