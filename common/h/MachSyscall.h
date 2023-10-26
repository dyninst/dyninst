#if !defined(MACH_SYSCALL_H_)
#define MACH_SYSCALL_H_

#include <string>
#include "boost/shared_ptr.hpp"

#include "Architecture.h"
#include "registers/MachRegister.h"
#include "dyntypes.h"

namespace Dyninst {
class Platform
{
    public: 
        Platform(Architecture a, OSType o): _arch(a), _os(o) {}

        Architecture arch() const { return _arch; }
        OSType os() const { return _os; }
        
        bool operator==(const Platform &) const;

    private:
        Architecture _arch; 
        OSType _os;
        //std::string _version;
};
}

namespace Dyninst  {

class MachSyscall;
namespace ProcControlAPI
{
    class EventSyscall;
    class Process;

    MachSyscall makeFromEvent(const EventSyscall * ev);
    MachSyscall makeFromID(boost::shared_ptr<Process> proc, unsigned long id);
}

class COMMON_EXPORT MachSyscall 
{
    public: 
        typedef unsigned long SyscallIDPlatform;
        typedef unsigned long SyscallIDIndependent; // e.g., Dyninst::Syscall::dyn_getpid
        typedef const char * SyscallName;

        // Factory methods
        // Allows ProcControlAPI to construct a MachSyscall for an event
        friend MachSyscall ProcControlAPI::makeFromEvent(const ProcControlAPI::EventSyscall *);

        // Allows users to construct a MachSyscall
        friend MachSyscall ProcControlAPI::makeFromID(boost::shared_ptr<ProcControlAPI::Process>, SyscallIDIndependent);
        
        static MachSyscall makeFromPlatform(Platform, SyscallIDIndependent);     

        // Accessors
        // Returns the platform-specific number for this system call
        SyscallIDPlatform num() const;

        // Returns the name for this system call (e.g., "getpid")
        SyscallName name() const;

        // Strict equality-based matching on Platforms and IDs
        bool operator==(const MachSyscall &) const;

    private:
        MachSyscall(Platform p, SyscallIDPlatform i, SyscallName n) : _plat(p), _id(i), _name(n) {}
        static SyscallName nameLookup(Platform plat, SyscallIDPlatform id);

        Platform _plat;
        SyscallIDPlatform _id;
        SyscallName _name;
};

}

#endif
