#ifndef DYNINST_DYNCOMPAT_CRC_HPP
#define DYNINST_DYNCOMPAT_CRC_HPP

#include <cstddef>
#include <cstdint>

namespace dyncompat {

class crc_32_type {
public:
  crc_32_type() { reset(); }

  void reset() { crc_ = 0xffffffffu; }

  void process_bytes(const void* data, std::size_t size) {
    const auto* bytes = static_cast<const std::uint8_t*>(data);
    for(std::size_t i = 0; i < size; ++i) {
      crc_ ^= bytes[i];
      for(unsigned bit = 0; bit < 8; ++bit) {
        const auto mask = static_cast<std::uint32_t>(-(crc_ & 1u));
        crc_ = (crc_ >> 1u) ^ (0xedb88320u & mask);
      }
    }
  }

  std::uint32_t checksum() const { return crc_ ^ 0xffffffffu; }

private:
  std::uint32_t crc_{0xffffffffu};
};

} // namespace dyncompat

#endif
