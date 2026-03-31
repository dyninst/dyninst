//
// Created by bill on 8/29/16.
//

#ifndef DYNINST_STRINGTABLE_H
#define DYNINST_STRINGTABLE_H

#include <filesystem>
#include <ostream>
#include <stddef.h>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "concurrent.h"
#include <dyncompat/shared_ptr.hpp>

namespace Dyninst {
    namespace SymtabAPI {
        // This is being used for storing filenames.
        // str:         usually the full filename, it depends on what was the filename stored
        // filename:    to be only the filename and extension without path. Ex: "foo.txt"
        struct StringTableEntry {
            std::string str;
            std::string filename;
            StringTableEntry(std::string s, std::string f)
                : str(std::move(s)),
                  filename(f.empty() ? std::filesystem::path(str).filename().string() : std::move(f)) {}
            bool operator==(std::string s) const {
                return s == str ||
                    s == str.substr(str.rfind("/")+1);
            }
        };

        struct StringTable {
            using container_type = std::vector<StringTableEntry>;
            using value_type = container_type::value_type;
            using iterator = container_type::iterator;
            using const_iterator = container_type::const_iterator;

            dyn_mutex lock;

            size_t size() const { return entries_.size(); }
            bool empty() const { return entries_.empty(); }

            value_type &operator[](size_t idx) { return entries_[idx]; }
            const value_type &operator[](size_t idx) const { return entries_[idx]; }

            iterator begin() { return entries_.begin(); }
            iterator end() { return entries_.end(); }
            const_iterator begin() const { return entries_.begin(); }
            const_iterator end() const { return entries_.end(); }

            template <typename S1, typename S2>
            void emplace_back(S1 &&str, S2 &&filename) {
                entries_.emplace_back(std::forward<S1>(str), std::forward<S2>(filename));
            }

            size_t ensure(std::string str, std::string filename = "") {
                if (auto idx = find(str)) {
                    return *idx;
                }
                entries_.emplace_back(std::move(str), std::move(filename));
                return entries_.size() - 1;
            }

            bool contains(const std::string &str) const { return static_cast<bool>(find(str)); }

            std::vector<size_t> find_by_filename(const std::string &filename) const {
                std::vector<size_t> matches;
                for (size_t i = 0; i < entries_.size(); ++i) {
                    if (entries_[i].filename == filename) {
                        matches.push_back(i);
                    }
                }
                return matches;
            }

            std::optional<size_t> find(const std::string &str) const {
                for (size_t i = 0; i < entries_.size(); ++i) {
                    if (entries_[i].str == str) {
                        return i;
                    }
                }
                return std::nullopt;
            }

        private:
            container_type entries_;
        };

        typedef dyncompat::shared_ptr<StringTable> StringTablePtr;

        inline std::ostream& operator<<(std::ostream& s, StringTableEntry e)
        {
            s << e.str;
            return s;
        }

        inline std::ostream& operator<<(std::ostream& stream, const StringTable& tbl)
        {
            for(size_t i = 0; i < tbl.size(); ++i)
            {
                stream << tbl[i] << " @ " << i << std::endl;
            }
            return stream;
        }



    }
}


#endif //DYNINST_STRINGTABLE_H
