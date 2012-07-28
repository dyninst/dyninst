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
#include "TestOutputDriver.h"

#include <map>
#include <string>

#include "test_info_new.h"

static void parseLabel(std::map<std::string, std::string> *attrs,
		       char *label);
static void parseLabel2(std::map<std::string, std::string> *attrs,
			std::string label);
static void parseLabel3(std::map<std::string, std::string> *attrs,
			std::string label);

static void parseLabel(std::map<std::string, std::string> *attrs,
		       char *label) {
  parseLabel2(attrs, std::string(label));
}

static void parseLabel2(std::map<std::string, std::string> *attrs,
			std::string label) {
  // This method is supposed to strip off the enclosing { and } and any
  // whitespace at the beginning or end of the label string
  std::string::size_type start = label.find_first_not_of("{ \t\n");
  std::string::size_type end = label.find_last_not_of("} \t\n");
  std::string stripped_label = label.substr(start, end - start + 1);
  parseLabel3(attrs, stripped_label);
}

static void parseLabel3(std::map<std::string, std::string> *attrs,
			std::string label) {
  // Now parse the first element in the label and recursively call this guy
  // until we get to an empty string
  if (label.empty()) { // Base case for recursion
    // There's an error here in my recursion base case
    return;
  } else {
    // Find the first comma in label; that's where our first attribute
    std::string::size_type first_comma = label.find(',', 0);
    // first_comma points to the comma character (one past the last character
    // we want to look at)
    if (std::string::npos == first_comma) {
      // If there's no comma, we'll look at the whole string
      first_comma = label.length();
      // first_comma points to the "null" (one past the last character we want
      // to look at)
    }
    // Name of the attribute starts at index 0
    std::string::size_type key_start = 0;
    std::string::size_type key_end = label.find(": ", 0);
    std::string::size_type val_start = key_end + 2;
    std::string::size_type val_end = first_comma;
    attrs->insert(make_pair(label.substr(key_start, key_end - key_start),
			    label.substr(val_start, val_end - val_start)));
    // TODO Set label to the string starting with the next attribute and make
    // the recursive call
    std::string next_attr = label.substr(first_comma);
    // Strip off the comma and any whitespace after it
    std::string::size_type fix_index;
    if ((fix_index = next_attr.find_first_not_of(", \t\n")) != std::string::npos) {
      next_attr = next_attr.substr(fix_index);
    }
    parseLabel3(attrs, next_attr);
  }
}

TESTLIB_DLL_EXPORT bool TestOutputDriver::getAttributesMap(TestInfo *test, 
                RunGroup *group, std::map<std::string, std::string> &attrs) {
  if ((NULL == test) || (NULL == test->label)) {
    return false;
  }

  // Fill in attributes corresponding to the TestInfo object
  // Ugh.  To do this properly I need to either parse TestInfo's label field
  // or change how TestInfo is constructed so it takes all the attributes I'm
  // interested in.  I think I'd be better off parsing the label field, so we
  // don't need to change TestInfo whenever build parameters change.
  // What's a good way to parse a string using C++?
  parseLabel(&attrs, const_cast<char *>(test->label));
  return true;
}

TESTLIB_DLL_EXPORT void TestOutputDriver::getMutateeArgs(std::vector<std::string> &args) {
  args.clear();
}

void TestOutputDriver::setNeedsHeader(bool h)
{
   needs_header = h;
}

TESTLIB_DLL_EXPORT TestOutputDriver::~TestOutputDriver()
{
}
