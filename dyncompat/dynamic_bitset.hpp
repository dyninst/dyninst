#ifndef DYNINST_DYNCOMPAT_DYNAMIC_BITSET_HPP
#define DYNINST_DYNCOMPAT_DYNAMIC_BITSET_HPP

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <limits>
#include <vector>

namespace dyncompat {

template <typename Block = unsigned long, typename Allocator = std::allocator<Block>>
class dynamic_bitset {
  static_assert(std::is_unsigned_v<Block>, "dynamic_bitset requires an unsigned block type");

public:
  using block_type = Block;
  using size_type = std::size_t;

  class reference {
  public:
    reference(dynamic_bitset& owner, size_type pos) : owner_(owner), pos_(pos) {}

    reference& operator=(bool value) {
      owner_.set(pos_, value);
      return *this;
    }

    operator bool() const { return owner_.test(pos_); }

  private:
    dynamic_bitset& owner_;
    size_type pos_;
  };

  dynamic_bitset() = default;

  explicit dynamic_bitset(size_type size, unsigned long value = 0)
      : size_(size), blocks_(block_count(size), 0) {
    for(size_type i = 0; i < size_ && value != 0; ++i, value >>= 1u) {
      if((value & 1u) != 0u) {
        set(i);
      }
    }
  }

  size_type size() const { return size_; }
  bool empty() const { return size_ == 0; }

  reference operator[](size_type pos) { return reference(*this, pos); }
  bool operator[](size_type pos) const { return test(pos); }

  dynamic_bitset& set() {
    std::fill(blocks_.begin(), blocks_.end(), all_bits_set());
    trim_unused_bits();
    return *this;
  }

  dynamic_bitset& set(size_type pos, bool value = true) {
    assert(pos < size_);
    const auto mask = block_type(1) << bit_offset(pos);
    if(value) {
      blocks_[block_index(pos)] |= mask;
    } else {
      blocks_[block_index(pos)] &= ~mask;
    }
    return *this;
  }

  dynamic_bitset& reset() {
    std::fill(blocks_.begin(), blocks_.end(), 0);
    return *this;
  }

  dynamic_bitset& reset(size_type pos) {
    return set(pos, false);
  }

  bool test(size_type pos) const {
    assert(pos < size_);
    return (blocks_[block_index(pos)] & (block_type(1) << bit_offset(pos))) != 0;
  }

  size_type count() const {
    size_type result = 0;
    for(auto block : blocks_) {
      while(block != 0) {
        result += block & 1u;
        block >>= 1u;
      }
    }
    return result;
  }

  dynamic_bitset& operator|=(const dynamic_bitset& other) {
    resize_to_match(other);
    for(size_type i = 0; i < blocks_.size(); ++i) {
      blocks_[i] |= other.blocks_[i];
    }
    trim_unused_bits();
    return *this;
  }

  dynamic_bitset& operator&=(const dynamic_bitset& other) {
    resize_to_match(other);
    for(size_type i = 0; i < blocks_.size(); ++i) {
      blocks_[i] &= other.blocks_[i];
    }
    return *this;
  }

  dynamic_bitset& operator^=(const dynamic_bitset& other) {
    resize_to_match(other);
    for(size_type i = 0; i < blocks_.size(); ++i) {
      blocks_[i] ^= other.blocks_[i];
    }
    trim_unused_bits();
    return *this;
  }

  friend dynamic_bitset operator|(dynamic_bitset lhs, const dynamic_bitset& rhs) {
    lhs |= rhs;
    return lhs;
  }

  friend dynamic_bitset operator&(dynamic_bitset lhs, const dynamic_bitset& rhs) {
    lhs &= rhs;
    return lhs;
  }

  friend dynamic_bitset operator^(dynamic_bitset lhs, const dynamic_bitset& rhs) {
    lhs ^= rhs;
    return lhs;
  }

  friend dynamic_bitset operator-(dynamic_bitset lhs, const dynamic_bitset& rhs) {
    lhs &= ~rhs;
    return lhs;
  }

  friend dynamic_bitset operator~(dynamic_bitset value) {
    for(auto& block : value.blocks_) {
      block = ~block;
    }
    value.trim_unused_bits();
    return value;
  }

  friend bool operator==(const dynamic_bitset& lhs, const dynamic_bitset& rhs) {
    return lhs.size_ == rhs.size_ && lhs.blocks_ == rhs.blocks_;
  }

  friend bool operator!=(const dynamic_bitset& lhs, const dynamic_bitset& rhs) {
    return !(lhs == rhs);
  }

  template <typename Char, typename Traits>
  friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
                                                      const dynamic_bitset& value) {
    for(size_type i = value.size_; i > 0; --i) {
      os << (value.test(i - 1) ? Char('1') : Char('0'));
    }
    return os;
  }

private:
  static constexpr size_type bits_per_block = std::numeric_limits<block_type>::digits;

  static size_type block_count(size_type size) {
    return size == 0 ? 0 : (size + bits_per_block - 1) / bits_per_block;
  }

  static constexpr block_type all_bits_set() {
    return static_cast<block_type>(~block_type(0));
  }

  static size_type block_index(size_type pos) {
    return pos / bits_per_block;
  }

  static size_type bit_offset(size_type pos) {
    return pos % bits_per_block;
  }

  void resize_to_match(const dynamic_bitset& other) {
    assert(size_ == other.size_);
    (void)other;
  }

  void trim_unused_bits() {
    if(size_ == 0 || blocks_.empty()) {
      return;
    }
    const auto used_bits = bit_offset(size_);
    if(used_bits != 0) {
      blocks_.back() &= (block_type(1) << used_bits) - 1;
    }
  }

  size_type size_{0};
  std::vector<block_type, Allocator> blocks_;
};

} // namespace dyncompat

#endif
