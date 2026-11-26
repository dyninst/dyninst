#ifndef DYNINST_TESTS_UNIT_PARSEAPI_MEMORY_CODE_SOURCE_H
#define DYNINST_TESTS_UNIT_PARSEAPI_MEMORY_CODE_SOURCE_H

#include "CodeSource.h"
#include "dyntypes.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

/*
 * A CodeSource over hand-encoded machine code, so a test can parse code for any
 * architecture without a binary. Each function is a hint; a call can be given a
 * fake try range so that it gets a CATCH edge.
 */
namespace memory_code_source {

  struct Insn {
    Dyninst::Address addr;
    std::vector<uint8_t> bytes;
  };

  struct Func {
    Dyninst::Address addr;
    std::string name;
  };

  class Region : public Dyninst::ParseAPI::CodeRegion {
    Dyninst::Address base_;
    std::vector<uint8_t> bytes_;
    Dyninst::Architecture arch_;

  public:
    Region(Dyninst::Address base, std::vector<uint8_t> bytes, Dyninst::Architecture arch)
        : base_(base), bytes_(std::move(bytes)), arch_(arch) {}

    bool isValidAddress(const Dyninst::Address a) const override {
      return a >= base_ && a < base_ + bytes_.size();
    }
    void *getPtrToInstruction(const Dyninst::Address a) const override {
      if(!isValidAddress(a)) {
        return nullptr;
      }
      return const_cast<uint8_t *>(bytes_.data() + (a - base_));
    }
    void *getPtrToData(const Dyninst::Address a) const override { return getPtrToInstruction(a); }
    unsigned int getAddressWidth() const override { return 8; }
    bool isCode(const Dyninst::Address a) const override { return isValidAddress(a); }
    bool isData(const Dyninst::Address) const override { return false; }
    bool isReadOnly(const Dyninst::Address) const override { return true; }
    Dyninst::Address offset() const override { return base_; }
    Dyninst::Address length() const override { return bytes_.size(); }
    Dyninst::Architecture getArch() const override { return arch_; }
    Dyninst::Address low() const override { return base_; }
    Dyninst::Address high() const override { return base_ + bytes_.size(); }
  };

  class Source : public Dyninst::ParseAPI::CodeSource {
    Region *region_;
    // call address -> catch block start
    std::map<Dyninst::Address, Dyninst::Address> catches_;

  public:
    // The code is laid out from 0; addresses between the instructions hold zeros.
    Source(Dyninst::Architecture arch, std::vector<Insn> const &insns, std::vector<Func> const &funcs,
           std::map<Dyninst::Address, Dyninst::Address> catches = {})
        : catches_(std::move(catches)) {
      std::vector<uint8_t> bytes;
      for(auto const &i : insns) {
        if(bytes.size() < i.addr + i.bytes.size()) {
          bytes.resize(i.addr + i.bytes.size(), 0);
        }
        std::copy(i.bytes.begin(), i.bytes.end(), bytes.begin() + static_cast<std::ptrdiff_t>(i.addr));
      }
      region_ = new Region(0, std::move(bytes), arch);
      addRegion(region_);
      for(auto const &f : funcs) {
        _hints.push_back(Dyninst::ParseAPI::Hint(f.addr, 0, region_, f.name));
      }
    }
    // ~CodeSource deletes the region.

    Region *region() const { return region_; }

    bool isValidAddress(const Dyninst::Address a) const override { return region_->isValidAddress(a); }
    void *getPtrToInstruction(const Dyninst::Address a) const override {
      return region_->getPtrToInstruction(a);
    }
    void *getPtrToData(const Dyninst::Address a) const override { return region_->getPtrToData(a); }
    unsigned int getAddressWidth() const override { return region_->getAddressWidth(); }
    bool isCode(const Dyninst::Address a) const override { return region_->isCode(a); }
    bool isData(const Dyninst::Address a) const override { return region_->isData(a); }
    bool isReadOnly(const Dyninst::Address a) const override { return region_->isReadOnly(a); }
    Dyninst::Address offset() const override { return region_->offset(); }
    Dyninst::Address length() const override { return region_->length(); }
    Dyninst::Architecture getArch() const override { return region_->getArch(); }

    bool findCatchBlockByTryRange(Dyninst::Address a, std::set<Dyninst::Address> &starts) const override {
      auto it = catches_.find(a);
      if(it == catches_.end()) {
        return false;
      }
      starts.insert(it->second);
      return true;
    }
  };

}

#endif
