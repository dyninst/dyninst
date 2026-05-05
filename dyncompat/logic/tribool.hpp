#ifndef DYNINST_DYNCOMPAT_LOGIC_TRIBOOL_HPP
#define DYNINST_DYNCOMPAT_LOGIC_TRIBOOL_HPP

namespace dyncompat {

class tribool;

struct indeterminate_keyword_t {
  tribool operator()() const;
  bool operator()(tribool value) const;
};

class tribool {
public:
  enum class value_t { false_value, true_value, indeterminate_value };

  tribool() : value_(value_t::false_value) {}
  tribool(bool value) : value_(value ? value_t::true_value : value_t::false_value) {}
  tribool(indeterminate_keyword_t) : value_(value_t::indeterminate_value) {}

  explicit operator bool() const { return value_ == value_t::true_value; }

  friend tribool operator!(tribool value) {
    if(value.value_ == value_t::indeterminate_value) {
      return tribool(indeterminate_keyword_t{});
    }
    return tribool(value.value_ == value_t::false_value);
  }

private:
  friend struct indeterminate_keyword_t;
  value_t value_;
};

inline tribool indeterminate_keyword_t::operator()() const {
  return tribool(*this);
}

inline bool indeterminate_keyword_t::operator()(tribool value) const {
  return value.value_ == tribool::value_t::indeterminate_value;
}

inline constexpr indeterminate_keyword_t indeterminate{};

} // namespace dyncompat

#endif
