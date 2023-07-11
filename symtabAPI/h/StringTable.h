//
// Created by bill on 8/29/16.
//

#ifndef DYNINST_STRINGTABLE_H
#define DYNINST_STRINGTABLE_H

#include <ostream>
#include <stddef.h>
#include <string>
#include "concurrent.h"
#include <boost/shared_ptr.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/synchronized_value.hpp>

namespace Dyninst {
    namespace SymtabAPI {
        // This is being used for storing filenames.
        // str:         usually the full filename, it depends on what was the filename stored
        // filename:    to be only the filename and extension without path. Ex: "foo.txt"
        struct StringTableEntry {
            std::string str;
            std::string filename;
            StringTableEntry(std::string s, std::string f) : str(s), filename(f){}
            bool operator==(std::string s) const {
                return s == str ||
                    s == str.substr(str.rfind("/")+1);
            }
        };

        namespace bmi = boost::multi_index;
        typedef boost::multi_index_container
        <
            StringTableEntry,
            bmi::indexed_by
            <
                bmi::random_access<>,
                bmi::ordered_non_unique
                <
                    bmi::member<StringTableEntry, const std::string, &StringTableEntry::str>
                >,
                bmi::ordered_non_unique
                <
                    bmi::member<StringTableEntry, const std::string, &StringTableEntry::filename>
                >
            >
        >
        StringTableBase;

        struct StringTable : public StringTableBase {
            StringTable() : StringTableBase() {}
            ~StringTable() {}
            dyn_mutex lock;
        };

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

