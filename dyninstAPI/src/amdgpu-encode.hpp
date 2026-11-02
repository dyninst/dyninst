/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * Generic, table-driven AMDGPU instruction packer over the generated per-gen encoding tables
 * (amdgpu-gfx<NNN>-encoding.h, namespace AmdgpuGfx<NNN>::enc). ARCH-GENERAL: the packer names no gen —
 * it is templated and DEDUCES the gen's FormatSpec/Field/Format types from its arguments (the caller
 * passes its own detail::formats array + detail::NO_FIELD). Given a format + opcode + (field,value) pairs it
 * returns the packed instruction word, honoring each field's bit ranges, valueLo and padBits.
 * Adding an arch = adding a generated table + calling the packer with that gen's detail:: symbols; the
 * packer itself is unchanged and serves every gen from one build.
 */
#ifndef AMDGPU_ENCODE_HPP
#define AMDGPU_ENCODE_HPP

#include <stdint.h>
#include <initializer_list>
#include <utility>

namespace Dyninst {
namespace amdgpu_enc {

// OR `value` into `word` at every FieldSpec range of `f` that targets `field` (a field may span several
// ranges). valueLo selects which slice of the value this range carries; padBits means the value's low
// padBits bits are implied (== padValue) and stored shifted right (e.g. SMEM SBASE). Sets hit if any
// range matched. On a padBits-precondition violation it leaves that range unset (a call-site bug).
// FormatSpecT / FieldT are the generated gen's detail::FormatSpec / detail::Field (deduced from the caller).
template <class FormatSpecT, class FieldT>
inline uint64_t putField(uint64_t word, const FormatSpecT &f, FieldT field, uint32_t value, bool &hit) {
  for (uint8_t i = 0; i < f.nfields; ++i) {
    const auto &fs = f.fields[i];
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

// Pack one instruction word: fixedBits | opcode@opField | each (field,value). The caller supplies its
// gen's `formats` table and `noField` sentinel (detail::NO_FIELD); FormatSpecT/FormatT/FieldT are deduced.
// Returns the word (low formats[fmt].bits bits meaningful); the caller appends 4 or 8 bytes per width.
template <class FormatSpecT, class FormatT, class FieldT>
inline uint64_t packWordU64(const FormatSpecT *formats, FormatT fmt, FieldT noField, uint32_t opcode,
                            std::initializer_list<std::pair<FieldT, uint32_t>> fieldVals) {
  const FormatSpecT &f = formats[fmt];
  uint64_t word = f.fixedBits;
  bool hit;
  if (f.opField != noField) {
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
template <class FormatSpecT, class FormatT>
inline int formatBytes(const FormatSpecT *formats, FormatT fmt) {
  return formats[fmt].bits / 8;
}

} // namespace amdgpu_enc
} // namespace Dyninst

#endif
