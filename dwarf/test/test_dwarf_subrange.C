/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// Regression test for the crash fixed by "Fix crash parsing Fortran DWARF
// with dynamic array bounds": gfortran encodes Fortran assumed-shape and
// allocatable array bounds as DW_FORM_exprloc location expressions rather
// than integer constants. dwarf_subrange_bounds() must treat that as an
// "unknown bound" (no value, no error), not as a decode error.
//
// FIXTURE_OBJECT_PATH (see test/CMakeLists.txt) points at an object file
// assembled from fixtures/subrange_bounds.s, which contains a hand-crafted
// DWARF4 compile unit with these DW_TAG_subrange_type children, in order:
//   1. DW_AT_upper_bound as DW_FORM_exprloc (the dynamic-bound case)
//   2. DW_AT_upper_bound as DW_FORM_udata == 9 (the ordinary case)
//   3. DW_AT_count == 5, no DW_AT_upper_bound/DW_AT_lower_bound (the
//      count-arithmetic fallback)
//   4. DW_AT_type -> a DW_TAG_base_type with DW_AT_encoding == DW_ATE_signed,
//      and DW_AT_upper_bound as DW_FORM_sdata == -1 (the signed decode path)
// A DW_TAG_base_type sibling sits between (3) and (4); it exists only as the
// DW_AT_type reference target for (4) and is skipped over via
// dwarf_siblingof().

#include "dwarf_subrange.h"

#include <gtest/gtest.h>

#include <fcntl.h>
#include <unistd.h>

#include <elfutils/libdw.h>

namespace {

using namespace Dyninst::DwarfDyninst;

class SubrangeBoundsFixture : public ::testing::Test {
protected:
  void SetUp() override {
    fd = open(FIXTURE_OBJECT_PATH, O_RDONLY);
    ASSERT_GE(fd, 0) << "failed to open " << FIXTURE_OBJECT_PATH;

    dbg = dwarf_begin(fd, DWARF_C_READ);
    ASSERT_NE(dbg, nullptr) << "dwarf_begin failed: " << dwarf_errmsg(-1);

    Dwarf_Off next_off;
    size_t header_size;
    ASSERT_EQ(dwarf_nextcu(dbg, 0, &next_off, &header_size, nullptr, nullptr,
                            nullptr),
              0)
        << "no compile unit found in fixture";

    Dwarf_Die *result = dwarf_offdie(dbg, header_size, &cu_die);
    ASSERT_NE(result, nullptr) << "dwarf_offdie failed for the CU DIE";
  }

  void TearDown() override {
    if (dbg)
      dwarf_end(dbg);
    if (fd >= 0)
      close(fd);
  }

  // Populates *out with the CU's n-th child DIE (1-based; see the fixture
  // layout in fixtures/subrange_bounds.s). Returns false on libdw traversal
  // failure so callers can ASSERT_TRUE() instead of using a partial Dwarf_Die.
  bool nthSubrangeChild(int n, Dwarf_Die *out) {
    if (dwarf_child(&cu_die, out) != 0)
      return false;
    for (int i = 1; i < n; ++i) {
      if (dwarf_siblingof(out, out) != 0)
        return false;
    }
    return true;
  }

  int fd = -1;
  Dwarf *dbg = nullptr;
  Dwarf_Die cu_die{};
};

TEST_F(SubrangeBoundsFixture, ExprlocUpperBoundIsUnknownNotError) {
  Dwarf_Die subrange;
  ASSERT_TRUE(nthSubrangeChild(1, &subrange));

  dwarf_bounds bounds = dwarf_subrange_bounds(&subrange);

  // A block/exprloc-form bound is a dynamic, runtime-only value: it must be
  // reported as "no value found", not as an error. Before the fix, this
  // returned dwarf_error{}, which propagated to parseSubrange() returning
  // nullptr and eventually a null-pointer dereference in dwarfWalker.C.
  EXPECT_TRUE(static_cast<bool>(bounds.upper))
      << "exprloc-form upper bound must not be treated as a decode error";
  EXPECT_FALSE(bounds.upper.value.has_value())
      << "exprloc-form upper bound has no statically-known value";
}

TEST_F(SubrangeBoundsFixture, UdataUpperBoundStillDecodesNormally) {
  Dwarf_Die subrange;
  ASSERT_TRUE(nthSubrangeChild(2, &subrange));

  dwarf_bounds bounds = dwarf_subrange_bounds(&subrange);

  // Regression check: the dwarf_formblock() probe added ahead of the
  // integer-decode path must not disturb ordinary constant-form bounds.
  ASSERT_TRUE(static_cast<bool>(bounds.upper));
  ASSERT_TRUE(bounds.upper.value.has_value());
  EXPECT_EQ(bounds.upper.value.get(), 9u);
}

TEST_F(SubrangeBoundsFixture, CountAttributeComputesUpperBound) {
  Dwarf_Die subrange;
  ASSERT_TRUE(nthSubrangeChild(3, &subrange));

  dwarf_bounds bounds = dwarf_subrange_bounds(&subrange);

  // No DW_AT_upper_bound/DW_AT_lower_bound is present, only DW_AT_count == 5.
  // With the implicit lower bound of 0, upper == lower + count - 1 == 4.
  ASSERT_TRUE(static_cast<bool>(bounds.upper));
  ASSERT_TRUE(bounds.upper.value.has_value());
  EXPECT_EQ(bounds.upper.value.get(), 4u);
}

TEST_F(SubrangeBoundsFixture, SignedUpperBoundDecodesNegativeValue) {
  Dwarf_Die subrange;
  ASSERT_TRUE(nthSubrangeChild(5, &subrange));

  dwarf_bounds bounds = dwarf_subrange_bounds(&subrange);

  // DW_AT_type refers to a DW_TAG_base_type with DW_AT_encoding ==
  // DW_ATE_signed, and DW_AT_upper_bound is DW_FORM_sdata == -1. This
  // exercises is_signed() and the dwarf_formsdata() decode branch instead of
  // the unsigned dwarf_formudata() path used by the other tests.
  ASSERT_TRUE(static_cast<bool>(bounds.upper));
  ASSERT_TRUE(bounds.upper.value.has_value());
  EXPECT_EQ(static_cast<Dwarf_Sword>(bounds.upper.value.get()), -1);
}

} // namespace
