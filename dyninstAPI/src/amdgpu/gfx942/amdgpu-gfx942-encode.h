/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * gfx942 glue: vendors the generated gfx942 encoding table and registers it with the arch-generic
 * packer in amdgpu-encode.hpp. Including this makes amdgpu_enc::packWordU64 work for gfx942 formats and
 * exposes AmdgpuGfx942::packWordU64 / formatBytes. This is the entire cost of adding an arch to the
 * packer: the generated table + one AMDGPU_ENC_DEFINE_TRAITS line.
 */
#ifndef AMDGPU_GFX942_ENCODE_H
#define AMDGPU_GFX942_ENCODE_H

#include "amdgpu/gfx942/amdgpu-gfx942-encoding.h"
#include "amdgpu-encode.hpp"

AMDGPU_ENC_DEFINE_TRAITS(AmdgpuGfx942)

#endif
