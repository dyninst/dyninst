#ifndef DYNINST_DYNCOMPAT_ITERATOR_FILTER_ITERATOR_HPP
#define DYNINST_DYNCOMPAT_ITERATOR_FILTER_ITERATOR_HPP

#include <iterator>
#include <utility>

namespace dyncompat {

template <typename Predicate, typename Iterator>
class filter_iterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = typename std::iterator_traits<Iterator>::value_type;
  using difference_type = typename std::iterator_traits<Iterator>::difference_type;
  using pointer = typename std::iterator_traits<Iterator>::pointer;
  using reference = typename std::iterator_traits<Iterator>::reference;

  filter_iterator() = default;
  filter_iterator(Predicate predicate, Iterator current, Iterator end)
      : predicate_(std::move(predicate)), current_(current), end_(end) {
    satisfy();
  }

  reference operator*() const { return *current_; }
  pointer operator->() const { return std::addressof(*current_); }

  filter_iterator& operator++() {
    ++current_;
    satisfy();
    return *this;
  }

  filter_iterator operator++(int) {
    filter_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  bool operator==(const filter_iterator& other) const { return current_ == other.current_; }
  bool operator!=(const filter_iterator& other) const { return !(*this == other); }

private:
  void satisfy() {
    while(current_ != end_ && !predicate_(*current_)) {
      ++current_;
    }
  }

  Predicate predicate_{};
  Iterator current_{};
  Iterator end_{};
};

template <typename Predicate, typename Iterator>
filter_iterator<Predicate, Iterator> make_filter_iterator(Predicate predicate, Iterator current, Iterator end) {
  return filter_iterator<Predicate, Iterator>(std::move(predicate), current, end);
}

} // namespace dyncompat

#endif
