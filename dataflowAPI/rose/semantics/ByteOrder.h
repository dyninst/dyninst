#ifndef ROSE_ByteOrder_H
#define ROSE_ByteOrder_H

#include <stddef.h>
#include <stdint.h>
#include "../typedefs.h"

namespace ByteOrder {

// Caution: the symbols LITTLE_ENDIAN and BIG_ENDIAN are already defined on some systems, so we use other names in ROSE.
enum Endianness {
    ORDER_UNSPECIFIED=0,                /**< Endianness is unspecified and unknown. */
    ORDER_LSB,                          /**< Least significant byte first, i.e., little-endian. */
    ORDER_MSB                           /**< Most significant byte first, i.e., big-endian. */
};

/** Byte order of host machine. */
Endianness host_order();

/** Convert data from one byte order to another in place. */
void convert(void *bytes_, size_t nbytes, Endianness from, Endianness to);

/** Reverse bytes.
 * @{ */
int8_t swap_bytes(int8_t n);
uint8_t swap_bytes(uint8_t n);
int16_t swap_bytes(int16_t n);
uint16_t swap_bytes(uint16_t n);
int32_t swap_bytes(int32_t n);
uint32_t swap_bytes(uint32_t n);
int64_t swap_bytes(int64_t n);
uint64_t swap_bytes(uint64_t n);
/** @} */

/** Convert a little-endian integer to host order.
 * @{ */
int8_t le_to_host(int8_t n);
uint8_t le_to_host(uint8_t n);
int16_t le_to_host(int16_t n);
uint16_t le_to_host(uint16_t n);
int32_t le_to_host(int32_t n);
uint32_t le_to_host(uint32_t n);
int64_t le_to_host(int64_t n);
uint64_t le_to_host(uint64_t n);
/** @} */

/** Convert host order to little-endian.
 * @{ */
void host_to_le(unsigned h, uint8_t *n);
void host_to_le(unsigned h, uint16_t *n);
void host_to_le(unsigned h, uint32_t *n);
void host_to_le(rose_addr_t h, uint64_t *n);
void host_to_le(rose_rva_t h, uint32_t *n);
void host_to_le(rose_rva_t h, uint64_t *n);
void host_to_le(int h, int8_t *n);
void host_to_le(int h, int16_t *n);
void host_to_le(int h, int32_t *n);
void host_to_le(int64_t h, int64_t *n);
/** @} */

/** Convert a bit-endian integer to host order.
 * @{ */
uint8_t be_to_host(uint8_t n);
uint16_t be_to_host(uint16_t n);
uint32_t be_to_host(uint32_t n);
uint64_t be_to_host(uint64_t n);
int8_t be_to_host(int8_t n);
int16_t be_to_host(int16_t n);
int32_t be_to_host(int32_t n);
int64_t be_to_host(int64_t n);
/** @} */

/** Convert host order to big endian.
 * @{ */
void host_to_be(unsigned h, uint8_t *n);
void host_to_be(unsigned h, uint16_t *n);
void host_to_be(unsigned h, uint32_t *n);
void host_to_be(rose_addr_t h, uint64_t *n);
void host_to_be(rose_rva_t h, uint32_t *n);
void host_to_be(rose_rva_t h, uint64_t *n);
void host_to_be(int h, int8_t *n);
void host_to_be(int h, int16_t *n);
void host_to_be(int h, int32_t *n);
void host_to_be(int64_t h, int64_t *n);
/** @} */

/** Convert call-specified order to host order.
 * @{ */
uint8_t disk_to_host(Endianness sex, uint8_t n);
uint16_t disk_to_host(Endianness sex, uint16_t n);
uint32_t disk_to_host(Endianness sex, uint32_t n);
uint64_t disk_to_host(Endianness sex, uint64_t n);
int8_t disk_to_host(Endianness sex, int8_t n);
int16_t disk_to_host(Endianness sex, int16_t n);
int32_t disk_to_host(Endianness sex, int32_t n);
int64_t disk_to_host(Endianness sex, int64_t n);

/** Convert host order to caller-specified order. */
void host_to_disk(Endianness sex, unsigned h, uint8_t *np);
void host_to_disk(Endianness sex, unsigned h, uint16_t *np);
void host_to_disk(Endianness sex, unsigned h, uint32_t *np);
void host_to_disk(Endianness sex, rose_addr_t h, uint64_t *np);
void host_to_disk(Endianness sex, rose_rva_t h, uint64_t *np);
void host_to_disk(Endianness sex, int h, int8_t *np);
void host_to_disk(Endianness sex, int h, int16_t *np);
void host_to_disk(Endianness sex, int h, int32_t *np);
void host_to_disk(Endianness sex, int64_t h, int64_t *np);
/** @} */

} // namespace
#endif
