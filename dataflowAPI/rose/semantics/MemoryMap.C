//#include "sage3basic.h"
#include "../util/StringUtility.h"

//#include "Diagnostics.h"
#include "../util/FileSystem.h"
#include "MemoryMap.h"
#include "../util/rose_getline.h"
#include "../util/rose_strtoull.h"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <boost/config.hpp>
#ifndef BOOST_WINDOWS
# include <fcntl.h>                                     // for open()
# include <sys/ptrace.h>                                // for ptrace()
# include <sys/wait.h>                                  // for waitpid()
# include <unistd.h>                                    // for access()
#endif

using namespace rose;
//using namespace rose::Diagnostics;


std::ostream& operator<<(std::ostream &o, const MemoryMap &x) { x.print(o); return o; }

/******************************************************************************************************************************
 *                                      Exceptions
 ******************************************************************************************************************************/

std::string
MemoryMap::Exception::leader(std::string dflt) const
{
    const char *s = what();
    return s && *s ? dflt : std::string(s);
}

std::string
MemoryMap::Exception::details(bool verbose) const
{
    std::ostringstream ss;
    if (verbose) {
        ss <<"\n";
        if (map)
            map->dump(ss, "  ");
    }
    return ss.str();
}

void
MemoryMap::Exception::print(std::ostream &o, bool verbose) const
{
    o <<leader("problem") <<details(verbose);
}

void
MemoryMap::Inconsistent::print(std::ostream &o, bool verbose) const
{
    o <<leader("inconsistent mapping") <<" for " <<new_range <<" vs. " <<old_range <<details(verbose);
        
}

void
MemoryMap::NotMapped::print(std::ostream &o, bool verbose) const
{
    o <<leader("no mapping") <<" at va " <<StringUtility::addrToString(va) <<details(verbose);
}

void
MemoryMap::NoFreeSpace::print(std::ostream &o, bool verbose) const
{
    o <<leader("no free space") <<" (nbytes=" <<size <<")" <<details(verbose);
}

void
MemoryMap::SyntaxError::print(std::ostream &o, bool verbose) const
{
    o <<leader("syntax error");
    if (!filename.empty()) {
        o <<" at " <<filename <<":" <<linenum;
        if (colnum>0)
            o <<"." <<colnum;
    }
    o <<details(verbose);
}

/******************************************************************************************************************************
 *                                      Buffer methods
 ******************************************************************************************************************************/

std::string
MemoryMap::segmentTitle(const Segment &segment) {
    std::string s;
    
    s += (segment.accessibility() & READABLE)  !=0 ? "r" : "-";
    s += (segment.accessibility() & WRITABLE)  !=0 ? "w" : "-";
    s += (segment.accessibility() & EXECUTABLE)!=0 ? "x" : "-";
    s += (segment.accessibility() & PRIVATE)   !=0 ? "p" : "-";
    unsigned otherAccess = segment.accessibility() & ~(READABLE|WRITABLE|EXECUTABLE|PRIVATE);

    std::string bufname = segment.buffer()->name();
    if (bufname.find_first_of(" \t\n()")==std::string::npos)
        bufname = "buffer " + bufname;
    s += " at " + (bufname+std::string(12, ' ')).substr(0, 12);

    s += " + " + StringUtility::addrToString(segment.offset());

    if (otherAccess != 0)
        s += " access=" + StringUtility::addrToString(otherAccess, 8*sizeof otherAccess);
    
    if (!segment.name().empty()) {
        static const size_t limit = 100;
        std::string name = escapeString(segment.name());
        if (name.size()>limit)
            name = name.substr(0, limit-3) + "...";
        s += " " + name;
    }
    return s;
}

/******************************************************************************************************************************
 *                                      MemoryMap methods
 ******************************************************************************************************************************/

size_t
MemoryMap::insertFile(const std::string &fileName, rose_addr_t startVa, bool writable, std::string segmentName) {
    if (segmentName.empty())
        segmentName = FileSystem::toString(boost::filesystem::path(fileName).filename());
    Segment segment = Segment::fileInstance(fileName, READABLE | (writable?WRITABLE:0), segmentName);
    AddressInterval fileInterval = AddressInterval::baseSize(startVa, segment.buffer()->size());
    insert(fileInterval, segment);
    return fileInterval.size();
}

static std::runtime_error
insertFileError(const std::string &locatorString, const std::string &mesg) {
    throw std::runtime_error("MemoryMap::insertFile: " + mesg + " in \"" + StringUtility::cEscape(locatorString) + "\"");
}

static rose_addr_t
parseInteger(const std::string &locatorString, const char *&s, const std::string &mesg) {
    char *rest = const_cast<char*>(s);
    errno = 0;
    rose_addr_t n = rose_strtoull(s, &rest, 0);
    if (errno!=0 || rest==s)
        throw insertFileError(locatorString, mesg);
    s = rest;
    return n;
}

std::string
MemoryMap::insertFileDocumentation() {
    return ("Beginning with the first colon, a memory map resource string has the form "
            "\":@v{memory_properties}:@v{file_properties}:@v{file_name}\" where @v{memory_properties} and "
            "@v{file_properties} are optional but the three colons are always required.  The @v{memory_properties} "
            "have the form \"[@v{address}][+@v{vsize}][=@v{access}]\" where each of the items is optional (indicated by "
            "the square brackets which should not be present in the actual resource string). The @v{address} is the "
            "starting address where the file will be mapped and defaults to the address of the lowest unmapped interval "
            "that is large enough to hold the new map segment; @v{vsize} is the size in bytes of the interval to be "
            "mapped, defaulting to the size of the file data; and @v{access} is the accessibility represented by "
            "zero or more of the characters \"r\" (readable), \"w\" (writable), and \"x\" (executable) in that order and "
            "defaulting to the accessibility of the file.  The @v{file_properties} have the form "
            "\"[@v{offset}][+@v{fsize}]\" where @v{offset} is an offset from the beginning of the file defaulting to zero; "
            "@v{size} is the number of bytes to read from the file, defaulting to the amount of data that is available. "
            "If @v{vsize} is specified then exactly that many bytes are mapped by zero-padding the file data if necessary; "
            "otherwise, when @v{fsize} is specified then exactly @v{fsize} bytes are mapped by zero padding the file data "
            "that could be read; otherwise the file size (adjusted by @v{offset}) determines the mapped size. The numeric "
            "properties can be specified in decimal, octal, or hexadecimal using the usual C syntax (leading \"0x\" for "
            "hexadecimal, leading \"0\" for octal, otherwise decimal).");
}

// Insert file from a locator string of the form:
//   :[VA][+VSIZE][=PERMS]:[OFFSET][+FSIZE]:FILENAME
AddressInterval
MemoryMap::insertFile(const std::string &locatorString) {

    //--------------------------------------
    // Parse the parts of the locator string
    //--------------------------------------

    // Leading colon
    const char *s = locatorString.c_str();
    if (':'!=*s++)
        throw insertFileError(locatorString, "not a locator string");

    // Virtual address
    Sawyer::Optional<rose_addr_t> optionalVa;
    if (isdigit(*s))
        optionalVa = parseInteger(locatorString, s /*in,out*/, "virtual address expected");

    // Virtual size
    Sawyer::Optional<size_t> optionalVSize;
    if ('+'==*s) {
        ++s;
        optionalVSize = parseInteger(locatorString, s /*in,out*/, "virtual size expected");
    }

    // Virtual accessibility
    Sawyer::Optional<unsigned> optionalAccess;
    if ('='==*s) {
        ++s;
        unsigned a = 0;
        if ('r'==*s) {
            ++s;
            a |= READABLE;
        }
        if ('w'==*s) {
            ++s;
            a |= WRITABLE;
        }
        if ('x'==*s) {
            ++s;
            a |= EXECUTABLE;
        }
        optionalAccess = a;
    }

    // Second colon
    if (':'!=*s) {
        if (*s && optionalAccess)
            throw insertFileError(locatorString, "invalid access spec");
        throw insertFileError(locatorString, "syntax error before second colon");
    }
    ++s;

    // File offset
    Sawyer::Optional<size_t> optionalOffset;
    if (isdigit(*s))
        optionalOffset = parseInteger(locatorString, s /*in,out*/, "file offset expected");
    
    // File size
    Sawyer::Optional<size_t> optionalFSize;
    if ('+'==*s) {
        ++s;
        optionalFSize = parseInteger(locatorString, s /*in,out*/, "file size expected");
    }

    // Third colon
    if (':'!=*s)
        throw insertFileError(locatorString, "syntax error before third colon");
    ++s;

    // File name
    if (!*s)
        throw insertFileError(locatorString, "file name expected after third colon");
    std::string fileName = s;
    if (fileName.size()!=strlen(fileName.c_str()))
        throw insertFileError(locatorString, "invalid file name");
    std::string segmentName = FileSystem::toString(boost::filesystem::path(fileName).filename());

    //-------------------------------- 
    // Open the file and read the data
    //-------------------------------- 

    // Open the file and seek to the start of data
    std::ifstream file(fileName.c_str());
    if (!file.good())
        throw std::runtime_error("MemoryMap::insertFile: cannot open file \""+StringUtility::cEscape(fileName)+"\"");
    if (optionalOffset)
        file.seekg(*optionalOffset);
    if (!file.good())
        throw std::runtime_error("MemoryMap::insertFile: cannot seek in file \""+StringUtility::cEscape(fileName)+"\"");

    // If no file size was specified then try to get one, or delay getting one until later.  On POSIX systems we can use stat
    // to get the file size, which is useful because infinite devices (like /dev/zero) will return zero.  Otherwise we'll get
    // the file size by trying to read from the file.
#if !defined(BOOST_WINDOWS)                             // not targeting Windows; i.e., not Microsoft C++ and not MinGW
    if (!optionalFSize) {
        struct stat sb;
        if (0==stat(fileName.c_str(), &sb))
            optionalFSize = sb.st_size;
    }
#endif

    // Limit the file size according to the virtual size.  We never need to read more than what would be mapped.
    if (optionalVSize) {
        if (optionalFSize) {
            optionalFSize = std::min(*optionalFSize, *optionalVSize);
        } else {
            optionalFSize = optionalVSize;
        }
    }

    // Read the file data.  If we know the file size then we can allocate a buffer and read it all in one shot, otherwise we'll
    // have to read a little at a time (only happens on Windows due to stat call above).
    uint8_t *data = NULL;                               // data read from the file
    size_t nRead = 0;                                   // bytes of data actually allocated, read, and initialized in "data"
    if (optionalFSize) {
        // This is reasonably fast and not too bad on memory
        if (0 != *optionalFSize) {
            data = new uint8_t[*optionalFSize];
            file.read((char*)data, *optionalFSize);
            nRead = file.gcount();
            if (nRead != *optionalFSize)
                throw std::runtime_error("MemoryMap::insertFile: short read from \""+StringUtility::cEscape(fileName)+"\"");
        }
    } else {
        while (file.good()) {
            uint8_t page[4096];
            file.read((char*)page, sizeof page);
            size_t n = file.gcount();
            uint8_t *tmp = new uint8_t[nRead + n];
            memcpy(tmp, data, nRead);
            memcpy(tmp+nRead, page, n);
            delete[] data;
            data = tmp;
            nRead += n;
        }
        optionalFSize = nRead;
    }

    // Choose virtual size
    if (!optionalVSize) {
        ASSERT_require(optionalFSize);
        optionalVSize = optionalFSize;
    }

    // Choose accessibility
    if (!optionalAccess) {
#ifdef BOOST_WINDOWS
        optionalAccess = READABLE | WRITABLE;
#else
        unsigned a = 0;
        if (0==::access(fileName.c_str(), R_OK))
            a |= READABLE;
        if (0==::access(fileName.c_str(), W_OK))
            a |= WRITABLE;
        if (0==::access(fileName.c_str(), X_OK))
            a |= EXECUTABLE;
        optionalAccess = a;
#endif
    }

    // Find a place to map the file.
    if (!optionalVa) {
        ASSERT_require(optionalVSize);
        optionalVa = findFreeSpace(*optionalVSize);
    }

    // Adjust the memory map
    ASSERT_require(optionalVa);
    ASSERT_require(optionalVSize);
    ASSERT_require(optionalAccess);
    ASSERT_require(nRead <= *optionalVSize);
    if (0 == *optionalVSize)
        return AddressInterval();                       // empty
    AddressInterval interval = AddressInterval::baseSize(*optionalVa, *optionalVSize);
    insert(interval, Segment::anonymousInstance(interval.size(), *optionalAccess, segmentName));
    size_t nCopied = at(interval.least()).limit(nRead).write(data).size();
    ASSERT_always_require(nRead==nCopied);              // better work since we just created the segment!
    return interval;
}

std::string
MemoryMap::insertProcessDocumentation() {
    return ("Beginning with the first colon, a process resource string has the form "
            "\":@v{options}:@v{pid}\" where @v{options} controls how the process memory is read and @v{pid} is the process ID. "
            "The @v{options} are a comma-separated list of words where the following are recognized:"

            "@bullet{\"noattach\" means do not attempt to attach or detach from the process. This is useful when the process "
            "is already running under some debugger (it has the \"T\" state in @man(ps)(1) output).}"

            "The process will be momentarily stopped (unless the \"noattach\" option was specified, in which case it is assumed "
            "to already be stopped) while its readable memory is copied into ROSE and mapped at the same addresses and with "
            "the same permissions as in the process. Then the process is resumed (unless \"noattach\").  If a read fails when "
            "copying a memory segment from the process into ROSE then"
            "the memory map will contain only that data which was successfully read and all subsequent addresses for that "
            "segment are not mapped in ROSE.  The segments will have names like \"proc:@v{pid}@v{error}(@v{name})\" where "
            "@v{error} is an optional error message in square brackets and @v{name} is the name of the memory segment "
            "according to the kernel (not all segments have names in the kernel).  For example, a segment named "
            "\"proc:24112[input/output error](/lib/ld-2.11.3.so)\" means it came from the \"/lib/ld-2.11.3.so\" library "
            "that was loaded for process 24112 but ROSE was unable to read the entire segment due to an error.  If an error "
            "occurs when reading the very first byte of a segment then no entry will appear in the final memory map since "
            "maps never have zero-length segments.");
}

static std::runtime_error
insertProcessError(const std::string &locatorString, const std::string &mesg) {
    throw std::runtime_error("MemoryMap::insertProcess: " + mesg + " in resource string \"" +
                             StringUtility::cEscape(locatorString) + "\"");
}

// FIXME[Robb P. Matzke 2014-10-09]: No idea how to do this in Microsoft Windows!
void
MemoryMap::insertProcess(const std::string &locatorString) {
#ifdef BOOST_WINDOWS                                    // FIXME[Robb P. Matzke 2014-10-10]
    throw std::runtime_error("MemoryMap::insertProcess is not available on Microsoft Windows");
#else

    // Resources that need to be cleaned up on return or exception
    struct T {
        FILE *mapsFile;                                 // file for /proc/xxx/maps
        char *buf;                                      // line read from /proc/xxx/maps
        size_t bufsz;                                   // bytes allocated for "buf"
        int memFile;                                    // file for /proc/xxx/mem
        pid_t resumeProcess;                            // subordinate process to resume
        T(): mapsFile(NULL), buf(NULL), bufsz(0), memFile(-1), resumeProcess(-1) {}
        ~T() {
            if (mapsFile)
                fclose(mapsFile);
            if (buf)
                free(buf);
            if (memFile>=0)
                close(memFile);
            if (resumeProcess != -1)
                ptrace(PTRACE_DETACH, resumeProcess, 0, 0);
        }
    } local;

    // Parse the locator string.
    bool doAttach = true;
    const char *s = locatorString.c_str();
    if (':'!=*s++)
        throw insertProcessError(locatorString, "initial colon expected");
    while (':'!=*s) {
        if (boost::starts_with(s, "noattach")) {
            doAttach = false;
            s += strlen("noattach");
        } else {
            throw insertProcessError(locatorString, "unknown option beginning at ...\"" + std::string(s) + "\"");
        }
        if (','==*s)
            ++s;
    }
    if (':'!=*s++)
        throw insertProcessError(locatorString, "second colon expected");
    
    int pid = parseInteger(locatorString, s /*in,out*/, "process ID expected");

    // We need to attach to the process with ptrace before we can read from its /proc/xxx/mem file.  We'll have
    // to detach if anything goes wrong or when we finish.
    if (doAttach) {
        if (-1 == ptrace(PTRACE_ATTACH, pid, 0, 0))
            throw insertProcessError(locatorString, "cannot attach: " + std::string(strerror(errno)));
        int wstat = 0;
        if (-1 == waitpid(pid, &wstat, 0))
            throw insertProcessError(locatorString, "cannot wait: " + std::string(strerror(errno)));
        if (WIFEXITED(wstat))
            throw insertProcessError(locatorString, "process exited before it could be read");
        if (WIFSIGNALED(wstat))
            throw insertProcessError(locatorString, "process died with " +
                                     boost::to_lower_copy(std::string(strsignal(WTERMSIG(wstat)))) +
                                     " before it could be read");
        local.resumeProcess = pid;
        ASSERT_require2(WIFSTOPPED(wstat) && WSTOPSIG(wstat)==SIGSTOP, "subordinate process did not stop");
    }

    // Prepare to read subordinate's memory
    std::string mapsName = "/proc/" + StringUtility::numberToString(pid) + "/maps";
    if (NULL==(local.mapsFile = fopen(mapsName.c_str(), "r")))
        throw insertProcessError(locatorString, "cannot open " + mapsName + ": " + strerror(errno));
    std::string memName = "/proc/" + StringUtility::numberToString(pid) + "/mem";
    if (-1 == (local.memFile = open(memName.c_str(), O_RDONLY)))
        throw insertProcessError(locatorString, "cannot open " + memName + ": " + strerror(errno));

    // Read each line from the /proc/xxx/maps to figure out what memory is mapped in the subordinate process. The format for
    // the part we're interested in is /^([0-9a-f]+)-([0-9a-f]+) ([-r][-w][-x])/ where $2 is the inclusive starting address, $2
    // is the exclusive ending address, and $3 are the permissions.
    int mapsFileLineNumber = 0;
    while (rose_getline(&local.buf, &local.bufsz, local.mapsFile)>0) {
        ++mapsFileLineNumber;

        // Begin address
        char *s=local.buf, *rest=s;
        errno = 0;
        rose_addr_t begin = rose_strtoull(s, &rest, 16);
        if (errno!=0 || rest==s || '-'!=*rest) {
            throw insertProcessError(locatorString, mapsName + " syntax error for beginning address at line " +
                                     StringUtility::numberToString(mapsFileLineNumber) + ": " + local.buf);
        }

        // End address
        s = rest+1;
        rose_addr_t end = rose_strtoull(s, &rest, 16);
        if (errno!=0 || rest==s || ' '!=*rest) {
            throw insertProcessError(locatorString, mapsName + " syntax error for ending address at line " +
                                     StringUtility::numberToString(mapsFileLineNumber) + ": " + local.buf);
        }
        if (begin >= end) {
            throw insertProcessError(locatorString, mapsName + " invalid address range at line " +
                                     StringUtility::numberToString(mapsFileLineNumber) + ": " + local.buf);
        }

        // Access permissions
        s = ++rest;
        if ((s[0]!='r' && s[0]!='-') || (s[1]!='w' && s[1]!='-') || (s[2]!='x' && s[2]!='-')) {
            throw insertProcessError(locatorString, mapsName + " invalid access permissions at line " +
                                     StringUtility::numberToString(mapsFileLineNumber) + ": " + local.buf);
        }
        unsigned accessibility = ('r'==s[0] ? READABLE : 0) | ('w'==s[1] ? WRITABLE : 0) | ('x'==s[2] ? EXECUTABLE : 0);

        // Skip over unused fields
        for (size_t nSpaces=0; nSpaces<4 && *s; ++s) {
            if (isspace(*s))
                ++nSpaces;
        }
        while (isspace(*s)) ++s;

        // Segment name according to the kernel
        std::string kernelSegmentName;
        while (*s && !isspace(*s))
            kernelSegmentName += *s++;

        // Create memory segment, but don't insert it until after we read all the data
        std::string segmentName = "proc:" + StringUtility::numberToString(pid);
        AddressInterval segmentInterval = AddressInterval::baseSize(begin, end-begin);
        Segment segment = Segment::anonymousInstance(segmentInterval.size(), accessibility,
                                                     segmentName + "(" + kernelSegmentName + ")");

        // Copy data from the subordinate process into our memory segment
        if (-1 == lseek(local.memFile, begin, SEEK_SET))
            throw insertProcessError(locatorString, memName + " seek failed: " + strerror(errno));
        size_t nRemain = segmentInterval.size();
        rose_addr_t segmentBufferOffset = 0;
        while (nRemain > 0) {
            uint8_t chunkBuf[8192];
            size_t chunkSize = std::min(nRemain, sizeof chunkBuf);
            ssize_t nRead = ::read(local.memFile, chunkBuf, chunkSize);
            if (-1==nRead) {
                if (EINTR==errno)
                    continue;
                //mlog[WARN] <<strerror(errno) <<" during read from " <<memName <<" for segment " <<kernelSegmentName
                //           <<" at " <<segmentInterval <<"\n";
                segmentName += "[" + boost::to_lower_copy(std::string(strerror(errno))) + "]";
                break;
            } else if (0==nRead) {
                //mlog[WARN] <<"short read from " <<memName <<" for segment " <<kernelSegmentName <<" at " <<segmentInterval <<"\n";
                segmentName += "[short read]";
                break;
            }
            rose_addr_t nWrite = segment.buffer()->write(chunkBuf, segmentBufferOffset, nRead);
            ASSERT_always_require(nWrite == (rose_addr_t)nRead);
            nRemain -= chunkSize;
            segmentBufferOffset += chunkSize;
        }
        if (nRemain > 0) {
            // If a read failed, map only what we could read
            segmentInterval = AddressInterval::baseSize(segmentInterval.least(), segmentInterval.size()-nRemain);
        }

        // Insert segment into memory map
        if (!segmentInterval.isEmpty())
            insert(segmentInterval, segment);
    }
#endif
}

SgUnsignedCharList
MemoryMap::readVector(rose_addr_t va, size_t desired, unsigned requiredPerms) const
{
    size_t canRead = at(va).limit(desired).require(requiredPerms).read(NULL).size();
    SgUnsignedCharList retval(canRead);
    size_t nRead = at(va).require(requiredPerms).read(retval).size();
    ASSERT_always_require(canRead == nRead);
    return retval;
}

std::string
MemoryMap::readString(rose_addr_t va, size_t desired, int(*validChar)(int), int(*invalidChar)(int),
                      unsigned requiredPerms, unsigned prohibitedPerms, char terminator) const
{
    std::vector<uint8_t> buf(desired, 0);
    size_t nread = at(va).require(requiredPerms).prohibit(prohibitedPerms).read(buf).size();
    for (size_t i=0; i<nread; ++i) {
        if (buf[i] == terminator)
            return std::string(buf.begin(), buf.begin()+i);

        if ((invalidChar && invalidChar(buf[i])) || (validChar && !validChar(buf[i])))
            return "";
    }
    return "";
}

void
MemoryMap::eraseZeros(size_t minsize)
{
    if (isEmpty())
        return;
    unsigned permissions = READABLE | EXECUTABLE;       // access permissions that must be present
    AddressIntervalSet toRemove;                        // to save up intervals until we're done iterating
    AddressInterval zeroInterval;
    uint8_t buf[8192];
    rose_addr_t va = hull().least();
    while (AddressInterval accessed = atOrAfter(va).require(permissions).limit(sizeof buf).read(buf)) {
        for (size_t offset=0; offset<accessed.size(); ++offset) {
            if (0 == buf[offset]) {
                if (zeroInterval.isEmpty()) {
                    zeroInterval = AddressInterval(accessed.least()+offset);
                } else if (zeroInterval.greatest()+1 < offset) {
                    if (zeroInterval.size() >= minsize)
                        toRemove.insert(zeroInterval);
                    zeroInterval = AddressInterval(accessed.least()+offset);
                } else {
                    zeroInterval = AddressInterval::hull(zeroInterval.least(), zeroInterval.greatest()+1);
                }
            } else if (!zeroInterval.isEmpty()) {
                if (zeroInterval.size() >= minsize)
                    toRemove.insert(zeroInterval);
                zeroInterval = AddressInterval();
            }
        }
        if (accessed.greatest() == hull().greatest())
            break;                                      // prevent overflow in next statement
        va += accessed.size();
    }
    if (zeroInterval.size() >= minsize)
        toRemove.insert(zeroInterval);
    BOOST_FOREACH (const AddressInterval &interval, toRemove.intervals())
        erase(interval);
}

Sawyer::Optional<rose_addr_t>
MemoryMap::findAny(const Extent &limits, const std::vector<uint8_t> &bytesToFind,
                   unsigned requiredPerms, unsigned prohibitedPerms) const {
    if (limits.empty() || bytesToFind.empty())
        return Sawyer::Nothing();
    AddressInterval interval = AddressInterval::hull(limits.first(), limits.last());
    return findAny(interval, bytesToFind, requiredPerms, prohibitedPerms);
}

Sawyer::Optional<rose_addr_t>
MemoryMap::findAny(const AddressInterval &limits, const std::vector<uint8_t> &bytesToFind,
                   unsigned requiredPerms, unsigned prohibitedPerms) const
{
    if (!limits || bytesToFind.empty())
        return Sawyer::Nothing();

    // Read a bunch of bytes at a time.  If the buffer size is large then we'll have fewer read calls before finding a match,
    // which is good if a match is unlikely.  But if a match is likely, then it's better to use a smaller buffer so we don't
    // ready more than necessary to find a match.  We'll compromise by starting with a small buffer that grows up to some
    // limit.
    size_t nremaining = limits.size();                  // bytes remaining to search (could be zero if limits is universe)
    size_t bufsize = 8;                                 // initial buffer size
    uint8_t buffer[4096];                               // full buffer

    Sawyer::Optional<rose_addr_t> atVa = this->at(limits.least()).require(requiredPerms).prohibit(prohibitedPerms).next();
    while (atVa && *atVa <= limits.greatest()) {
        if (nremaining > 0)                             // zero implies entire address space
            bufsize = std::min(bufsize, nremaining);
        size_t nread = at(*atVa).limit(bufsize).require(requiredPerms).prohibit(prohibitedPerms).read(buffer).size();
        assert(nread > 0);                              // because of the next() calls
        for (size_t offset=0; offset<nread; ++offset) {
            if (std::find(bytesToFind.begin(), bytesToFind.end(), buffer[offset]) != bytesToFind.end())
                return *atVa + offset;                  // found
        }
        atVa = at(*atVa+nread).require(requiredPerms).prohibit(prohibitedPerms).next();
        bufsize = std::min(2*bufsize, sizeof buffer);   // use a larger buffer next time if possible
        nremaining -= nread;                            // ok if nremaining is already zero
    }

    return Sawyer::Nothing();
}

Sawyer::Optional<rose_addr_t>
MemoryMap::findSequence(const AddressInterval &interval, const std::vector<uint8_t> &sequence) const {
    if (interval.isEmpty())
        return Sawyer::Nothing();
    if (sequence.empty())
        return interval.least();
    std::vector<uint8_t> buffer(4096);                  // size is arbitrary
    ASSERT_require2(sequence.size() <= buffer.size(), "long sequences not implemented yet");
    rose_addr_t searchVa = interval.least();
    while (AddressInterval window = atOrAfter(searchVa).read(buffer)) {
        for (size_t offset=0; offset+sequence.size()<=window.size(); ++offset) {
            if (std::equal(sequence.begin(), sequence.end(), &buffer[offset]))
                return window.least() + offset;
        }
        if (window.size()==buffer.size()) {
            searchVa = window.greatest() - buffer.size() + 2; // search for sequence that overlaps window boundary
        } else if (window.greatest() == hull().greatest()) {
            break;                                      // avoid possible overflow
        } else {
            searchVa = window.greatest() + 1;
        }
    }
    return Sawyer::Nothing();
}

void
MemoryMap::dump(FILE *f, const char *prefix) const
{
    std::ostringstream ss;
    dump(ss, prefix);
    fputs(ss.str().c_str(), f);
}

void
MemoryMap::dump(std::ostream &out, std::string prefix) const
{
    if (isEmpty()) {
        out <<prefix <<"empty\n";
        return;
    }

    BOOST_FOREACH (const Node &node, nodes()) {
        const AddressInterval &range = node.key();
        const Segment &segment = node.value();
        out <<prefix
            <<"va " <<StringUtility::addrToString(range.least())
            <<" + " <<StringUtility::addrToString(range.size())
            <<" = " <<StringUtility::addrToString(range.greatest()+1) <<" "
            <<segmentTitle(segment)
            <<"\n";
    }
}
