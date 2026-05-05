#ifndef DYNINST_DYNCOMPAT_RANGE_ITERATOR_RANGE_HPP
#define DYNINST_DYNCOMPAT_RANGE_ITERATOR_RANGE_HPP

namespace dyncompat {

template <typename Iterator>
class iterator_range {
public:
  using iterator = Iterator;
  using const_iterator = Iterator;

  iterator_range() = default;
  iterator_range(Iterator first, Iterator last) : first_(first), last_(last) {}

  Iterator begin() const { return first_; }
  Iterator end() const { return last_; }
  bool empty() const { return first_ == last_; }

private:
  Iterator first_{};
  Iterator last_{};
};

template <typename Iterator>
iterator_range<Iterator> make_iterator_range(Iterator first, Iterator last) {
  return iterator_range<Iterator>(first, last);
}

} // namespace dyncompat

#endif
