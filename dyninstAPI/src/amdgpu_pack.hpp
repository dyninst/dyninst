/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * Generic, table-driven AMDGPU instruction packer over the AMDGEN-generated encoding header
 * (amdgpu_<arch>_encoding.h). This is the low-level primitive the emit-amdgpu encoder calls instead
 * of hand-coded bit layouts: given a format + opcode + (Field,value) pairs it returns the packed
 * instruction word, honoring each field's bit ranges, valueLo, and padBits. Adding an arch = adding a
 * generated header; the packer is unchanged. See docs/ENCODER_GEN.md.
 */
#ifndef AMDGPU_PACK_HPP
#define AMDGPU_PACK_HPP

#include <stdint.h>
#include <initializer_list>
#include <utility>

#include "amdgpu_gfx908_encoding.h"

namespace amdgpu_pack {

namespace E = amdgpu_gfx908_enc;

// OR `value` into `word` at every FieldSpec range that targets `field` (a field may span several
// ranges). valueLo selects which slice of the value this range carries; padBits means the value's low
// padBits bits are implied (== padValue) and stored shifted right (e.g. SMEM SBASE). Sets hit if any
// range matched. On a padBits-precondition violation it leaves that range unset (a call-site bug).
inline uint64_t putField(uint64_t word, const E::FormatSpec &f, E::Field field, uint32_t value,
                         bool &hit) {
  for (uint8_t i = 0; i < f.nfields; ++i) {
    const E::FieldSpec &fs = f.fields[i];
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
// `formats[fmt].bits` bits meaningful); the caller appends 4 or 8 bytes per the format width. A field
// not present in the format is a call-site error (asserted in debug via the returned hit tracking).
inline uint64_t packWordU64(E::Format fmt, uint32_t opcode,
                            std::initializer_list<std::pair<E::Field, uint32_t>> fieldVals) {
  const E::FormatSpec &f = E::formats[fmt];
  uint64_t word = f.fixedBits;
  bool hit;
  if (f.opField != E::NO_FIELD) {
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
inline int formatBytes(E::Format fmt) { return E::formats[fmt].bits / 8; }

} // namespace amdgpu_pack
#endif
