/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * Generic, table-driven AMDGPU instruction packer over the generated encoding tables
 * (amdgpu-<arch>-encoding.h, namespace Amdgpu<Arch>::enc). This is the low-level primitive the
 * emit-amdgpu encoder calls instead of hand-coded bit layouts: given a format + opcode + (field,value)
 * pairs it returns the packed instruction word, honoring each field's bit ranges, valueLo and padBits.
 *
 * The packer is arch-agnostic: putField/packWordU64/formatBytes are templated on the generated
 * FormatSpec/Format types (the generated structs are identical in shape across archs; only the enum
 * sets differ). Each arch registers its table once with AMDGPU_ENC_DEFINE_TRAITS(Amdgpu<Arch>),
 * which specializes the traits and pulls packWordU64/formatBytes into that arch's namespace so call
 * sites written as packWordU64(enc::ENC_SOP2, {{enc::SDST, ...}}) resolve unchanged. Adding an arch =
 * vendoring its generated table + one AMDGPU_ENC_DEFINE_TRAITS line; the packer itself is unchanged.
 */
#ifndef AMDGPU_ENCODE_HPP
#define AMDGPU_ENCODE_HPP

#include <stdint.h>
#include <initializer_list>
#include <utility>

// gfx908 is the production arch; its table is always available. Other archs include their own slim
// table + AMDGPU_ENC_DEFINE_TRAITS from a per-arch glue header (see amdgpu/gfx942/).
#include "amdgpu/gfx908/amdgpu-gfx908-encoding.h"

namespace amdgpu_enc {

// OR `value` into `word` at every FieldSpec range that targets `field` (a field may span several
// ranges). valueLo selects which slice of the value this range carries; padBits means the value's low
// padBits bits are implied (== padValue) and stored shifted right (e.g. SMEM SBASE). Sets hit if any
// range matched. On a padBits-precondition violation it leaves that range unset (a call-site bug).
// Templated on the generated FormatSpec/Field types so it works for any arch's table.
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

// Per-arch traits: maps a generated Format enum type to its table + sentinels. The primary template is
// left undefined; AMDGPU_ENC_DEFINE_TRAITS(Amdgpu<Arch>) provides the specialization per arch.
template <class FormatT>
struct Traits;

// Pack one instruction word: fixedBits | opcode@opField | each (field,value). Returns the word (low
// formats[fmt].bits bits meaningful); the caller appends 4 or 8 bytes per the format width. FormatT is
// deduced from the format argument, which selects the arch's table via Traits<FormatT>.
template <class FormatT>
inline uint64_t packWordU64(
    FormatT fmt, uint32_t opcode,
    std::initializer_list<std::pair<typename Traits<FormatT>::Field, uint32_t>> fieldVals) {
  typedef Traits<FormatT> T;
  const typename T::FormatSpec &f = T::format(fmt);
  uint64_t word = f.fixedBits;
  bool hit;
  if (f.opField != T::noField()) {
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
template <class FormatT>
inline int formatBytes(FormatT fmt) {
  return Traits<FormatT>::format(fmt).bits / 8;
}

} // namespace amdgpu_enc

// Register an arch's generated table with the packer and expose unqualified packWordU64/formatBytes in
// that arch's namespace. Invoke once per arch after its amdgpu-<arch>-encoding.h is included.
#define AMDGPU_ENC_DEFINE_TRAITS(NS)                                                                    \
  namespace amdgpu_enc {                                                                                \
  template <>                                                                                           \
  struct Traits<NS::enc::Format> {                                                                      \
    typedef NS::enc::Field Field;                                                                       \
    typedef NS::enc::FormatSpec FormatSpec;                                                             \
    static const FormatSpec &format(NS::enc::Format f) { return NS::enc::formats[f]; }                  \
    static NS::enc::Field noField() { return NS::enc::NO_FIELD; }                                        \
  };                                                                                                    \
  }                                                                                                     \
  namespace NS {                                                                                        \
  using amdgpu_enc::formatBytes;                                                                        \
  using amdgpu_enc::packWordU64;                                                                        \
  }

// gfx908: production arch, registered here so emit-amdgpu call sites resolve packWordU64 unqualified.
AMDGPU_ENC_DEFINE_TRAITS(AmdgpuGfx908)

#endif
