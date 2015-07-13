#include "MachSyscall.h"
#include "dyntypes.h"

using namespace std;
using namespace Dyninst;

bool Platform::operator==(const Platform & p) const
{
    return ((_arch == p._arch) && (_os == p._os));
}


class SyscallInformation
{
    public:
        static SyscallInformation * getInstance();

        MachSyscall::SyscallName findName(Platform, MachSyscall::SyscallIDPlatform);
        MachSyscall::SyscallIDPlatform findIDPlatform(Platform, MachSyscall::SyscallIDIndependent);

    private:
        SyscallInformation();
        static SyscallInformation * theInstance;

        typedef dyn_hash_map<MachSyscall::SyscallIDIndependent, MachSyscall::SyscallIDPlatform> syscallNumbersMap;
        typedef dyn_hash_map<MachSyscall::SyscallIDPlatform, MachSyscall::SyscallName> syscallNamesMap;

        syscallNumbersMap Linux_Arch_x86_syscallNumbers;
        syscallNamesMap Linux_Arch_x86_syscallNames;

        syscallNumbersMap Linux_Arch_x86_64_syscallNumbers;
        syscallNamesMap Linux_Arch_x86_64_syscallNames;

        syscallNumbersMap Linux_Arch_ppc32_syscallNumbers;
        syscallNamesMap Linux_Arch_ppc32_syscallNames;

        syscallNumbersMap Linux_Arch_ppc64_syscallNumbers;
        syscallNamesMap Linux_Arch_ppc64_syscallNames;

        syscallNumbersMap Linux_Arch_aarch64_syscallNumbers;
        syscallNamesMap Linux_Arch_aarch64_syscallNames;
};

SyscallInformation * SyscallInformation::theInstance = NULL;

SyscallInformation * SyscallInformation::getInstance()
{
    if(SyscallInformation::theInstance == NULL) {
        SyscallInformation::theInstance = new SyscallInformation;
    }
    return SyscallInformation::theInstance;
}

MachSyscall::SyscallName SyscallInformation::findName(Platform plat, MachSyscall::SyscallIDPlatform id)
{
    syscallNamesMap curMap;
    if (plat == Platform(Arch_x86, Linux)) {
        curMap = Linux_Arch_x86_syscallNames;
    } else if (plat == Platform(Arch_x86_64, Linux)) {
        curMap = Linux_Arch_x86_64_syscallNames;
    } else if (plat == Platform(Arch_ppc32, Linux)) {
        curMap = Linux_Arch_ppc32_syscallNames;
    } else if (plat == Platform(Arch_ppc64, Linux)) {
        curMap = Linux_Arch_ppc64_syscallNames;
    } else if (plat == Platform(Arch_aarch64, Linux)) {
        curMap = Linux_Arch_aarch64_syscallNames;
    } else {
        // We don't know anything about this platform
        assert(0);
        return "unknownSyscall";
    }

    auto found = curMap.find(id);
    if (found == curMap.end()) {
        // Well, crap
        assert(0);
        return "unknownSyscall";
    }
    return found->second;
}

MachSyscall::SyscallIDPlatform SyscallInformation::findIDPlatform(Platform plat, MachSyscall::SyscallIDIndependent id)
{
    syscallNumbersMap curMap;
    if (plat == Platform(Arch_x86, Linux)) {
        curMap = Linux_Arch_x86_syscallNumbers;
    } else if (plat == Platform(Arch_x86_64, Linux)) {
        curMap = Linux_Arch_x86_64_syscallNumbers;
    } else if (plat == Platform(Arch_ppc32, Linux)) {
        curMap = Linux_Arch_ppc32_syscallNumbers;
    } else if (plat == Platform(Arch_ppc64, Linux)) {
        curMap = Linux_Arch_ppc64_syscallNumbers;
    } else if (plat == Platform(Arch_aarch64, Linux)) {
        curMap = Linux_Arch_aarch64_syscallNumbers;
    } else {
        // We don't know anything about this platform
        return -1;
    }

    auto found = curMap.find(id);
    if (found == curMap.end()) {
        // Well, crap
        return -1;
    }
    return found->second;
}

/* This file is auto-generated from syscalls/generateSyscallInformation.py */
#include "SyscallInformation.C"

/* Lookup the system call string name based on platform-specific ID */
MachSyscall::SyscallName MachSyscall::nameLookup(Platform plat, SyscallIDPlatform id)
{
    return SyscallInformation::getInstance()->findName(plat, id);
}

MachSyscall MachSyscall::makeFromPlatform(Platform plat, SyscallIDIndependent id)
{
    // 1. Convert SyscallIDIndependent to SyscallIDPlatform
    SyscallIDPlatform syscallNumber = SyscallInformation::getInstance()->findIDPlatform(plat, id);
    if (syscallNumber == (MachSyscall::SyscallIDPlatform)-1) {
        return MachSyscall(plat, -1, "unknownSyscall");
    }

    // 2. MachSyscall::nameLookup(SyscallIDPlatform)
    SyscallName syscallName= nameLookup(plat, syscallNumber);

    return MachSyscall(plat, syscallNumber, syscallName);
}

unsigned long MachSyscall::num() const
{
    return _id;
}

MachSyscall::SyscallName MachSyscall::name() const
{
    return _name;
}

bool MachSyscall::operator==(const MachSyscall & m) const
{
    return ((_plat == m._plat) && (_id == m._id));
}

