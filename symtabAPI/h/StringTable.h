//
// Created by bill on 8/29/16.
//

#ifndef DYNINST_STRINGTABLE_H
#define DYNINST_STRINGTABLE_H

#include <boost/shared_ptr.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/ordered_index.hpp>

namespace Dyninst {
    namespace SymtabAPI {
        struct StringTableEntry {
            std::string str;
            StringTableEntry(const char* s) : str(s) {}
            StringTableEntry(std::string s) : str(s) {}
            bool operator==(std::string s) const {
                return s == str ||
                       s == str.substr(str.rfind("/"));
            }
        };
        typedef boost::multi_index_container<StringTableEntry,
                boost::multi_index::indexed_by<
                        boost::multi_index::random_access<>,
                        boost::multi_index::ordered_non_unique<
                                boost::multi_index::member<StringTableEntry, const std::string, &StringTableEntry::str>
                        >
                        >
                > StringTable;

        typedef boost::shared_ptr<StringTable> StringTablePtr;

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
