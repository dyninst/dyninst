/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * Generic, table-driven AMDGPU instruction packer over the generated encoding tables
 * (amdgpu-gfx908-encoding.{h,C}, namespace AmdgpuGfx908::enc). This is the low-level primitive the
 * emit-amdgpu encoder calls instead of hand-coded bit layouts: given a format + opcode + (field,value)
 * pairs it returns the packed instruction word, honoring each field's bit ranges, valueLo and padBits.
 * Adding an arch = adding a generated table; the packer is unchanged.
 */
#ifndef AMDGPU_ENCODE_HPP
#define AMDGPU_ENCODE_HPP

#include <stdint.h>
#include <initializer_list>
#include <utility>

#include "amdgpu-gfx908-encoding.h"

namespace AmdgpuGfx908 {

// OR `value` into `word` at every FieldSpec range that targets `field` (a field may span several
// ranges). valueLo selects which slice of the value this range carries; padBits means the value's low
// padBits bits are implied (== padValue) and stored shifted right (e.g. SMEM SBASE). Sets hit if any
// range matched. On a padBits-precondition violation it leaves that range unset (a call-site bug).
inline uint64_t putField(uint64_t word, const enc::FormatSpec &f, enc::Field field, uint32_t value,
                         bool &hit) {
  for (uint8_t i = 0; i < f.nfields; ++i) {
    const enc::FieldSpec &fs = f.fields[i];
    if (fs.field != field)
      continue;
    hit = true;
    if (fs.padBits && (value & ((1u << fs.padBits) - 1u)) != fs.padValue)
      return word;
    uint32_t mask = (fs.width >= 32) ? 0xffffffffu : ((1u << fs.width) - 1u);
    uint64_t slice = (uint64_t)((value >> fs.valueLo) & mask);
    word |= slice << fs.loBit;
  }
  return word;
}

// Pack one instruction word: fixedBits | opcode@opField | each (field,value). Returns the word (low
// `enc::formats[fmt].bits` bits meaningful); the caller appends 4 or 8 bytes per the format width.
inline uint64_t packWordU64(enc::Format fmt, uint32_t opcode,
                            std::initializer_list<std::pair<enc::Field, uint32_t>> fieldVals) {
  const enc::FormatSpec &f = enc::formats[fmt];
  uint64_t word = f.fixedBits;
  bool hit;
  if (f.opField != enc::NO_FIELD) {
    hit = false;
    word = putField(word, f, f.opField, opcode, hit);
  }
  for (const auto &fv : fieldVals) {
    hit = false;
    word = putField(word, f, fv.first, fv.second, hit);
  }
  return word;
}

// Width (in bytes) of a format's instruction word.
inline int formatBytes(enc::Format fmt) { return enc::formats[fmt].bits / 8; }

} // namespace AmdgpuGfx908
#endif
